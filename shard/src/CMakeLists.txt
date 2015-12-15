# source fils
file(GLOB SRCS *.cpp)

# target
add_library(libshard shard_mod.cpp)

# compile flags
set(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(".")
