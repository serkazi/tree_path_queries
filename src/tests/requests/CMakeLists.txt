cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_requests test_pq_requests.cpp)
target_link_libraries(test_requests PUBLIC pq_requests)
target_include_directories(test_requests PUBLIC ${PROJECT_SOURCE_DIR}/include/requests)

