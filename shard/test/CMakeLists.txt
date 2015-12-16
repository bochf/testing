enable_testing()
# add gtest as external project
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# source fils
file(GLOB SRCS *.cpp)

# target
add_executable(test_shard ${SRCS})
target_link_libraries(test_shard
	libshard
	${GTEST_LIBRARIES}
	pthread
	)

# compile flags
set(CMAKE_CXX_FLAGS "-std=c++11")
include_directories(".")
