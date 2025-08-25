#!/bin/bash

# Embedded Test Framework - Test Runner
# This script flashes firmware and runs tests with RTT monitoring

set -e  # Exit on any error

# Configuration
DEVICE=""
INTERFACE="SWD"
SPEED="4000"
TIMEOUT="60"
FIRMWARE_FILE=""
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LOGS_DIR="$PROJECT_ROOT/logs"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[TEST_RUNNER]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[TEST_RUNNER]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[TEST_RUNNER]${NC} $1"
}

print_error() {
    echo -e "${RED}[TEST_RUNNER]${NC} $1"
}

# Help function
show_help() {
    echo "Embedded Test Framework - Test Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -d, --device DEVICE     Target device (required)"
    echo "  -f, --firmware FILE     Firmware file to flash (.hex/.bin/.elf)"
    echo "  -i, --interface IF      Debug interface (default: SWD)"
    echo "  -s, --speed SPEED       Debug speed in kHz (default: 4000)"
    echo "  -t, --timeout TIMEOUT   Test timeout in seconds (default: 60)"
    echo "  -l, --logs-only        Only monitor RTT, don't flash firmware"
    echo "  -h, --help             Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 -d STM32F407VG -f build/firmware.hex"
    echo "  $0 -d STM32F407VG -l  # Monitor only"
    echo ""
}

# Parse command line arguments
LOGS_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--device)
            DEVICE="$2"
            shift 2
            ;;
        -f|--firmware)
            FIRMWARE_FILE="$2"
            shift 2
            ;;
        -i|--interface)
            INTERFACE="$2"
            shift 2
            ;;
        -s|--speed)
            SPEED="$2"
            shift 2
            ;;
        -t|--timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        -l|--logs-only)
            LOGS_ONLY=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Validate required parameters
if [[ -z "$DEVICE" ]]; then
    print_error "Device is required. Use -d or --device option."
    show_help
    exit 1
fi

if [[ "$LOGS_ONLY" == false ]] && [[ -z "$FIRMWARE_FILE" ]]; then
    print_error "Firmware file is required when not in logs-only mode. Use -f or --firmware option."
    show_help
    exit 1
fi

# Create logs directory
mkdir -p "$LOGS_DIR"

print_status "Starting test execution for device: $DEVICE"
print_status "Interface: $INTERFACE, Speed: ${SPEED}kHz, Timeout: ${TIMEOUT}s"

# Function to flash firmware
flash_firmware() {
    local firmware="$1"
    
    if [[ ! -f "$firmware" ]]; then
        print_error "Firmware file not found: $firmware"
        return 1
    fi
    
    print_status "Flashing firmware: $firmware"
    
    # Create J-Link script for flashing
    local jlink_script="$LOGS_DIR/flash_script.jlink"
    cat > "$jlink_script" << EOF
device $DEVICE
si $INTERFACE
speed $SPEED
loadfile $firmware
r
g
qc
EOF
    
    # Flash using J-Link
    if JLinkExe -CommanderScript "$jlink_script" > "$LOGS_DIR/flash.log" 2>&1; then
        print_success "Firmware flashed successfully"
        return 0
    else
        print_error "Failed to flash firmware. Check $LOGS_DIR/flash.log"
        return 1
    fi
}

# Function to check J-Link tools
check_jlink_tools() {
    local tools=("JLinkExe" "JLinkRTTClient")
    local missing=false
    
    for tool in "${tools[@]}"; do
        if ! command -v "$tool" > /dev/null 2>&1; then
            print_error "$tool not found in PATH"
            missing=true
        fi
    done
    
    if [[ "$missing" == true ]]; then
        print_error "J-Link tools not found. Please install J-Link software package."
        exit 1
    fi
}

# Function to wait for target reset
wait_for_target() {
    print_status "Waiting for target to start..."
    sleep 2
}

# Main execution
main() {
    print_status "=== Embedded Test Framework Runner ==="
    
    # Check prerequisites
    check_jlink_tools
    
    # Flash firmware if not in logs-only mode
    if [[ "$LOGS_ONLY" == false ]]; then
        if ! flash_firmware "$FIRMWARE_FILE"; then
            exit 1
        fi
        wait_for_target
    else
        print_warning "Logs-only mode: Skipping firmware flash"
    fi
    
    # Start RTT monitoring
    print_status "Starting RTT monitoring..."
    
    local timestamp=$(date "+%Y%m%d_%H%M%S")
    local results_file="$LOGS_DIR/test_results_${timestamp}.json"
    
    # Run RTT monitor with Python script
    if python3 "$SCRIPT_DIR/rtt_monitor.py" "$DEVICE" "$INTERFACE" "$SPEED" "$TIMEOUT"; then
        print_success "Test execution completed successfully"
        
        # Move results file to timestamped location
        if [[ -f "$PROJECT_ROOT/logs/test_results_*.json" ]]; then
            mv "$PROJECT_ROOT"/logs/test_results_*.json "$results_file"
            print_status "Results saved to: $results_file"
        fi
        
        # Display summary if results file exists
        if [[ -f "$results_file" ]] && command -v jq > /dev/null 2>&1; then
            print_status "=== Test Summary ==="
            local total=$(jq -r '.summary.total_tests' "$results_file")
            local passed=$(jq -r '.summary.passed_tests' "$results_file")
            local failed=$(jq -r '.summary.failed_tests' "$results_file")
            local success_rate=$((passed * 100 / total))
            
            echo "Total Tests: $total"
            echo "Passed: $passed"
            echo "Failed: $failed"
            echo "Success Rate: $success_rate%"
            
            if [[ $failed -eq 0 ]]; then
                print_success "All tests passed!"
                exit 0
            else
                print_warning "$failed test(s) failed"
                exit 1
            fi
        fi
        
    else
        print_error "Test execution failed or timed out"
        exit 1
    fi
}

# Trap to cleanup on exit
cleanup() {
    print_status "Cleaning up..."
    # Kill any remaining J-Link processes
    pkill -f "JLinkRTTClient" 2>/dev/null || true
    pkill -f "JLinkExe" 2>/dev/null || true
}

trap cleanup EXIT

# Run main function
main "$@"