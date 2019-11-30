cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# adding tests
set(TEST_TARGET test_hpd)
add_executable(${TEST_TARGET} test_hpd.cpp)
target_link_libraries(${TEST_TARGET} PUBLIC st hpd gtest gtest_main)
target_include_directories(${TEST_TARGET}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree
        ${PROJECT_SOURCE_DIR}/include/data_structures/hpd)
target_include_directories(${TEST_TARGET} PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc)
