#ifndef SIMPLE_TEST_FRAMEWORK_H
#define SIMPLE_TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

// Simple test framework without external dependencies
class SimpleTestFramework {
private:
    static int total_tests;
    static int passed_tests;
    static std::vector<std::string> failed_tests;

public:
    static void reset() {
        total_tests = 0;
        passed_tests = 0;
        failed_tests.clear();
    }

    static void assert_true(bool condition, const std::string& test_name, const std::string& message = "") {
        total_tests++;
        if (condition) {
            passed_tests++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            failed_tests.push_back(test_name + ": " + message);
            std::cout << "[FAIL] " << test_name << " - " << message << std::endl;
        }
    }

    static void assert_false(bool condition, const std::string& test_name, const std::string& message = "") {
        assert_true(!condition, test_name, message);
    }

    static void assert_equals(double expected, double actual, const std::string& test_name, double tolerance = 1e-6) {
        bool equal = std::abs(expected - actual) < tolerance;
        std::string message = "Expected: " + std::to_string(expected) + ", Got: " + std::to_string(actual);
        assert_true(equal, test_name, message);
    }

    static void assert_equals(int expected, int actual, const std::string& test_name) {
        bool equal = (expected == actual);
        std::string message = "Expected: " + std::to_string(expected) + ", Got: " + std::to_string(actual);
        assert_true(equal, test_name, message);
    }

    static void assert_equals(const std::string& expected, const std::string& actual, const std::string& test_name) {
        bool equal = (expected == actual);
        std::string message = "Expected: '" + expected + "', Got: '" + actual + "'";
        assert_true(equal, test_name, message);
    }

    static void assert_vector_equals(const std::vector<double>& expected, const std::vector<double>& actual, 
                                   const std::string& test_name, double tolerance = 1e-6) {
        if (expected.size() != actual.size()) {
            std::string message = "Vector sizes differ. Expected: " + std::to_string(expected.size()) + 
                                ", Got: " + std::to_string(actual.size());
            assert_true(false, test_name, message);
            return;
        }

        for (size_t i = 0; i < expected.size(); i++) {
            if (std::abs(expected[i] - actual[i]) > tolerance) {
                std::string message = "Vectors differ at index " + std::to_string(i) + 
                                    ". Expected: " + std::to_string(expected[i]) + 
                                    ", Got: " + std::to_string(actual[i]);
                assert_true(false, test_name, message);
                return;
            }
        }
        assert_true(true, test_name);
    }

    static void run_test(const std::string& test_name, std::function<void()> test_func) {
        std::cout << "\n--- Running " << test_name << " ---" << std::endl;
        try {
            test_func();
        } catch (const std::exception& e) {
            failed_tests.push_back(test_name + ": Exception: " + e.what());
            std::cout << "[FAIL] " << test_name << " - Exception: " << e.what() << std::endl;
            total_tests++;
        }
    }

    static void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total tests: " << total_tests << std::endl;
        std::cout << "Passed: " << passed_tests << std::endl;
        std::cout << "Failed: " << (total_tests - passed_tests) << std::endl;
        
        if (!failed_tests.empty()) {
            std::cout << "\nFailed tests:" << std::endl;
            for (const auto& failure : failed_tests) {
                std::cout << "  - " << failure << std::endl;
            }
        }
        
        if (passed_tests == total_tests) {
            std::cout << "\nAll tests passed! ✓" << std::endl;
        } else {
            std::cout << "\nSome tests failed! ✗" << std::endl;
        }
    }

    static bool all_passed() {
        return passed_tests == total_tests;
    }
};

// Static member definitions
int SimpleTestFramework::total_tests = 0;
int SimpleTestFramework::passed_tests = 0;
std::vector<std::string> SimpleTestFramework::failed_tests;

// Convenience macros
#define ASSERT_TRUE(condition, message) SimpleTestFramework::assert_true(condition, __FUNCTION__, message)
#define ASSERT_FALSE(condition, message) SimpleTestFramework::assert_false(condition, __FUNCTION__, message)
#define ASSERT_EQ(expected, actual) SimpleTestFramework::assert_equals(expected, actual, __FUNCTION__)
#define ASSERT_VEC_EQ(expected, actual) SimpleTestFramework::assert_vector_equals(expected, actual, __FUNCTION__)
#define RUN_TEST(test_func) SimpleTestFramework::run_test(#test_func, test_func)

#endif // SIMPLE_TEST_FRAMEWORK_H