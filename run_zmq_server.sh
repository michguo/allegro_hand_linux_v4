#!/bin/bash

# Script to run the Allegro Hand ZMQ server
# Executes the grasp binary built by build.sh

set -e  # Exit on any error

echo "Starting Allegro Hand ZMQ server..."

# Check if grasp binary exists
if [ ! -f "build/bin/grasp" ]; then
    echo "Error: grasp binary not found at build/bin/grasp"
    echo "Please run ./build.sh first to build the project"
    exit 1
fi

# Function to handle cleanup on exit
cleanup() {
    echo ""
    echo "Shutting down Allegro Hand ZMQ server..."
    exit 0
}

# Set up signal handlers for graceful shutdown
trap cleanup SIGINT SIGTERM

# Check for required libraries and try to resolve library path issues
echo "Checking system libraries..."

# Try to use system libraries instead of conda libraries
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/local/lib:${LD_LIBRARY_PATH}"

# If BHand library is installed in a custom location, add it
if [ -d "/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64/lib" ]; then
    export LD_LIBRARY_PATH="/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64/lib:${LD_LIBRARY_PATH}"
fi

# If PCAN library is installed in custom location, add it
if [ -d "/usr/local/lib" ]; then
    export LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH}"
fi

echo "Library path set to: $LD_LIBRARY_PATH"

# Check if required hardware interfaces are available (optional check)
echo "Checking for hardware interfaces..."
if lsusb | grep -q "Peak-System"; then
    echo "✓ PCAN-USB device detected"
else
    echo "⚠ Warning: PCAN-USB device not detected. Make sure it's connected."
fi

# Test if the binary can start (dependency check)
echo "Testing binary dependencies..."
if ! ldd build/bin/grasp >/dev/null 2>&1; then
    echo "Error: Binary has unresolved dependencies"
    echo "Please check that all required libraries are installed:"
    echo "  - BHand library"
    echo "  - PCAN basic library" 
    echo "  - ZMQ libraries"
    echo ""
    echo "Run 'ldd build/bin/grasp' to see missing dependencies"
    exit 1
fi

# Display important information before starting
echo ""
echo "================================================"
echo "Allegro Hand ZMQ Server"
echo "================================================"
echo "Server will listen on: tcp://*:5556"
echo "Ready to accept JSON commands from Python clients"
echo "Press Ctrl+C to stop the server"
echo ""
echo "Supported command format:"
echo '{"motion_type": 11, "desired_positions": [0.0, 0.0, ...], "time_interval": 0.003}'
echo ""
echo "Make sure:"
echo "1. Allegro Hand is powered OFF before starting"
echo "2. PCAN-USB adapter is connected"
echo "3. After server shows 'Ready', power ON the Allegro Hand"
echo "================================================"
echo ""

# Execute the grasp binary with error handling
echo "Starting ZMQ server..."
if ! ./build/bin/grasp; then
    echo ""
    echo "Error: Server failed to start or crashed"
    echo ""
    echo "Common issues:"
    echo "1. Library compatibility: Try rebuilding with ./build.sh"
    echo "2. Hardware not connected: Check PCAN-USB connection"
    echo "3. Permissions: Make sure you have access to USB devices"
    echo "4. Port already in use: Check if another server is running"
    echo ""
    echo "For debugging, run: ldd build/bin/grasp"
    exit 1
fi