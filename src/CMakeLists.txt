set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

add_executable (TFsample TFsample.cpp)

target_link_libraries (TFsample ThermalFIST)

set_property(TARGET TFsample PROPERTY FOLDER "TFsample")

install (TARGETS TFsample
         RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)	