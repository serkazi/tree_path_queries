cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# adding tests
set(TEST_TARGET test_wt)
add_executable(${TEST_TARGET} test_wt.cpp)
target_link_libraries(${TEST_TARGET} PUBLIC wt gtest gtest_main)
target_include_directories(${TEST_TARGET}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/wavelet_tree)
target_include_directories(${TEST_TARGET} PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc)