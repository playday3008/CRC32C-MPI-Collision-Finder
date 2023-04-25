# Set the executable name
add_executable(mpi_crc32c_finder app.cpp)
# Set the C++ standard to C++11 for the executable
target_compile_features(mpi_crc32c_finder PRIVATE cxx_std_11)

# Link the library to the executable (first internal, then external)
target_link_libraries(mpi_crc32c_finder PRIVATE crc32c_library)
target_link_libraries(mpi_crc32c_finder PRIVATE fmt::fmt)
target_link_libraries(mpi_crc32c_finder PRIVATE MPI::MPI_CXX)

# Enable all warnings
target_compile_options(mpi_crc32c_finder PRIVATE -Wall -Wextra -pedantic)
