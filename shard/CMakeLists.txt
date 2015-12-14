cmake_minimum_required (VERSION 2.8)
project (Sharding)

set (VersionMajor 1)
set (VersionMinor 0)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)

configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
	)

include_directories("${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/src")
add_subdirectory(src)

enable_testing()
find_package(GTest REQUIRED)
include_directories("/usr/local/include")

add_executable(test test/gtest_shard.cpp)
target_link_libraries(test libshard)
target_link_libraries(test ${GTEST_LIBRARIES})
