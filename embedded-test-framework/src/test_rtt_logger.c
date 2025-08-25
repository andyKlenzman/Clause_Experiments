#include "test_rtt_logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static const char* log_level_strings[] = {
    "ERROR", "WARN", "INFO", "DEBUG"
};

static uint32_t test_counter = 0;
static uint32_t passed_tests = 0;
static uint32_t failed_tests = 0;

void test_rtt_init(void) {
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, RTT_BUFFER_UP_SIZE, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    
    TEST_LOG_INFO("=== RTT Test Framework Initialized ===");
    TEST_LOG_INFO("RTT Buffer Size: %d bytes", RTT_BUFFER_UP_SIZE);
    
    test_status(TEST_STATUS_INIT, "Test Framework");
}

void test_log(int level, const char* format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    uint32_t timestamp = SEGGER_RTT_GetUpBufferReadPos(0);
    
    SEGGER_RTT_printf(0, "[%08lu] [%s] %s\r\n", 
                     timestamp, 
                     log_level_strings[level], 
                     buffer);
}

void test_status(const char* status, const char* test_name) {
    SEGGER_RTT_printf(0, "STATUS:%s:%s\r\n", status, test_name);
}

void test_result(const char* test_name, bool passed, uint32_t duration_ms) {
    test_counter++;
    
    if (passed) {
        passed_tests++;
        TEST_LOG_INFO("✓ PASS: %s (%lu ms)", test_name, duration_ms);
        test_status(TEST_STATUS_PASS, test_name);
    } else {
        failed_tests++;
        TEST_LOG_ERROR("✗ FAIL: %s (%lu ms)", test_name, duration_ms);
        test_status(TEST_STATUS_FAIL, test_name);
    }
    
    SEGGER_RTT_printf(0, "RESULT:%s:%s:%lu\r\n", 
                     test_name, 
                     passed ? "PASS" : "FAIL", 
                     duration_ms);
}

void test_assert(bool condition, const char* message) {
    if (!condition) {
        TEST_LOG_ERROR("ASSERTION FAILED: %s", message);
        test_status(TEST_STATUS_FAIL, "Assertion");
    }
}

void test_summary(void) {
    TEST_LOG_INFO("=== Test Summary ===");
    TEST_LOG_INFO("Total Tests: %lu", test_counter);
    TEST_LOG_INFO("Passed: %lu", passed_tests);
    TEST_LOG_INFO("Failed: %lu", failed_tests);
    TEST_LOG_INFO("Success Rate: %lu%%", 
                 test_counter > 0 ? (passed_tests * 100) / test_counter : 0);
    
    test_status(TEST_STATUS_COMPLETE, "All Tests");
    
    SEGGER_RTT_printf(0, "SUMMARY:%lu:%lu:%lu\r\n", 
                     test_counter, passed_tests, failed_tests);
}