cmake_minimum_required(VERSION 3.16)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tfhic_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/tfhic_config.h
  @ONLY
)

add_library(tfhic_common
  hadron_catalog.cpp
  runtime_paths.cpp
)

target_include_directories(tfhic_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(tfhic_common PUBLIC cxx_std_17)
set_target_properties(tfhic_common PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS tfhic_common
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
