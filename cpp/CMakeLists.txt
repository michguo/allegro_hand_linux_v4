# Source files are in the src/ subdirectory
set(SOURCE_FILES
    src/main.cpp
    src/canAPI.cpp
    src/RockScissorsPaper.cpp
    src/allegroZmqJsonParser.cpp
)

# Create the executable
add_executable(grasp ${SOURCE_FILES})

# Include directories for this target
target_include_directories(grasp PRIVATE include)

# Add conda environment include path for nlohmann/json
if(DEFINED ENV{CONDA_PREFIX})
    target_include_directories(grasp PRIVATE $ENV{CONDA_PREFIX}/include)
endif()

# Find cppzmq wrapper
find_package(cppzmq QUIET)
if(cppzmq_FOUND)
    target_link_libraries(grasp cppzmq)
    message(STATUS "Found cppzmq")
else()
    message(WARNING "cppzmq not found, linking with zmq directly")
    find_library(ZMQ_LIB zmq)
    if(ZMQ_LIB)
        target_link_libraries(grasp ${ZMQ_LIB})
    else()
        message(FATAL_ERROR "ZMQ library not found")
    endif()
endif()

# Link required libraries
target_link_libraries(grasp 
    BHand 
    pcanbasic 
    Threads::Threads
)

# Note: No install step needed since CMAKE_RUNTIME_OUTPUT_DIRECTORY 
# is set in the root CMakeLists.txt to build directly to bin/

# Print build information
message(STATUS "Building grasp executable with sources:")
foreach(source ${SOURCE_FILES})
    message(STATUS "  ${source}")
endforeach()
