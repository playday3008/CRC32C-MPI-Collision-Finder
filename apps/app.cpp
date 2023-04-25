#include <crc32c/crc32c.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <mpi.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <signal.h>

// All ASCII printable characters
// Sorted in index order
// Technically, can be any character, except for '\0'
const char default_alphabet[] = "!\"#$%&'()*+,-./"
                                "0123456789"
                                ":;<=>?@"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "[\\]^_`"
                                "abcdefghijklmnopqrstuvwxyz"
                                "{|}~"; // We can comment out any line to exclude characters from alphabet

std::vector<
    std::tuple<
        uint32_t, // Hash output
        std::vector<uint8_t>, // String that produced hash
        long double // Time (in nanoseconds)
        >>
    results;

// On Ctrl+C, save results to file (on master)
void master_signal_handler(int sig)
{
    if (sig == SIGINT) {
        fmt::print("Caught signal '{0}' ({1}), saving results to file...\n",
            strsignal(sig), sig);
        fmt::print("Saving results to file...\n");
        fmt::print("Total results: {0}\n", results.size());
        std::ofstream file("results.txt");
        file << fmt::format(
            "Hash\tString\tTime (ns)\n");
        for (auto& result : results) {
            auto [hash, str, time] = result;
            file << fmt::format(
                "{0:#08x}\t{1}\t{2}\n",
                hash, std::string(str.begin(), str.end()), time);
        }
        file.close();
        MPI::Finalize();
        fmt::print("Done, check 'results.txt!\n");
        exit(0);
    } else {
        fmt::print("Caught signal '{0}' ({1}), stopping...\n",
            strsignal(sig), sig);
        MPI::Finalize();
        exit(1);
    }
}

// On Ctrl+C, peacefully stop yourself
void slave_signal_handler(int sig)
{
    fmt::print("Caught signal '{0}' ({1}), stopping...\n",
        strsignal(sig), sig);
    MPI::Finalize();
    exit(0);
}

int main(int argc, char** argv)
{
    MPI::Init(argc, argv);

    int rank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();

    uint32_t target_hash = 0;
    uint32_t alphabet_len = 0;
    uint8_t* alphabet = nullptr;

    if (rank == 0) {
        if (argc > 1) {
            target_hash = std::stoul(argv[1], nullptr, 16);
            if (argc > 2) {
                // TODO: improve this
                alphabet = (uint8_t*)argv[2];
                if (argc > 3)
                    alphabet_len = std::stoul(argv[3], nullptr, 10);
            } else {
                alphabet = (uint8_t*)default_alphabet;
            }
            if (!alphabet_len)
                alphabet_len = (uint32_t)strlen((char*)alphabet);
        } else {
            std::cerr << "No hash provided, aborting..." << std::endl;
            std::cout << "Usage: " << argv[0] << " <hash> [alphabet] [alphabet length]" << std::endl;
            std::cout << "Example: " << argv[0] << " 0xdeadbeef" << std::endl;
            // MPI stop all processes
            MPI::COMM_WORLD.Abort(1);
            MPI::Finalize();
            return 1;
        }

        // Print sent data
        fmt::print("Hash to brute force: 0x{0:08x}\n"
                   "Alphabet: {1:.{2}}\n"
                   "Alphabet length: {2}\n",
            target_hash, (char*)alphabet, alphabet_len);

        MPI::COMM_WORLD.Bcast(&target_hash, 1, MPI::UNSIGNED, 0);
        MPI::COMM_WORLD.Bcast(&alphabet_len, 1, MPI::UNSIGNED, 0);
        MPI::COMM_WORLD.Bcast(alphabet, alphabet_len, MPI::UNSIGNED_CHAR, 0);
    } else {
        // Use Bcast instead of Probe/Recv to avoid loss of data after \0 character
        MPI::COMM_WORLD.Bcast(&target_hash, 1, MPI::UNSIGNED, 0);
        MPI::COMM_WORLD.Bcast(&alphabet_len, 1, MPI::UNSIGNED, 0);
        alphabet = new uint8_t[alphabet_len];
        MPI::COMM_WORLD.Bcast(alphabet, alphabet_len, MPI::UNSIGNED_CHAR, 0);
    }

    // Get number of threads using Reduce
    uint num_threads = std::thread::hardware_concurrency();
    uint g_num_threads;
    MPI::COMM_WORLD.Reduce(&num_threads, &g_num_threads, 1, MPI::UNSIGNED, MPI::SUM, 0);
    if (rank == 0) {
        fmt::print("Number of processes: {0}\n"
                   "Number of threads: {1}\n",
            size, g_num_threads);
    }

    uint32_t length = rank;
    auto start = std::chrono::high_resolution_clock::now();
    auto crc32c = CRC32C();
    std::function<void(const uint8_t*, size_t, unsigned, std::vector<uint8_t>&)>
        combo;

    combo = [&](const uint8_t* alphabet, size_t alphabet_len, unsigned size, std::vector<uint8_t>& line) {
        for (size_t i = 0; i < alphabet_len; i++) {
            line.push_back(alphabet[i]);
            if (size <= 1) { // Condition that prevents infinite loop in recursion
                uint32_t hash = crc32c.calc(line.data(), line.size(), 0);
                if (hash == target_hash) {
                    auto end = std::chrono::high_resolution_clock::now();
                    if (rank != 0) {
                        // Current state of sending
                        static int state = 0; // 0 - ready, 1 - pending, 2 - done
                        // Data to send
                        static uint8_t* raw_data;
                        static MPI::Request req;
                        static MPI::Status status;
                        // Meaning of states:
                        // Throw data to master process
                        // On next success iteration, wait for previous request to complete
                        // Until then data will be probably accepted by master process, so we will not wait so long
                        // Then delete data and reset state
                        // Repeat
                        if (state == 1) {
                            // Wait for previous request to complete
                            if (!req.Test(status))
                                req.Wait(status);
                            // Done
                            state = 2;
                        }
                        if (state == 2) {
                            // Reset state and free memory
                            req = MPI::Request();
                            status = MPI::Status();
                            delete[] raw_data;
                            // Reset state
                            state = 0;
                        }
                        if (state == 0) {
                            // Prepare raw data to send as one message
                            // Format: hash (4 bytes) + time (8/10 bytes) + text (line.size() bytes)
                            raw_data = new uint8_t[sizeof(uint32_t) + sizeof(long double) + line.size()];
                            // Time in nanoseconds
                            long double time = (end - start).count();

                            memcpy(raw_data, &hash, sizeof(uint32_t));
                            memcpy(raw_data + sizeof(uint32_t), &time, sizeof(long double));
                            memcpy(raw_data + sizeof(uint32_t) + sizeof(long double), line.data(), line.size());

                            // Send data
                            req = MPI::COMM_WORLD.Isend(raw_data,
                                sizeof(uint32_t) + sizeof(long double) + line.size(),
                                MPI::BYTE, 0, 1);

                            // Set state to pending
                            state = 1;
                        }
                    } else {
                        results.push_back(std::make_tuple(hash, line, (end - start).count()));
                    }
                }
                if (rank == 0) {
                    // Current state of sending
                    static int state = 0; // 0 - ready, 1 - pending, 2 - done
                    // Data to send
                    static uint8_t* raw_data;
                    static uint raw_data_size;
                    static MPI::Request resp;
                    static MPI::Status status;
                    // Meaning of states:
                    // Get data from other processes
                    // On next success iteration, wait for previous request to complete
                    // Until then data will be probably accepted by master process, so we will not wait so long
                    // Then delete data and reset state
                    // Repeat
                    if (state == 1) {
                        // Wait for previous request to complete
                        if (resp.Test(status))
                            state = 2;
                    }
                    if (state == 2) {
                        // Save data and then reset state and free memory
                        uint32_t hash;
                        long double time;
                        memcpy(&hash, raw_data, sizeof(uint32_t));
                        memcpy(&time, raw_data + sizeof(uint32_t), sizeof(long double));
                        std::vector<uint8_t> line(raw_data + sizeof(uint32_t) + sizeof(long double), raw_data + raw_data_size);
                        delete[] raw_data;

                        results.push_back(std::make_tuple(hash, line, time));

                        // Reset state and free memory
                        resp = MPI::Request();
                        status = MPI::Status();
                        // Reset state
                        state = 0;
                    }
                    if (state == 0) {
                        // Check if there is any data to receive
                        auto probe = MPI::COMM_WORLD.Iprobe(MPI::ANY_SOURCE, 1, status);
                        if (probe) {
                            auto count = status.Get_count(MPI::BYTE);
                            // If data is too small, ignore it
                            if (count > (int)(sizeof(uint32_t) + sizeof(long double))) {
                                raw_data = new uint8_t[count];
                                resp = MPI::COMM_WORLD.Irecv(raw_data, count, MPI::BYTE, status.Get_source(), 1);
                                raw_data_size = count;
                                // Set state to pending
                                state = 1;
                            }
                        }
                    }
                }
                line.erase(line.end() - 1);
            } else {
                // Recursion happens here
                combo(alphabet, alphabet_len, size - 1, line);
                line.erase(line.end() - 1);
            }
        }
    };

    // Register signal handler if we are master process
    if (rank == 0)
        signal(SIGINT, master_signal_handler);
    else
        signal(SIGINT, slave_signal_handler);

    fmt::print(
        "Starting...\n"
        "Press Ctrl+C to stop and write results to file.\n");

    bool flip = false;
    do {
        std::vector<uint8_t> line;
        combo(alphabet, alphabet_len, length, line);

        if (flip)
            length += size - (size - rank) + (size - (length % size));
        else
            length += size - rank + (size - (length % size)) - 1;
        flip = !flip;
    } while (true);
}
