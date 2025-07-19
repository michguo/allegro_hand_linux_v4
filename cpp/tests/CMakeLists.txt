# Unit tests for Allegro Hand ZMQ interface
cmake_minimum_required(VERSION 3.10)
project(AllegroTests)

# Create a minimal unit test for JSON functions
add_executable(test_simple_json test_simple_json_functions.cpp)

# Include directories
target_include_directories(test_simple_json PRIVATE 
    ../include
    .
)

# Add conda environment include path for nlohmann/json
if(DEFINED ENV{CONDA_PREFIX})
    target_include_directories(test_simple_json PRIVATE $ENV{CONDA_PREFIX}/include)
endif()

# Just need threading support for the test framework
find_package(Threads REQUIRED)
target_link_libraries(test_simple_json Threads::Threads)

# Set C++ standard
target_compile_features(test_simple_json PRIVATE cxx_std_11)

# Add test targets
enable_testing()
add_test(NAME SimpleJsonTest COMMAND test_simple_json)

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_simple_json
    COMMENT "Running JSON function tests"
)