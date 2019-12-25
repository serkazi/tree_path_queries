cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
find_library(GTest REQUIRED)
find_package(Threads REQUIRED)

set(execs test_ext_ptr test_nv_lca test_nv test_wt_hpd_ptr)
set(srcs test_ext_ptr.cpp test_nv_lca.cpp test_nv.cpp test_wt_hpd_ptr.cpp)
list(LENGTH execs len1)
math(EXPR len2 "${len1}-1")
foreach(val RANGE ${len2})
        list(GET execs ${val} ex)
        list(GET srcs ${val} sr)
        add_executable(${ex} ${sr})
        target_include_directories(
                ${ex}
                PUBLIC
                ${PROJECT_SOURCE_DIR}/include/data_structures/plain
                ${PROJECT_SOURCE_DIR}/include/data_structures/hpd
                ${PROJECT_SOURCE_DIR}/include/data_structures/wavelet_tree
                ${PROJECT_SOURCE_DIR}/src/tests/data_structures/plain
                ${PROJECT_SOURCE_DIR}/include/interfaces
                ${PROJECT_SOURCE_DIR}/src/misc
        )
        target_link_libraries(${ex} PUBLIC raw_processor bender_farach gtest pthread)
endforeach(val)
