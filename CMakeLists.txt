cmake_minimum_required(VERSION 3.16)

project(TFHIC LANGUAGES CXX)

option(TFHIC_WITH_ROOT "Build blastwave targets (requires ROOT)" ON)

set(TFHIC_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/tfhic/data"
    CACHE PATH "Default TFHIC data directory")
set(TFHIC_CONF_DIR "${CMAKE_INSTALL_PREFIX}/share/tfhic/conf"
    CACHE PATH "Default TFHIC config directory")
set(TFHIC_OUT_DIR ""
    CACHE PATH "Default TFHIC output directory (optional)")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(common)
add_subdirectory(thermal_yields)

if(TFHIC_WITH_ROOT)
  add_subdirectory(blastwave)
endif()

install(FILES common/data/hadrons.json DESTINATION share/tfhic/data)
install(DIRECTORY blastwave/data/ DESTINATION share/tfhic/data)
install(DIRECTORY thermal_yields/conf/ DESTINATION share/tfhic/conf)
