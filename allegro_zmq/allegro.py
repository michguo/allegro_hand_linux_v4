import json
import numpy as np
from typing import Optional, List, Tuple, Union
from dataclasses import dataclass, asdict

# Constants from BHand.h
NOF = 4  # number of fingers
NOJ = 4  # number of joints in each finger
TOTAL_JOINTS = NOF * NOJ  # 16 total joints

class AllegroMotionType:
    """Motion types for Allegro Hand as defined in BHand.h"""
    eMotionType_NONE = 0                # power off
    eMotionType_HOME = 1                # go to home position
    eMotionType_READY = 2               # finger position move motion (ready)
    eMotionType_GRAVITY_COMP = 3        # finger position move motion (gravity compensation)
    eMotionType_PRE_SHAPE = 4           # pre-shape motion
    eMotionType_GRASP_3 = 5             # grasping using 3 fingers
    eMotionType_GRASP_4 = 6             # grasping using 4 fingers
    eMotionType_PINCH_IT = 7            # pinching using index finger and thumb
    eMotionType_PINCH_MT = 8            # pinching using middle finger and thumb
    eMotionType_OBJECT_MOVING = 9       # object moving motion
    eMotionType_ENVELOP = 10            # enveloping grasp
    eMotionType_JOINT_PD = 11           # joint pd control
    eMotionType_MOVE_OBJ = 12           # object manipulation
    eMotionType_FINGERTIP_MOVING = 13   # fingertip moving control
    NUMBER_OF_MOTION_TYPE = 14          # total number of motion types

    # Motion type names for display
    MOTION_NAMES = {
        0: "NONE",
        1: "HOME",
        2: "READY",
        3: "GRAVITY_COMP",
        4: "PRE_SHAPE",
        5: "GRASP_3",
        6: "GRASP_4",
        7: "PINCH_IT",
        8: "PINCH_MT",
        9: "OBJECT_MOVING",
        10: "ENVELOP",
        11: "JOINT_PD",
        12: "MOVE_OBJ",
        13: "FINGERTIP_MOVING"
    }

    @classmethod
    def get_motion_name(cls, motion_type: int) -> str:
        """Get human-readable name for motion type"""
        return cls.MOTION_NAMES.get(motion_type, f"UNKNOWN_{motion_type}")

    @classmethod
    def is_valid_motion_type(cls, motion_type: int) -> bool:
        """Check if motion type is valid"""
        return 0 <= motion_type < cls.NUMBER_OF_MOTION_TYPE

class AllegroHandType:
    """Hand type enumeration"""
    LEFT = 0
    RIGHT = 1

@dataclass
class AllegroCommand:
    """Command structure for Allegro Hand"""
    def __init__(self, motion_type: int = AllegroMotionType.eMotionType_NONE, 
                 joint_positions: Optional[np.ndarray] = None,
                 desired_positions: Optional[np.ndarray] = None,
                 grasping_forces: Optional[np.ndarray] = None,
                 fingertip_positions: Optional[List[np.ndarray]] = None,
                 object_displacement: Optional[np.ndarray] = None,
                 time_interval: Optional[float] = None,
                 **kwargs):
        self.motion_type = motion_type
        self.joint_positions = joint_positions if joint_positions is not None else np.zeros(TOTAL_JOINTS)
        self.desired_positions = desired_positions if desired_positions is not None else np.zeros(TOTAL_JOINTS)
        self.grasping_forces = grasping_forces if grasping_forces is not None else np.zeros(NOF)
        self.fingertip_positions = fingertip_positions
        self.object_displacement = object_displacement
        self.time_interval = time_interval
        
        # Additional parameters
        for key, value in kwargs.items():
            setattr(self, key, value)

    def to_dict(self):
        return asdict(self)

    def to_json(self):
        return json.dumps(self.to_dict())

    def __repr__(self):
        return self.__str__()

    def __eq__(self, other):
        if not isinstance(other, AllegroCommand):
            return False
        return (self.motion_type == other.motion_type and 
                np.array_equal(self.joint_positions, other.joint_positions))

    def validate(self) -> bool:
        """Validate command parameters"""
        if not AllegroMotionType.is_valid_motion_type(self.motion_type):
            return False
        
        if len(self.joint_positions) != TOTAL_JOINTS:
            return False
            
        if len(self.desired_positions) != TOTAL_JOINTS:
            return False
            
        if len(self.grasping_forces) != NOF:
            return False
            
        return True

@dataclass 
class AllegroHandState:
    """Complete hand state response from ZMQ server"""
    def __init__(self, response_dict: Optional[dict] = None):
        if response_dict is None:
            response_dict = {}
            
        # Response metadata
        self.type = response_dict.get('type', 0)
        self.success = response_dict.get('success', False)
        self.message = response_dict.get('message', '')
        
        # Hand configuration
        self.hand_type = response_dict.get('hand_type', 0)  # 0=Left, 1=Right
        self.time_interval = response_dict.get('time_interval', 0.003)
        self.motion_type = response_dict.get('motion_type', 0)
        
        # Joint data (16 joints)
        self.qpos_measured = np.array(response_dict.get('qpos_measured', [0.0] * 16))
        self.qpos_commanded = np.array(response_dict.get('qpos_commanded', [0.0] * 16))
        self.tau_commanded = np.array(response_dict.get('tau_commanded', [0.0] * 16))
        
        # Fingertip positions (4 fingers x 3 coordinates)
        fingertip_pos = response_dict.get('fingertip_positions', [[0.0, 0.0, 0.0]] * 4)
        self.fingertip_positions = np.array(fingertip_pos)
        
        # Grasping forces (4 fingers x 3 directions)  
        grasp_forces = response_dict.get('grasping_forces', [[0.0, 0.0, 0.0]] * 4)
        self.grasping_forces = np.array(grasp_forces)
        
        # Additional data
        self.data = response_dict.get('data', [])
    
    @classmethod
    def from_json(cls, json_str: str):
        """Create AllegroHandState from JSON string"""
        try:
            response_dict = json.loads(json_str)
            return cls(response_dict)
        except json.JSONDecodeError as e:
            # Return error state
            error_state = cls()
            error_state.success = False
            error_state.message = f"JSON decode error: {e}"
            return error_state
    
    def get_motion_name(self) -> str:
        """Get human-readable motion type name"""
        return AllegroMotionType.get_motion_name(self.motion_type)
    
    def get_hand_type_name(self) -> str:
        """Get human-readable hand type name"""
        return "LEFT" if self.hand_type == AllegroHandType.LEFT else "RIGHT"
    
    def is_successful(self) -> bool:
        """Check if the command was successful"""
        return self.success
    
    def __str__(self):
        return f"AllegroHandState(success={self.success}, motion={self.get_motion_name()}, hand={self.get_hand_type_name()})"
    
    def __repr__(self):
        return self.__str__()


class AllegroHandClient:
    """ZMQ Client for communicating with Allegro Hand server"""
    
    def __init__(self, server_address: str = "tcp://localhost:5556"):
        """Initialize ZMQ client
        
        Args:
            server_address: ZMQ server address (default: tcp://localhost:5556)
        """
        import zmq
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect(server_address)
        self.server_address = server_address
    
    def send_command(self, command: AllegroCommand) -> AllegroHandState:
        """Send command to server and get hand state response
        
        Args:
            command: AllegroCommand to send
            
        Returns:
            AllegroHandState: Current hand state after command execution
        """
        try:
            # Convert command to JSON
            json_cmd = {
                "motion_type": command.motion_type,
                "joint_positions": command.joint_positions.tolist(),
                "desired_positions": command.desired_positions.tolist(), 
                "grasping_forces": command.grasping_forces.tolist(),
                "fingertip_positions": command.fingertip_positions,
                "object_displacement": command.object_displacement,
                "time_interval": command.time_interval
            }
            
            # Add any additional parameters
            for attr_name in dir(command):
                if not attr_name.startswith('_') and attr_name not in json_cmd:
                    attr_value = getattr(command, attr_name)
                    if not callable(attr_value):
                        json_cmd[attr_name] = attr_value
            
            json_str = json.dumps(json_cmd)
            
            # Send command
            self.socket.send_string(json_str)
            
            # Receive response
            response_str = self.socket.recv_string()
            
            # Parse response
            return AllegroHandState.from_json(response_str)
            
        except Exception as e:
            # Return error state
            error_state = AllegroHandState()
            error_state.success = False
            error_state.message = f"Communication error: {e}"
            return error_state
    
    def get_hand_state(self) -> AllegroHandState:
        """Get current hand state without sending a command
        
        Returns:
            AllegroHandState: Current hand state
        """
        # Send a minimal command just to get state
        cmd = AllegroCommand(motion_type=AllegroMotionType.eMotionType_NONE)
        return self.send_command(cmd)
    
    def close(self):
        """Close ZMQ connection"""
        self.socket.close()
        self.context.term()
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


def create_allegro_hand(hand_type: str = "right", server_address: str = "tcp://localhost:5556") -> AllegroHandClient:
    """Create and return an AllegroHandClient
    
    Args:
        hand_type: "left" or "right" (for compatibility, not used by client)
        server_address: ZMQ server address
        
    Returns:
        AllegroHandClient: Ready-to-use client
    """
    return AllegroHandClient(server_address)
