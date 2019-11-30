cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(SRC typed_interface_test.cpp)
add_executable(test_all_ds ${SRC})
target_link_libraries(test_all_ds PUBLIC hybrid tree_ext_sct gtest gtest_main)
target_include_directories(
        test_all_ds
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/interfaces
        )

add_executable(typed_succinct_tree_test EXCLUDE_FROM_ALL typed_succinct_tree_test.cpp)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
message(WARNING "Gtest include dir: " ${GTEST_INCLUDE_DIR})
message(WARNING "Gtest library dir: " ${GTEST_LIBRARIES})
message(WARNING "Gtest main library dir: " ${GTEST_MAIN_LIBRARIES})
target_compile_options(typed_succinct_tree_test BEFORE PUBLIC -mcmodel=large PRIVATE -pg -g -O2)
target_link_libraries(typed_succinct_tree_test PUBLIC st bp_tree_sada gtest Threads::Threads)
target_link_libraries(typed_succinct_tree_test
        PRIVATE Threads::Threads GTest::GTest GTest::Main sdsl pthread)
target_link_directories(
        typed_succinct_tree_test
        PRIVATE
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
)
target_include_directories(
        typed_succinct_tree_test
        PUBLIC
        ${GTEST_INCLUDE_DIR}/gtest
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree
)

add_executable(typed_test_example EXCLUDE_FROM_ALL typed_test_example.cpp)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
message(WARNING "Gtest include dir: " ${GTEST_INCLUDE_DIR})
message(WARNING "Gtest library dir: " ${GTEST_LIBRARIES})
message(WARNING "Gtest main library dir: " ${GTEST_MAIN_LIBRARIES})
target_link_libraries(typed_test_example
        PRIVATE Threads::Threads GTest::GTest GTest::Main pthread)
target_link_directories(
        typed_test_example
        PRIVATE
        /usr/local/lib
)
target_include_directories(
        typed_succinct_tree_test
        PUBLIC
        ${GTEST_INCLUDE_DIR}/gtest
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree
)
