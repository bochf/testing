# source fils
file(GLOB SRCS *.cpp)

# target
add_library(libshard ${SRCS})

# compile flags
set(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(".")
