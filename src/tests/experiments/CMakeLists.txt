cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_experimental_run test_experimental_run.cpp)
target_link_libraries(test_experimental_run PUBLIC experiment_utils)
target_include_directories(test_experimental_run PUBLIC ${PROJECT_SOURCE_DIR}/experiments/utils)
