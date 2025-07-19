#ifndef MOCK_BHAND_H
#define MOCK_BHAND_H

#include <vector>
#include <cstring>

// Mock BHand class for testing
class MockBHand {
private:
    int current_motion_type = 0;
    std::vector<double> joint_positions;
    std::vector<double> desired_positions;
    std::vector<double> joint_torques;
    std::vector<double> grasping_forces;
    std::vector<double> kp_gains;
    std::vector<double> kd_gains;
    double time_interval = 0.003;

public:
    MockBHand() {
        joint_positions.resize(16, 0.0);
        desired_positions.resize(16, 0.0);
        joint_torques.resize(16, 0.0);
        grasping_forces.resize(4, 0.0);
        kp_gains.resize(16, 1.0);
        kd_gains.resize(16, 0.1);
    }

    // Mock BHand API methods
    void SetMotionType(int motionType) {
        current_motion_type = motionType;
    }

    void SetJointPosition(double* q) {
        for (int i = 0; i < 16; i++) {
            joint_positions[i] = q[i];
        }
    }

    void SetJointDesiredPosition(double* q_des) {
        for (int i = 0; i < 16; i++) {
            desired_positions[i] = q_des[i];
        }
    }

    void SetGainsEx(double* kp, double* kd) {
        for (int i = 0; i < 16; i++) {
            kp_gains[i] = kp[i];
            kd_gains[i] = kd[i];
        }
    }

    void SetGraspingForce(double* forces) {
        for (int i = 0; i < 4; i++) {
            grasping_forces[i] = forces[i];
        }
    }

    void SetTimeInterval(double dt) {
        time_interval = dt;
    }

    void GetJointTorque(double* tau) {
        for (int i = 0; i < 16; i++) {
            tau[i] = joint_torques[i];
        }
    }

    void UpdateControl(double time) {
        // Mock control update - could simulate some basic behavior
        for (int i = 0; i < 16; i++) {
            joint_torques[i] = kp_gains[i] * (desired_positions[i] - joint_positions[i]);
        }
    }

    // Test helper methods
    int GetMotionType() const { return current_motion_type; }
    const std::vector<double>& GetJointPositions() const { return joint_positions; }
    const std::vector<double>& GetDesiredPositions() const { return desired_positions; }
    const std::vector<double>& GetGraspingForces() const { return grasping_forces; }
    const std::vector<double>& GetKpGains() const { return kp_gains; }
    const std::vector<double>& GetKdGains() const { return kd_gains; }
    double GetTimeInterval() const { return time_interval; }
};

// Alias for compatibility with actual BHand
using BHand = MockBHand;

#endif // MOCK_BHAND_H