#!/usr/bin/env python3
"""
Test .OP directive CPU validation

With CPU validation enabled, vasm now enforces CPU-specific instructions
based on the .OP directive. The .OP directive changes cpu_type flags which:
  1. Control which instructions are available
  2. Set bytes per address (2 for 6502/65C02, 3 for 65816)
  3. Enable CPU-specific addressing modes

This test validates that .OP directive:
  - Rejects CPU-specific instructions in wrong mode
  - Accepts instructions after appropriate .OP directive
  - Switches CPU modes correctly
"""

import os
import sys
import subprocess
import tempfile

VASM = "../../vasm6502_scmasm"
failed = 0
passed = 0

print("CPU .OP Directive Validation Tests")
print("=" * 40)
print()

def test_case(name, expected, source):
    """Run a test case and check result"""
    global failed, passed

    print(f"Test: {name} ... ", end="", flush=True)

    # Create temporary test file
    with open("test_cpu_temp.s", "w") as f:
        f.write(source)

    try:
        result = subprocess.run(
            [VASM, "-Fbin", "-o", "/tmp/cpu_test.bin", "test_cpu_temp.s"],
            capture_output=True,
            timeout=10
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

    # Cleanup
    try:
        os.remove("test_cpu_temp.s")
    except:
        pass
    try:
        os.remove("/tmp/cpu_test.bin")
    except:
        pass

print("=== 6502 Base Instructions (should always work) ===")
test_case("6502: NOP", "SUCCESS", """
        .OR $8000
        NOP
""")

test_case("6502: LDA immediate", "SUCCESS", """
        .OR $8000
        LDA #$00
""")

test_case("6502: STA absolute", "SUCCESS", """
        .OR $8000
        STA $D020
""")

print()
print("=== 65C02 Instructions (need .OP 65C02) ===")

test_case("65C02: STZ without .OP 65C02", "FAILURE", """
        .OR $8000
        STZ $00
""")

test_case("65C02: STZ with .OP 65C02", "SUCCESS", """
        .OR $8000
        .OP 65C02
        STZ $00
""")

test_case("65C02: BRA without .OP 65C02", "FAILURE", """
        .OR $8000
        BRA $8010
""")

test_case("65C02: BRA with .OP 65C02", "SUCCESS", """
        .OR $8000
        .OP 65C02
        BRA $8010
""")

print()
print("=== 65816 Instructions (need .OP 816) ===")

test_case("65816: REP without .OP 816", "FAILURE", """
        .OR $8000
        REP #$30
""")

test_case("65816: REP with .OP 816", "SUCCESS", """
        .OR $8000
        .OP 816
        REP #$30
""")

test_case("65816: PEA without .OP 816", "FAILURE", """
        .OR $8000
        PEA $1234
""")

test_case("65816: PEA with .OP 816", "SUCCESS", """
        .OR $8000
        .OP 816
        PEA $1234
""")

print()
print("=== .OP Mode Switching ===")

test_case("Switch from 6502 to 65C02", "SUCCESS", """
        .OR $8000
        NOP             ; 6502
        .OP 65C02
        STZ $00        ; 65C02
""")

test_case("Switch from 65C02 to 65816", "SUCCESS", """
        .OR $8000
        .OP 65C02
        STZ $00        ; 65C02
        .OP 816
        REP #$30       ; 65816
""")

print()
print("=== .OP Variant Names ===")

test_case(".OP 6502", "SUCCESS", """
        .OR $8000
        .OP 6502
        NOP
""")

test_case(".OP 65C02", "SUCCESS", """
        .OR $8000
        .OP 65C02
        STZ $00
""")

test_case(".OP WDC02 (Western Design Center 65C02)", "SUCCESS", """
        .OR $8000
        .OP WDC02
        STZ $00
""")

test_case(".OP 816", "SUCCESS", """
        .OR $8000
        .OP 816
        REP #$30
""")

test_case(".OP 65816", "SUCCESS", """
        .OR $8000
        .OP 65816
        REP #$30
""")

test_case(".OP CE02 (CSG 65CE02)", "SUCCESS", """
        .OR $8000
        .OP CE02
        STZ $00
""")

test_case(".OP invalid CPU type", "FAILURE", """
        .OR $8000
        .OP INVALID
""")

print()
print("=" * 40)
print(f"Results: {passed} passed, {failed} failed")
print("=" * 40)

sys.exit(1 if failed > 0 else 0)
