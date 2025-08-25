#include "example_module.h"
#include "test_rtt_logger.h"
#include <string.h>

extern void test_summary(void);

static uint32_t get_timestamp_ms(void) {
    return get_system_tick() * 10;
}

void test_system_initialization(void) {
    const char* test_name = "System Initialization";
    uint32_t start_time = get_timestamp_ms();
    
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    reset_system();
    TEST_ASSERT(!is_system_ready(), "System should not be ready before init");
    
    system_init();
    TEST_ASSERT(is_system_ready(), "System should be ready after init");
    
    uint32_t tick1 = get_system_tick();
    uint32_t tick2 = get_system_tick();
    TEST_ASSERT(tick2 > tick1, "System tick should increment");
    
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, true, duration);
}

void test_calculate_sum_normal_cases(void) {
    const char* test_name = "Calculate Sum Normal Cases";
    uint32_t start_time = get_timestamp_ms();
    
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    bool all_passed = true;
    
    int32_t result1 = calculate_sum(10, 20);
    if (result1 != 30) {
        TEST_LOG_ERROR("Expected 30, got %ld", result1);
        all_passed = false;
    }
    
    int32_t result2 = calculate_sum(-5, 15);
    if (result2 != 10) {
        TEST_LOG_ERROR("Expected 10, got %ld", result2);
        all_passed = false;
    }
    
    int32_t result3 = calculate_sum(0, 0);
    if (result3 != 0) {
        TEST_LOG_ERROR("Expected 0, got %ld", result3);
        all_passed = false;
    }
    
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, all_passed, duration);
}

void test_calculate_sum_edge_cases(void) {
    const char* test_name = "Calculate Sum Edge Cases";
    uint32_t start_time = get_timestamp_ms();
    
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    bool all_passed = true;
    
    int32_t result1 = calculate_sum(INT32_MAX, 0);
    if (result1 != INT32_MAX) {
        TEST_LOG_ERROR("Max value test failed: expected %ld, got %ld", INT32_MAX, result1);
        all_passed = false;
    }
    
    int32_t result2 = calculate_sum(INT32_MIN, 0);
    if (result2 != INT32_MIN) {
        TEST_LOG_ERROR("Min value test failed: expected %ld, got %ld", INT32_MIN, result2);
        all_passed = false;
    }
    
    int32_t result3 = calculate_sum(INT32_MAX, 1);
    if (result3 != 0) {
        TEST_LOG_WARN("Overflow test: expected 0 (overflow protection), got %ld", result3);
    }
    
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, all_passed, duration);
}

void test_validate_range_function(void) {
    const char* test_name = "Validate Range Function";
    uint32_t start_time = get_timestamp_ms();
    
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    bool all_passed = true;
    
    if (!validate_range(50, 0, 100)) {
        TEST_LOG_ERROR("Valid range test failed");
        all_passed = false;
    }
    
    if (!validate_range(0, 0, 100)) {
        TEST_LOG_ERROR("Lower boundary test failed");
        all_passed = false;
    }
    
    if (!validate_range(100, 0, 100)) {
        TEST_LOG_ERROR("Upper boundary test failed");
        all_passed = false;
    }
    
    if (validate_range(-1, 0, 100)) {
        TEST_LOG_ERROR("Below range test failed");
        all_passed = false;
    }
    
    if (validate_range(101, 0, 100)) {
        TEST_LOG_ERROR("Above range test failed");
        all_passed = false;
    }
    
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, all_passed, duration);
}

void test_system_reset_functionality(void) {
    const char* test_name = "System Reset Functionality";
    uint32_t start_time = get_timestamp_ms();
    
    test_status(TEST_STATUS_RUNNING, test_name);
    TEST_LOG_INFO("Starting test: %s", test_name);
    
    system_init();
    TEST_ASSERT(is_system_ready(), "System should be ready after init");
    
    get_system_tick();
    get_system_tick();
    uint32_t tick_before_reset = get_system_tick();
    
    reset_system();
    TEST_ASSERT(!is_system_ready(), "System should not be ready after reset");
    
    system_init();
    uint32_t tick_after_reset = get_system_tick();
    
    bool reset_successful = (tick_after_reset < tick_before_reset);
    
    uint32_t duration = get_timestamp_ms() - start_time;
    test_result(test_name, reset_successful, duration);
}

int main(void) {
    test_rtt_init();
    
    TEST_LOG_INFO("=== Starting Embedded Test Suite ===");
    
    system_init();
    
    test_system_initialization();
    test_calculate_sum_normal_cases();
    test_calculate_sum_edge_cases();
    test_validate_range_function();
    test_system_reset_functionality();
    
    test_summary();
    
    TEST_LOG_INFO("=== Test Suite Complete ===");
    
    return 0;
}