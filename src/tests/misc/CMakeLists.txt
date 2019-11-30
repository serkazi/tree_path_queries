cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(driver_query_stream_builder driver_query_stream_builder.cpp)
target_link_libraries(driver_query_stream_builder PUBLIC experiment_utils)
target_include_directories(
        driver_query_stream_builder
        PUBLIC
        ${PROJECT_SOURCE_DIR}/experiments/utils
        ${PROJECT_SOURCE_DIR}/src/misc)