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

# benchmark for balanced parentheses support libraries
set(SDSL_INCLUDE_DIR ~/local/include)
set(SDSL_LIBRARY ~/local/lib)

# needed to figure out which configuration to choose for trees
add_executable(bp_bench bp_bench_runner.cpp)
target_link_directories(bp_bench PUBLIC ${SDSL_LIBRARY})
target_link_libraries(
		bp_bench
        PUBLIC
        sdsl benchmark benchmark_main pthread nlohmann_json::nlohmann_json 
        )
target_link_directories(bp_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
		bp_bench
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/misc
)

add_executable(trees_bench bench_underlying_trees.cpp)
target_link_directories(trees_bench PUBLIC ${SDSL_LIBRARY})
target_link_libraries(
		trees_bench
        PUBLIC
        sdsl benchmark wt_hpd tree_ext_sct benchmark_main pthread nlohmann_json::nlohmann_json 
        )
target_link_directories(trees_bench PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
		trees_bench
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/requests
        ${PROJECT_SOURCE_DIR}/experiments/utils
)

add_executable(aggregate_bench_sct aggregate_bench_refactored_sct.cpp)
target_link_libraries(
        aggregate_bench_sct
        PUBLIC
        benchmark benchmark_main pthread nlohmann_json::nlohmann_json naive
        tree_ext_sct)
target_link_directories(aggregate_bench_sct PUBLIC ${BENCHMARK_ROOT})
target_include_directories(
        aggregate_bench_sct
        PUBLIC
        ${BENCHMARK_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/include/requests
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
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
