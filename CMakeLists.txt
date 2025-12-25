cmake_minimum_required(VERSION 3.16)

project(TFHIC LANGUAGES CXX)

option(TFHIC_WITH_ROOT "Build blastwave targets (requires ROOT)" ON)
option(TFHIC_WITH_QT "Enable QtThermalFIST GUI (not required)" OFF)

if(TFHIC_WITH_QT)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets OFF CACHE BOOL "Allow Qt5Widgets discovery" FORCE)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5PrintSupport OFF CACHE BOOL "Allow Qt5PrintSupport discovery" FORCE)
else()
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets ON CACHE BOOL "Disable Qt5Widgets discovery" FORCE)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5PrintSupport ON CACHE BOOL "Disable Qt5PrintSupport discovery" FORCE)
endif()

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

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/tfhic-env.sh.in
  ${CMAKE_BINARY_DIR}/tfhic-env.sh
  @ONLY
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/tfhic-env.sh DESTINATION share/tfhic)

install(FILES common/data/hadrons.json DESTINATION share/tfhic/data)
install(DIRECTORY blastwave/data/ DESTINATION share/tfhic/data)
install(DIRECTORY thermal_yields/conf/ DESTINATION share/tfhic/conf)
