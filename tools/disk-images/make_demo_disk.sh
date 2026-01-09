#!/bin/bash
# Create a demo TRS-80 disk with Phase 2 test programs

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== TRS-80 Phase 2 Test Disk Creator ===${NC}"
echo

# Determine script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VASM_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TEST_DIR="$VASM_ROOT/tests/edtasm-m80/phase2"
OUTPUT_DIR="$VASM_ROOT/disk-images"
OUTPUT_FILE="$OUTPUT_DIR/phase2_tests.dmk"

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Step 1: Build assembler if needed
echo -e "${GREEN}Step 1: Checking assembler...${NC}"
if [ ! -f "$VASM_ROOT/vasmz80_edtasm-m80" ]; then
    echo "  Building assembler..."
    cd "$VASM_ROOT"
    make CPU=z80 SYNTAX=edtasm-m80
else
    echo "  Assembler already built: vasmz80_edtasm-m80"
fi
echo

# Step 2: Assemble test programs
echo -e "${GREEN}Step 2: Assembling test programs...${NC}"
cd "$TEST_DIR"

# Count .asm files
ASM_COUNT=$(ls -1 test_*.asm 2>/dev/null | wc -l)
echo "  Found $ASM_COUNT test files"

# Assemble each test to .cmd format
for test in test_*.asm; do
    if [ -f "$test" ]; then
        base="${test%.asm}"
        echo "    Assembling: $test -> ${base}.cmd"
        "$VASM_ROOT/vasmz80_edtasm-m80" -Fcmd -Iinc -o "${base}.cmd" "$test" 2>&1 | grep -E "(error|warning)" || true
    fi
done

# Count .cmd files created
CMD_COUNT=$(ls -1 test_*.cmd 2>/dev/null | wc -l)
echo "  Created $CMD_COUNT /CMD files"
echo

# Step 3: Create disk image
echo -e "${GREEN}Step 3: Creating disk image...${NC}"
cd "$SCRIPT_DIR"

# Check if Python script exists
if [ ! -f "create_test_disk.py" ]; then
    echo "  ERROR: create_test_disk.py not found!"
    exit 1
fi

# Create the disk
python3 create_test_disk.py "$TEST_DIR" "$OUTPUT_FILE"
echo

# Step 4: List disk contents
echo -e "${GREEN}Step 4: Disk contents:${NC}"
if [ -f "$OUTPUT_FILE" ]; then
    ls -lh "$OUTPUT_FILE"
    echo
    echo "Disk created: $OUTPUT_FILE"
    echo
fi

# Step 5: Usage instructions
echo -e "${GREEN}Step 5: How to use the disk:${NC}"
echo
echo "To test in trs80gp emulator:"
echo "  trs80gp -m3 -disk0 $OUTPUT_FILE"
echo
echo "To test in sdltrs emulator:"
echo "  sdltrs -model 3 -disk0 $OUTPUT_FILE"
echo
echo "Commands in emulator:"
echo "  DIR                  - List files"
echo "  TEST_CMD_BASIC       - Run basic /CMD test"
echo "  TEST_MACRO_PARAMS    - Run macro parameter test"
echo "  TEST_REPT            - Run REPT directive test"
echo
echo -e "${BLUE}=== Complete! ===${NC}"
