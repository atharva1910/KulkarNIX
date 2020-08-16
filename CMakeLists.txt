cmake_minimum_required(VERSION 3.8)
project(KulkarNIX)

# setup the correct compiler
set(CMAKE_CXX_COMPILER "i686-elf-g++")

# Setup output directories
set(KNIX_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY KNIX_OUTPUT_DIR) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY KNIX_OUTPUT_DIR) 
set(CMAKE_BINARY_OUTPUT_DIRECTORY  KNIX_OUTPUT_DIR) 

# Add projects
add_subdirectory(Bootloader)


