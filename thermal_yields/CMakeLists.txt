cmake_minimum_required(VERSION 3.16)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(thermal_yields LANGUAGES CXX)
endif()

# ThermalFIST
add_subdirectory(Thermal-FIST)

# Common helpers (paths, catalog) if not provided by parent
if(NOT TARGET tfhic_common)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/common)
endif()

# apps
add_subdirectory(src)
