cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# adding tests
set(TEST_TARGET test_simple_tree)
add_executable(${TEST_TARGET} test_simple_tree.cpp)
target_include_directories(${TEST_TARGET}
                        PUBLIC
                        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree)
# target_link_libraries(${TEST_TARGET} PUBLIC st GTest::GTest GTest::Main)
target_link_libraries(${TEST_TARGET} PUBLIC st gtest gtest_main)
