#include "example_module.h"
#include "test_rtt_logger.h"

static uint32_t system_tick_counter = 0;
static bool system_initialized = false;

void system_init(void) {
    TEST_LOG_INFO("Initializing system...");
    
    system_tick_counter = 0;
    system_initialized = true;
    
    TEST_LOG_INFO("System initialization complete");
}

uint32_t get_system_tick(void) {
    if (!system_initialized) {
        TEST_LOG_ERROR("System not initialized!");
        return 0;
    }
    
    return system_tick_counter++;
}

bool is_system_ready(void) {
    return system_initialized;
}

int32_t calculate_sum(int32_t a, int32_t b) {
    TEST_LOG_DEBUG("Calculating sum: %ld + %ld", a, b);
    
    int64_t result = (int64_t)a + (int64_t)b;
    
    if (result > INT32_MAX || result < INT32_MIN) {
        TEST_LOG_ERROR("Integer overflow in sum calculation");
        return 0;
    }
    
    TEST_LOG_DEBUG("Sum result: %ld", (int32_t)result);
    return (int32_t)result;
}

bool validate_range(int32_t value, int32_t min, int32_t max) {
    TEST_LOG_DEBUG("Validating range: %ld in [%ld, %ld]", value, min, max);
    
    bool valid = (value >= min) && (value <= max);
    
    if (!valid) {
        TEST_LOG_WARN("Value %ld out of range [%ld, %ld]", value, min, max);
    }
    
    return valid;
}

void reset_system(void) {
    TEST_LOG_INFO("Resetting system...");
    
    system_tick_counter = 0;
    system_initialized = false;
    
    TEST_LOG_INFO("System reset complete");
}