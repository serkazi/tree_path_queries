cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(gflags_DIR "~/local/lib/cmake/gflags")
find_package(gflags REQUIRED)

add_executable(gentree gentree_uar.cpp)
target_link_libraries(
	gentree
	PUBLIC
	gflags
)

add_executable(bin2ordinal bin2ordered.cpp)
target_link_libraries(
	bin2ordinal
	PUBLIC
	gflags
)
