cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_ff_bitvector test_ff_bitvector.cpp)
target_link_libraries(test_ff_bitvector PUBLIC ff_bitvector gtest gtest_main)
target_include_directories(test_ff_bitvector PUBLIC ${PROJECT_SOURCE_DIR}/include/wrappers)