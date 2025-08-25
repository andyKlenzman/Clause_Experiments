# Embedded Test Framework for J-Link RTT

A comprehensive test framework designed for embedded development with J-Link RTT (Real Time Transfer) debugging and logging. This framework provides structured test execution, organized RTT logs, and automated test monitoring.

## Features

- 🔧 **Structured Test Flow**: Organized test cases with clear pass/fail criteria
- 📊 **RTT Logging**: Real-time transfer logs with different severity levels
- 🔍 **Automated Monitoring**: Python script monitors RTT until success conditions are met
- 📈 **Test Results**: JSON output with detailed test results and timing
- 🚀 **Easy Integration**: Simple C API for embedding in your projects
- 📝 **Detailed Reporting**: Comprehensive test summaries and logs

## Project Structure

```
embedded-test-framework/
├── src/                    # Source code
│   ├── test_rtt_logger.c  # RTT logging implementation
│   └── example_module.c   # Example module to test
├── include/               # Header files
│   ├── test_rtt_logger.h  # RTT logging API
│   └── example_module.h   # Example module API
├── tests/                 # Test cases
│   └── test_example_module.c  # Example test cases
├── scripts/               # Automation scripts
│   ├── rtt_monitor.py    # RTT monitoring script
│   └── run_tests.sh      # Test execution script
├── config/               # Build configuration
│   └── Makefile          # Build system
└── logs/                 # Test results and logs
```

## Quick Start

### Prerequisites

1. **J-Link Software Package**: Install from SEGGER website
   - JLinkExe (command line interface)
   - JLinkRTTClient (RTT viewer)

2. **ARM GCC Toolchain**: For cross-compilation
   ```bash
   # On Ubuntu/Debian
   sudo apt-get install gcc-arm-none-eabi
   
   # On macOS with Homebrew
   brew install arm-none-eabi-gcc
   ```

3. **Python 3**: For RTT monitoring script
   ```bash
   pip3 install dataclasses  # If using Python < 3.7
   ```

### Basic Usage

1. **Build the test framework**:
   ```bash
   cd embedded-test-framework
   make all TARGET_DEVICE=STM32F407VG
   ```

2. **Flash and run tests**:
   ```bash
   make test TARGET_DEVICE=STM32F407VG
   ```

3. **Monitor RTT logs only** (without flashing):
   ```bash
   make monitor TARGET_DEVICE=STM32F407VG
   ```

4. **Manual test execution**:
   ```bash
   # Flash firmware and monitor
   ./scripts/run_tests.sh -d STM32F407VG -f build/embedded_test_framework.hex
   
   # Monitor only (no flashing)
   ./scripts/run_tests.sh -d STM32F407VG -l
   ```

## RTT Logging API

### Basic Logging

```c
#include "test_rtt_logger.h"

// Initialize RTT logging
test_rtt_init();

// Log messages with different levels
TEST_LOG_ERROR("Critical error occurred");
TEST_LOG_WARN("Warning message");
TEST_LOG_INFO("Informational message"); 
TEST_LOG_DEBUG("Debug details");
```

### Test Status Reporting

```c
// Report test status
test_status(TEST_STATUS_RUNNING, "My Test Case");
test_status(TEST_STATUS_PASS, "My Test Case");

// Report test results with timing
test_result("My Test Case", true, 150);  // 150ms duration

// Assertions
TEST_ASSERT(condition, "Condition should be true");
```

## Writing Test Cases

### Example Test Function

```c
void test_my_function(void) {
    const char* test_name = "My Function Test";
    uint32_t start_time = get_timestamp_ms();
    
    // Signal test start
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    // Test implementation
    bool test_passed = true;
    
    int result = my_function(10, 20);
    if (result != 30) {
        TEST_LOG_ERROR("Expected 30, got %d", result);
        test_passed = false;
    }
    
    // Additional test assertions
    TEST_ASSERT(result > 0, "Result should be positive");
    
    // Report results
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, test_passed, duration);
}
```

### Test Runner Main Function

```c
int main(void) {
    // Initialize RTT logging
    test_rtt_init();
    
    TEST_LOG_INFO("=== Starting Test Suite ===");
    
    // Initialize your system
    system_init();
    
    // Run test cases
    test_my_function();
    test_another_function();
    
    // Print summary
    test_summary();
    
    TEST_LOG_INFO("=== Test Suite Complete ===");
    
    return 0;
}
```

## RTT Log Format

The framework uses structured RTT output for easy parsing:

### Log Messages
```
[12345] [INFO] System initialization complete
[12346] [ERROR] Critical error occurred
[12347] [DEBUG] Debug information
```

### Status Messages
```
STATUS:TEST_RUNNING:My Test Case
STATUS:TEST_PASS:My Test Case
STATUS:TEST_FAIL:My Test Case
STATUS:TEST_COMPLETE:All Tests
```

### Result Messages
```
RESULT:My Test Case:PASS:150
RESULT:Another Test:FAIL:75
```

### Summary Messages
```
SUMMARY:5:4:1  # Total:Passed:Failed
```

## Monitoring and Success Conditions

The RTT monitor script waits for specific conditions before considering tests complete:

### Default Success Conditions
- `TEST_COMPLETE` status received
- All individual tests show `PASS` status

### Custom Success Conditions
You can modify the success conditions in `rtt_monitor.py`:

```python
# Add custom condition function
def custom_success_condition(test_results):
    return len(test_results) >= 3 and \
           all(r.status == TestStatus.PASS for r in test_results.values())

# Add to success conditions
monitor.success_conditions.append(custom_success_condition)
```

## Configuration Options

### Script Parameters

**run_tests.sh options:**
- `-d, --device`: Target device (required)
- `-f, --firmware`: Firmware file to flash
- `-i, --interface`: Debug interface (default: SWD)
- `-s, --speed`: Debug speed in kHz (default: 4000)
- `-t, --timeout`: Test timeout in seconds (default: 60)
- `-l, --logs-only`: Monitor RTT without flashing

**rtt_monitor.py options:**
```bash
python3 rtt_monitor.py <device> [interface] [speed] [timeout]
```

### Makefile Variables
- `TARGET_DEVICE`: Target microcontroller (default: STM32F407VG)
- `BUILD_DIR`: Build output directory (default: build)

## Output and Results

### Console Output
Real-time test progress with color-coded messages:
- 🔵 Blue: General status
- 🟢 Green: Success messages
- 🟡 Yellow: Warnings
- 🔴 Red: Errors

### JSON Results File
Detailed test results saved to `logs/test_results_YYYYMMDD_HHMMSS.json`:

```json
{
  "test_results": {
    "System Initialization": {
      "name": "System Initialization",
      "status": "TEST_PASS",
      "duration_ms": 45,
      "timestamp": "2024-01-15T10:30:00",
      "log_messages": []
    }
  },
  "log_buffer": [
    {
      "timestamp": "2024-01-15T10:30:00",
      "raw": "[12345] [INFO] Test started"
    }
  ],
  "summary": {
    "total_tests": 5,
    "passed_tests": 4,
    "failed_tests": 1
  }
}
```

## Integration Guide

### Adding to Existing Projects

1. **Copy framework files** to your project
2. **Include RTT source**: Add SEGGER RTT library
3. **Update build system**: Include framework sources
4. **Initialize in main**: Call `test_rtt_init()`
5. **Add test cases**: Write functions using the test API

### SEGGER RTT Setup

Download SEGGER RTT library and add to your project:
```c
// In your linker script or memory configuration
// Reserve memory for RTT buffer (typically in RAM)

// In your main.c or startup code
#include "SEGGER_RTT.h"
#include "test_rtt_logger.h"

int main(void) {
    // Hardware initialization
    SystemInit();
    
    // Initialize test framework
    test_rtt_init();
    
    // Your tests here...
}
```

## Troubleshooting

### Common Issues

**J-Link not found:**
- Ensure J-Link software is installed and in PATH
- Check device connection and power

**RTT not working:**
- Verify RTT buffer allocation in linker script
- Check that SEGGER_RTT_Init() is called early
- Ensure target is running (not halted)

**Tests timeout:**
- Increase timeout value with `-t` option
- Check if target is actually running tests
- Verify RTT output format matches expected patterns

**Build errors:**
- Verify ARM GCC toolchain installation
- Check include paths in Makefile
- Ensure SEGGER RTT source files are available

### Debug Tips

1. **Use manual RTT client** for debugging:
   ```bash
   JLinkRTTClient -Device STM32F407VG
   ```

2. **Check raw RTT output** before automated parsing

3. **Enable debug logging** in Python monitor script

4. **Verify timing** - ensure tests complete within timeout period

## License

This framework is provided as-is for educational and development purposes. Adapt as needed for your specific embedded projects.

## Contributing

Feel free to extend this framework with additional features:
- More sophisticated test assertions
- Performance benchmarking
- Memory usage tracking
- Multi-target support
- CI/CD integration