#!/bin/bash
# SCASM Syntax Module Test Runner

VASM="../../vasm6502_scmasm"
TESTS_PASSED=0
TESTS_FAILED=0

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "SCASM Syntax Module Tests"
echo "========================================"
echo ""

# Check if assembler exists
if [ ! -f "$VASM" ]; then
    echo -e "${RED}Error: $VASM not found${NC}"
    echo "Please build the assembler first:"
    echo "  cd ../.."
    echo "  make CPU=6502 SYNTAX=scmasm"
    exit 1
fi

# Function to run a test
run_test() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .s)

    echo -n "Testing $test_name... "

    # Run assembler
    if $VASM -Fbin -wdc02 -quiet -o /tmp/${test_name}.bin "$test_file" 2>/tmp/${test_name}.err; then
        echo -e "${GREEN}PASS${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Error output:"
        sed 's/^/    /' /tmp/${test_name}.err
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Run all test files
for test in test_*.s; do
    if [ -f "$test" ]; then
        run_test "$test"
    fi
done

# Summary
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo "Total:  $((TESTS_PASSED + TESTS_FAILED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    exit 1
fi
