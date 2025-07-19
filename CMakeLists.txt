cmake_minimum_required(VERSION 3.10)
project(AllegroHand)

set(AllegroHand_VERSION_MAJOR 1)
set(AllegroHand_VERSION_MINOR 0)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O2 -Wall -std=c++11")

# Set output directories to avoid permission issues
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Set library paths (can be overridden by cmake arguments)
if(NOT DEFINED BHAND_DIR)
    set(BHAND_DIR "/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64")
endif()

if(NOT DEFINED PCAN_DIR)
    set(PCAN_DIR "/usr/local")
endif()

# Include directories for BHand library
include_directories(${BHAND_DIR}/include)
include_directories(cpp/include)

# Library directories
link_directories(${BHAND_DIR}/lib)
link_directories(${PCAN_DIR}/lib)

# Add the cpp subdirectory which contains the actual build configuration
add_subdirectory(cpp)
