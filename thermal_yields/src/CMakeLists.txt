cmake_minimum_required(VERSION 3.16)

# This directory may be added by a parent CMakeLists; don't reset project() here if it's already set.
# project(thermal_yields CXX)

# Global C++ standard for this subdir
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put executables in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# If ThermalFIST is already a target (e.g. from add_subdirectory in the root), use it.
# Otherwise try to find an installed package that exports the target.
if(NOT TARGET ThermalFIST)
  find_package(ThermalFIST CONFIG QUIET)
  if(NOT TARGET ThermalFIST)
    message(FATAL_ERROR "ThermalFIST target not found. Either add_subdirectory(ThermalFIST) in the parent CMakeLists.txt or install/export ThermalFIST so find_package can locate it.")
  endif()
endif()

# If you truly need extra includes for your own headers in this subdir, add them here.
# target_include_directories(<target> PRIVATE "${PROJECT_SOURCE_DIR}/include")

add_executable(export_dndy_json export_dndy_json.cpp)
target_link_libraries(export_dndy_json PRIVATE ThermalFIST)
if(TARGET tfhic_common)
  target_link_libraries(export_dndy_json PRIVATE tfhic_common)
endif()
target_compile_features(export_dndy_json PRIVATE cxx_std_17)

# Helpful RPATH so executables run from the build/install tree with shared ThermalFIST
if(UNIX AND NOT APPLE)
  set_target_properties(export_dndy_json PROPERTIES
    BUILD_RPATH "\$ORIGIN;${CMAKE_BINARY_DIR};${CMAKE_BINARY_DIR}/lib"
    INSTALL_RPATH "\$ORIGIN"
  )
endif()

# Visual Studio folder grouping (optional)
set_property(TARGET export_dndy_json   PROPERTY FOLDER "apps")

# Install to <prefix>/bin
install(TARGETS export_dndy_json
        RUNTIME DESTINATION bin)
