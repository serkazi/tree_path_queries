cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
find_library(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(test_wt_hpd test_wt_hpd.cpp)
target_include_directories(test_wt_hpd
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/tests/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/wrappers
        )
target_link_libraries(test_wt_hpd PUBLIC wt_hpd raw_processor gtest pthread)

add_executable(test_tree_ext_sct test_tree_ext_sct.cpp)
target_include_directories(
        test_tree_ext_sct
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/tests/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/src/misc
        )
target_link_libraries(test_tree_ext_sct PUBLIC tree_ext_sct raw_processor gtest pthread)

add_executable(test_nsrs test_nsrs.cpp)
target_include_directories(
        test_nsrs
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/tests/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/src/misc
)
target_link_libraries(test_nsrs PUBLIC naive raw_processor pthread gtest)
