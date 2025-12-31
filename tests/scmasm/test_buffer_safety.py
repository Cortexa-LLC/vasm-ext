#!/usr/bin/env python3
"""
Buffer safety tests for A2osX -a2osx-auto flag
Tests boundary conditions and error handling
"""

import os
import sys
import subprocess
import tempfile
import shutil
from pathlib import Path

# Get absolute path to vasm binary (relative to this script's location)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
VASM = os.path.join(SCRIPT_DIR, "../../vasm6502_scmasm")
MAXPATHLEN = 1024
failed = 0
passed = 0

# Use system temp directory to avoid polluting git repo
TEST_TEMP_DIR = tempfile.mkdtemp(prefix="vasm_test_")
TEST_SOURCE = os.path.join(TEST_TEMP_DIR, "test_temp.s")

print("Buffer Safety Tests for A2osX Build Automation")
print("=" * 50)
print(f"Test directory: {TEST_TEMP_DIR}")
print()

def test_case(name, expected, cmd, env=None):
    """Run a test case and check result"""
    global failed, passed

    print(f"Test: {name} ... ", end="", flush=True)

    test_env = os.environ.copy()
    if env:
        test_env.update(env)

    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            timeout=10,
            env=test_env,
            cwd=TEST_TEMP_DIR
        )
        actual = "SUCCESS" if result.returncode == 0 else "FAILURE"
    except subprocess.TimeoutExpired:
        actual = "FAILURE"
    except Exception:
        actual = "FAILURE"

    if actual == expected:
        print("PASS")
        passed += 1
    else:
        print(f"FAIL (expected {expected}, got {actual})")
        failed += 1

def create_test_source(tf_path):
    """Create a test source file with .TF directive"""
    with open(TEST_SOURCE, 'w') as f:
        f.write(f"""* Test source
                .OR $8000
                .TF {tf_path}
START           NOP
                RTS
""")

try:
    # Test 1: Normal short path
    print("=== Normal Operation Tests ===")
    create_test_source("BIN/TEST")
    test_case("Normal short path", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out1.bin test_temp.s")

    # Test 2: Empty .TF path
    create_test_source("")
    test_case("Empty .TF path", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out2.bin test_temp.s")

    # Test 3: Very long but valid path
    print()
    print("=== Boundary Tests ===")
    long_path = 'A' + '/B' * 490  # ~1000 chars
    create_test_source(long_path)
    test_case("Long valid path (~1000 chars)", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out3.bin test_temp.s")

    # Test 4: Path exceeding MAXPATHLEN
    max_path = 'A' + '/B' * 512  # >1024 chars
    create_test_source(max_path)
    test_case("Path exceeding MAXPATHLEN", "FAILURE",
              f"{VASM} -Fbin -a2osx-auto test_temp.s")

    # Test 5: Combined path exceeding limit
    print()
    print("=== Combined Path Tests ===")
    long_base = '/tmp/' + 'A' * 500
    long_tf = 'B' * 500
    create_test_source(long_tf)
    test_case("Combined path exceeding limit", "FAILURE",
              f"{VASM} -Fbin -a2osx-auto test_temp.s",
              env={'A2OSX_BUILD_DIR': long_base})

    # Test 6: Base path at limit
    max_base = '/tmp/' + 'A' * 1020
    create_test_source("BIN/TEST")
    test_case("Base path at MAXPATHLEN", "FAILURE",
              f"{VASM} -Fbin -a2osx-auto test_temp.s",
              env={'A2OSX_BUILD_DIR': max_base})

    # Test 7: Very deep directory structure
    print()
    print("=== Directory Depth Tests ===")
    deep_path = '/'.join(['d'] * 200)
    create_test_source(deep_path)
    test_case("Very deep directory structure", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out4.bin test_temp.s")

    # Test 8: Base path with trailing slash
    create_test_source("BIN/TEST")
    test_case("Base path with trailing slash", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto test_temp.s && rm -rf /tmp/test_base",
              env={'A2OSX_BUILD_DIR': '/tmp/test_base/'})

    # Test 9: Path with consecutive slashes
    create_test_source("BIN//TEST")
    test_case("Path with consecutive slashes", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out5.bin test_temp.s")

    # Test 10-11: Special characters
    print()
    print("=== Special Character Tests ===")
    create_test_source("BIN/TEST_FILE")
    test_case("Path with underscores", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out6.bin test_temp.s")

    create_test_source("BIN/MY TEST")
    test_case("Path with spaces", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto -o /tmp/test_out7.bin test_temp.s")

    # Test 12-13: Environment variable tests
    create_test_source("BIN/TEST")
    test_env = os.environ.copy()
    if 'A2OSX_BUILD_DIR' in test_env:
        del test_env['A2OSX_BUILD_DIR']
    test_case("Missing A2OSX_BUILD_DIR (use default)", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto test_temp.s && rm -rf BIN",
              env=test_env)

    create_test_source("BIN/TEST")
    test_case("Empty A2OSX_BUILD_DIR (use default)", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto test_temp.s && rm -rf BIN",
              env={'A2OSX_BUILD_DIR': ''})

    # Test 14: Absolute path in .TF
    print()
    print("=== Absolute Path Tests ===")
    create_test_source("/tmp/BIN/TEST")
    test_case("Absolute path in .TF", "SUCCESS",
              f"{VASM} -Fbin -a2osx-auto test_temp.s && rm -f /tmp/BIN/TEST && rmdir /tmp/BIN 2>/dev/null || true")

finally:
    # Cleanup: Always remove temp directory even if tests fail
    print()
    print(f"Cleaning up test directory: {TEST_TEMP_DIR}")
    try:
        shutil.rmtree(TEST_TEMP_DIR, ignore_errors=True)
    except:
        pass

    # Clean up test output files
    for i in range(1, 8):
        try:
            os.remove(f"/tmp/test_out{i}.bin")
        except:
            pass

print()
print("=" * 50)
print(f"Results: {passed} passed, {failed} failed")
print("=" * 50)

sys.exit(1 if failed > 0 else 0)
