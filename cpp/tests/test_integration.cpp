#include "simple_test_framework.h"
#include "mock_bhand.h"
#include "../include/allegroZmqJsonParser.h"
#include <nlohmann/json.hpp>

class IntegrationTests {
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

    // Test full workflow: Home -> Ready -> PD Control
    void testFullWorkflow() {
        // Step 1: Set to HOME position
        nlohmann::json homeCmd = {
            {"motion_type", 1},  // HOME
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        AllegroZmqResponse response = parser->parseJsonAndExecute(homeCmd.dump());
        ASSERT_TRUE(response.success, "HOME command should succeed");
        ASSERT_EQ(1, mockBHand->GetMotionType());

        // Step 2: Set to READY position
        nlohmann::json readyCmd = {
            {"motion_type", 2},  // READY
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(readyCmd.dump());
        ASSERT_TRUE(response.success, "READY command should succeed");
        ASSERT_EQ(2, mockBHand->GetMotionType());

        // Step 3: Switch to PD control with specific joint angles
        std::vector<double> targetAngles(16);
        for (int i = 0; i < 16; i++) {
            targetAngles[i] = sin(i * 0.1) * 0.5;  // Some realistic joint angles
        }

        nlohmann::json pdCmd = {
            {"motion_type", 11},  // JOINT_PD
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", targetAngles},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(pdCmd.dump());
        ASSERT_TRUE(response.success, "PD control command should succeed");
        ASSERT_EQ(11, mockBHand->GetMotionType());

        // Verify desired positions were set correctly
        const auto& setDesired = mockBHand->GetDesiredPositions();
        ASSERT_VEC_EQ(targetAngles, setDesired);
    }

    // Test grasping workflow
    void testGraspingWorkflow() {
        // Step 1: Move to ready position
        nlohmann::json readyCmd = {
            {"motion_type", 2},  // READY
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        AllegroZmqResponse response = parser->parseJsonAndExecute(readyCmd.dump());
        ASSERT_TRUE(response.success, "READY command should succeed");

        // Step 2: Perform 3-finger grasp with specific forces
        std::vector<double> graspForces = {5.0, 7.0, 6.0, 0.0};  // No force on thumb for 3-finger grasp

        nlohmann::json graspCmd = {
            {"motion_type", 5},  // GRASP_3
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", graspForces},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(graspCmd.dump());
        ASSERT_TRUE(response.success, "3-finger grasp should succeed");
        ASSERT_EQ(5, mockBHand->GetMotionType());

        // Verify grasping forces were set
        const auto& setForces = mockBHand->GetGraspingForces();
        ASSERT_VEC_EQ(graspForces, setForces);

        // Step 3: Switch to 4-finger grasp
        graspForces[3] = 4.0;  // Add thumb force

        nlohmann::json grasp4Cmd = {
            {"motion_type", 6},  // GRASP_4
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", graspForces},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(grasp4Cmd.dump());
        ASSERT_TRUE(response.success, "4-finger grasp should succeed");
        ASSERT_EQ(6, mockBHand->GetMotionType());

        const auto& setForces4 = mockBHand->GetGraspingForces();
        ASSERT_VEC_EQ(graspForces, setForces4);
    }

    // Test mixed JSON and legacy commands
    void testMixedCommandFormats() {
        // Start with JSON command
        nlohmann::json jsonCmd = {
            {"motion_type", 1},  // HOME
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        AllegroZmqResponse response = parser->parseJsonAndExecute(jsonCmd.dump());
        ASSERT_TRUE(response.success, "JSON command should succeed");

        // Follow with legacy command via the underlying parser
        AllegroZmqCommandParser& cmdParser = parser->getCommandParser();
        response = cmdParser.parseAndExecute("1:2");  // Set motion type to READY
        ASSERT_TRUE(response.success, "Legacy command should succeed");
        ASSERT_EQ(2, mockBHand->GetMotionType());

        // Return to JSON command
        nlohmann::json pdCmd = {
            {"motion_type", 11},  // JOINT_PD
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.1)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(pdCmd.dump());
        ASSERT_TRUE(response.success, "JSON command after legacy should succeed");
        ASSERT_EQ(11, mockBHand->GetMotionType());
    }

    // Test error recovery
    void testErrorRecovery() {
        // Send invalid JSON
        std::string invalidJson = "{invalid json";
        AllegroZmqResponse response = parser->parseJsonAndExecute(invalidJson);
        ASSERT_FALSE(response.success, "Invalid JSON should fail");

        // System should still work after error
        nlohmann::json validCmd = {
            {"motion_type", 1},  // HOME
            {"joint_positions", std::vector<double>(16, 0.0)},
            {"desired_positions", std::vector<double>(16, 0.0)},
            {"grasping_forces", std::vector<double>(4, 0.0)},
            {"time_interval", 0.003}
        };

        response = parser->parseJsonAndExecute(validCmd.dump());
        ASSERT_TRUE(response.success, "Valid command after error should succeed");
        ASSERT_EQ(1, mockBHand->GetMotionType());
    }

    // Test performance with multiple rapid commands
    void testRapidCommands() {
        const int numCommands = 100;
        int successCount = 0;

        for (int i = 0; i < numCommands; i++) {
            // Alternate between different motion types
            int motionType = (i % 3) + 1;  // HOME, READY, GRAVITY_COMP

            nlohmann::json cmd = {
                {"motion_type", motionType},
                {"joint_positions", std::vector<double>(16, 0.0)},
                {"desired_positions", std::vector<double>(16, i * 0.001)},
                {"grasping_forces", std::vector<double>(4, 0.0)},
                {"time_interval", 0.003}
            };

            AllegroZmqResponse response = parser->parseJsonAndExecute(cmd.dump());
            if (response.success) {
                successCount++;
            }
        }

        ASSERT_EQ(numCommands, successCount);
        ASSERT_TRUE(mockBHand->GetMotionType() >= 1 && mockBHand->GetMotionType() <= 3, 
                   "Final motion type should be valid");
    }

    // Test complex JSON with all fields
    void testComplexJsonStructure() {
        // Create a comprehensive command with all possible fields
        std::vector<std::vector<double>> fingertips = {
            {0.1, 0.2, 0.3},
            {0.4, 0.5, 0.6},
            {0.7, 0.8, 0.9},
            {1.0, 1.1, 1.2}
        };

        std::vector<double> objectDisplacement = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06};

        nlohmann::json complexCmd = {
            {"motion_type", 11},  // JOINT_PD
            {"joint_positions", std::vector<double>(16, 0.05)},
            {"desired_positions", std::vector<double>(16, 0.1)},
            {"grasping_forces", std::vector<double>{2.0, 3.0, 4.0, 5.0}},
            {"fingertip_positions", fingertips},
            {"object_displacement", objectDisplacement},
            {"time_interval", 0.005}
        };

        AllegroZmqResponse response = parser->parseJsonAndExecute(complexCmd.dump());
        ASSERT_TRUE(response.success, "Complex JSON command should succeed");
        ASSERT_EQ(11, mockBHand->GetMotionType());
        ASSERT_EQ(0.005, mockBHand->GetTimeInterval());

        // Verify arrays were parsed correctly
        const auto& setDesired = mockBHand->GetDesiredPositions();
        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(0.1, setDesired[i]);
        }

        const auto& setForces = mockBHand->GetGraspingForces();
        std::vector<double> expectedForces = {2.0, 3.0, 4.0, 5.0};
        ASSERT_VEC_EQ(expectedForces, setForces);
    }

    // Run all integration tests
    void runAllTests() {
        setUp();
        RUN_TEST([this]() { testFullWorkflow(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testGraspingWorkflow(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testMixedCommandFormats(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testErrorRecovery(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testRapidCommands(); });
        tearDown(); setUp();
        
        RUN_TEST([this]() { testComplexJsonStructure(); });
        tearDown();
    }
};

int main() {
    std::cout << "=== Allegro JSON Parser Integration Tests ===" << std::endl;
    
    SimpleTestFramework::reset();
    
    IntegrationTests tests;
    tests.runAllTests();
    
    SimpleTestFramework::print_summary();
    
    return SimpleTestFramework::all_passed() ? 0 : 1;
}