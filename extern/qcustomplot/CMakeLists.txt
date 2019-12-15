add_library(qcustomplot qcustomplot.cpp)
set_target_properties(qcustomplot PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(qcustomplot PUBLIC Qt5::Core Qt5::Widgets Qt5::PrintSupport)
target_include_directories(qcustomplot PUBLIC ${PROJECT_SOURCE_DIR}/extern/qcustomplot)