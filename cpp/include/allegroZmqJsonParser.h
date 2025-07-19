#ifndef ALLEGRO_ZMQ_JSON_PARSER_H
#define ALLEGRO_ZMQ_JSON_PARSER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <BHand/BHand.h>
#include "rDeviceAllegroHandCANDef.h"

/**
 * @brief Response types for ZMQ communication
 */
enum AllegroZmqResponseType {
    RESP_SUCCESS = 0,
    RESP_ERROR,
    RESP_DATA,
    RESP_STATUS
};

/**
 * @brief Structure for ZMQ response
 */
struct AllegroZmqResponse {
    AllegroZmqResponseType type;
    bool success;
    std::string message;
    
    // Hand state data
    std::vector<double> qpos_measured;     // Current joint positions (16)
    std::vector<double> tau_commanded;      // Computed joint torques (16)
    std::vector<double> qpos_commanded;    // Desired joint positions (16)
    
    // Fingertip positions (4 fingers x 3 coordinates)
    std::vector<double> fingertip_x;           // X coordinates of fingertips (4)
    std::vector<double> fingertip_y;           // Y coordinates of fingertips (4)
    std::vector<double> fingertip_z;           // Z coordinates of fingertips (4)
    
    // Grasping forces (4 fingers x 3 directions)
    std::vector<double> grasp_force_x;         // X-direction forces (4)
    std::vector<double> grasp_force_y;         // Y-direction forces (4)
    std::vector<double> grasp_force_z;         // Z-direction forces (4)
    
    // Hand configuration
    int hand_type;                             // Left (0) or Right (1) hand
    double time_interval;                      // Control time interval
    int motion_type;                           // Current motion type
    
    // Additional data for custom responses
    std::vector<double> data;
    
    AllegroZmqResponse() : type(RESP_SUCCESS), success(true), hand_type(0), time_interval(0.003), motion_type(0) {
        qpos_measured.resize(16, 0.0);
        tau_commanded.resize(16, 0.0);
        qpos_commanded.resize(16, 0.0);
        fingertip_x.resize(4, 0.0);
        fingertip_y.resize(4, 0.0);
        fingertip_z.resize(4, 0.0);
        grasp_force_x.resize(4, 0.0);
        grasp_force_y.resize(4, 0.0);
        grasp_force_z.resize(4, 0.0);
    }
    
    AllegroZmqResponse(AllegroZmqResponseType t, bool s = true) : type(t), success(s), hand_type(0), time_interval(0.003), motion_type(0) {
        qpos_measured.resize(16, 0.0);
        tau_commanded.resize(16, 0.0);
        qpos_commanded.resize(16, 0.0);
        fingertip_x.resize(4, 0.0);
        fingertip_y.resize(4, 0.0);
        fingertip_z.resize(4, 0.0);
        grasp_force_x.resize(4, 0.0);
        grasp_force_y.resize(4, 0.0);
        grasp_force_z.resize(4, 0.0);
    }
};

/**
 * @brief Structure representing an AllegroCommand from Python
 */
struct AllegroJsonCommand {
    int motion_type;
    std::vector<double> joint_positions;
    std::vector<double> desired_positions;
    std::vector<double> grasping_forces;
    std::vector<std::vector<double>> fingertip_positions;  // 4 x 3 matrix
    std::vector<double> object_displacement;
    double time_interval;
    std::vector<double> kp_gains;  // Proportional gains (16 elements)
    std::vector<double> kd_gains;  // Derivative gains (16 elements)
    
    // Default constructor
    AllegroJsonCommand() 
        : motion_type(0)
        , joint_positions(16, 0.0)
        , desired_positions(16, 0.0)
        , grasping_forces(4, 0.0)
        , time_interval(0.003)
        , kp_gains(16, 0.0)
        , kd_gains(16, 0.0) {}
};

/**
 * @brief JSON parser for Allegro Hand commands
 */
class AllegroZmqJsonParser {
public:
    /**
     * @brief Constructor
     * @param pBHand Pointer to BHand instance
     */
    AllegroZmqJsonParser(BHand* pBHand);
    double* GetDesiredQ() { return desiredQ.data(); }
    /**
     * @brief Destructor
     */
    ~AllegroZmqJsonParser();
    
    /**
     * @brief Parse JSON command string and execute it
     * @param jsonStr JSON command string from Python AllegroCommand
     * @return Response object
     */
    AllegroZmqResponse parseJsonAndExecute(const std::string& jsonStr);
    void UpdateControl(double* current_q, double* result_torques);
    
    /**
     * @brief Convert response to JSON string
     * @param response Response object
     * @return JSON string representation
     */
    std::string responseToJson(const AllegroZmqResponse& response);

private:
    BHand* pBHand;                    ///< Pointer to BHand instance
    double currentTime;               ///< Current control time
    std::vector<double> currentQ;     ///< Current joint positions
    std::vector<double> desiredQ;     ///< Desired joint positions
    std::vector<double> currentTau;   ///< Current joint torques
    
    /**
     * @brief Parse JSON string into AllegroJsonCommand
     * @param jsonStr JSON string
     * @return Parsed command structure
     * @throws std::exception on parse error
     */
    AllegroJsonCommand parseJsonCommand(const std::string& jsonStr);
    
    /**
     * @brief Validate parsed JSON command
     * @param cmd Command to validate
     * @return True if valid
     */
    bool validateJsonCommand(const AllegroJsonCommand& cmd);
    
    /**
     * @brief Convert numpy array JSON to std::vector<double>
     * @param jsonArray JSON array
     * @param expectedSize Expected array size (-1 for any size)
     * @return Vector of doubles
     */
    std::vector<double> jsonArrayToVector(const nlohmann::json& jsonArray, int expectedSize = -1);
    
    /**
     * @brief Convert 2D numpy array JSON to vector of vectors
     * @param jsonArray 2D JSON array
     * @return Vector of vectors
     */
    std::vector<std::vector<double>> json2DArrayToVector(const nlohmann::json& jsonArray);
    
    /**
     * @brief Set motion type
     * @param motionType Motion type ID
     * @return Success status
     */
    bool setMotionType(int motionType);
    
    /**
     * @brief Set desired joint positions for PD control
     * @param positions Vector of 16 desired joint positions
     * @return Success status
     */
    bool setDesiredJointPositions(const std::vector<double>& positions);
    
    /**
     * @brief Set grasping forces
     * @param forces Vector of 4 finger forces
     * @return Success status
     */
    bool setGraspingForces(const std::vector<double>& forces);
    
    /**
     * @brief Set PID gains
     * @param kp Proportional gains (16 elements)
     * @param kd Derivative gains (16 elements)
     * @return Success status
     */
    bool setGains(const std::vector<double>& kp, const std::vector<double>& kd);
    
    /**
     * @brief Update control algorithm
     * @param time Current time
     * @return Success status
     */
    bool updateControl(double time);
    
    /**
     * @brief Validate joint positions array
     * @param positions Vector to validate
     * @return True if valid (16 elements, reasonable values)
     */
    bool validateJointPositions(const std::vector<double>& positions);
    
    /**
     * @brief Validate finger forces array
     * @param forces Vector to validate
     * @return True if valid (4 elements, reasonable values)
     */
    bool validateFingerForces(const std::vector<double>& forces);
    
    /**
     * @brief Validate gains arrays
     * @param kp Proportional gains to validate
     * @param kd Derivative gains to validate
     * @return True if valid (16 elements each, reasonable values)
     */
    bool validateGains(const std::vector<double>& kp, const std::vector<double>& kd);
    
    /**
     * @brief Create error response
     * @param message Error message
     * @return Error response object
     */
    AllegroZmqResponse createErrorResponse(const std::string& message);
    
    /**
     * @brief Create success response with full hand state
     * @param message Success message
     * @param data Optional data
     * @return Success response object with all hand states populated
     */
    AllegroZmqResponse createSuccessResponse(const std::string& message, 
                                           const std::vector<double>& data = std::vector<double>());
    
    /**
     * @brief Populate response with current hand state from BHand
     * @param response Response object to populate
     * @return True if successful
     */
    bool populateHandState(AllegroZmqResponse& response);
    
    /**
     * @brief Check if BHand is initialized
     * @return True if BHand is available
     */
    bool isHandReady() const { return pBHand != nullptr; }
};

#endif // ALLEGRO_ZMQ_JSON_PARSER_H