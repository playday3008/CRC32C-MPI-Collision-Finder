# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CRC32CCollisionFinder_SOURCE_DIR}/include/crc32c/*.hpp")
#set(HEADER_LIST "${CRC32CCollisionFinder_SOURCE_DIR}/include/crc32c/crc32c.hpp")

file(GLOB SRC_LIST CONFIGURE_DEPENDS "${CRC32CCollisionFinder_SOURCE_DIR}/src/crc32c/*.cpp")
#set(SRC_LIST "${CRC32CCollisionFinder_SOURCE_DIR}/src/crc32c/crc32c.cpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(crc32c_library ${SRC_LIST} ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(crc32c_library PUBLIC ../../include)

# This depends on (header only) fmt
#target_link_libraries(crc32c_library PRIVATE fmt::fmt)

# All users of this library will need at least C++11
target_compile_features(crc32c_library PUBLIC cxx_std_11)

# Enable all warnings
target_compile_options(crc32c_library PRIVATE -Wall -Wextra -pedantic)

# Add PCLMUL, SSE2, SSE3, SSE4.1, SSE4.2
target_compile_options(crc32c_library PRIVATE -msse2 -msse3 -msse4.1 -msse4.2 -mpclmul)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)
