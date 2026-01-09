# Z80 EDTASM-M80 Test Suite

**Syntax Module:** edtasm-m80
**CPU:** Z80
**Test Coverage:** Phase 1 (Foundation + M80 Basics)
**Total Test Files:** 10
**Total Test Cases:** ~40

---

## Overview

This test suite provides comprehensive coverage of Phase 1 features for the Z80 EDTASM-M80 assembler. The suite is designed to verify:

1. **Core EDTASM directives** (ORG, EQU, DEFL, END, DEFB, DEFW, DEFH, DEFS, DEFM)
2. **M80 data aliases** (DB, DW, DS, DC)
3. **M80 segments** (ASEG, CSEG, DSEG)
4. **Basic conditionals** (COND/ENDC)
5. **M80 advanced conditionals** (IF variants, ELSE, ENDIF)
6. **Case-insensitive mode** (default behavior)
7. **Z80 instruction integration**
8. **Little-endian word encoding** (CRITICAL!)

---

## Building and Running Tests

### Build the Assembler

```bash
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80
```

### Run All Tests

```bash
# Using make target (when implemented)
make CPU=z80 SYNTAX=edtasm-m80 test

# Manual execution
cd tests/edtasm-m80
./run_tests.sh
```

### Run Individual Test

```bash
# Assemble a test
./vasmz80_edtasm-m80 -Fbin -o tests/edtasm-m80/test_directives.bin tests/edtasm-m80/test_directives.asm

# View binary output
hexdump -C tests/edtasm-m80/test_directives.bin

# Compare with expected output (documented in test file)
```

---

## Test Files

### 1. test_directives.asm
**Purpose:** Core EDTASM directives (ORG, EQU, DEFL, END)
**Test Cases:** 5
- ORG directive sets origin correctly
- EQU defines constants
- DEFL creates redefinable symbols
- Multiple ORG directives create segments
- END directive stops assembly

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  01 02 03           At ORG $1000
$0003:  2A                 Using EQU constant (42)
$0004:  64                 Using DEFL value (100)
$0005:  C8                 Using redefined DEFL (200)
```

### 2. test_data.asm
**Purpose:** Data definition directives (DEFB, DEFW, DEFH, DEFS, DEFM)
**Test Cases:** 8
- DEFB single byte
- DEFB multiple bytes
- DEFW single word (little-endian)
- DEFW multiple words (little-endian)
- DEFH (alias for DEFW)
- DEFS reserves space
- DEFM with double quotes
- DEFM with single quotes

**Expected Output:**
```
Offset  Bytes                      Description
------  -------------------------  -----------
$0000:  42                         DEFB 42H
$0001:  01 02 03 04 05             DEFB 1,2,3,4,5
$0006:  34 12                      DEFW 1234H (little-endian!)
$0008:  78 56 BC 9A                DEFW 5678H,9ABCH
$000C:  CD AB                      DEFH 0ABCDH
$000E:  00 00 00 00 00             DEFS 5 (zeros)
$0013:  48 65 6C 6C 6F             DEFM "Hello"
$0018:  5A 38 30                   DEFM 'Z80'
```

### 3. test_aliases.asm
**Purpose:** M80 data directive aliases (DB, DW, DS, DC)
**Test Cases:** 5
- DB is alias for DEFB
- DW is alias for DEFW (little-endian!)
- DS is alias for DEFS
- DC defines constants
- Mixed aliases in same program

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  AA BB CC           DB 0AAH,0BBH,0CCH
$0003:  34 12              DW 1234H (little-endian!)
$0005:  00 00 00           DS 3
$0008:  2A                 DC 42
$0009:  78 56              DW 5678H
```

### 4. test_segments.asm
**Purpose:** M80 segment directives (ASEG, CSEG, DSEG)
**Test Cases:** 7
- ASEG creates absolute segment
- CSEG creates code segment
- DSEG creates data segment
- Multiple CSEG sections merge
- ORG in ASEG sets absolute address
- Switching between segments
- Default segment is CSEG

**Expected Output:**
```
Segment   Offset  Bytes              Description
--------  ------  -----------------  -----------
CSEG      $0000:  3E 42              CODE1: LD A,42H
DSEG      $0000:  00 00 00 00        BUFFER: DS 4
CSEG      $0002:  C9                 CODE2: RET (continues CSEG)
ASEG      $1000:  01 02 03           ABS1: DEFB 1,2,3 (at $1000)
```

### 5. test_conditionals.asm
**Purpose:** Basic conditional assembly (COND/ENDC)
**Test Cases:** 6
- COND with true expression includes code
- COND with false expression excludes code
- Nested COND (2 levels)
- Nested COND (5 levels)
- COND with expression evaluation
- Empty COND block

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  01                 Inside true COND
$0001:  02                 Nested true COND
$0002:  03                 After conditionals
```

### 6. test_m80_cond.asm
**Purpose:** M80 advanced conditionals (IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF)
**Test Cases:** 15
- IF with non-zero expression
- IFE with zero expression
- IF1 in first pass
- IF2 in subsequent passes
- IFDEF with defined symbol
- IFNDEF with undefined symbol
- IFB with blank argument
- IFNB with non-blank argument
- IFIDN with identical strings
- IFDIF with different strings
- ELSE clause
- ENDIF terminator
- Nested IF conditionals (3 levels)
- Mixed conditional types
- IF/ELSE/ENDIF chain

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  01                 IF 1 (true)
$0001:  02                 IFE 0 (true)
$0002:  03                 IF1 (pass 1 only)
$0003:  04                 IFDEF DEFINED (true)
$0004:  05                 IFNDEF UNDEFINED (true)
$0005:  06                 ELSE clause
$0006:  07                 IFIDN match
```

### 7. test_endianness.asm
**Purpose:** CRITICAL TEST - Verify little-endian word encoding
**Test Cases:** 10
- Single DEFW word
- Multiple DEFW words
- DW alias (must match DEFW)
- DEFH alias (must match DEFW)
- Maximum word value (0FFFFH)
- Minimum word value (0)
- Expression in DEFW
- Label address in DEFW
- Mixed bytes and words
- Word array

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  34 12              DEFW 1234H → 34 12 (little-endian!)
$0002:  78 56              DEFW 5678H → 78 56
$0004:  CD AB              DEFW 0ABCDH → CD AB
$0006:  FF FF              DEFW 0FFFFH → FF FF
$0008:  00 00              DEFW 0 → 00 00
$000A:  2A 00              DEFW 42 → 2A 00
$000C:  00 10              DEFW START → 00 10 ($1000)
```

**CRITICAL:** If any word shows big-endian encoding (12 34 instead of 34 12), the implementation is BROKEN and must be fixed immediately!

### 8. test_nested_cond.asm
**Purpose:** Deep nesting of conditionals
**Test Cases:** 3
- 10-level nested COND
- 10-level nested IF
- Mixed nesting (COND inside IF inside COND)

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  0A                 10 levels deep (COND)
$0001:  0A                 10 levels deep (IF)
$0002:  03                 Mixed nesting level 3
```

### 9. test_case_insensitive.asm
**Purpose:** Verify case-insensitive mode (default)
**Test Cases:** 8
- Labels case-insensitive
- Instructions case-insensitive
- Directives case-insensitive
- Symbols case-insensitive
- Mixed case in same program
- EQU symbols case-insensitive
- Segment names case-insensitive
- DEFM preserves string case

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  2A                 START = start = Start
$0001:  3E 42              LD = ld = Ld
$0003:  2A                 DEFB = defb = DeFb
$0004:  48 65 6C 6C 6F     DEFM preserves "Hello"
```

### 10. test_z80_instructions.asm
**Purpose:** Z80 instruction integration with directives
**Test Cases:** 10
- Basic 8-bit loads
- 16-bit loads
- Arithmetic operations
- Jumps and calls
- Stack operations
- I/O operations
- Block operations
- Bit operations
- Labels with instructions
- Instructions with EQU constants

**Expected Output:**
```
Offset  Bytes              Description
------  -----------------  -----------
$0000:  3E 42              LD A,42H
$0002:  06 10              LD B,16
$0004:  21 00 10           LD HL,1000H
$0007:  CD 20 00           CALL SUB1
$000A:  C9                 RET
...
(See test file for complete byte sequence)
```

---

## Test Execution

### Expected Behavior

All tests should:
1. Assemble without errors
2. Produce binary output matching documented expectations
3. Complete within reasonable time (<1 second per test)
4. Use case-insensitive mode by default

### Verification Steps

For each test:

1. **Assemble:**
   ```bash
   ./vasmz80_edtasm-m80 -Fbin -o test.bin test.asm
   ```

2. **Check exit code:**
   ```bash
   echo $?  # Should be 0 for success
   ```

3. **Verify byte count:**
   ```bash
   ls -l test.bin  # Should match expected size
   ```

4. **Inspect binary:**
   ```bash
   hexdump -C test.bin  # Should match expected bytes
   ```

5. **Compare with reference:**
   ```bash
   diff test.bin expected/test.bin  # If reference available
   ```

---

## Test Coverage Summary

| Category | Directives | Test Files | Test Cases |
|----------|-----------|------------|------------|
| Core EDTASM | 9 | 2 | 13 |
| M80 Aliases | 4 | 1 | 5 |
| M80 Segments | 3 | 1 | 7 |
| Basic Conditionals | 2 | 1 | 6 |
| M80 Conditionals | 10 | 1 | 15 |
| Endianness | 2 | 1 | 10 |
| Nesting | - | 1 | 3 |
| Case Handling | - | 1 | 8 |
| Z80 Integration | - | 1 | 10 |
| **Total** | **24** | **10** | **~77** |

---

## Known Limitations (Phase 1)

**Not Tested (Future Phases):**
- Advanced macros (LOCAL, REPT, IRP, IRPC) - Phase 2
- INCLUDE directive - Phase 2
- /CMD output format - Phase 2
- .PHASE/.DEPHASE - Phase 3
- Full documentation - Phase 4

**Not Supported (Phase 1):**
- Asterisk comments (only semicolons)
- Case-sensitive mode (only case-insensitive)
- Macro parameters

---

## Error Testing

**Intentional Failures** (for error handling verification):

Create these tests after basic tests pass:
- `test_error_undefined_symbol.asm` - Reference undefined label
- `test_error_unmatched_endc.asm` - ENDC without COND
- `test_error_unmatched_endif.asm` - ENDIF without IF
- `test_error_nested_overflow.asm` - >100 levels of nesting
- `test_error_invalid_expression.asm` - Malformed expression

---

## Performance Testing

**Stress Tests** (create after basic tests pass):

```bash
# Generate large test file
python3 generate_stress_test.py --lines 10000 --output test_stress.asm

# Assemble and time
time ./vasmz80_edtasm-m80 -Fbin -o test_stress.bin test_stress.asm
```

Expected performance:
- 10,000 lines: <1 second
- 100,000 lines: <10 seconds
- 1,000,000 lines: <2 minutes

---

## Test Maintenance

### Adding New Tests

1. Create test file: `test_<feature>.asm`
2. Add expected output documentation in comments
3. Update this README with test description
4. Add to `run_tests.sh`
5. Verify test passes
6. Commit test with implementation

### Test Naming Convention

```
test_<category>.asm         # General category test
test_<directive>.asm        # Single directive test
test_<feature>_edge.asm     # Edge case test
test_<feature>_error.asm    # Error condition test
test_<feature>_stress.asm   # Performance test
```

---

## References

- **Phase 1 Contract:** `.ai/tasks/2026-01-08_phase1-foundation-m80/00-contract.md`
- **PRD:** `.ai/tasks/2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md`
- **Architecture:** `.ai/tasks/2026-01-08_z80-edtasm-research/architecture-m80-assessment.md`
- **6809 EDTASM Reference:** `syntax/edtasm/syntax.c`
- **vasm Documentation:** `doc/vasm.texi`

---

**Last Updated:** 2026-01-09
**Test Suite Version:** 1.0 (Phase 1)
**Status:** Ready for Implementation
