#!/bin/bash

# Build script for Allegro Hand ZMQ tests
set -e

echo "Building Allegro Hand ZMQ Tests..."

# Create test build directory
TEST_BUILD_DIR="build_tests"
mkdir -p $TEST_BUILD_DIR
cd $TEST_BUILD_DIR

echo "Running cmake for tests..."
cmake ../cpp/tests \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=11

echo "Building the tests..."
make -j$(nproc)

echo "Running tests..."
echo "===================="
make run_tests

echo ""
echo "Test build completed!"
echo "Test executables are in $TEST_BUILD_DIR/"
echo ""
echo "To run individual tests:"
echo "  cd $TEST_BUILD_DIR && ./test_json_parser"
echo "  cd $TEST_BUILD_DIR && ./test_command_parser"
echo ""
echo "To run all tests:"
echo "  cd $TEST_BUILD_DIR && make run_tests"