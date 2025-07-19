#!/bin/bash

# Debug version of the ZMQ server script
# Shows detailed library information for troubleshooting

set -e

echo "=== Allegro Hand ZMQ Server Debug Mode ==="
echo ""

# Check if grasp binary exists
if [ ! -f "build/bin/grasp" ]; then
    echo "Error: grasp binary not found at build/bin/grasp"
    echo "Please run ./build.sh first to build the project"
    exit 1
fi

echo "1. Binary Information:"
echo "   Path: $(pwd)/build/bin/grasp"
echo "   Size: $(ls -lh build/bin/grasp | awk '{print $5}')"
echo "   Permissions: $(ls -l build/bin/grasp | awk '{print $1}')"
echo ""

echo "2. Library Dependencies:"
echo "   Checking dependencies with ldd..."
if ! ldd build/bin/grasp; then
    echo "   ERROR: Cannot resolve dependencies"
    exit 1
fi
echo ""

echo "3. Current Environment:"
echo "   LD_LIBRARY_PATH=${LD_LIBRARY_PATH:-"(not set)"}"
echo "   Current working directory: $(pwd)"
echo ""

echo "4. System Information:"
echo "   OS: $(lsb_release -d 2>/dev/null | cut -f2 || uname -s)"
echo "   Kernel: $(uname -r)"
echo "   Architecture: $(uname -m)"
echo ""

echo "5. ZMQ Libraries:"
if pkg-config --exists libzmq; then
    echo "   libzmq version: $(pkg-config --modversion libzmq)"
    echo "   libzmq cflags: $(pkg-config --cflags libzmq)"
    echo "   libzmq libs: $(pkg-config --libs libzmq)"
else
    echo "   libzmq: not found via pkg-config"
fi
echo ""

echo "6. Hardware Check:"
if command -v lsusb >/dev/null; then
    echo "   USB devices:"
    lsusb | grep -E "(Peak|PCAN)" || echo "   No PCAN devices found"
else
    echo "   lsusb not available"
fi
echo ""

echo "7. Network Check:"
echo "   Checking if port 5556 is available..."
if netstat -ln 2>/dev/null | grep -q ":5556 "; then
    echo "   WARNING: Port 5556 is already in use!"
    netstat -ln | grep ":5556 "
else
    echo "   Port 5556 is available"
fi
echo ""

echo "8. Running Server:"
echo "   Starting server with debug output..."
echo "   Press Ctrl+C to stop"
echo ""

# Set up better library path
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/local/lib:${LD_LIBRARY_PATH}"

# Add BHand library if available
if [ -d "/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64/lib" ]; then
    export LD_LIBRARY_PATH="/home/tml-franka-ws/robot_libraries/allegro_driver/libBHand_64/lib:${LD_LIBRARY_PATH}"
fi

echo "   Final LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo ""

# Run with verbose output
exec ./build/bin/grasp