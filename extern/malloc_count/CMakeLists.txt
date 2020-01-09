add_library(malloc_count malloc_count.c)
set_target_properties(malloc_count PROPERTIES LINKER_LANGUAGE CXX)
# @see https://github.com/bingmann/malloc_count -- we must add "-ldl"
target_link_libraries(malloc_count PUBLIC dl)
target_include_directories(malloc_count PUBLIC ${PROJECT_SOURCE_DIR}/extern/malloc_count/malloc_count.h)