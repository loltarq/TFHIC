cmake_minimum_required(VERSION 3.16)

if(NOT TARGET tfhic_common)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/common)
endif()

find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Graf Gpad Tree MathCore Physics)

add_library(tfhic_blastwave SHARED
  src/hadron_list.cpp
  src/io_utils.cpp
  src/blastwave_utils.cpp
  src/io_thermal.cpp
)

set_target_properties(tfhic_blastwave PROPERTIES OUTPUT_NAME TFHIC)

target_include_directories(tfhic_blastwave
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tfhic_blastwave PUBLIC cxx_std_17)

target_link_libraries(tfhic_blastwave
  PUBLIC
    tfhic_common
    ROOT::Core
    ROOT::RIO
    ROOT::Hist
    ROOT::Graf
    ROOT::Gpad
    ROOT::Tree
    ROOT::MathCore
    ROOT::Physics
)

add_executable(blastwave_thermal apps/blastwave_thermal.cpp)
target_link_libraries(blastwave_thermal PRIVATE tfhic_blastwave)
target_compile_features(blastwave_thermal PRIVATE cxx_std_17)

add_executable(predict_light_spectra apps/predict_light_spectra.cpp)
target_link_libraries(predict_light_spectra PRIVATE tfhic_blastwave)
target_compile_features(predict_light_spectra PRIVATE cxx_std_17)

install(TARGETS tfhic_blastwave blastwave_thermal predict_light_spectra
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
