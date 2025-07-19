#include "simple_test_framework.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <limits>

// Simple JSON validation functions to test
bool validateMotionType(int motionType) {
    return motionType >= 0 && motionType < 14;
}

bool validateJointPositions(const std::vector<double>& positions) {
    if (positions.size() != 16) return false;
    
    for (double pos : positions) {
        if (std::isnan(pos) || std::isinf(pos) || std::abs(pos) > 6.28) {
            return false;
        }
    }
    return true;
}

bool validateGraspingForces(const std::vector<double>& forces) {
    if (forces.size() != 4) return false;
    
    for (double force : forces) {
        if (std::isnan(force) || std::isinf(force) || std::abs(force) > 100.0) {
            return false;
        }
    }
    return true;
}

std::vector<double> jsonArrayToVector(const nlohmann::json& jsonArray, int expectedSize = -1) {
    std::vector<double> result;
    
    if (jsonArray.is_null()) {
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
            result.push_back(0.0);
        }
    }
    
    if (expectedSize > 0 && result.size() != static_cast<size_t>(expectedSize)) {
        result.resize(expectedSize, 0.0);
    }
    
    return result;
}

struct SimpleAllegroCommand {
    int motion_type = 0;
    std::vector<double> joint_positions;
    std::vector<double> desired_positions;
    std::vector<double> grasping_forces;
    double time_interval = 0.003;
    
    SimpleAllegroCommand() {
        joint_positions.resize(16, 0.0);
        desired_positions.resize(16, 0.0);
        grasping_forces.resize(4, 0.0);
    }
};

SimpleAllegroCommand parseJsonCommand(const std::string& jsonStr) {
    SimpleAllegroCommand cmd;
    
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    
    if (j.contains("motion_type")) {
        cmd.motion_type = j["motion_type"].get<int>();
    }
    
    if (j.contains("joint_positions")) {
        cmd.joint_positions = jsonArrayToVector(j["joint_positions"], 16);
    }
    
    if (j.contains("desired_positions")) {
        cmd.desired_positions = jsonArrayToVector(j["desired_positions"], 16);
    }
    
    if (j.contains("grasping_forces")) {
        cmd.grasping_forces = jsonArrayToVector(j["grasping_forces"], 4);
    }
    
    if (j.contains("time_interval")) {
        cmd.time_interval = j["time_interval"].get<double>();
    }
    
    return cmd;
}

bool validateCommand(const SimpleAllegroCommand& cmd) {
    return validateMotionType(cmd.motion_type) &&
           validateJointPositions(cmd.joint_positions) &&
           validateJointPositions(cmd.desired_positions) &&
           validateGraspingForces(cmd.grasping_forces) &&
           cmd.time_interval >= 0 && !std::isnan(cmd.time_interval);
}

class SimpleJsonTests {
public:
    void testBasicJsonParsing() {
        std::string jsonStr = R"({
            "motion_type": 11,
            "joint_positions": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6],
            "desired_positions": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            "grasping_forces": [1.0, 2.0, 3.0, 4.0],
            "time_interval": 0.003
        })";
        
        SimpleAllegroCommand cmd = parseJsonCommand(jsonStr);
        
        ASSERT_EQ(11, cmd.motion_type);
        ASSERT_EQ(16, cmd.joint_positions.size());
        ASSERT_EQ(16, cmd.desired_positions.size());
        ASSERT_EQ(4, cmd.grasping_forces.size());
        ASSERT_EQ(0.003, cmd.time_interval);
        
        // Check first few joint positions
        ASSERT_EQ(0.1, cmd.joint_positions[0]);
        ASSERT_EQ(0.2, cmd.joint_positions[1]);
        ASSERT_EQ(1.6, cmd.joint_positions[15]);
        
        // Check grasping forces
        ASSERT_EQ(1.0, cmd.grasping_forces[0]);
        ASSERT_EQ(4.0, cmd.grasping_forces[3]);
    }
    
    void testValidationFunctions() {
        // Test motion type validation
        ASSERT_TRUE(validateMotionType(0), "Motion type 0 should be valid");
        ASSERT_TRUE(validateMotionType(13), "Motion type 13 should be valid");
        ASSERT_FALSE(validateMotionType(-1), "Negative motion type should be invalid");
        ASSERT_FALSE(validateMotionType(14), "Motion type 14 should be invalid");
        
        // Test joint position validation
        std::vector<double> validPositions(16, 1.0);
        ASSERT_TRUE(validateJointPositions(validPositions), "Valid joint positions should pass");
        
        std::vector<double> wrongSize(10, 1.0);
        ASSERT_FALSE(validateJointPositions(wrongSize), "Wrong size should fail");
        
        std::vector<double> nanPositions(16, 1.0);
        nanPositions[5] = std::numeric_limits<double>::quiet_NaN();
        ASSERT_FALSE(validateJointPositions(nanPositions), "NaN positions should fail");
        
        // Test grasping force validation
        std::vector<double> validForces = {10.0, 15.0, 20.0, 25.0};
        ASSERT_TRUE(validateGraspingForces(validForces), "Valid forces should pass");
        
        std::vector<double> tooLargeForces = {150.0, 200.0, 300.0, 250.0};
        ASSERT_FALSE(validateGraspingForces(tooLargeForces), "Too large forces should fail");
    }
    
    void testJsonArrayConversion() {
        // Test normal array
        nlohmann::json normalArray = {1.0, 2.0, 3.0, 4.0};
        std::vector<double> result = jsonArrayToVector(normalArray);
        std::vector<double> expected = {1.0, 2.0, 3.0, 4.0};
        ASSERT_VEC_EQ(expected, result);
        
        // Test array with expected size
        result = jsonArrayToVector(normalArray, 6);
        std::vector<double> expectedPadded = {1.0, 2.0, 3.0, 4.0, 0.0, 0.0};
        ASSERT_VEC_EQ(expectedPadded, result);
        
        // Test null array
        nlohmann::json nullArray = nullptr;
        result = jsonArrayToVector(nullArray, 4);
        std::vector<double> expectedZeros = {0.0, 0.0, 0.0, 0.0};
        ASSERT_VEC_EQ(expectedZeros, result);
    }
    
    void testCompleteWorkflow() {
        // Create a complete JSON command
        std::string jsonStr = R"({
            "motion_type": 5,
            "joint_positions": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            "desired_positions": [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1],
            "grasping_forces": [5.0, 6.0, 7.0, 8.0],
            "time_interval": 0.005
        })";
        
        // Parse the command
        SimpleAllegroCommand cmd = parseJsonCommand(jsonStr);
        
        // Validate the command
        bool isValid = validateCommand(cmd);
        ASSERT_TRUE(isValid, "Complete command should be valid");
        
        // Check specific values
        ASSERT_EQ(5, cmd.motion_type);
        ASSERT_EQ(0.005, cmd.time_interval);
        
        // Check all desired positions are 0.1
        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(0.1, cmd.desired_positions[i]);
        }
    }
    
    void testInvalidJson() {
        bool exceptionThrown = false;
        try {
            parseJsonCommand("{invalid json");
        } catch (const std::exception&) {
            exceptionThrown = true;
        }
        ASSERT_TRUE(exceptionThrown, "Invalid JSON should throw exception");
    }
    
    void runAllTests() {
        RUN_TEST([this]() { testBasicJsonParsing(); });
        RUN_TEST([this]() { testValidationFunctions(); });
        RUN_TEST([this]() { testJsonArrayConversion(); });
        RUN_TEST([this]() { testCompleteWorkflow(); });
        RUN_TEST([this]() { testInvalidJson(); });
    }
};

int main() {
    std::cout << "=== Simple JSON Functions Unit Tests ===" << std::endl;
    
    SimpleTestFramework::reset();
    
    SimpleJsonTests tests;
    tests.runAllTests();
    
    SimpleTestFramework::print_summary();
    
    return SimpleTestFramework::all_passed() ? 0 : 1;
}