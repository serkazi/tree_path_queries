add_library(sha512 sha512.hh)
set_target_properties(sha512 PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(sha512 PUBLIC ${PROJECT_SOURCE_DIR}/extern/sha512)