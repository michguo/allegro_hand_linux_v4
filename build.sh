#!/bin/bash

# Build script for Allegro Hand ZMQ Interface
# Updated for new repository structure with C++ code in cpp/ folder

set -e  # Exit on any error

echo "Building Allegro Hand ZMQ Interface..."

# Check if we're in the correct directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "cpp" ]; then
    echo "Error: Please run this script from the repository root directory"
    echo "Current directory: $(pwd)"
    echo "Expected files: CMakeLists.txt and cpp/ directory"
    exit 1
fi

# Clean previous build if requested
if [ "$1" = "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf build
    echo "Clean completed successfully!"
    exit 0
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Set library paths for BHand and PCAN
export BHAND_DIR="/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64"
export PCAN_DIR="/usr/local"

# Run cmake from root directory with explicit paths
echo "Running cmake..."
echo "  BHand library path: $BHAND_DIR"
echo "  PCAN library path: $PCAN_DIR"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBHAND_DIR="$BHAND_DIR" \
    -DPCAN_DIR="$PCAN_DIR"

# Build the project
echo "Building the project..."
make -j$(nproc)

# No install step needed - CMAKE_RUNTIME_OUTPUT_DIRECTORY builds directly to bin/
echo "Build completed! Executable should be in bin/ directory..."

# Verify the executable exists and is in the right place
if [ ! -f "bin/grasp" ]; then
    echo "Warning: Executable not found in expected location bin/grasp"
    echo "Searching for the executable..."
    
    # Look for the executable in common locations
    if [ -f "grasp" ]; then
        echo "Found executable in build root, moving to bin/"
        mkdir -p bin
        mv grasp bin/grasp
    elif [ -f "cpp/grasp" ]; then
        echo "Found executable in cpp/, moving to bin/"
        mkdir -p bin
        mv cpp/grasp bin/grasp
    else
        echo "Error: Could not find built executable anywhere!"
        echo "Contents of build directory:"
        find . -name "grasp" -type f 2>/dev/null | head -10
        exit 1
    fi
fi

# Check if build was successful
if [ -f "bin/grasp" ]; then
    echo "Build completed successfully!"
    echo "Executable created at: $(pwd)/bin/grasp"
    
    # Make the executable... executable
    chmod +x bin/grasp
    
    echo ""
    echo "To run the Allegro Hand interface:"
    echo "  cd build && ./bin/grasp"
    echo ""
    echo "Or from repository root:"
    echo "  ./build/bin/grasp"
else
    echo "Build failed: executable not found at $(pwd)/bin/grasp"
    echo "Contents of build directory:"
    ls -la
    echo "Contents of bin directory (if it exists):"
    ls -la bin/ 2>/dev/null || echo "bin/ directory does not exist"
    exit 1
fi