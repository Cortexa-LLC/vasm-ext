# Z80 EDTASM-M80 Test Manifest

**Generated:** 2026-01-09
**Test Suite Version:** 1.0 (Phase 1)
**Total Test Files:** 11
**Total Test Cases:** ~85

---

## Test File 1: test_directives.asm

**Features Tested:** Core EDTASM directives (ORG, EQU, DEFL, END)

**Test Cases (6):**
1. ORG sets origin to specified address
2. EQU defines read-only constant symbol
3. DEFL creates redefinable symbol
4. EQU constant can be used in expressions
5. DEFL value can be used and redefined multiple times
6. END directive terminates assembly

**Expected Output:** 6 bytes
**Verification:** `01 02 03 2A 64 C8`

---

## Test File 2: test_data.asm

**Features Tested:** Data definition directives (DEFB, DEFW, DEFH, DEFS, DEFM)

**Test Cases (8):**
1. DEFB defines single byte
2. DEFB defines multiple bytes
3. DEFW defines single 16-bit word (little-endian)
4. DEFW defines multiple 16-bit words (little-endian)
5. DEFH is alias for DEFW
6. DEFS reserves space (filled with zeros)
7. DEFM with double-quote delimiter
8. DEFM with single-quote delimiter

**Expected Output:** 27 bytes
**Verification:** `42 01 02 03 04 05 34 12 78 56 BC 9A CD AB 00 00 00 00 00 48 65 6C 6C 6F 5A 38 30`

---

## Test File 3: test_aliases.asm

**Features Tested:** M80 data directive aliases (DB, DW, DS, DC)

**Test Cases (5):**
1. DB is functionally identical to DEFB
2. DW is functionally identical to DEFW (little-endian)
3. DS is functionally identical to DEFS
4. DC defines constant (similar to DEFB)
5. Mixed aliases work correctly in same program

**Expected Output:** 10 bytes
**Verification:** `AA BB CC 34 12 00 00 00 2A 78 56`

---

## Test File 4: test_segments.asm

**Features Tested:** M80 segment directives (ASEG, CSEG, DSEG)

**Test Cases (7):**
1. Default segment is CSEG (code segment)
2. CSEG directive creates/continues code segment
3. DSEG creates separate data segment
4. Multiple CSEG sections merge correctly
5. ASEG creates absolute segment at ORG address
6. Switching between segments maintains separate address counters
7. Segments can be interleaved

**Expected Output:** Multi-segment (CSEG: 4 bytes, DSEG: 6 bytes, ASEG: 3 bytes at $2000)

---

## Test File 5: test_conditionals.asm

**Features Tested:** Basic conditional assembly (COND/ENDC)

**Test Cases (6):**
1. COND with non-zero expression includes code
2. COND with zero expression excludes code
3. COND with complex expression (2+2) evaluates correctly
4. Nested COND (2 levels) works correctly
5. Nested COND with false inner condition excludes code
6. Empty COND block doesn't break assembly

**Expected Output:** 3 bytes
**Verification:** `01 02 03`

---

## Test File 6: test_m80_cond.asm

**Features Tested:** M80 advanced conditionals

**Test Cases (20):**
1. IF with non-zero expression (true case)
2. IF with zero expression (false case)
3. IFE with zero expression (true case)
4. IFE with non-zero expression (false case)
5. IF with expression evaluation (2+2)
6. IF1 includes code only in pass 1
7. IF2 includes code only in pass 2+
8. IFDEF with defined symbol (true case)
9. IFDEF with undefined symbol (false case)
10. IFNDEF with undefined symbol (true case)
11. IFNDEF with defined symbol (false case)
12. IFB with blank argument (true case)
13. IFNB with non-blank argument (true case)
14. IFIDN with identical strings (true case)
15. IFIDN with different strings (false case)
16. IFDIF with different strings (true case)
17. IFDIF with identical strings (false case)
18. ELSE clause executes when IF condition is false
19. Nested IF with ELSE works correctly
20. Multiple ELSE conditions chain correctly

**Expected Output:** 13 bytes (pass 2+)
**Verification:** `01 02 03 05 06 07 08 09 0A 0B 0C 0D 0E`

---

## Test File 7: test_endianness.asm ⚠️ CRITICAL

**Features Tested:** Little-endian word encoding verification

**Test Cases (10):**
1. DEFW produces little-endian byte order (34 12 not 12 34)
2. Multiple DEFW words maintain little-endian order
3. DW alias produces identical little-endian output
4. DEFH alias produces identical little-endian output
5. Maximum word value (0FFFFH) encodes correctly
6. Minimum word value (0) encodes correctly
7. Expression result in DEFW encodes as little-endian
8. Label address in DEFW encodes as little-endian
9. Mixing bytes and words maintains correct order
10. Word array maintains little-endian for all elements

**Expected Output:** 20 bytes
**Verification:** `34 12 78 56 BC 9A CD AB 01 EF FF FF 00 00 2A 00 00 10 AA CC BB`

**CRITICAL:** First word MUST be `34 12`, NOT `12 34`! Big-endian output indicates critical bug.

---

## Test File 8: test_nested_cond.asm

**Features Tested:** Deep nesting of conditionals

**Test Cases (5):**
1. 10-level nested COND (all true)
2. 10-level nested IF (all true)
3. Mixed nesting (COND inside IF inside COND)
4. False condition at deep level excludes code correctly
5. 15-level nesting stress test (verifies stack depth)

**Expected Output:** 3 bytes
**Verification:** `0A 0A 03`

---

## Test File 9: test_case_insensitive.asm

**Features Tested:** Case-insensitive mode (default)

**Test Cases (8):**
1. Labels with different cases resolve to same symbol
2. Directives with different cases work identically
3. Instructions with different cases assemble identically
4. EQU symbols are case-insensitive
5. Mixed case in same program works correctly
6. Segment names are case-insensitive
7. Case-insensitive throughout entire program
8. DEFM string contents preserve original case

**Expected Output:** 11 bytes
**Verification:** `2A 01 02 03 63 63 48 65 6C 6C 6F`

---

## Test File 10: test_z80_instructions.asm

**Features Tested:** Z80 instruction integration with directives

**Test Cases (10):**
1. Basic 8-bit loads (LD A/B/C/D/E/H/L, immediate)
2. 16-bit loads (LD BC/DE/HL/SP, immediate)
3. Memory operations (LD A,(HL), LD (HL),A, absolute addressing)
4. Arithmetic operations (ADD, ADC, SUB, SBC, AND, OR, XOR, CP)
5. Increment/Decrement (INC, DEC, 8-bit and 16-bit)
6. Jumps and calls (JP, JR, CALL, RET)
7. Stack operations (PUSH, POP)
8. I/O operations (IN, OUT)
9. Bit operations (BIT, SET, RES)
10. Data directives mixed with instructions

**Expected Output:** ~83 bytes
**Verification:** First bytes: `3E 42 06 10 0E 20 16 30 1E 40 26 50 2E 60...`

---

## Test File 11: test_comprehensive.asm

**Features Tested:** Integration of multiple Phase 1 features

**Combined Test Cases (10):**
1. Instructions using EQU constants
2. Conditional assembly with IFDEF
3. Nested conditionals (IF1/IF2 inside IF)
4. Data directives mixed with code
5. Conditional with ELSE clause
6. Data segment with space reservation
7. Code segment continuation after DSEG
8. Absolute segment at specific address
9. Multiple conditional types (IFNDEF, IFIDN)
10. DEFL redefinition and reuse

**Expected Output:** Multi-segment binary (~320 bytes total)

**Features Demonstrated:**
- All 24 Phase 1 directives
- Multi-segment assembly
- Case-insensitive mode
- Little-endian encoding
- Nested conditionals
- Z80 instruction integration

---

## Test Coverage Analysis

### Directives Tested

**Core EDTASM (9/9 = 100%):**
- ✓ ORG
- ✓ EQU
- ✓ DEFL
- ✓ END
- ✓ DEFB
- ✓ DEFW
- ✓ DEFH
- ✓ DEFS
- ✓ DEFM

**M80 Aliases (4/4 = 100%):**
- ✓ DB
- ✓ DW
- ✓ DS
- ✓ DC

**M80 Segments (3/3 = 100%):**
- ✓ ASEG
- ✓ CSEG
- ✓ DSEG

**Basic Conditionals (2/2 = 100%):**
- ✓ COND
- ✓ ENDC

**M80 Conditionals (10/10 = 100%):**
- ✓ IF
- ✓ IFE
- ✓ IF1
- ✓ IF2
- ✓ IFDEF
- ✓ IFNDEF
- ✓ IFB
- ✓ IFNB
- ✓ IFIDN
- ✓ IFDIF
- ✓ ELSE
- ✓ ENDIF

**Total: 24/24 Phase 1 directives = 100% coverage**

### Edge Cases Tested

- ✓ Deep nesting (15 levels)
- ✓ Empty conditional blocks
- ✓ Multiple segment switching
- ✓ Redefining DEFL symbols
- ✓ Expression evaluation in conditionals
- ✓ Label addresses in data
- ✓ Maximum word value (0FFFFH)
- ✓ Minimum word value (0)
- ✓ Mixed case identifiers
- ✓ String delimiter variants (single/double quotes)

### Integration Tests

- ✓ Data mixed with instructions
- ✓ Conditionals with expressions
- ✓ Multi-segment programs
- ✓ Nested conditionals (multiple types)
- ✓ Case-insensitive operation throughout
- ✓ Z80 instructions with directive-defined constants

---

## Running the Test Suite

### Quick Start

```bash
cd tests/edtasm-m80
python3 run_tests.py
```

### Expected Output (All Pass)

```
================================================
Z80 EDTASM-M80 Test Suite
================================================

Running tests...

Testing test_directives... PASS (6 bytes)
Testing test_data... PASS (27 bytes)
Testing test_aliases... PASS (10 bytes)
Testing test_segments... PASS (varies)
Testing test_conditionals... PASS (3 bytes)
Testing test_m80_cond... PASS (13 bytes)
Testing test_endianness... PASS (20 bytes)
Testing test_nested_cond... PASS (3 bytes)
Testing test_case_insensitive... PASS (11 bytes)
Testing test_z80_instructions... PASS (83 bytes)
Testing test_comprehensive... PASS (varies)

================================================
Test Summary
================================================
Total tests:  11
Passed:       11
Failed:       0

All tests passed!
```

---

## Test Files Not Included (Future Work)

### Error Condition Tests (Phase 1 - Later)
- `test_error_undefined_symbol.asm` - Reference undefined label
- `test_error_unmatched_endc.asm` - ENDC without COND
- `test_error_unmatched_endif.asm` - ENDIF without IF
- `test_error_nested_overflow.asm` - >100 levels of nesting
- `test_error_invalid_expression.asm` - Malformed expression
- `test_error_duplicate_label.asm` - Label defined twice
- `test_error_redefine_equ.asm` - Attempt to redefine EQU

### Stress Tests (Phase 1 - Later)
- `test_stress_large_file.asm` - 10,000+ lines
- `test_stress_deep_nesting.asm` - 50+ levels
- `test_stress_many_symbols.asm` - 1000+ symbols
- `test_stress_large_segment.asm` - 64KB segment

### Phase 2+ Features (Not in Phase 1)
- Macro tests (LOCAL, parameters, REPT, IRP, IRPC)
- INCLUDE directive tests
- /CMD output format tests
- .PHASE/.DEPHASE tests
- Advanced expression tests

---

## Success Criteria

**Phase 1 test suite is successful when:**

1. ✓ All 11 test files assemble without errors
2. ✓ All binary outputs match expected byte sequences
3. ✓ test_endianness.asm shows little-endian encoding
4. ✓ Multi-segment tests produce correct segment layout
5. ✓ Nested conditionals work up to 15 levels
6. ✓ Case-insensitive mode works throughout
7. ✓ All 24 Phase 1 directives are tested
8. ✓ Z80 instructions integrate correctly
9. ✓ Tests complete in <10 seconds total
10. ✓ No memory leaks (valgrind clean)

---

**Prepared by:** Tester Agent
**For:** Phase 1 Z80 EDTASM-M80 Implementation
**Reference:** `.ai/tasks/2026-01-08_phase1-foundation-m80/`
