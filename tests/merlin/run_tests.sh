#!/bin/bash
# Merlin syntax module test suite
# Tests all major features with clean, focused examples

set -e

VASM="../../vasm6502_merlin"
TEST_DIR="."
TMP_DIR="/tmp/vasm_merlin_tests"

mkdir -p "$TMP_DIR"

echo "================================"
echo "Merlin Syntax Module Test Suite"
echo "================================"
echo

# Test counter
PASS=0
FAIL=0

run_test() {
    local name="$1"
    local file="$2"
    local extra_flags="$3"

    echo -n "Testing $name... "

    if $VASM $extra_flags -Fbin -o "$TMP_DIR/${name}.bin" "$TEST_DIR/$file" > "$TMP_DIR/${name}.log" 2>&1; then
        echo "✓ PASS"
        PASS=$((PASS + 1))
    else
        echo "✗ FAIL"
        cat "$TMP_DIR/${name}.log"
        FAIL=$((FAIL + 1))
    fi
}

# Run all tests
echo "6502 CPU Tests:"
run_test "6502 Basic" "test_6502.asm" ""

echo
echo "65C02 CPU Tests:"
run_test "65C02 Extensions" "test_65c02.asm" ""

echo
echo "65816 CPU Tests:"
run_test "65816 Instructions" "test_65816.asm" ""
run_test "GS Toolbox Calls" "test_toolbox.asm" ""

echo
echo "Directive Tests:"
run_test "Merlin Directives" "test_directives.asm" ""

# Summary
echo
echo "================================"
echo "Test Summary"
echo "================================"
echo "PASSED: $PASS"
echo "FAILED: $FAIL"
echo

if [ $FAIL -eq 0 ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed"
    exit 1
fi
