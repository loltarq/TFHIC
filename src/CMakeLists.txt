set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

add_executable (TF_CSM-vs-Vc TF_CSM-vs-Vc.cpp)
add_executable (TF_VCSM-vs-dNpidy TF_VCSM-vs-dNpidy.cpp)
add_executable (TF_CSM-vs-dNpidy TF_CSM-vs-dNpidy.cpp)

target_link_libraries (TF_CSM-vs-Vc ThermalFIST)
target_link_libraries (TF_VCSM-vs-dNpidy ThermalFIST)
target_link_libraries (TF_CSM-vs-dNpidy ThermalFIST)

set_property(TARGET TF_CSM-vs-Vc PROPERTY FOLDER "TF_CSM-vs-Vc")
set_property(TARGET TF_VCSM-vs-dNpidy PROPERTY FOLDER "TF_VCSM-vs-dNpidy")
set_property(TARGET TF_CSM-vs-dNpidy PROPERTY FOLDER "TF_CSM-vs-dNpidy")

install (TARGETS TF_CSM-vs-Vc TF_VCSM-vs-dNpidy TF_CSM-vs-dNpidy
         RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)	
