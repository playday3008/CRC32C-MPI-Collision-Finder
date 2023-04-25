#pragma once

#include <cstdint>

/*
 * \brief CRC32C
 *
 * This class provides a CRC32C calculator.
 *
 * The CRC32C algorithm is a CRC algorithm that uses the CRC32C polynomial.
 * It is used in many places, including the Castagnoli polynomial in the
 * Ethernet standard.
 *
 * This class provides a CRC32C calculator. It will use the fastest
 * implementation available on the current CPU.
 *
 * \note This class is thread-safe.
 */
class CRC32C {
public:
  CRC32C();
  ~CRC32C();

  /*
   * \brief Universal function declaration for future mapping to real functions
   * 
   * \param data Pointer to the data to be processed
   * \param size Size of the data to be processed (in bytes)
   * \param crc Initial CRC value
   * 
   * \return uint32_t CRC32C value
   */
  uint32_t (*calc)(const void *data, uint32_t size, uint32_t crc) = nullptr;

private:
  
  /// \brief Implemented CPU hardware accelerators
  enum class CpuVendor {
    Unset,
    Unsupported,
    Intel,
    AMD,
  };
  
  /// \brief Implemented CRC32C algorithms
  enum class CRC32CAlgorithm {
    Unset,
    Software,
    Intel,
    AMD,
  };

  CpuVendor detect_cpu_vendor();
  uint64_t detect_cpu_features();
  CRC32CAlgorithm detect_crc32c_algorithm();

  CpuVendor cpu_vendor = CpuVendor::Unset;
  uint64_t cpu_features = 0;
  CRC32CAlgorithm algorithm = CRC32CAlgorithm::Unset;
};