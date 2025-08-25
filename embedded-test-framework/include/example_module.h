#ifndef EXAMPLE_MODULE_H
#define EXAMPLE_MODULE_H

#include <stdint.h>
#include <stdbool.h>

void system_init(void);
uint32_t get_system_tick(void);
bool is_system_ready(void);
int32_t calculate_sum(int32_t a, int32_t b);
bool validate_range(int32_t value, int32_t min, int32_t max);
void reset_system(void);

#endif