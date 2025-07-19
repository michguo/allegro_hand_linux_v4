#include "allegroZmqJsonParser.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

// Constructor
AllegroZmqJsonParser::AllegroZmqJsonParser(BHand* pBHand) 
    : pBHand(pBHand), currentTime(0.0) {
    currentQ.resize(MAX_DOF, 0.0);
    desiredQ.resize(MAX_DOF, 0.0);
    currentTau.resize(MAX_DOF, 0.0);
}

// Destructor
AllegroZmqJsonParser::~AllegroZmqJsonParser() {
    // Nothing to clean up - pBHand is managed externally
}

// Parse JSON command and execute
AllegroZmqResponse AllegroZmqJsonParser::parseJsonAndExecute(const std::string& jsonStr) {
    if (!isHandReady()) {
        return createErrorResponse("BHand not initialized");
    }
    
    try {
        // Parse JSON command
        AllegroJsonCommand jsonCmd = parseJsonCommand(jsonStr);
        
        // Validate command
        if (!validateJsonCommand(jsonCmd)) {
            return createErrorResponse("Invalid JSON command structure");
        }
        
        // First set the motion type
        bool success = setMotionType(jsonCmd.motion_type);
        if (!success) {
            return createErrorResponse("Failed to set motion type");
        }
        
        // Handle different motion types
        switch (jsonCmd.motion_type) {
            case 11: // eMotionType_JOINT_PD
                // For PD control, set desired positions
                success = setDesiredJointPositions(jsonCmd.desired_positions);
                if (!success) {
                    return createErrorResponse("Failed to set desired joint positions");
                }
                break;
            default:
                // For other motion types, just setting the motion type is sufficient
                std::cout << "Motion type not implemented: " << jsonCmd.motion_type << std::endl;
                break;
        }
        
        // Set time interval if provided
        if (jsonCmd.time_interval > 0) {
            pBHand->SetTimeInterval(jsonCmd.time_interval);
        }
        
        // Set gains if provided
        if (!jsonCmd.kp_gains.empty() && !jsonCmd.kd_gains.empty()) {
            success = setGains(jsonCmd.kp_gains, jsonCmd.kd_gains);
            if (!success) {
                return createErrorResponse("Failed to set gains");
            }
        }
        
        // Update control
        success = updateControl(0.0);
        if (!success) {
            return createErrorResponse("Failed to update control");
        }
        
        return createSuccessResponse("JSON command executed successfully");
        
    } catch (const std::exception& e) {
        return createErrorResponse("JSON parsing error: " + std::string(e.what()));
    }
}

// Compute joint torques
void AllegroZmqJsonParser::UpdateControl(double* current_q, double* result_torques) {
    for (int i = 0; i < MAX_DOF; i++) {
        currentQ[i] = current_q[i];
    }
    pBHand->SetJointPosition(current_q); // tell BHand library the current joint positions
    pBHand->SetJointDesiredPosition(desiredQ.data());
    pBHand->UpdateControl(0);
    pBHand->GetJointTorque(result_torques);
}

// Parse JSON command
AllegroJsonCommand AllegroZmqJsonParser::parseJsonCommand(const std::string& jsonStr) {
    AllegroJsonCommand cmd;
    
    try {
        nlohmann::json j = nlohmann::json::parse(jsonStr);
        
        // Parse motion type
        if (j.contains("motion_type")) {
            cmd.motion_type = j["motion_type"].get<int>();
        }
        
        // Parse joint positions
        if (j.contains("joint_positions")) {
            cmd.joint_positions = jsonArrayToVector(j["joint_positions"], 16);
        }
        
        // Parse desired positions
        if (j.contains("desired_positions")) {
            cmd.desired_positions = jsonArrayToVector(j["desired_positions"], 16);
        }
        
        // Parse grasping forces
        if (j.contains("grasping_forces")) {
            cmd.grasping_forces = jsonArrayToVector(j["grasping_forces"], 4);
        }
        
        // Parse fingertip positions (4x3 matrix)
        if (j.contains("fingertip_positions")) {
            cmd.fingertip_positions = json2DArrayToVector(j["fingertip_positions"]);
        }
        
        // Parse object displacement
        if (j.contains("object_displacement")) {
            cmd.object_displacement = jsonArrayToVector(j["object_displacement"]);
        }
        
        // Parse time interval
        if (j.contains("time_interval")) {
            cmd.time_interval = j["time_interval"].get<double>();
        }
        
        // Parse proportional gains
        if (j.contains("kp_gains")) {
            cmd.kp_gains = jsonArrayToVector(j["kp_gains"], 16);
        }
        
        // Parse derivative gains
        if (j.contains("kd_gains")) {
            cmd.kd_gains = jsonArrayToVector(j["kd_gains"], 16);
        }
        
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }
    
    return cmd;
}

// Validate JSON command
bool AllegroZmqJsonParser::validateJsonCommand(const AllegroJsonCommand& cmd) {
    // Check motion type range
    if (cmd.motion_type < 0 || cmd.motion_type >= 14) {
        return false;
    }
    
    // Check joint positions size
    if (cmd.joint_positions.size() != 16) {
        return false;
    }
    
    // Check desired positions size
    if (cmd.desired_positions.size() != 16) {
        return false;
    }
    
    // Check grasping forces size
    if (cmd.grasping_forces.size() != 4) {
        return false;
    }
    
    // Check gains sizes
    if (cmd.kp_gains.size() != 16 || cmd.kd_gains.size() != 16) {
        return false;
    }
    
    // Check for NaN or infinite values in joint positions
    for (double pos : cmd.joint_positions) {
        if (std::isnan(pos) || std::isinf(pos)) {
            return false;
        }
    }
    
    // Check for NaN or infinite values in desired positions
    for (double pos : cmd.desired_positions) {
        if (std::isnan(pos) || std::isinf(pos)) {
            return false;
        }
    }
    
    // Check for NaN or infinite values in forces
    for (double force : cmd.grasping_forces) {
        if (std::isnan(force) || std::isinf(force)) {
            return false;
        }
    }
    
    // Check time interval
    if (cmd.time_interval < 0 || std::isnan(cmd.time_interval) || std::isinf(cmd.time_interval)) {
        return false;
    }
    
    return true;
}

// Convert JSON array to vector
std::vector<double> AllegroZmqJsonParser::jsonArrayToVector(const nlohmann::json& jsonArray, int expectedSize) {
    std::vector<double> result;
    
    if (jsonArray.is_null()) {
        // Return default-sized vector filled with zeros
        if (expectedSize > 0) {
            result.resize(expectedSize, 0.0);
        }
        return result;
    }
    
    if (!jsonArray.is_array()) {
        throw std::runtime_error("Expected JSON array");
    }
    
    for (const auto& element : jsonArray) {
        if (element.is_number()) {
            result.push_back(element.get<double>());
        } else {
            result.push_back(0.0);  // Default for non-numeric values
        }
    }
    
    // Validate size if expected
    if (expectedSize > 0 && result.size() != static_cast<size_t>(expectedSize)) {
        // Resize to expected size, padding with zeros or truncating
        result.resize(expectedSize, 0.0);
    }
    
    return result;
}

// Convert 2D JSON array to vector of vectors
std::vector<std::vector<double>> AllegroZmqJsonParser::json2DArrayToVector(const nlohmann::json& jsonArray) {
    std::vector<std::vector<double>> result;
    
    if (jsonArray.is_null() || !jsonArray.is_array()) {
        return result;  // Return empty vector
    }
    
    for (const auto& row : jsonArray) {
        if (row.is_array()) {
            std::vector<double> rowVec;
            for (const auto& element : row) {
                if (element.is_number()) {
                    rowVec.push_back(element.get<double>());
                } else {
                    rowVec.push_back(0.0);
                }
            }
            result.push_back(rowVec);
        }
    }
    
    return result;
}

// Set motion type
bool AllegroZmqJsonParser::setMotionType(int motionType) {
    if (!pBHand) return false;
    
    // Validate motion type range (based on BHand.h motion types)
    if (motionType < 0 || motionType >= 14) {
        return false;
    }
    
    pBHand->SetMotionType(motionType);
    return true;
}

// Set desired joint positions for PD control
bool AllegroZmqJsonParser::setDesiredJointPositions(const std::vector<double>& positions) {
    if (!pBHand || !validateJointPositions(positions)) return false;
    
    // Copy to internal array
    for (size_t i = 0; i < positions.size() && i < MAX_DOF; i++) {
        desiredQ[i] = positions[i];
        std::cout << "desiredQ[" << i << "] = " << desiredQ[i] << std::endl;
    }
    return true;
}

// Set grasping forces
bool AllegroZmqJsonParser::setGraspingForces(const std::vector<double>& forces) {
    if (!pBHand || !validateFingerForces(forces)) return false;
    
    // Create non-const copy for BHand API
    std::vector<double> forceCopy = forces;
    pBHand->SetGraspingForce(forceCopy.data());
    return true;
}

// Set PID gains
bool AllegroZmqJsonParser::setGains(const std::vector<double>& kp, const std::vector<double>& kd) {
    if (!pBHand || !validateGains(kp, kd)) return false;
    
    // Create non-const copies for BHand API
    std::vector<double> kpCopy = kp;
    std::vector<double> kdCopy = kd;
    pBHand->SetGainsEx(kpCopy.data(), kdCopy.data());
    return true;
}

// Update control algorithm
bool AllegroZmqJsonParser::updateControl(double time) {
    if (!pBHand) return false;
    
    currentTime = time;
    pBHand->UpdateControl(time);
    return true;
}

// Validate joint positions array
bool AllegroZmqJsonParser::validateJointPositions(const std::vector<double>& positions) {
    if (positions.size() != MAX_DOF) return false;
    
    // Check for reasonable joint angle limits (roughly -π to π)
    for (double pos : positions) {
        if (std::isnan(pos) || std::isinf(pos) || std::abs(pos) > 6.28) {
            return false;
        }
    }
    
    return true;
}

// Validate finger forces array
bool AllegroZmqJsonParser::validateFingerForces(const std::vector<double>& forces) {
    if (forces.size() != 4) return false;
    
    // Check for reasonable force limits
    for (double force : forces) {
        if (std::isnan(force) || std::isinf(force) || std::abs(force) > 100.0) {
            return false;
        }
    }
    
    return true;
}

// Validate gains arrays
bool AllegroZmqJsonParser::validateGains(const std::vector<double>& kp, const std::vector<double>& kd) {
    if (kp.size() != 16 || kd.size() != 16) return false;
    
    // Check for reasonable gain limits
    for (double gain : kp) {
        if (std::isnan(gain) || std::isinf(gain) || gain < 0.0 || gain > 10000.0) {
            return false;
        }
    }
    
    for (double gain : kd) {
        if (std::isnan(gain) || std::isinf(gain) || gain < 0.0 || gain > 1000.0) {
            return false;
        }
    }
    
    return true;
}

// Create error response
AllegroZmqResponse AllegroZmqJsonParser::createErrorResponse(const std::string& message) {
    AllegroZmqResponse response(RESP_ERROR, false);
    response.message = message;
    return response;
}

// Create success response with full hand state
AllegroZmqResponse AllegroZmqJsonParser::createSuccessResponse(const std::string& message, 
                                                             const std::vector<double>& data) {
    AllegroZmqResponse response(RESP_SUCCESS, true);
    response.message = message;
    response.data = data;
    
    // Populate with current hand state
    populateHandState(response);
    
    return response;
}

// Populate response with current hand state from BHand
bool AllegroZmqJsonParser::populateHandState(AllegroZmqResponse& response) {
    if (!pBHand) return false;
    
    try {
        // Get hand type
        response.hand_type = static_cast<int>(pBHand->GetType());
        
        // Get time interval
        response.time_interval = pBHand->GetTimeInterval();
        
        // Get current joint positions (copy from internal state)
        for (size_t i = 0; i < currentQ.size() && i < 16; i++) {
            response.qpos_measured[i] = currentQ[i];
        }
        
        // Get desired joint positions
        for (size_t i = 0; i < desiredQ.size() && i < 16; i++) {
            response.qpos_commanded[i] = desiredQ[i];
        }
        
        // Get joint torques
        double tau[16];
        pBHand->GetJointTorque(tau);
        for (int i = 0; i < 16; i++) {
            response.tau_commanded[i] = tau[i];
        }
        
        // Get fingertip positions
        double x[4], y[4], z[4];
        pBHand->GetFKResult(x, y, z);
        for (int i = 0; i < 4; i++) {
            response.fingertip_x[i] = x[i];
            response.fingertip_y[i] = y[i];
            response.fingertip_z[i] = z[i];
        }
        
        // Get grasping forces
        double fx[4], fy[4], fz[4];
        pBHand->GetGraspingForce(fx, fy, fz);
        for (int i = 0; i < 4; i++) {
            response.grasp_force_x[i] = fx[i];
            response.grasp_force_y[i] = fy[i];
            response.grasp_force_z[i] = fz[i];
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

// Convert response to JSON string
std::string AllegroZmqJsonParser::responseToJson(const AllegroZmqResponse& response) {
    nlohmann::json j;
    
    try {
        // Response metadata
        j["type"] = static_cast<int>(response.type);
        j["success"] = response.success;
        j["message"] = response.message;
        
        // Hand configuration
        j["hand_type"] = response.hand_type;
        j["time_interval"] = response.time_interval;
        j["motion_type"] = response.motion_type;
        
        // Joint data
        j["qpos_measured"] = response.qpos_measured;
        j["qpos_commanded"] = response.qpos_commanded;
        j["tau_commanded"] = response.tau_commanded;
        
        // Fingertip positions
        j["fingertip_positions"] = {
            {response.fingertip_x[0], response.fingertip_y[0], response.fingertip_z[0]},
            {response.fingertip_x[1], response.fingertip_y[1], response.fingertip_z[1]},
            {response.fingertip_x[2], response.fingertip_y[2], response.fingertip_z[2]},
            {response.fingertip_x[3], response.fingertip_y[3], response.fingertip_z[3]}
        };
        
        // Grasping forces
        j["grasping_forces"] = {
            {response.grasp_force_x[0], response.grasp_force_y[0], response.grasp_force_z[0]},
            {response.grasp_force_x[1], response.grasp_force_y[1], response.grasp_force_z[1]},
            {response.grasp_force_x[2], response.grasp_force_y[2], response.grasp_force_z[2]},
            {response.grasp_force_x[3], response.grasp_force_y[3], response.grasp_force_z[3]}
        };
        
        // Additional data if present
        if (!response.data.empty()) {
            j["data"] = response.data;
        }
        
        return j.dump();
    } catch (const std::exception& e) {
        // Return error JSON if serialization fails
        nlohmann::json error_j;
        error_j["type"] = static_cast<int>(RESP_ERROR);
        error_j["success"] = false;
        error_j["message"] = "JSON serialization error: " + std::string(e.what());
        return error_j.dump();
    }
}
