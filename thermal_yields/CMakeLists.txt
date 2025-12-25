cmake_minimum_required(VERSION 3.16)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(thermal_yields LANGUAGES CXX)
endif()

# Default: skip Qt GUI unless explicitly enabled.
option(TFHIC_WITH_QT "Enable QtThermalFIST GUI (not required)" OFF)
if(TFHIC_WITH_QT)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets OFF CACHE BOOL "Allow Qt5Widgets discovery" FORCE)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5PrintSupport OFF CACHE BOOL "Allow Qt5PrintSupport discovery" FORCE)
else()
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets ON CACHE BOOL "Disable Qt5Widgets discovery" FORCE)
  set(CMAKE_DISABLE_FIND_PACKAGE_Qt5PrintSupport ON CACHE BOOL "Disable Qt5PrintSupport discovery" FORCE)
endif()

# ThermalFIST
add_subdirectory(Thermal-FIST)

# Common helpers (paths, catalog) if not provided by parent
if(NOT TARGET tfhic_common)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/common)
endif()

# apps
add_subdirectory(src)
