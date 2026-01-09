# Z80 EDTASM-M80 Test Suite - Quick Reference

## Running Tests

```bash
# Build assembler first
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80

# Run all tests
cd tests/edtasm-m80
python3 run_tests.py

# Run with verbose output
python3 run_tests.py --verbose

# Run single test manually
../../vasmz80_edtasm-m80 -Fbin -o test.bin test_directives.asm
hexdump -C test.bin
```

## Test Coverage Matrix

| Test File | Features Tested | Critical? |
|-----------|----------------|-----------|
| `test_directives.asm` | ORG, EQU, DEFL, END | ✓ Core |
| `test_data.asm` | DEFB, DEFW, DEFH, DEFS, DEFM | ✓ Core |
| `test_aliases.asm` | DB, DW, DS, DC | ✓ M80 |
| `test_segments.asm` | ASEG, CSEG, DSEG | ✓ M80 |
| `test_conditionals.asm` | COND, ENDC | ✓ Basic |
| `test_m80_cond.asm` | IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF | ✓ M80 |
| `test_endianness.asm` | Little-endian word encoding | ✓✓ CRITICAL! |
| `test_nested_cond.asm` | Deep nesting (10-15 levels) | Edge case |
| `test_case_insensitive.asm` | Case-insensitive mode | ✓ Core |
| `test_z80_instructions.asm` | Z80 instruction integration | ✓ Integration |
| `test_comprehensive.asm` | All features combined | ✓ Integration |

## Expected Test Results (Pass 2+)

```
Test File                  Size (bytes)  Key Verification
-------------------------  ------------  ----------------
test_directives.asm        6             01 02 03 2A 64 C8
test_data.asm              27            42 01 02 03 04 05 34 12 78 56...
test_aliases.asm           10            AA BB CC 34 12 00 00 00 2A 78 56
test_segments.asm          varies        Multi-segment output
test_conditionals.asm      3             01 02 03
test_m80_cond.asm          13            01 02 03 05 06 07 08 09 0A 0B 0C 0D 0E
test_endianness.asm        20            34 12 78 56 BC 9A... (MUST be little-endian!)
test_nested_cond.asm       3             0A 0A 03
test_case_insensitive.asm  11            2A 01 02 03 63 63 48 65 6C 6C 6F
test_z80_instructions.asm  ~83           3E 42 06 10 0E 20...
test_comprehensive.asm     varies        Multi-feature integration
```

## Critical Verification: test_endianness.asm

**MUST CHECK MANUALLY:**

```bash
../../vasmz80_edtasm-m80 -Fbin -o test_endianness.bin test_endianness.asm
hexdump -C test_endianness.bin | head -2
```

**Expected output (first line):**
```
00000000  34 12 78 56 bc 9a cd ab  01 ef ff ff 00 00 2a 00  |4.xV..........*.|
```

**CRITICAL:** First word (1234H) MUST show as `34 12` (little-endian), NOT `12 34` (big-endian)!

If you see big-endian byte order, the CPU module or syntax module has a critical bug and must be fixed before proceeding.

## Test Failures - Common Issues

### Issue: Assembler not found
**Solution:** Build with `make CPU=z80 SYNTAX=edtasm-m80` from repo root

### Issue: Syntax errors
**Possible causes:**
- Semicolon comments not working
- Case-insensitive mode not enabled
- Directive names incorrect
- DEFM quote handling broken

### Issue: Wrong byte values
**Possible causes:**
- Endianness bug (check test_endianness.asm first!)
- Expression evaluation error
- Symbol resolution error

### Issue: Test hangs
**Possible causes:**
- Infinite loop in conditional evaluation
- Stack overflow in nested conditionals
- Multi-pass deadlock

### Issue: Segment errors
**Possible causes:**
- ASEG/CSEG/DSEG not creating proper sections
- ORG not working in ASEG
- Section merging broken

## Adding New Tests

1. Create `test_<feature>.asm` with documented expected output
2. Add to `tests` array in `run_tests.py`
3. Update `README.md` with test description
4. Run test suite to verify
5. Commit with feature implementation

## Test File Format

```asm
; Test: Feature Name
; File: test_feature.asm
; Purpose: What this test verifies
; Expected: N bytes output

  ORG 1000H

; Test case 1: Description
  ; Test code here

; Test case 2: Description
  ; More test code

  END

; Expected binary output (N bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  XX YY ZZ           What these bytes are
;
; Total: N bytes
```

## Performance Benchmarks

Target performance (when implementation complete):
- Simple test (<10 lines): <0.1s
- Medium test (100 lines): <0.5s
- Large test (1000 lines): <2s
- Comprehensive test: <1s

## Next Steps After Tests Pass

1. Review code coverage (target: 80-90% for new code)
2. Add error condition tests (undefined symbols, etc.)
3. Add stress tests (large files, deep nesting limits)
4. Performance profiling
5. Memory leak detection (valgrind)
6. Cross-platform testing (Linux, macOS, Windows)

---

**Quick Links:**
- Full documentation: `README.md`
- Phase 1 contract: `.ai/tasks/2026-01-08_phase1-foundation-m80/00-contract.md`
- Architecture: `.ai/tasks/2026-01-08_z80-edtasm-research/architecture-m80-assessment.md`
