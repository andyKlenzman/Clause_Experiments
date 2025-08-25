# SEGGER RTT Integration Guide

This document explains how to integrate SEGGER RTT (Real Time Transfer) with your embedded project for the test framework.

## What is SEGGER RTT?

SEGGER RTT is a technology for real-time data transfer between a target microcontroller and a host computer. It provides:
- High-speed bidirectional communication
- No additional hardware required (uses existing debug interface)
- Minimal target overhead
- Real-time data transfer without halting the target

## Download and Setup

### 1. Download SEGGER RTT

Get the latest RTT source code from SEGGER:
- Website: https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/
- GitHub: https://github.com/SEGGERMicro/RTT

### 2. File Structure

Extract RTT files and organize them:
```
your_project/
├── SEGGER_RTT/
│   ├── SEGGER_RTT.c
│   ├── SEGGER_RTT.h
│   ├── SEGGER_RTT_Conf.h
│   ├── SEGGER_RTT_printf.c
│   └── ...
└── embedded-test-framework/
    └── ...
```

## Integration Steps

### 1. Add RTT Sources to Build System

**Makefile integration:**
```makefile
# RTT configuration
RTT_DIR = ../SEGGER_RTT
RTT_SOURCES = $(RTT_DIR)/SEGGER_RTT.c $(RTT_DIR)/SEGGER_RTT_printf.c

# Add to your source files
SOURCES += $(RTT_SOURCES)

# Add include path
CFLAGS += -I$(RTT_DIR)
```

**CMake integration:**
```cmake
# Add RTT directory
set(RTT_DIR ${CMAKE_SOURCE_DIR}/SEGGER_RTT)

# Add RTT sources
set(RTT_SOURCES
    ${RTT_DIR}/SEGGER_RTT.c
    ${RTT_DIR}/SEGGER_RTT_printf.c
)

# Add to target
target_sources(your_target PRIVATE ${RTT_SOURCES})
target_include_directories(your_target PRIVATE ${RTT_DIR})
```

### 2. Memory Configuration

**Linker Script (STM32 example):**
```ld
/* Add RTT buffer to RAM section */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Reserve space for RTT at beginning of RAM */
.rtt_buffer :
{
  . = ALIGN(4);
  _rtt_start = .;
  . = . + 0x400;  /* 1KB for RTT buffer */
  _rtt_end = .;
} > RAM
```

**Alternative: Static Buffer Allocation**
```c
// In your main.c or dedicated RTT file
#include "SEGGER_RTT.h"

// Static buffer allocation (alternative to linker script)
static char rtt_buffer[1024];
```

### 3. RTT Configuration

**SEGGER_RTT_Conf.h customization:**
```c
#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H

// Buffer configuration
#define BUFFER_SIZE_UP    1024    // Size of up buffer (target -> host)
#define BUFFER_SIZE_DOWN  16      // Size of down buffer (host -> target)

// Number of buffers
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS     3
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS   3

// RTT mode (blocking behavior)
#define SEGGER_RTT_MODE_DEFAULT SEGGER_RTT_MODE_NO_BLOCK_SKIP

// Performance settings
#define SEGGER_RTT_CPU_CACHE_LINE_SIZE    32
#define SEGGER_RTT_UNCACHED_OFF           0

#endif
```

### 4. Early Initialization

**In your startup code or main:**
```c
#include "SEGGER_RTT.h"

int main(void) {
    // Hardware initialization first
    SystemInit();
    SystemClock_Config();
    
    // Initialize RTT early
    SEGGER_RTT_Init();
    
    // Configure buffer (optional)
    SEGGER_RTT_ConfigUpBuffer(0, "Terminal", NULL, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    
    // Initialize test framework
    test_rtt_init();
    
    // Rest of your application...
}
```

## Memory Requirements

### Typical Memory Usage

**RAM Usage:**
- RTT Control Block: ~80 bytes
- Up Buffer (Terminal output): 1024 bytes (configurable)
- Down Buffer (Host input): 16 bytes (configurable)
- **Total: ~1120 bytes**

**Flash Usage:**
- RTT Library: ~2-3 KB
- Test Framework: ~1-2 KB
- **Total: ~3-5 KB**

### Memory Optimization

**For resource-constrained devices:**
```c
// Reduce buffer sizes
#define BUFFER_SIZE_UP    512     // Smaller output buffer
#define BUFFER_SIZE_DOWN  0       // Disable input if not needed

// Reduce number of buffers
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS     1
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS   0
```

## Target-Specific Configuration

### STM32 Configuration

**STM32CubeMX Integration:**
1. Enable SWD in debug configuration
2. Ensure sufficient RAM allocation
3. Configure system clock for stable operation

**STM32 HAL Integration:**
```c
// In main.c
#include "stm32f4xx_hal.h"
#include "SEGGER_RTT.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // Initialize RTT before any logging
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0, "RTT initialized\r\n");
    
    // Continue with application...
}
```

### ESP32 Configuration

**ESP-IDF Integration:**
```c
// In CMakeLists.txt
idf_component_register(SRCS "main.c" 
                           "../SEGGER_RTT/SEGGER_RTT.c"
                           "../SEGGER_RTT/SEGGER_RTT_printf.c"
                      INCLUDE_DIRS "." "../SEGGER_RTT")

// In main.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SEGGER_RTT.h"

void app_main() {
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0, "ESP32 RTT ready\r\n");
    // Your code...
}
```

### Nordic nRF Configuration

**Nordic SDK Integration:**
```c
// In sdk_config.h
#define RTT_ENABLED 1
#define RTT_LOG_ENABLED 1

// In main.c
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

int main(void) {
    // Initialize logging
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    // RTT is automatically initialized by Nordic SDK
    SEGGER_RTT_WriteString(0, "nRF RTT ready\r\n");
    
    // Your code...
}
```

## Debugging RTT Issues

### Common Problems and Solutions

**1. RTT not detected:**
```c
// Ensure RTT is initialized before first use
SEGGER_RTT_Init();

// Check if RTT control block is placed correctly
extern char _rtt_start, _rtt_end;
printf("RTT buffer: %p - %p\n", &_rtt_start, &_rtt_end);
```

**2. Missing output:**
```c
// Check buffer mode
SEGGER_RTT_SetModeUpBuffer(0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

// Explicit flush (usually not needed)
SEGGER_RTT_Flush();
```

**3. Corrupted output:**
```c
// Ensure RTT calls are atomic in multi-threaded environment
// Use critical sections if necessary
__disable_irq();
SEGGER_RTT_WriteString(0, "Protected output\r\n");
__enable_irq();
```

### RTT Viewer Connection

**Manual connection test:**
```bash
# Connect RTT Viewer
JLinkRTTViewer -Device STM32F407VG -If SWD -Speed 4000

# Or use command line client
JLinkRTTClient -Device STM32F407VG -If SWD -Speed 4000
```

**J-Link Commander RTT test:**
```bash
JLinkExe -Device STM32F407VG -If SWD -Speed 4000
# In J-Link commander:
# connect
# rtt start
# rtt read 0 100
```

## Performance Considerations

### Optimization Tips

**1. Buffer Management:**
- Use appropriate buffer sizes for your application
- Consider using multiple buffers for different log levels
- Implement buffer overflow handling

**2. Timing Considerations:**
- RTT has minimal impact on real-time performance
- Non-blocking mode prevents target stalls
- Consider timing-critical sections

**3. Multi-threading:**
- RTT is generally thread-safe
- Consider protection for high-frequency logging
- Use dedicated logging task if needed

### Benchmarking RTT Performance

```c
// Performance test code
uint32_t start_time = HAL_GetTick();

for (int i = 0; i < 1000; i++) {
    SEGGER_RTT_printf(0, "Test message %d\r\n", i);
}

uint32_t duration = HAL_GetTick() - start_time;
SEGGER_RTT_printf(0, "1000 messages in %lu ms\r\n", duration);
```

## Advanced Features

### Multiple RTT Channels

```c
// Configure multiple buffers
SEGGER_RTT_ConfigUpBuffer(1, "Errors", NULL, 256, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
SEGGER_RTT_ConfigUpBuffer(2, "Debug", NULL, 512, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

// Use different channels
SEGGER_RTT_WriteString(0, "General output\r\n");
SEGGER_RTT_WriteString(1, "Error message\r\n");
SEGGER_RTT_WriteString(2, "Debug info\r\n");
```

### RTT Input Handling

```c
// Read input from host
char input_buffer[16];
int bytes_read = SEGGER_RTT_Read(0, input_buffer, sizeof(input_buffer) - 1);

if (bytes_read > 0) {
    input_buffer[bytes_read] = '\0';
    SEGGER_RTT_printf(0, "Received: %s\r\n", input_buffer);
}
```

This integration guide should help you successfully integrate SEGGER RTT with your embedded test framework. The key is early initialization and proper memory configuration for your specific target platform.