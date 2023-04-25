# Testing library
FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.3.2)
FetchContent_MakeAvailable(catch)
# Adds Catch2::Catch2

# Tests need to be added as executables first
add_executable(test_crc32c test_crc32c.cpp)

# I'm using C++14 in the test
target_compile_features(test_crc32c PRIVATE cxx_std_14)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(test_crc32c PRIVATE crc32c_library Catch2::Catch2WithMain)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME test_crc32c_test COMMAND test_crc32c) # Command can be a target
