cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(sample_bench sample_bench.cpp)
target_link_libraries(sample_bench PUBLIC benchmark benchmark_main pthread)
target_link_directories(sample_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
        sample_bench
        PUBLIC
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/requests
)
