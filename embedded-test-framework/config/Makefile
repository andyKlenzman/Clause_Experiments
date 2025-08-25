# Embedded Test Framework Makefile
# This Makefile builds the test framework for embedded targets

# Project configuration
PROJECT_NAME = embedded_test_framework
TARGET_DEVICE ?= STM32F407VG
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
SCRIPTS_DIR = scripts

# Compiler configuration
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -DUSE_HAL_DRIVER -DSTM32F407xx
CFLAGS += -IInc -IDrivers/STM32F4xx_HAL_Driver/Inc -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy -IDrivers/CMSIS/Device/ST/STM32F4xx/Include -IDrivers/CMSIS/Include
CFLAGS += -I$(INCLUDE_DIR)
CFLAGS += -Og -Wall -fdata-sections -ffunction-sections -g -gdwarf-2 -MMD -MP

# Linker flags
LDFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -specs=nano.specs -T$(TARGET_DEVICE)_FLASH.ld -lc -lm -lnosys
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT_NAME).map,--cref -Wl,--gc-sections

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(TEST_DIR)/*.c)

# Include RTT sources (assuming SEGGER RTT is available)
RTT_DIR = ../SEGGER_RTT
ifneq ("$(wildcard $(RTT_DIR))","")
    SOURCES += $(wildcard $(RTT_DIR)/*.c)
    CFLAGS += -I$(RTT_DIR)
endif

# Object files
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

# Default target
all: $(BUILD_DIR)/$(PROJECT_NAME).hex

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)
	mkdir -p $(BUILD_DIR)/$(RTT_DIR)

# Compile C files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

# Create hex file
$(BUILD_DIR)/$(PROJECT_NAME).hex: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(OBJCOPY) -O ihex $< $@

# Create bin file
$(BUILD_DIR)/$(PROJECT_NAME).bin: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(OBJCOPY) -O binary -S $< $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Flash and run tests
test: $(BUILD_DIR)/$(PROJECT_NAME).hex
	@echo "Flashing and running tests..."
	$(SCRIPTS_DIR)/run_tests.sh -d $(TARGET_DEVICE) -f $<

# Monitor RTT logs only (no flashing)
monitor:
	@echo "Monitoring RTT logs..."
	$(SCRIPTS_DIR)/run_tests.sh -d $(TARGET_DEVICE) -l

# Help
help:
	@echo "Embedded Test Framework Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build hex file (default)"
	@echo "  clean   - Clean build directory"
	@echo "  test    - Flash firmware and run tests"
	@echo "  monitor - Monitor RTT logs only"
	@echo "  help    - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  TARGET_DEVICE - Target device (default: STM32F407VG)"
	@echo ""
	@echo "Examples:"
	@echo "  make                                    # Build"
	@echo "  make test TARGET_DEVICE=STM32F407VG     # Test on STM32F407VG"
	@echo "  make monitor                            # Monitor only"

# Include dependencies
-include $(DEPENDS)

.PHONY: all clean test monitor help