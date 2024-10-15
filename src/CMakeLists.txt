set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

add_executable (TF_CSM-vs-Vc TF_CSM-vs-Vc.cpp)
add_executable (TF_VCSM-vs-dNpidy TF_VCSM-vs-dNpidy.cpp)

target_link_libraries (TF_CSM-vs-Vc ThermalFIST)
target_link_libraries (TF_VCSM-vs-dNpidy ThermalFIST)

set_property(TARGET TF_CSM-vs-Vc PROPERTY FOLDER "TF_CSM-vs-Vc")
set_property(TARGET TF_VCSM-vs-dNpidy PROPERTY FOLDER "TF_VCSM-vs-dNpidy")

install (TARGETS TF_CSM-vs-Vc TF_VCSM-vs-dNpidy
         RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)	
