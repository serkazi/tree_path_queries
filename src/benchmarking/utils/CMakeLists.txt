cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(hunch_bench sample_bench.cpp)
target_link_libraries(
        hunch_bench
        PUBLIC
        benchmark benchmark_main pthread nlohmann_json::nlohmann_json naive
        hybrid wt_hpd ext_ptr tree_ext_sct)
target_link_directories(hunch_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
        hunch_bench
        PUBLIC
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/requests
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid
)

add_executable(aggregate_bench aggregate_bench_refactored.cpp)
target_link_libraries(
        aggregate_bench
        PUBLIC
        benchmark benchmark_main pthread nlohmann_json::nlohmann_json naive
        hybrid wt_hpd ext_ptr tree_ext_sct)
target_link_directories(aggregate_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
        aggregate_bench
        PUBLIC
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/requests
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid
        ${PROJECT_SOURCE_DIR}/experiments/utils
)

add_executable(hpd_bench hpd_bench.cpp)
target_link_libraries(
        hpd_bench
        PUBLIC
        benchmark benchmark_main pthread nlohmann_json::nlohmann_json naive
        hybrid wt_hpd)
target_link_directories(hunch_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
        hpd_bench
        PUBLIC
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/requests
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid
        ${PROJECT_SOURCE_DIR}/experiments/utils
)
