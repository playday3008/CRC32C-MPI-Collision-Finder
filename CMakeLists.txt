# Works with 3.14 and tested through 3.26
cmake_minimum_required(VERSION 3.14...3.26)

# Project name and a few useful settings. Other commands can pick up the results
project(
  CRC32CCollisionFinder
  VERSION 0.1
  DESCRIPTION "A program to find CRC32C collisions"
  LANGUAGES ASM C CXX
)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Disable C and C++ extensions
  set(CMAKE_C_EXTENSIONS OFF)
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
# FetchContent_MakeAvailable was added in CMake 3.14; simpler usage
include(FetchContent)

# Message Passing Interface library
# This is header only, so could be replaced with git submodules or FetchContent
find_package(MPI REQUIRED)
# Adds MPI::MPI_C, MPI::MPI_CXX

# Formatting library
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0)
FetchContent_MakeAvailable(fmtlib)
# Adds fmt::fmt

# The compiled library code is here
add_subdirectory(src/crc32c)

# The executable code is here
add_subdirectory(apps)

# Testing only available if this is the main app
# Emergency override CRC32C_COLLISION_FINDER_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR CRC32C_COLLISION_FINDER_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Copy compile_commands.json to the root of the project
add_custom_target(
	copy-compile-commands ALL
	${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_LIST_DIR}
)
