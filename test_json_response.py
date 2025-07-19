#!/usr/bin/env python3
"""
Test script demonstrating the new JSON response structure with full hand state.
"""

import json
import numpy as np
from allegro_zmq.allegro import AllegroHandClient, AllegroCommand, AllegroMotionType

def test_json_response():
    """Test the new JSON response structure"""
    
    print("Testing new JSON response structure...")
    print("Note: Make sure the C++ server is running first with: ./build/bin/grasp")
    print()
    
    try:
        # Create client
        with AllegroHandClient() as client:
            
            # Test 1: Send HOME command
            print("1. Testing HOME motion command...")
            home_cmd = AllegroCommand(motion_type=AllegroMotionType.eMotionType_HOME)
            state = client.send_command(home_cmd)
            
            print(f"   Response: {state}")
            print(f"   Success: {state.is_successful()}")
            print(f"   Motion: {state.get_motion_name()}")
            print(f"   Hand Type: {state.get_hand_type_name()}")
            print(f"   Message: {state.message}")
            print()
            
            # Test 2: Send PD control command
            print("2. Testing JOINT_PD control command...")
            desired_pos = np.zeros(16)  # All joints to zero
            pd_cmd = AllegroCommand(
                motion_type=AllegroMotionType.eMotionType_JOINT_PD,
                desired_positions=desired_pos
            )
            state = client.send_command(pd_cmd)
            
            print(f"   Response: {state}")
            print(f"   Current joint positions: {state.qpos_measured[:4]}... (first 4 joints)")
            print(f"   Commanded torques: {state.tau_commanded[:4]}... (first 4 joints)")
            print(f"   Fingertip positions:")
            for i, pos in enumerate(state.fingertip_positions):
                print(f"     Finger {i}: [{pos[0]:.3f}, {pos[1]:.3f}, {pos[2]:.3f}]")
            print()
            
            # Test 3: Get hand state only
            print("3. Testing get hand state...")
            state = client.get_hand_state()
            print(f"   Hand state: {state}")
            print(f"   Time interval: {state.time_interval}")
            print()
            
            # Test 4: Show all available data
            print("4. Full hand state data:")
            print(f"   Type: {state.type}")
            print(f"   Success: {state.success}")
            print(f"   Message: '{state.message}'")
            print(f"   Hand Type: {state.hand_type} ({state.get_hand_type_name()})")
            print(f"   Motion Type: {state.motion_type} ({state.get_motion_name()})")
            print(f"   Time Interval: {state.time_interval}")
            print(f"   Measured Positions: {len(state.qpos_measured)} joints")
            print(f"   Commanded Positions: {len(state.qpos_commanded)} joints") 
            print(f"   Commanded Torques: {len(state.tau_commanded)} joints")
            print(f"   Fingertip Positions: {state.fingertip_positions.shape}")
            print(f"   Grasping Forces: {state.grasping_forces.shape}")
            if state.data:
                print(f"   Additional Data: {len(state.data)} elements")
            print()
            
    except Exception as e:
        print(f"Error: {e}")
        print("Make sure the C++ server is running on port 5556")
        print("Run: ./build/bin/grasp")

if __name__ == "__main__":
    test_json_response()