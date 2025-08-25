#!/usr/bin/env python3

import subprocess
import time
import re
import sys
import json
from datetime import datetime
from typing import Dict, List, Optional
from dataclasses import dataclass
from enum import Enum

class TestStatus(Enum):
    INIT = "TEST_INIT"
    RUNNING = "TEST_RUNNING"
    PASS = "TEST_PASS"
    FAIL = "TEST_FAIL"
    COMPLETE = "TEST_COMPLETE"

@dataclass
class TestResult:
    name: str
    status: TestStatus
    duration_ms: Optional[int] = None
    timestamp: str = None
    log_messages: List[str] = None
    
    def __post_init__(self):
        if self.log_messages is None:
            self.log_messages = []
        if self.timestamp is None:
            self.timestamp = datetime.now().isoformat()

class RTTMonitor:
    def __init__(self, device="", interface="SWD", speed=4000):
        self.device = device
        self.interface = interface
        self.speed = speed
        self.process = None
        self.test_results = {}
        self.log_buffer = []
        
        # RTT log parsing patterns
        self.status_pattern = re.compile(r'STATUS:(\w+):(.+)')
        self.result_pattern = re.compile(r'RESULT:(.+):(PASS|FAIL):(\d+)')
        self.summary_pattern = re.compile(r'SUMMARY:(\d+):(\d+):(\d+)')
        self.log_pattern = re.compile(r'\[(\d+)\] \[(\w+)\] (.+)')
        
        self.success_conditions = [
            TestStatus.COMPLETE,
            lambda results: all(r.status == TestStatus.PASS for r in results.values())
        ]
    
    def start_rtt_viewer(self):
        """Start J-Link RTT Viewer process"""
        cmd = [
            "JLinkRTTClient",
            "-Device", self.device,
            "-If", self.interface,
            "-Speed", str(self.speed)
        ]
        
        try:
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            print(f"[RTT_MONITOR] Started J-Link RTT Client for device: {self.device}")
            return True
        except FileNotFoundError:
            print("[RTT_MONITOR] ERROR: J-Link RTT Client not found. Please install J-Link software.")
            return False
        except Exception as e:
            print(f"[RTT_MONITOR] ERROR: Failed to start RTT viewer: {e}")
            return False
    
    def parse_rtt_line(self, line: str):
        """Parse a single RTT output line"""
        line = line.strip()
        if not line:
            return
        
        # Store all log messages
        self.log_buffer.append({
            'timestamp': datetime.now().isoformat(),
            'raw': line
        })
        
        # Parse status messages
        status_match = self.status_pattern.search(line)
        if status_match:
            status_str, test_name = status_match.groups()
            try:
                status = TestStatus(status_str)
                if test_name not in self.test_results:
                    self.test_results[test_name] = TestResult(test_name, status)
                else:
                    self.test_results[test_name].status = status
                
                print(f"[TEST_STATUS] {test_name}: {status.value}")
            except ValueError:
                print(f"[RTT_MONITOR] Unknown status: {status_str}")
        
        # Parse result messages
        result_match = self.result_pattern.search(line)
        if result_match:
            test_name, result_str, duration = result_match.groups()
            status = TestStatus.PASS if result_str == "PASS" else TestStatus.FAIL
            
            if test_name in self.test_results:
                self.test_results[test_name].status = status
                self.test_results[test_name].duration_ms = int(duration)
            
            print(f"[TEST_RESULT] {test_name}: {result_str} ({duration}ms)")
        
        # Parse summary
        summary_match = self.summary_pattern.search(line)
        if summary_match:
            total, passed, failed = map(int, summary_match.groups())
            print(f"[TEST_SUMMARY] Total: {total}, Passed: {passed}, Failed: {failed}")
            return {
                'total': total,
                'passed': passed,
                'failed': failed,
                'success_rate': (passed / total * 100) if total > 0 else 0
            }
        
        # Parse regular log messages
        log_match = self.log_pattern.search(line)
        if log_match:
            timestamp, level, message = log_match.groups()
            print(f"[{level}] {message}")
        else:
            print(f"[RTT] {line}")
        
        return None
    
    def check_success_condition(self) -> bool:
        """Check if success conditions are met"""
        for condition in self.success_conditions:
            if isinstance(condition, TestStatus):
                # Check if any test has reached this status
                if any(result.status == condition for result in self.test_results.values()):
                    return True
            elif callable(condition):
                # Execute custom condition function
                try:
                    if condition(self.test_results):
                        return True
                except Exception as e:
                    print(f"[RTT_MONITOR] Error in success condition: {e}")
        return False
    
    def monitor_until_success(self, timeout_seconds=60):
        """Monitor RTT output until success condition is met"""
        if not self.start_rtt_viewer():
            return False
        
        start_time = time.time()
        summary_data = None
        
        try:
            print(f"[RTT_MONITOR] Monitoring for {timeout_seconds}s or until success condition...")
            
            while time.time() - start_time < timeout_seconds:
                if self.process.poll() is not None:
                    print("[RTT_MONITOR] RTT process terminated")
                    break
                
                # Read available output
                try:
                    line = self.process.stdout.readline()
                    if line:
                        result = self.parse_rtt_line(line)
                        if result and 'total' in result:
                            summary_data = result
                        
                        # Check success condition after each line
                        if self.check_success_condition():
                            print("[RTT_MONITOR] Success condition met!")
                            time.sleep(1)  # Allow final messages
                            break
                    else:
                        time.sleep(0.1)  # Small delay if no output
                        
                except Exception as e:
                    print(f"[RTT_MONITOR] Error reading output: {e}")
                    break
            
            else:
                print("[RTT_MONITOR] Timeout reached")
            
            return summary_data
            
        finally:
            self.stop_monitoring()
    
    def stop_monitoring(self):
        """Stop RTT monitoring"""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
            print("[RTT_MONITOR] RTT monitoring stopped")
    
    def save_results(self, filename="test_results.json"):
        """Save test results to JSON file"""
        output_data = {
            'test_results': {name: {
                'name': result.name,
                'status': result.status.value,
                'duration_ms': result.duration_ms,
                'timestamp': result.timestamp,
                'log_messages': result.log_messages
            } for name, result in self.test_results.items()},
            'log_buffer': self.log_buffer,
            'summary': {
                'total_tests': len(self.test_results),
                'passed_tests': sum(1 for r in self.test_results.values() if r.status == TestStatus.PASS),
                'failed_tests': sum(1 for r in self.test_results.values() if r.status == TestStatus.FAIL)
            }
        }
        
        with open(filename, 'w') as f:
            json.dump(output_data, f, indent=2)
        
        print(f"[RTT_MONITOR] Results saved to {filename}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 rtt_monitor.py <device> [interface] [speed] [timeout]")
        print("Example: python3 rtt_monitor.py STM32F407VG SWD 4000 60")
        sys.exit(1)
    
    device = sys.argv[1]
    interface = sys.argv[2] if len(sys.argv) > 2 else "SWD"
    speed = int(sys.argv[3]) if len(sys.argv) > 3 else 4000
    timeout = int(sys.argv[4]) if len(sys.argv) > 4 else 60
    
    monitor = RTTMonitor(device=device, interface=interface, speed=speed)
    
    print(f"[RTT_MONITOR] Starting RTT monitoring for {device}")
    summary = monitor.monitor_until_success(timeout_seconds=timeout)
    
    if summary:
        print(f"[RTT_MONITOR] Test execution completed:")
        print(f"  Total: {summary['total']}")
        print(f"  Passed: {summary['passed']}")
        print(f"  Failed: {summary['failed']}")
        print(f"  Success Rate: {summary['success_rate']:.1f}%")
    
    monitor.save_results(f"logs/test_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json")

if __name__ == "__main__":
    main()