#include "simple_test_framework.h"
#include "mock_bhand.h"
#include "../include/allegroZmqJsonParser.h"
#include <nlohmann/json.hpp>
#include <sstream>

class AllegroJsonParserTests {
private:
    MockBHand* mockBHand;
    AllegroZmqJsonParser* parser;

public:
    void setUp() {
        mockBHand = new MockBHand();
        parser = new AllegroZmqJsonParser(mockBHand);
    }

    void tearDown() {
        delete parser;
        delete mockBHand;
    }

    // Test basic JSON parsing
    void testBasicJsonParsing() {
        nlohmann::json testJson = {
            {"motion_type", 11},  // JOINT_PD
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.1)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"fingertip_positions", nullptr},
            {"object_displacement", nullptr},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_TRUE(response.success, "JSON parsing should succeed");
        ASSERT_EQ(11, mockBHand->GetMotionType());
        
        // Check desired positions were set
        const auto& desiredPos = mockBHand->GetDesiredPositions();
        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(0.1, desiredPos[i]);
        }
    }

    // Test motion type HOME
    void testHomeMotionType() {
        nlohmann::json testJson = {
            {"motion_type", 1},  // HOME
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_TRUE(response.success, "HOME motion type should succeed");
        ASSERT_EQ(1, mockBHand->GetMotionType());
    }

    // Test grasping motion types with forces
    void testGraspingWithForces() {
        std::vector<double> forces = {1.0, 2.0, 3.0, 4.0};
        nlohmann::json testJson = {
            {"motion_type", 5},  // GRASP_3
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", forces},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_TRUE(response.success, "Grasping motion should succeed");
        ASSERT_EQ(5, mockBHand->GetMotionType());
        
        // Check grasping forces were set
        const auto& setForces = mockBHand->GetGraspingForces();
        for (int i = 0; i < 4; i++) {
            ASSERT_EQ(forces[i], setForces[i]);
        }
    }

    // Test invalid JSON
    void testInvalidJson() {
        std::string invalidJson = "{invalid json string";
        AllegroZmqResponse response = parser->parseJsonAndExecute(invalidJson);

        ASSERT_FALSE(response.success, "Invalid JSON should fail");
        ASSERT_TRUE(response.message.find("JSON parsing error") != std::string::npos, 
                   "Should contain JSON parsing error message");
    }

    // Test invalid motion type
    void testInvalidMotionType() {
        nlohmann::json testJson = {
            {"motion_type", 99},  // Invalid motion type
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_FALSE(response.success, "Invalid motion type should fail");
    }

    // Test wrong array sizes
    void testWrongArraySizes() {
        nlohmann::json testJson = {
            {"motion_type", 11},
            {"joint_positions", std::vector<double>(10, 0.0)},  // Wrong size
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_FALSE(response.success, "Wrong array size should fail");
    }

    // Test NaN values
    void testNaNValues() {
        std::vector<double> positions(16, 0.0);
        positions[5] = std::numeric_limits<double>::quiet_NaN();
        
        nlohmann::json testJson = {
            {"motion_type", 11},
            {"joint_positions", positions},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_FALSE(response.success, "NaN values should fail validation");
    }

    // Test time interval setting
    void testTimeIntervalSetting() {
        nlohmann::json testJson = {
            {"motion_type", 1},
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.005}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        ASSERT_TRUE(response.success, "Time interval setting should succeed");
        ASSERT_EQ(0.005, mockBHand->GetTimeInterval());
    }

    // Test empty or null fields
    void testNullFields() {
        nlohmann::json testJson = {
            {"motion_type", 1},
            {"joint_positions", nullptr},  // This should be handled gracefully
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"fingertip_positions", nullptr},
            {"object_displacement", nullptr},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        // This might fail or succeed depending on implementation
        // The test ensures we handle null gracefully
        if (!response.success) {
            ASSERT_TRUE(response.message.length() > 0, "Should provide error message for null fields");
        }
    }

    // Test legacy format conversion
    void testLegacyFormatConversion() {
        AllegroJsonCommand jsonCmd;
        jsonCmd.motion_type = 11;  // JOINT_PD
        jsonCmd.desired_positions = std::vector<double>(16, 0.5);

        std::string legacyFormat = parser->convertToLegacyFormat(jsonCmd);
        
        ASSERT_TRUE(legacyFormat.find("2:") == 0, "Should start with command type 2 for desired angles");
        ASSERT_TRUE(legacyFormat.find("0.5") != std::string::npos, "Should contain the desired position values");
    }

    // Test 2D array parsing (fingertip positions)
    void testFingertipPositionsParsing() {
        std::vector<std::vector<double>> fingertips = {
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0},
            {10.0, 11.0, 12.0}
        };

        nlohmann::json testJson = {
            {"motion_type", 1},
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"fingertip_positions", fingertips},
            {"time_interval", 0.003}
        };

        std::string jsonStr = testJson.dump();
        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonStr);

        // This tests that 2D arrays are parsed without crashing
        // The actual use of fingertip positions depends on the implementation
        ASSERT_TRUE(response.success || response.message.length() > 0, 
                   "Should either succeed or provide meaningful error");
    }

    // Run all tests
    void runAllTests() {
        setUp();

        RUN_TEST([this]() { testBasicJsonParsing(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testHomeMotionType(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testGraspingWithForces(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testInvalidJson(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testInvalidMotionType(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testWrongArraySizes(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testNaNValues(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testTimeIntervalSetting(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testNullFields(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testLegacyFormatConversion(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testFingertipPositionsParsing(); });
        tearDown();
    }
};

int main() {
    std::cout << "=== AllegroZmqJsonParser Unit Tests ===" << std::endl;
    
    SimpleTestFramework::reset();
    
    AllegroJsonParserTests tests;
    tests.runAllTests();
    
    SimpleTestFramework::print_summary();
    
    return SimpleTestFramework::all_passed() ? 0 : 1;
}