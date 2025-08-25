#ifndef TEST_RTT_LOGGER_H
#define TEST_RTT_LOGGER_H

#include "SEGGER_RTT.h"
#include <stdint.h>
#include <stdbool.h>

#define RTT_BUFFER_UP_SIZE 1024
#define RTT_BUFFER_DOWN_SIZE 16

#define TEST_LOG_LEVEL_ERROR   0
#define TEST_LOG_LEVEL_WARN    1
#define TEST_LOG_LEVEL_INFO    2
#define TEST_LOG_LEVEL_DEBUG   3

#define TEST_STATUS_INIT       "TEST_INIT"
#define TEST_STATUS_RUNNING    "TEST_RUNNING"
#define TEST_STATUS_PASS       "TEST_PASS"
#define TEST_STATUS_FAIL       "TEST_FAIL"
#define TEST_STATUS_COMPLETE   "TEST_COMPLETE"

typedef struct {
    const char* name;
    uint32_t test_id;
    uint32_t start_time;
    uint32_t end_time;
    bool passed;
} test_case_t;

void test_rtt_init(void);
void test_log(int level, const char* format, ...);
void test_status(const char* status, const char* test_name);
void test_result(const char* test_name, bool passed, uint32_t duration_ms);
void test_assert(bool condition, const char* message);

#define TEST_LOG_ERROR(...)   test_log(TEST_LOG_LEVEL_ERROR, __VA_ARGS__)
#define TEST_LOG_WARN(...)    test_log(TEST_LOG_LEVEL_WARN, __VA_ARGS__)
#define TEST_LOG_INFO(...)    test_log(TEST_LOG_LEVEL_INFO, __VA_ARGS__)
#define TEST_LOG_DEBUG(...)   test_log(TEST_LOG_LEVEL_DEBUG, __VA_ARGS__)

#define TEST_ASSERT(cond, msg) test_assert(cond, msg)

#endif