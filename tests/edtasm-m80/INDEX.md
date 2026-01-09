# Z80 EDTASM-M80 Test Suite - Index

**Version:** 1.0 (Phase 1)
**Created:** 2026-01-09
**Status:** Ready for Implementation
**Total Lines:** ~2,600

---

## Quick Navigation

| Document | Purpose | For |
|----------|---------|-----|
| **[README.md](README.md)** | Complete test suite documentation | Everyone |
| **[QUICKREF.md](QUICKREF.md)** | Quick reference card | Engineer |
| **[TEST-MANIFEST.md](TEST-MANIFEST.md)** | Detailed test case listing | Tester |
| **[TESTING-CHECKLIST.md](TESTING-CHECKLIST.md)** | Step-by-step implementation guide | Engineer |
| **[run_tests.py](run_tests.py)** | Test runner script | Automation |

---

## Test Files Overview

### Core Directive Tests (3 files)

| File | Lines | Features | Cases |
|------|-------|----------|-------|
| [test_directives.asm](test_directives.asm) | 43 | ORG, EQU, DEFL, END | 6 |
| [test_data.asm](test_data.asm) | 54 | DEFB, DEFW, DEFH, DEFS, DEFM | 8 |
| [test_aliases.asm](test_aliases.asm) | 39 | DB, DW, DS, DC | 5 |

### Segment Tests (1 file)

| File | Lines | Features | Cases |
|------|-------|----------|-------|
| [test_segments.asm](test_segments.asm) | 64 | ASEG, CSEG, DSEG | 7 |

### Conditional Tests (3 files)

| File | Lines | Features | Cases |
|------|-------|----------|-------|
| [test_conditionals.asm](test_conditionals.asm) | 45 | COND, ENDC | 6 |
| [test_m80_cond.asm](test_m80_cond.asm) | 149 | IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF | 20 |
| [test_nested_cond.asm](test_nested_cond.asm) | 100 | Deep nesting (10-15 levels) | 5 |

### Integration Tests (4 files)

| File | Lines | Features | Cases |
|------|-------|----------|-------|
| [test_endianness.asm](test_endianness.asm) | 91 | ⚠️ CRITICAL: Little-endian verification | 10 |
| [test_case_insensitive.asm](test_case_insensitive.asm) | 50 | Case-insensitive mode | 8 |
| [test_z80_instructions.asm](test_z80_instructions.asm) | 124 | Z80 instruction integration | 10 |
| [test_comprehensive.asm](test_comprehensive.asm) | 121 | All Phase 1 features combined | 10 |

**Total:** 11 test files, 880 lines, 85+ test cases

---

## Test Coverage Summary

### Phase 1 Directives (24 total)

**Core EDTASM (9):**
```
✓ ORG     - Set origin address
✓ EQU     - Define constant symbol
✓ DEFL    - Define redefinable symbol
✓ END     - End assembly
✓ DEFB    - Define bytes
✓ DEFW    - Define words (little-endian)
✓ DEFH    - Define halfwords (alias)
✓ DEFS    - Reserve space
✓ DEFM    - Define message string
```

**M80 Data Aliases (4):**
```
✓ DB      - Alias for DEFB
✓ DW      - Alias for DEFW
✓ DS      - Alias for DEFS
✓ DC      - Define constant
```

**M80 Segments (3):**
```
✓ ASEG    - Absolute segment
✓ CSEG    - Code segment
✓ DSEG    - Data segment
```

**Conditionals (8):**
```
✓ COND    - Basic conditional
✓ ENDC    - End COND
✓ IF      - If expression != 0
✓ IFE     - If expression == 0
✓ IF1     - If pass 1
✓ IF2     - If pass 2+
✓ IFDEF   - If symbol defined
✓ IFNDEF  - If symbol not defined
✓ IFB     - If blank
✓ IFNB    - If not blank
✓ IFIDN   - If identical
✓ IFDIF   - If different
✓ ELSE    - Else clause
✓ ENDIF   - End IF
```

**Coverage:** 24/24 directives = **100%**

---

## Test Execution

### Quick Start

```bash
# Build assembler
make CPU=z80 SYNTAX=edtasm-m80

# Run all tests
cd tests/edtasm-m80
python3 run_tests.py
```

### Expected Results

```
Total tests:  11
Passed:       11  ✓
Failed:       0
```

### Critical Verification

```bash
# MUST verify little-endian encoding!
./vasmz80_edtasm-m80 -Fbin -o test_endianness.bin test_endianness.asm
hexdump -C test_endianness.bin | head -1

# Expected: 00000000  34 12 78 56 bc 9a cd ab ...
# NOT:      00000000  12 34 56 78 9a bc ab cd ...
```

---

## Documentation Structure

### For Different Roles

**Engineer Implementing Phase 1:**
1. Start with: [TESTING-CHECKLIST.md](TESTING-CHECKLIST.md)
2. Reference: [QUICKREF.md](QUICKREF.md)
3. Details: [TEST-MANIFEST.md](TEST-MANIFEST.md)

**Tester Verifying Implementation:**
1. Start with: [TEST-MANIFEST.md](TEST-MANIFEST.md)
2. Run: [run_tests.py](run_tests.py)
3. Reference: [README.md](README.md)

**Reviewer Checking Quality:**
1. Start with: [README.md](README.md)
2. Check: Test coverage in [TEST-MANIFEST.md](TEST-MANIFEST.md)
3. Verify: Results match expected outputs

**Project Manager:**
1. Overview: [README.md](README.md)
2. Status: Run test suite, check pass rate

---

## File Statistics

| Category | Files | Lines | Percentage |
|----------|-------|-------|------------|
| Test Files (.asm) | 11 | 880 | 33.7% |
| Documentation (.md) | 5 | 1,600 | 61.3% |
| Automation (.py) | 1 | 133 | 5.0% |
| **Total** | **17** | **2,613** | **100%** |

---

## Integration with Phase 1 Task Packet

This test suite integrates with:

- **Contract:** `.ai/tasks/2026-01-08_phase1-foundation-m80/00-contract.md`
- **Plan:** `.ai/tasks/2026-01-08_phase1-foundation-m80/10-plan.md`
- **PRD:** `.ai/tasks/2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md`
- **Architecture:** `.ai/tasks/2026-01-08_z80-edtasm-research/architecture-m80-assessment.md`

---

## Next Steps

### For Engineer Starting Implementation:

1. **Day 1:** Fork EDTASM → EDTASM-M80
2. **Day 1:** Run critical endianness test
3. **Week 1:** Implement core directives, run tests
4. **Week 2:** Implement segments, run tests
5. **Week 3:** Implement conditionals, run tests
6. **Week 4-5:** Full test suite, QA, polish

### Success Criteria:

- [ ] All 11 tests pass (100%)
- [ ] Endianness verified (little-endian)
- [ ] No memory leaks
- [ ] Code coverage >80%
- [ ] Performance <10s total
- [ ] Code review approved

---

## Visual Test Coverage Map

```
Phase 1 Features (24 directives)
├── Core EDTASM (9)
│   ├── test_directives.asm ───┐
│   └── test_data.asm ──────────┤
│                               │
├── M80 Aliases (4)             ├──> All covered
│   └── test_aliases.asm ───────┤
│                               │
├── M80 Segments (3)            │
│   └── test_segments.asm ──────┤
│                               │
└── Conditionals (8)            │
    ├── test_conditionals.asm ──┤
    ├── test_m80_cond.asm ──────┤
    └── test_nested_cond.asm ───┘

Integration Tests
├── test_endianness.asm ────> ⚠️ CRITICAL: Endianness verification
├── test_case_insensitive.asm > Case handling
├── test_z80_instructions.asm > Z80 integration
└── test_comprehensive.asm ──> All features together

Documentation
├── README.md ────────────────> Complete documentation
├── QUICKREF.md ──────────────> Quick reference
├── TEST-MANIFEST.md ─────────> Detailed test listing
├── TESTING-CHECKLIST.md ─────> Implementation guide
└── INDEX.md ─────────────────> This file

Automation
└── run_tests.py ─────────────> Test runner
```

---

## Support

**Issues during implementation?**

1. Check test file comments for expected output
2. Review [TESTING-CHECKLIST.md](TESTING-CHECKLIST.md) for step-by-step guidance
3. Consult [QUICKREF.md](QUICKREF.md) for common commands
4. Read [README.md](README.md) for detailed test descriptions

**Questions about test design?**

1. Review [TEST-MANIFEST.md](TEST-MANIFEST.md) for test case details
2. Check Phase 1 contract for requirements
3. Consult PRD for feature specifications

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-09 | Initial test suite for Phase 1 |

---

**Prepared by:** Tester Agent
**Role:** Test Suite Designer
**For:** Phase 1 Z80 EDTASM-M80 Implementation
**Status:** ✓ Complete and Ready for Use
