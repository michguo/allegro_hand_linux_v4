#!/usr/bin/env python3

import json
import zmq
import time

def test_simple_json_commands():
    """Test simple JSON commands without requiring allegro_zmq module"""
    
    # Create ZMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.RCVTIMEO, 5000)  # 5 second timeout
    socket.connect("tcp://localhost:5556")
    
    print("Connected to ZMQ server on port 5556")
    print("=" * 50)
    
    # Test 1: HOME motion
    print("Test 1: HOME Motion")
    home_cmd = {
        "motion_type": 1,  # HOME
        "joint_positions": [0.0] * 16,
        "desired_positions": [0.0] * 16,
        "grasping_forces": [0.0] * 4,
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": 0.003
    }
    
    json_str = json.dumps(home_cmd)
    print(f"Sending: {json_str}")
    
    socket.send_string(json_str)
    response = socket.recv_string()
    print(f"Response: {response}")
    print()
    
    # Test 2: JOINT_PD motion with specific angles
    print("Test 2: Joint PD Control")
    pd_cmd = {
        "motion_type": 11,  # JOINT_PD
        "joint_positions": [0.0] * 16,
        "desired_positions": [0.1] * 16,  # Small joint angles
        "grasping_forces": [0.0] * 4,
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": 0.003
    }
    
    json_str = json.dumps(pd_cmd)
    print(f"Sending: {json_str}")
    
    socket.send_string(json_str)
    response = socket.recv_string()
    print(f"Response: {response}")
    print()
    
    # Test 3: GRASP_3 motion with forces
    print("Test 3: 3-Finger Grasp")
    grasp_cmd = {
        "motion_type": 5,  # GRASP_3
        "joint_positions": [0.0] * 16,
        "desired_positions": [0.0] * 16,
        "grasping_forces": [5.0, 5.0, 5.0, 0.0],  # Forces for 3 fingers
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": 0.003
    }
    
    json_str = json.dumps(grasp_cmd)
    print(f"Sending: {json_str}")
    
    socket.send_string(json_str)
    response = socket.recv_string()
    print(f"Response: {response}")
    print()
    
    # Test 4: Invalid motion type (should fail)
    print("Test 4: Invalid Motion Type (should fail)")
    invalid_cmd = {
        "motion_type": 99,  # Invalid
        "joint_positions": [0.0] * 16,
        "desired_positions": [0.0] * 16,
        "grasping_forces": [0.0] * 4,
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": 0.003
    }
    
    json_str = json.dumps(invalid_cmd)
    print(f"Sending: {json_str}")
    
    socket.send_string(json_str)
    response = socket.recv_string()
    print(f"Response: {response}")
    print("(Expected: error message)")
    print()
    
    # Test 5: Non-JSON message (should fail)
    print("Test 5: Non-JSON Message (should fail)")
    non_json = "This is not JSON"
    print(f"Sending: {non_json}")
    
    socket.send_string(non_json)
    response = socket.recv_string()
    print(f"Response: {response}")
    print("(Expected: error message)")
    print()
    
    socket.close()
    context.term()
    print("All tests completed!")

if __name__ == "__main__":
    print("Simple JSON Command Tester")
    print("=" * 50)
    print("Make sure the Allegro ZMQ server is running:")
    print("  ./run_zmq_server.sh")
    print()
    
    try:
        test_simple_json_commands()
    except zmq.error.Again:
        print("Error: Connection timeout")
        print("Make sure the server is running on port 5556")
    except ConnectionRefusedError:
        print("Error: Connection refused")
        print("Make sure the server is running on port 5556")
    except Exception as e:
        print(f"Error: {e}")
        print("Make sure the C++ server is running on port 5556")