cmake_minimum_required(VERSION 3.15)
project(tree_path_queries)

# find_library(GTest REQUIRED)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

add_compile_options(-mcmodel=large -g -O2)

find_package(Threads REQUIRED)
# adding gtest as a subproject
# add_subdirectory(${PROJECT_SOURCE_DIR}/extern/gtest)
find_package(GTest REQUIRED)
message(WARNING "Gtest include dir: " ${GTEST_INCLUDE_DIR})
message(WARNING "Gtest library dir: " ${GTEST_LIBRARIES})
message(WARNING "Gtest main library dir: " ${GTEST_MAIN_LIBRARIES})

set(BENCHMARK_ROOT ~/local/benchmark/build/src)
set(BENCHMARK_INCLUDE_DIR ~/local/benchmark/include)
# set(BENCHMARK_ROOT /usr/local/lib/)
# set(BENCHMARK_INCLUDE_DIR /usr/local/local/include)


# set the variable point to the cmake-finder of nlohmann::json
set(nlohmann_json_DIR ~/local/lib/cmake/nlohmann_json)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Qt5 inclusion
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

# if(CMAKE_VERSION VERSION_LESS "3.7.0")
#     set(CMAKE_INCLUDE_CURRENT_DIR ON)
# endif()

# set(CMAKE_MODULE_PATH /users/grad/kazi/Qt)
# find_package(Qt5 COMPONENTS Widgets REQUIRED)
#end of Qt5 inclusion

# find_package(sdsl REQUIRED)

# set(GTEST_LIBRARY ~/local/lib/libgtest.a)
# set(GTEST_INCLUDE_DIR ~/local/include/gtest/)
# set(GTEST_MAIN_LIBRARY ~/local/lib/libgtest_main.a)
# set(GTEST_ROOT ~/local/include CACHE PATH "Path to googletest")
# find_package(GTest REQUIRED)

# set(BENCHMARK_LIBRARY ~/local/benchmark/build/src/libbenchmark.a)
# set(BENCHMARK_INCLUDE_DIR ~/local/benchmark/include/benchmark)
# set(BENCHMARK_MAIN_LIBRARY ~/local/benchmark/build/src/libbenchmark_main.a)
# find_package(benchmark REQUIRED)

# adding library for simple_bitset

add_subdirectory(${PROJECT_SOURCE_DIR}/extern/sha512)

set(SIMPLE_BITSET sb)
add_library(${SIMPLE_BITSET} ${PROJECT_SOURCE_DIR}/src/misc/simple_bitset.cpp)
target_include_directories(${SIMPLE_BITSET} PUBLIC ${PROJECT_SOURCE_DIR}/src/misc)

# adding library for simple_tree
set(SIMPLE_TREE st)
set(ST_SRC ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree/simple_tree.hpp)
add_library(${SIMPLE_TREE} ${ST_SRC})
set_target_properties(${SIMPLE_TREE} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${SIMPLE_TREE} PUBLIC ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree)

# raw_processor for testing purposes
add_library(raw_processor ${PROJECT_SOURCE_DIR}/src/misc/raw_processor.hpp)
set_target_properties(raw_processor PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(raw_processor PUBLIC st)
target_include_directories(raw_processor
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/src/misc
        )

#adding hpd_preprocessor
add_library(hpd_preprocessor ${PROJECT_SOURCE_DIR}/src/misc/hpd_preprocessor.hpp)
set_target_properties(hpd_preprocessor PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(hpd_preprocessor PUBLIC sdsl)
target_include_directories(
        hpd_preprocessor
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/src/misc
)

# bender_farach_colton lca
add_library(bender_farach ${PROJECT_SOURCE_DIR}/src/misc/bender_farach_colton.hpp)
set_target_properties(bender_farach PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(bender_farach PUBLIC st)
target_include_directories(
        bender_farach
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc
)

# adding library for HPD
find_library(${SIMPLE_TREE} REQUIRED)
set(LIB_SRC ${PROJECT_SOURCE_DIR}/include/data_structures/hpd/heavy_path_decomp.hpp)
set(HPD hpd)
add_library(${HPD} ${LIB_SRC})
set_target_properties(${HPD} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${HPD} PUBLIC st)
target_include_directories(${HPD} PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/simple_tree)
target_include_directories(${HPD} PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc)
target_include_directories(${HPD} PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/hpd)

# adding library for range_tree
set(RANGE_TREE rt)
set(LIB_SRC ${PROJECT_SOURCE_DIR}/include/data_structures/range_tree/range_tree.hpp include/data_structures/range_tree/range_tree_simple.hpp)
add_library(${RANGE_TREE} ${LIB_SRC} include/data_structures/range_tree/range_tree_simple.hpp)
set_target_properties(${RANGE_TREE} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${RANGE_TREE} PUBLIC sb)
target_include_directories(${RANGE_TREE} PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc)
target_include_directories(${RANGE_TREE} PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/range_tree)

# adding library for wavelet_tree
set(LIB_SRC ${PROJECT_SOURCE_DIR}/include/data_structures/wavelet_tree/wavelet_tree.hpp)
set(WAV_TREE wt)
add_library(${WAV_TREE} ${LIB_SRC})
set_target_properties(${WAV_TREE} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${WAV_TREE} PUBLIC sb)
target_include_directories(${WAV_TREE} PUBLIC
        ${PROJECT_SOURCE_DIR}/src/misc)
target_include_directories(${WAV_TREE} PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/wavelet_tree)

# adding library for the hybrid data structure
set(LIB_SRC ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid/hybrid_processor.hpp)
add_library(hybrid ${LIB_SRC})
set_target_properties(hybrid PROPERTIES LINKER_LANGUAGE CXX)
set(DEPS_LIBS "wt;rt;hpd")
foreach(lib ${DEPS_LIBS})
    target_link_libraries(hybrid PUBLIC ${lib})
endforeach(lib)
set(DEPS_DIRS "wavelet_tree;range_tree;hpd;misc")
foreach(dir in ${DEPS_DIRS})
    target_include_directories(hybrid PUBLIC ${PROJECT_SOURCE_DIR}/include/${dir})
endforeach(dir)
target_include_directories(hybrid PUBLIC ${PROJECT_SOURCE_DIR}/src/misc)
target_include_directories(hybrid PUBLIC ${PROJECT_SOURCE_DIR}/include/interfaces)
set(BENCHMARK_MAIN_LIBRARY ~/local/benchmark/build/src/libbenchmark_main.a)

# setting up SDSL shortcuts
set(SDSL_INCLUDE_DIR ~/local/include)
set(SDSL_LIBRARY ~/local/lib)

# adding the ff_bitvector, needed for e.g. succinct tree extraction
add_library(ff_bitvector ${PROJECT_SOURCE_DIR}/include/wrappers/ff_bitvector.hpp)
target_link_directories(ff_bitvector PUBLIC ${SDSL_LIBRARY})
target_link_libraries(ff_bitvector PUBLIC sdsl)
set_target_properties(ff_bitvector PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
        ff_bitvector
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/src/misc
)

# adding a wrapper over bp_sequence to represent a tree topology with navigation
add_library(bp_tree_sada ${PROJECT_SOURCE_DIR}/include/wrappers/bp_tree_sada.hpp)
target_link_directories(bp_tree_sada PUBLIC ${SDSL_LIBRARY})
target_link_libraries(bp_tree_sada PUBLIC sdsl)
set_target_properties(bp_tree_sada PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
        bp_tree_sada
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/src/misc
)

#adding the library for tree_extraction_succinct
add_library(tree_ext_sct ${PROJECT_SOURCE_DIR}/include/data_structures/succinct/tree_ext_sct.hpp)
set_target_properties(tree_ext_sct PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
        tree_ext_sct
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/wrappers
)
target_link_directories(tree_ext_sct PUBLIC ${SDSL_LIBRARY})
target_link_libraries(tree_ext_sct PUBLIC sdsl)

#adding the library for wt_hpd
add_library(
        wt_hpd
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct/wt_hpd.hpp
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct/wt_hpd_rrr.hpp
        )
set_target_properties(wt_hpd PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
        wt_hpd
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/interfaces
)
target_link_directories(wt_hpd PUBLIC ${SDSL_LIBRARY})
target_link_libraries(wt_hpd PUBLIC sdsl)

#adding succinct tree implementations based on balaced parens stuff
add_library(
        bp_trees
        ${PROJECT_SOURCE_DIR}/include/wrappers/bp_tree_sada.hpp
)
set_target_properties(bp_trees PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(bp_trees PUBLIC sdsl)
target_link_directories(bp_trees PUBLIC ${SDSL_LIBRARY})
target_include_directories(
        bp_trees
        PUBLIC
        ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/interfaces
)

# adding the library for naive
# although SDSL is needed for nsrs structure only, we just bundle all the
# three data structures together, for brevity
add_library(
        naive
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain/naive_processor.hpp
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain/naive_processor_lca.hpp
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct/nsrs.hpp
)
set_target_properties(naive PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(naive PUBLIC bp_tree_sada)
target_include_directories(
        naive
        PUBLIC
        # ${SDSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain/
)

# adding library for plain pointer-based tree extraction
add_library(
        ext_ptr
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain/ext_ptr.hpp
)
set_target_properties(ext_ptr PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
        ext_ptr
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/interfaces
)

# TODO: add library "succinct" as INTERFACE for wt_hpd and extraction

# building the requests library
add_library(
        pq_requests
        ${PROJECT_SOURCE_DIR}/include/requests/abstract_query.hpp
        ${PROJECT_SOURCE_DIR}/include/requests/counting_query.hpp
        ${PROJECT_SOURCE_DIR}/include/requests/median_query.hpp
        ${PROJECT_SOURCE_DIR}/include/requests/pq_request.hpp
        ${PROJECT_SOURCE_DIR}/include/requests/reporting_query.hpp
        ${PROJECT_SOURCE_DIR}/include/requests/selection_query.hpp
        )
set_target_properties(pq_requests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(pq_requests PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(pq_requests
        PUBLIC ${PROJECT_SOURCE_DIR}/include/requests
        PUBLIC ${PROJECT_SOURCE_DIR}/include/interfaces
        )

# adding experiment_utils
add_library(
        experiment_utils
        ${PROJECT_SOURCE_DIR}/src/experiments/experimental_run.cpp
        ${PROJECT_SOURCE_DIR}/experiments/utils/experiments_container.hpp
        ${PROJECT_SOURCE_DIR}/experiments/utils/query_stream_builder.hpp
        ${PROJECT_SOURCE_DIR}/src/misc/random1d_interval_generator.hpp
        ${PROJECT_SOURCE_DIR}/experiments/managers/fixed_processor_manager.hpp
        ${PROJECT_SOURCE_DIR}/experiments/managers/fixed_dataset_manager.hpp
        )
set_target_properties(experiment_utils PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(experiment_utils PUBLIC nlohmann_json::nlohmann_json pq_requests sha512)
target_include_directories(
        experiment_utils PUBLIC
        ${PROJECT_SOURCE_DIR}/experiments/utils
        ${PROJECT_SOURCE_DIR}/experiments/managers
        ${PROJECT_SOURCE_DIR}/include/interfaces
        ${PROJECT_SOURCE_DIR}/src/misc
        ${PROJECT_SOURCE_DIR}/include/wrappers
        ${PROJECT_SOURCE_DIR}/include/data_structures/plain
        ${PROJECT_SOURCE_DIR}/include/data_structures/hybrid
        ${PROJECT_SOURCE_DIR}/include/data_structures/succinct
        ${PROJECT_SOURCE_DIR}/extern/sha512
)

# adding the test
add_subdirectory(${PROJECT_SOURCE_DIR}/src/misc)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/simple_tree)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/range_tree)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/hpd)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/wavelet_tree)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/hybrid)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/experiments)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/requests)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/wrappers)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/succinct)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/data_structures/plain)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/misc)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/tests/integration)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/benchmarking/utils)

