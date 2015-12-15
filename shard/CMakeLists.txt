cmake_minimum_required (VERSION 2.8)
project (Sharding)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

# add gtest as external project
find_package(Threads REQUIRED)

include(ExternalProject)
ExternalProject_Add(gtest
GIT_REPOSITORY https://github.com/google/googletest.git
PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
# disable install step
INSTALL_COMMAND ""
)

# create a libgtest target to be used as a dependency by target programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
"IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
"IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
# "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/googletest/include"
)
# above setting should work if the ${source_dir}/googletest/include directory already exists when cmake is running
# because it does not exist, we need to set the include directory manually
include_directories("${source_dir}/googletest/include")
include_directories("${source_dir}/googlemock/include")
