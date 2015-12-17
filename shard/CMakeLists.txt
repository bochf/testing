cmake_minimum_required (VERSION 2.8)
project (Sharding)

message("build type: " ${CMAKE_BUILD_TYPE})

add_subdirectory(src)
add_subdirectory(test)
