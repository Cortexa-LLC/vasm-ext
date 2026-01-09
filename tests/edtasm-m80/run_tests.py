#!/usr/bin/env python3
"""
Test runner for Z80 EDTASM-M80 assembler
Usage: python3 run_tests.py [--verbose]
"""

import os
import sys
import subprocess
from pathlib import Path

# ANSI color codes
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[0;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

# Test counters
total = 0
passed = 0
failed = 0

# Verbose mode
verbose = '--verbose' in sys.argv or '-v' in sys.argv

def run_test(test_file, vasm_path):
    """Run a single test and return success status"""
    global total, passed, failed

    test_name = Path(test_file).stem
    output_file = f"{test_name}.bin"
    log_file = f"{test_name}.log"

    print(f"Testing {test_name}... ", end='', flush=True)
    total += 1

    try:
        # Run assembler
        with open(log_file, 'w') as log:
            result = subprocess.run(
                [vasm_path, '-Fbin', '-o', output_file, test_file],
                stdout=log,
                stderr=subprocess.STDOUT,
                text=True
            )

        # Check if assembly succeeded
        if result.returncode == 0:
            # Check if binary was created
            if os.path.exists(output_file):
                size = os.path.getsize(output_file)
                print(f"{Colors.GREEN}PASS{Colors.NC} ({size} bytes)")
                passed += 1

                # Show first bytes in verbose mode
                if verbose:
                    try:
                        with open(output_file, 'rb') as f:
                            data = f.read(32)
                        print("  First bytes:", ' '.join(f'{b:02X}' for b in data))
                    except Exception as e:
                        print(f"  (Could not read binary: {e})")
            else:
                print(f"{Colors.RED}FAIL{Colors.NC} (no binary output)")
                failed += 1
                show_log(log_file)
        else:
            print(f"{Colors.RED}FAIL{Colors.NC} (assembler error)")
            failed += 1
            show_log(log_file)

    except Exception as e:
        print(f"{Colors.RED}FAIL{Colors.NC} (exception: {e})")
        failed += 1

def show_log(log_file):
    """Display log file contents with indentation"""
    try:
        with open(log_file, 'r') as f:
            for line in f:
                print(f"  {line.rstrip()}")
    except:
        pass

def main():
    """Main test runner"""
    # Change to test directory
    test_dir = Path(__file__).parent
    os.chdir(test_dir)

    # Find assembler
    vasm_path = test_dir / '../../vasmz80_edtasm-m80'
    if not vasm_path.exists():
        print(f"{Colors.RED}ERROR: Assembler not found at {vasm_path}{Colors.NC}")
        print("Please build it first with: make CPU=z80 SYNTAX=edtasm-m80")
        return 1

    print(f"{Colors.BLUE}================================================{Colors.NC}")
    print(f"{Colors.BLUE}Z80 EDTASM-M80 Test Suite{Colors.NC}")
    print(f"{Colors.BLUE}================================================{Colors.NC}")
    print()

    # Test files in order
    tests = [
        'test_directives.asm',
        'test_data.asm',
        'test_aliases.asm',
        'test_segments.asm',
        'test_conditionals.asm',
        'test_m80_cond.asm',
        'test_endianness.asm',
        'test_nested_cond.asm',
        'test_case_insensitive.asm',
        'test_z80_instructions.asm',
        'test_comprehensive.asm',
    ]

    print(f"{Colors.YELLOW}Running tests...{Colors.NC}")
    print()

    for test in tests:
        if os.path.exists(test):
            run_test(test, str(vasm_path))
        else:
            print(f"Skipping {test} (not found)")

    # Summary
    print()
    print(f"{Colors.BLUE}================================================{Colors.NC}")
    print(f"{Colors.BLUE}Test Summary{Colors.NC}")
    print(f"{Colors.BLUE}================================================{Colors.NC}")
    print(f"Total tests:  {total}")
    print(f"Passed:       {Colors.GREEN}{passed}{Colors.NC}")
    print(f"Failed:       {Colors.RED}{failed}{Colors.NC}")
    print()

    if failed == 0:
        print(f"{Colors.GREEN}All tests passed!{Colors.NC}")
        return 0
    else:
        print(f"{Colors.RED}Some tests failed!{Colors.NC}")
        return 1

if __name__ == '__main__':
    sys.exit(main())
