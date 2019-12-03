cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

add_executable(test_experimental_run test_experimental_run.cpp test_experiment_utils.cpp)
target_link_libraries(test_experimental_run PUBLIC experiment_utils gtest gtest_main)
target_include_directories(test_experimental_run PUBLIC ${PROJECT_SOURCE_DIR}/experiments/utils)

add_executable(test_experiment_utils test_experiment_utils.cpp)
target_link_libraries(
        test_experiment_utils
        PUBLIC
        sdsl
        tree_ext_sct
        wt_hpd
        hybrid
        naive
        experiment_utils
)
target_include_directories(
        test_experiment_utils
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/tests/experiments
        ${PROJECT_SOURCE_DIR}/experiments/managers
)

add_executable(observe_memusage test_memusage.cpp)
target_link_libraries(
        observe_memusage
        PUBLIC
        sdsl
        tree_ext_sct
        wt_hpd
        hybrid
        naive
        experiment_utils
)
target_compile_options(observe_memusage PUBLIC -g -O2)
target_include_directories(
        observe_memusage
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/tests/experiments
        ${PROJECT_SOURCE_DIR}/experiments/managers
)
