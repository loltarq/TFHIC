cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project (TFHIC)

# ThermalFIST
add_subdirectory(Thermal-FIST)

# research
add_subdirectory(src)
