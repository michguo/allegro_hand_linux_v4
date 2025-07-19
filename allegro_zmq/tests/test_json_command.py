#!/usr/bin/env python3

import json
import zmq
import numpy as np
from allegro_zmq.allegro import AllegroCommand, AllegroMotionType

def test_json_command():
    """Test sending JSON command to C++ server"""
    
    # Create ZMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5556")
    
    # Create an AllegroCommand
    cmd = AllegroCommand(
        motion_type=AllegroMotionType.eMotionType_JOINT_PD,
        desired_positions=np.zeros(16),  # All joints to 0
        time_interval=0.003
    )
    
    # Convert to JSON and send
    json_cmd = {
        "motion_type": cmd.motion_type,
        "joint_positions": cmd.joint_positions.tolist(),
        "desired_positions": cmd.desired_positions.tolist(),
        "grasping_forces": cmd.grasping_forces.tolist(),
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": cmd.time_interval
    }
    
    json_str = json.dumps(json_cmd)
    print(f"Sending JSON command: {json_str}")
    
    # Send message
    socket.send_string(json_str)
    
    # Wait for reply
    response = socket.recv_string()
    print(f"Received response: {response}")
    
    socket.close()
    context.term()

def test_non_json_command():
    """Test sending non-JSON command (should fail gracefully)"""
    
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5556")
    
    # Send non-JSON format (should get error response)
    non_json_cmd = "0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0"
    print(f"Sending non-JSON command: {non_json_cmd}")
    
    socket.send_string(non_json_cmd)
    response = socket.recv_string()
    print(f"Received response: {response}")
    print("(Expected: failure message since only JSON commands are supported)")
    
    socket.close()
    context.term()

if __name__ == "__main__":
    print("Note: Make sure the C++ server is running first!")
    print("Run: ./build/bin/grasp")
    print()
    
    try:
        print("Testing JSON command...")
        test_json_command()
        print()
        
        print("Testing non-JSON command (should fail)...")
        test_non_json_command()
        
    except Exception as e:
        print(f"Error: {e}")
        print("Make sure the C++ server is running on port 5556")