# Allegro Hand ZMQ Interface

Adapted from https://github.com/simlabrobotics/allegro_hand_linux_v4 and https://github.com/michguo/allegro_hand_linux_v4.

This interface allows communicating with a v4 Allegro hand via ZMQ, providing both C++ and Python APIs for controlling the hand.

## Repository Structure

```
allegro_hand_zmq/
├── cpp/                    # C++ source code and headers
│   ├── src/               # C++ implementation files
│   ├── include/           # C++ header files
│   └── CMakeLists.txt     # C++ build configuration
├── allegro_zmq/           # Python package
│   ├── allegro.py         # Python wrapper for BHand library
│   ├── examples/          # Python usage examples
│   └── utils/             # Python utilities
├── build.sh              # Build script for C++ components
├── run_zmq_server.sh     # Script to start ZMQ server
└── README.md             # This file
```

## Required Hardware

1. [Allegro hand v4](http://wiki.wonikrobotics.com/AllegroHandWiki/index.php/Allegro_Hand_v4.0)
2. [PCAN-USB interface](https://www.peak-system.com/PCAN-USB.199.0.html?&L=1)

## Setup Instructions

### Prerequisites

#### 1. System Dependencies
```bash
# Install required packages
sudo apt update
sudo apt install build-essential cmake pkg-config libpopt-dev

# Install ZMQ development libraries
sudo apt install libzmq3-dev libcppzmq-dev

# Install nlohmann/json for JSON parsing
conda install -c conda-forge nlohmann_json
```

#### 2. PCAN-USB Driver
Download, build, and install PCAN-USB driver for Linux: [libpcan](https://www.peak-system.com/fileadmin/media/linux/can-version-history.php)

```bash
# Download and extract the driver
tar -xzvf peak-linux-driver-x.x.tar.gz
cd peak-linux-driver-x.x

# Build and install
make NET=NO
sudo make install
```
#### Troubleshooting
* If encounter 
  ```bash
  src/pcan-settings.c:47:10: fatal error: popt.h: No such file or directory
     47 | #include <popt.h>
      |      	^~~~~~~~
  compilation terminated.
  ```
  Run
  ```bash
  sudo apt install libpopt-dev
  ```

#### 3. PCAN-Basic API
Download, build, and install PCAN-Basic API for Linux: [libpcanbasic](https://www.peak-system.com/PCAN-Basic-Linux.433.0.html)

```bash
# Download and extract
tar -xzvf PCAN_Basic_Linux-x.x.x.tar.gz
cd PCAN_Basic_Linux-x.x.x/libpcanbasic

# Build and install
make
sudo make install
sudo ldconfig
```

#### 4. BHand Grasping Library
Download, build, and install Grasping Library for Linux: [libBHand](https://www.allegrohand.com/v4-main/grasping-library-for-linux)

```bash
# Extract the appropriate version (32-bit or 64-bit)
unzip LibBHand_64.zip  # or LibBHand_32.zip for 32-bit systems
cd libBHand_64

# Install the library
sudo make install
sudo ldconfig
```


### Building the Interface

#### C++ Components
Use the provided build script to compile the C++ ZMQ server:

```bash
# Navigate to the repository root
cd allegro_hand_zmq

# Build the C++ components
./build.sh

# For a clean build (removes previous build files)
./build.sh clean
```

The build script will:
- Configure CMake with proper library paths
- Compile the C++ source code in `cpp/src/`
- Create the executable at `build/bin/grasp`
- Handle dependency linking automatically

#### Python Components
Install the Python package for client-side control:

```bash
# Install in development mode
pip install -e .
```

## Usage

### 1. Hardware Setup
1. Connect the PCAN-USB adapter to your computer
2. Connect the PCAN-USB to the Allegro Hand CAN interface
3. **Important:** Ensure the Allegro Hand is powered OFF before starting the software

### 2. Starting the ZMQ Server
Launch the C++ ZMQ server that interfaces with the hand:

```bash
# Start the ZMQ server (keep this running)
./run_zmq_server.sh
```

The server will:
- Initialize the PCAN-USB connection
- Load the BHand control library
- Start the ZMQ server on the default port
- Wait for client connections

### 3. Power On the Hand
After the server is running and shows "Ready", power on the Allegro Hand.

### 4. Controlling the Hand

#### Python API
Use the Python wrapper for high-level control:

```python
from allegro_zmq.allegro import create_allegro_hand, AllegroMotionType

# Create hand controller
hand = create_allegro_hand("right")
hand.initialize_hand()

# Move to home position
home_cmd = hand.create_home_command()
hand.execute_command(home_cmd)

# PD control with custom positions
import numpy as np
positions = np.zeros(16)  # 16 joint positions
pd_cmd = hand.create_pd_command(positions)
hand.execute_command(pd_cmd)

# Get hand state
state = hand.get_hand_state()
print(f"Current motion: {state['motion_type']}")
```

#### Example Scripts
Run the provided examples:

```bash
# Rock Paper Scissors demo
python allegro_zmq/examples/run_rock_paper_scissors.py
```

### 5. Available Motion Types

The Python API supports all BHand library motion types:

- `eMotionType_NONE` - Power off
- `eMotionType_HOME` - Home position
- `eMotionType_READY` - Ready position
- `eMotionType_GRAVITY_COMP` - Gravity compensation
- `eMotionType_JOINT_PD` - Joint PD control
- `eMotionType_GRASP_3` - 3-finger grasp
- `eMotionType_GRASP_4` - 4-finger grasp
- `eMotionType_PINCH_IT` - Index-thumb pinch
- `eMotionType_PINCH_MT` - Middle-thumb pinch
- `eMotionType_ENVELOP` - Enveloping grasp
- `eMotionType_MOVE_OBJ` - Object manipulation
- `eMotionType_FINGERTIP_MOVING` - Fingertip control

## Development

### Project Structure
- **C++ Code**: All C++ source files are in `cpp/src/` with headers in `cpp/include/`
- **Python Code**: Python wrapper and utilities in `allegro_zmq/`
- **Build System**: Modern CMake with automatic dependency detection
- **Examples**: Both C++ and Python examples provided

### Building for Development
```bash
# Clean build
./build.sh clean
