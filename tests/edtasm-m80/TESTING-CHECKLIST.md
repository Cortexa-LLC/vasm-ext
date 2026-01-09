# Testing Checklist for Phase 1 Implementation

**For:** Engineer implementing Phase 1
**Purpose:** Step-by-step testing guide during implementation
**Status:** Ready to use

---

## Pre-Implementation Checklist

**Before starting implementation, verify:**

- [ ] Read Phase 1 contract: `.ai/tasks/2026-01-08_phase1-foundation-m80/00-contract.md`
- [ ] Read PRD: `.ai/tasks/2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md`
- [ ] Read architecture: `.ai/tasks/2026-01-08_z80-edtasm-research/architecture-m80-assessment.md`
- [ ] Test suite exists: `tests/edtasm-m80/` (11 test files)
- [ ] Build system ready: `make CPU=z80 SYNTAX=edtasm-m80` works (will fail initially)

---

## Day 1: Fork and Build

### Step 1.1: Fork EDTASM to EDTASM-M80

```bash
# Create new syntax module
cp -r syntax/edtasm syntax/edtasm-m80

# Test build (will fail if CPU=z80 doesn't work with syntax)
make CPU=z80 SYNTAX=edtasm-m80
```

**Checklist:**
- [ ] New directory created: `syntax/edtasm-m80/`
- [ ] Files present: `syntax.c`, `syntax.h`, `syntax_errors.h`
- [ ] Build completes successfully (or shows expected errors)
- [ ] Binary created: `vasmz80_edtasm-m80`

### Step 1.2: Critical Endianness Test

```bash
# Create minimal test
echo "  ORG 1000H" > /tmp/test_end.asm
echo "  DEFW 1234H" >> /tmp/test_end.asm
echo "  END" >> /tmp/test_end.asm

# Assemble
./vasmz80_edtasm-m80 -Fbin -o /tmp/test_end.bin /tmp/test_end.asm

# Check byte order (CRITICAL!)
hexdump -C /tmp/test_end.bin
```

**Expected:** `34 12` (little-endian)
**If you see:** `12 34` (big-endian) - STOP! Fix endianness before continuing!

**Checklist:**
- [ ] Test assembles successfully
- [ ] Output shows `34 12` (little-endian) ✓
- [ ] NOT `12 34` (big-endian) ✗

---

## Week 1: Core Directives

### Day 2-3: Strip 6809-Specific Code

**Changes to make:**
- [ ] Remove asterisk comment handling (`iscomment()`)
- [ ] Remove dollar sign from identifier chars (`isidchar()`)
- [ ] Remove `\\param` macro notation (`expand_macro()`)
- [ ] Update directive table (remove 6809-specific)

**Test after each change:**
```bash
make CPU=z80 SYNTAX=edtasm-m80 clean
make CPU=z80 SYNTAX=edtasm-m80
./vasmz80_edtasm-m80 --help
```

**Checklist:**
- [ ] Build succeeds after each change
- [ ] `--help` output shows correct syntax name

### Day 4: Implement Core Data Directives

**Implement:**
- [ ] DEFB (rename from FCB if needed)
- [ ] DEFW (rename from FDB, ensure little-endian!)
- [ ] DEFH (alias to DEFW)
- [ ] DEFS (rename from RMB)
- [ ] DEFM (simplify from FCC, quote-only)

**Run tests:**
```bash
cd tests/edtasm-m80
python3 run_tests.py --verbose
```

**Checklist:**
- [ ] `test_directives.asm` passes
- [ ] `test_data.asm` passes
- [ ] `test_endianness.asm` passes (CRITICAL!)

### Day 5: Implement M80 Aliases

**Implement:**
- [ ] DB → DEFB
- [ ] DW → DEFW
- [ ] DS → DEFS
- [ ] DC → DEFB (or similar)

**Test:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_aliases.asm
hexdump -C test.bin
```

**Checklist:**
- [ ] `test_aliases.asm` passes
- [ ] DW produces little-endian output

---

## Week 2: Segments and Basic Conditionals

### Day 6-7: Implement M80 Segments

**Implement:**
- [ ] ASEG (absolute segment)
- [ ] CSEG (code segment, default)
- [ ] DSEG (data segment)

**Test:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_segments.asm
# Check for multiple segments in output
```

**Checklist:**
- [ ] `test_segments.asm` passes
- [ ] CSEG is default segment
- [ ] ASEG creates absolute segment at ORG address
- [ ] DSEG creates separate data segment
- [ ] Can switch between segments

### Day 8-10: Implement Basic Conditionals

**Implement:**
- [ ] COND directive
- [ ] ENDC directive
- [ ] Conditional stack (if not present)
- [ ] Nested conditionals

**Test:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_conditionals.asm
hexdump -C test.bin
# Should show: 01 02 03
```

**Checklist:**
- [ ] `test_conditionals.asm` passes
- [ ] True conditions include code
- [ ] False conditions exclude code
- [ ] Nested conditionals work
- [ ] `test_nested_cond.asm` passes (10-15 levels)

---

## Week 3: Advanced Conditionals

### Day 11-13: Implement IF/IFE/IF1/IF2

**Implement:**
- [ ] IF directive (expression != 0)
- [ ] IFE directive (expression == 0)
- [ ] IF1 directive (pass 1 only)
- [ ] IF2 directive (pass 2+)
- [ ] ENDIF directive

**Test:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_m80_cond.asm
hexdump -C test.bin
# Check for pass 2+ output
```

**Checklist:**
- [ ] IF expression evaluation works
- [ ] IFE expression evaluation works
- [ ] IF1 only includes in pass 1
- [ ] IF2 only includes in pass 2+
- [ ] ENDIF terminates conditional

### Day 14: Implement IFDEF/IFNDEF

**Implement:**
- [ ] IFDEF directive (symbol defined)
- [ ] IFNDEF directive (symbol not defined)
- [ ] Symbol table lookup integration

**Test:**
```bash
echo "SYM EQU 1" > /tmp/test_ifdef.asm
echo "  IFDEF SYM" >> /tmp/test_ifdef.asm
echo "  DEFB 1" >> /tmp/test_ifdef.asm
echo "  ENDIF" >> /tmp/test_ifdef.asm
echo "  IFNDEF UNDEF" >> /tmp/test_ifdef.asm
echo "  DEFB 2" >> /tmp/test_ifdef.asm
echo "  ENDIF" >> /tmp/test_ifdef.asm
echo "  END" >> /tmp/test_ifdef.asm

./vasmz80_edtasm-m80 -Fbin -o /tmp/test_ifdef.bin /tmp/test_ifdef.asm
hexdump -C /tmp/test_ifdef.bin
# Should show: 01 02
```

**Checklist:**
- [ ] IFDEF works with defined symbols
- [ ] IFDEF excludes with undefined symbols
- [ ] IFNDEF works with undefined symbols
- [ ] IFNDEF excludes with defined symbols

### Day 15: Implement IFB/IFNB/IFIDN/IFDIF/ELSE

**Implement:**
- [ ] IFB directive (blank argument)
- [ ] IFNB directive (non-blank argument)
- [ ] IFIDN directive (identical strings)
- [ ] IFDIF directive (different strings)
- [ ] ELSE directive

**Test:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_m80_cond.asm
hexdump -C test.bin
# Verify all conditional types work
```

**Checklist:**
- [ ] IFB/IFNB work (typically for macros)
- [ ] IFIDN compares strings correctly
- [ ] IFDIF compares strings correctly
- [ ] ELSE clause works in IF/ENDIF
- [ ] `test_m80_cond.asm` passes completely

---

## Week 4: Integration Testing

### Day 16-17: Run Full Test Suite

**Run all tests:**
```bash
cd tests/edtasm-m80
python3 run_tests.py
```

**Checklist:**
- [ ] `test_directives.asm` passes
- [ ] `test_data.asm` passes
- [ ] `test_aliases.asm` passes
- [ ] `test_segments.asm` passes
- [ ] `test_conditionals.asm` passes
- [ ] `test_m80_cond.asm` passes
- [ ] `test_endianness.asm` passes (CRITICAL!)
- [ ] `test_nested_cond.asm` passes
- [ ] `test_case_insensitive.asm` passes
- [ ] `test_z80_instructions.asm` passes
- [ ] `test_comprehensive.asm` passes

**All tests pass:** ✓ 11/11 = 100%

### Day 18: Case-Insensitive Mode

**Verify:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_case_insensitive.asm
hexdump -C test.bin
```

**Checklist:**
- [ ] Labels case-insensitive
- [ ] Directives case-insensitive
- [ ] Instructions case-insensitive
- [ ] Symbols case-insensitive
- [ ] String contents preserve case

### Day 19: Z80 Integration

**Verify:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_z80_instructions.asm
hexdump -C test.bin
```

**Checklist:**
- [ ] All Z80 instructions assemble correctly
- [ ] EQU constants work with instructions
- [ ] Labels work with jumps/calls
- [ ] Data mixed with code works

### Day 20: Comprehensive Test

**Verify:**
```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_comprehensive.asm
```

**Checklist:**
- [ ] Multi-segment assembly works
- [ ] All directive types work together
- [ ] Conditionals work with segments
- [ ] Complex program assembles correctly

---

## Week 5: Quality Assurance

### Day 21: Memory Leak Testing

```bash
# Install valgrind if not present
# macOS: brew install valgrind
# Linux: sudo apt-get install valgrind

cd tests/edtasm-m80
for test in test_*.asm; do
  echo "Testing $test..."
  valgrind --leak-check=full --error-exitcode=1 \
    ../../vasmz80_edtasm-m80 -Fbin -o /tmp/out.bin "$test" 2>&1 | \
    grep -E "(definitely lost|ERROR SUMMARY)"
done
```

**Checklist:**
- [ ] No memory leaks in any test
- [ ] All "definitely lost" = 0 bytes

### Day 22: Code Coverage

```bash
# Rebuild with coverage
make CPU=z80 SYNTAX=edtasm-m80 clean
CFLAGS="--coverage" make CPU=z80 SYNTAX=edtasm-m80

# Run tests
cd tests/edtasm-m80
python3 run_tests.py

# Check coverage
cd ../..
gcov syntax/edtasm-m80/syntax.c
```

**Target:** 80-90% coverage for new code

**Checklist:**
- [ ] Coverage report generated
- [ ] New code >80% covered
- [ ] Critical paths 100% covered
- [ ] Edge cases tested

### Day 23: Performance Testing

```bash
# Time each test
cd tests/edtasm-m80
time python3 run_tests.py
```

**Target:** <10 seconds for all 11 tests

**Checklist:**
- [ ] All tests complete in <10 seconds
- [ ] No test takes >2 seconds individually
- [ ] Performance acceptable

### Day 24: Cross-Platform Testing

**Test on available platforms:**

**macOS:**
```bash
make CPU=z80 SYNTAX=edtasm-m80 clean
make CPU=z80 SYNTAX=edtasm-m80
cd tests/edtasm-m80 && python3 run_tests.py
```

**Linux:**
```bash
make CPU=z80 SYNTAX=edtasm-m80 clean
make CPU=z80 SYNTAX=edtasm-m80
cd tests/edtasm-m80 && python3 run_tests.py
```

**Windows (if available):**
```bash
make -f Makefile.Win32 CPU=z80 SYNTAX=edtasm-m80 clean
make -f Makefile.Win32 CPU=z80 SYNTAX=edtasm-m80
cd tests\edtasm-m80 && python run_tests.py
```

**Checklist:**
- [ ] Builds on macOS
- [ ] Builds on Linux
- [ ] Tests pass on macOS
- [ ] Tests pass on Linux
- [ ] Tests pass on Windows (if tested)

### Day 25: Documentation and Code Review

**Checklist:**
- [ ] Code follows C90 standards
- [ ] 2-space indentation used
- [ ] No C++ comments (`//`)
- [ ] No C99 features
- [ ] Comments explain complex logic
- [ ] Error messages are clear
- [ ] No magic numbers (use constants)
- [ ] Memory properly freed
- [ ] Edge cases handled

---

## Final Verification

### Critical Test: Endianness (One More Time!)

```bash
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_endianness.asm
hexdump -C test.bin | head -1
```

**MUST show:** `00000000  34 12 78 56 bc 9a cd ab ...`

**If shows:** `00000000  12 34 56 78 9a bc ab cd ...` ← CRITICAL BUG!

### Complete Test Suite Pass

```bash
cd tests/edtasm-m80
python3 run_tests.py
```

**Expected output:**
```
================================================
Z80 EDTASM-M80 Test Suite
================================================

Running tests...

Testing test_directives... PASS (6 bytes)
Testing test_data... PASS (27 bytes)
Testing test_aliases... PASS (10 bytes)
Testing test_segments... PASS
Testing test_conditionals... PASS (3 bytes)
Testing test_m80_cond... PASS (13 bytes)
Testing test_endianness... PASS (20 bytes)
Testing test_nested_cond... PASS (3 bytes)
Testing test_case_insensitive... PASS (11 bytes)
Testing test_z80_instructions... PASS (83 bytes)
Testing test_comprehensive... PASS

================================================
Test Summary
================================================
Total tests:  11
Passed:       11
Failed:       0

All tests passed!
```

---

## Sign-Off Checklist

**Phase 1 Implementation Complete When:**

- [ ] All 11 tests pass (11/11 = 100%)
- [ ] Endianness verified (little-endian)
- [ ] No memory leaks (valgrind clean)
- [ ] Code coverage >80% (new code)
- [ ] Performance acceptable (<10s for all tests)
- [ ] Cross-platform build successful
- [ ] Code follows C90 standards
- [ ] All 24 Phase 1 directives implemented
- [ ] Documentation updated (basic README)
- [ ] Ready for code review

**Sign-off:**
- [ ] Engineer: Implementation complete
- [ ] Tester: All tests pass
- [ ] Reviewer: Code quality acceptable
- [ ] Ready for Phase 2

---

**Notes Section (Use During Implementation):**

```
Date       Note
--------   --------------------------------------------------------
YYYY-MM-DD [Your notes here as you implement]




```

---

**Quick Commands Reference:**

```bash
# Build
make CPU=z80 SYNTAX=edtasm-m80

# Clean build
make CPU=z80 SYNTAX=edtasm-m80 clean
make CPU=z80 SYNTAX=edtasm-m80

# Run all tests
cd tests/edtasm-m80 && python3 run_tests.py

# Run single test
./vasmz80_edtasm-m80 -Fbin -o test.bin tests/edtasm-m80/test_directives.asm

# View binary
hexdump -C test.bin

# Check endianness (CRITICAL!)
hexdump -C test_endianness.bin | head -1

# Memory leak check
valgrind --leak-check=full ./vasmz80_edtasm-m80 -Fbin test.asm

# Code coverage
CFLAGS="--coverage" make CPU=z80 SYNTAX=edtasm-m80
cd tests/edtasm-m80 && python3 run_tests.py
gcov syntax/edtasm-m80/syntax.c
```

---

**Prepared by:** Tester Agent
**For:** Phase 1 Engineer
**Date:** 2026-01-09
**Status:** Ready to Use
