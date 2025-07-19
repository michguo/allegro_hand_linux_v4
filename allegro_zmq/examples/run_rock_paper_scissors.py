#
#   Allegro client in Python - Rock Paper Scissors Demo
#   Uses JSON commands to send joint positions to ZMQ server
#
import json
import numpy as np
import time
import zmq
from allegro_zmq.allegro import AllegroCommand, AllegroMotionType

context = zmq.Context()

#  Socket to talk to server
print("Connecting to Allegro ZMQ server...")
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5556")

# Define joint positions for rock, paper, scissors gestures
rock_q = np.array([	-0.1194, 1.2068, 1.0, 1.4042,
	-0.0093, 1.2481, 1.4073, 0.8163,
	0.1116, 1.2712, 1.3881, 1.0122,
	0.6017, 0.2976, 0.9034, 0.7929])

paper_q = np.array([
	-0.1220, 0.4, 0.6, -0.0769,
	0.0312, 0.4, 0.6, -0.0,
	0.1767, 0.4, 0.6, -0.0528,
	0.5284, 0.3693, 0.8977, 0.4863])

scissors_q = np.array([
	0.0885, 0.4, 0.6, -0.0704,
	0.0312, 0.4, 0.6, -0.0,
	0.1019, 1.2375, 1.1346,
	1.0244, 1.0, 0.6331, 1.3509, 1.0])

# Set the gains
kp = np.array([
		500, 800, 900, 500,
		500, 800, 900, 500,
		500, 800, 900, 500,
		1000, 700, 600, 600
	]);
kd = np.array([
		25, 50, 55, 40,
		25, 50, 55, 40,
		25, 50, 55, 40,
		50, 50, 50, 40
	]);


def send_json_command(desired_positions, name):
    """Send JSON command to move hand to desired positions"""
    json_cmd = {
        "motion_type": AllegroMotionType.eMotionType_JOINT_PD,  # JOINT_PD
        "joint_positions": [0.0] * 16,  # Current positions (will be updated by server)
        "desired_positions": desired_positions.tolist(),
        "grasping_forces": [0.0] * 4,
        "fingertip_positions": None,
        "object_displacement": None,
        "time_interval": 0.003,
        "kp_gains": kp.tolist(),
        "kd_gains": kd.tolist()
    }
    
    json_str = json.dumps(json_cmd)
    print(f"Sending {name}: {json_str}")
    socket.send_string(json_str)
    
    # Get the reply
    message = socket.recv_string()
    print(f"Received reply: {message}")
    return message

# Perform rock-paper-scissors sequence 3 times
gestures = [
    (rock_q, "Rock"),
    (paper_q, "Paper"), 
    (scissors_q, "Scissors")
]

print("Starting Rock-Paper-Scissors demo...")
print("Make sure the Allegro Hand ZMQ server is running!")
print()

try:
    for cycle in range(3):
        print(f"=== Cycle {cycle + 1} ===")
        
        for q, name in gestures:
            send_json_command(q, name)
            time.sleep(2)  # Wait 2 seconds between gestures
        
        print()
        
except Exception as e:
    print(f"Error: {e}")
    print("Make sure the C++ ZMQ server is running on port 5556")
finally:
    socket.close()
    context.term()
    print("Demo completed!")