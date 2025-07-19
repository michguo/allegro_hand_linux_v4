# Allegro Hand ZMQ JSON Parser Unit Tests

This directory contains comprehensive unit tests for the Allegro Hand ZMQ JSON parsing functionality.

## Overview

The test suite validates the JSON parsing capabilities that allow Python AllegroCommand objects to be processed by the C++ ZMQ server. The tests cover:

- JSON parsing and validation
- AllegroCommand structure conversion
- Error handling and edge cases
- Motion type validation
- Joint position and force validation
- Integration workflows

## Test Structure

### Files

- `simple_test_framework.h` - Lightweight testing framework (no external dependencies)
- `mock_bhand.h` - Mock BHand class for testing without hardware
- `test_simple_json_functions.cpp` - Core JSON function tests (currently working)
- `test_allegro_json_parser.cpp` - JSON parser class tests (comprehensive)
- `test_allegro_command_parser.cpp` - Command parser validation tests
- `test_integration.cpp` - End-to-end integration tests
- `CMakeLists.txt` - Build configuration for tests
- `build_tests.sh` - Build script for all tests

### Test Categories

#### 1. JSON Function Tests (`test_simple_json_functions.cpp`)
**Status: ✅ PASSING (42/42 tests)**

Tests core JSON parsing functions:
- Basic JSON structure parsing
- Array conversion (`jsonArrayToVector`)
- Motion type validation (0-13)
- Joint position validation (16 elements, reasonable ranges)
- Grasping force validation (4 elements, force limits)
- Invalid JSON handling
- Complete workflow validation

**Coverage:**
- ✅ Valid JSON parsing
- ✅ Invalid JSON error handling  
- ✅ Array size validation
- ✅ NaN/Infinity detection
- ✅ Motion type range checking
- ✅ Joint angle limits
- ✅ Force limits

#### 2. JSON Parser Class Tests (`test_allegro_json_parser.cpp`)
**Status: 📝 WRITTEN (requires AllegroZmqCommandParser)**

Comprehensive tests for the full JSON parser:
- JSON command execution
- Motion type handling (HOME, READY, GRASP_3, GRASP_4, JOINT_PD)
- Time interval setting
- Error response generation
- Legacy format conversion
- 2D array parsing (fingertip positions)

#### 3. Command Parser Validation (`test_allegro_command_parser.cpp`)  
**Status: 📝 WRITTEN (requires AllegroZmqCommandParser)**

Tests for command validation and execution:
- Motion type setting and validation
- Joint position setting/getting
- Control gains configuration
- Grasping force commands
- Command string parsing
- Response formatting

#### 4. Integration Tests (`test_integration.cpp`)
**Status: 📝 WRITTEN (requires full implementation)**

End-to-end workflow tests:
- Full control sequences (Home → Ready → PD Control)
- Grasping workflows (Ready → Grasp)
- Mixed JSON/legacy command formats
- Error recovery
- Performance testing (rapid commands)
- Complex JSON structures

## Building and Running Tests

### Quick Start
```bash
# Build and run all available tests
./build_tests.sh
```

### Manual Build
```bash
mkdir build_tests
cd build_tests
cmake ../cpp/tests
make
make run_tests
```

### Individual Test Execution
```bash
cd build_tests
./test_simple_json        # Core JSON functions (working)
./test_json_parser         # Full JSON parser (when available)
./test_command_parser      # Command validation (when available)
./test_integration         # Integration tests (when available)
```

## Test Results

### Current Status (Working Tests)
```
=== Simple JSON Functions Unit Tests ===
Total tests: 42
Passed: 42
Failed: 0
All tests passed! ✓
```

### Test Coverage Summary

| Component | Tests Written | Tests Passing | Coverage |
|-----------|---------------|---------------|----------|
| JSON Functions | ✅ 42 | ✅ 42 | 100% |
| JSON Parser | ✅ 10 | ⏳ Pending | N/A |
| Command Parser | ✅ 14 | ⏳ Pending | N/A |
| Integration | ✅ 6 | ⏳ Pending | N/A |
| **Total** | **72** | **42** | **58%** |

## Tested Scenarios

### ✅ Currently Tested
- JSON structure parsing with all AllegroCommand fields
- Motion type validation (0-13 range)
- Joint position arrays (16 elements, angle limits)
- Grasping force arrays (4 elements, force limits)
- Time interval validation
- NaN/Infinity detection
- Array size validation
- Invalid JSON error handling
- Null field handling
- Array conversion with padding/truncation

### 📝 Tests Written (Pending Integration)
- Complete AllegroCommand JSON execution
- Motion type switching (HOME, READY, PD, GRASP modes)
- Force-based grasping commands
- Legacy format conversion
- Error response generation
- Command string parsing
- Rapid command sequences
- Multi-step control workflows

## Example Test JSON

The tests validate parsing of complete AllegroCommand JSON:

```json
{
  "motion_type": 11,
  "joint_positions": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
  "desired_positions": [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1],
  "grasping_forces": [5.0, 6.0, 7.0, 8.0],
  "fingertip_positions": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0], [10.0, 11.0, 12.0]],
  "object_displacement": [0.01, 0.02, 0.03, 0.04, 0.05, 0.06],
  "time_interval": 0.005
}
```

## Dependencies

- **nlohmann/json** - JSON parsing library
- **C++11** - Standard requirement
- **CMake 3.10+** - Build system
- **Threads** - For test framework

## Future Enhancements

1. **Complete Test Integration** - Connect remaining tests to full implementation
2. **Performance Benchmarks** - Add timing measurements for JSON parsing
3. **Memory Leak Detection** - Add memory validation
4. **Fuzzing Tests** - Random JSON generation for robustness
5. **Hardware-in-Loop** - Optional real hardware testing

## Contributing

When adding new tests:

1. Use the `SimpleTestFramework` for consistency
2. Add tests to appropriate category file
3. Update CMakeLists.txt if adding new test files
4. Ensure tests are isolated and don't depend on external state
5. Add documentation for complex test scenarios