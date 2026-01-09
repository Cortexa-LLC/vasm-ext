# Phase 1 Implementation: Z80 EDTASM-M80 Foundation

**Task Packet:** 2026-01-08_phase1-foundation-m80
**Phase:** 1 of 4 (Foundation + M80 Basics)
**Duration:** 4-5 weeks
**Role:** Engineer
**Status:** Ready to Start

---

## Quick Start for Engineer

### 1. Read These Documents (Priority Order)

**Essential Reading (MUST READ FIRST):**
1. `00-contract.md` (this task packet) - Your Phase 1 contract
2. `../2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md` - Full requirements
3. `../2026-01-08_z80-edtasm-research/architecture-m80-assessment.md` - Technical guidance

**Reference Documentation:**
4. `../2026-01-08_z80-edtasm-research/architecture-response.md` - Original architecture
5. `syntax/edtasm/syntax.c` - 6809 EDTASM reference implementation
6. `.ai-pack/roles/engineer.md` - Engineer role definition
7. `.ai/repo-overrides.md` - C90 specific rules (CRITICAL!)

### 2. Understand the Scope

**What You're Building (Phase 1):**
- ✅ Syntax module: `syntax/edtasm-m80/` (fork from syntax/edtasm/)
- ✅ 24 directives: Core EDTASM (9) + M80 data aliases (4) + M80 segments (3) + Conditionals (8)
- ✅ Semicolon-only comments
- ✅ Case-insensitive mode
- ✅ Flat binary output

**What You're NOT Building (Later Phases):**
- ❌ Advanced macros (LOCAL/REPT/IRP/IRPC) - Phase 2
- ❌ /CMD output format - Phase 2
- ❌ INCLUDE directive - Phase 2
- ❌ .PHASE/.DEPHASE - Phase 3
- ❌ Full documentation - Phase 4

### 3. Implementation Strategy (from Architect)

```bash
# Week 1: Fork and Foundation
Day 1:   Fork 6809 EDTASM → syntax/edtasm-m80/
         Test build works
         CRITICAL: Test DEFW endianness (must be little-endian!)

Day 2-3: Strip 6809-specific code:
         - Remove asterisk comment handling
         - Remove dollar sign in identifiers
         - Remove \\param macro code
         - Update directive names (FCB→DEFB, etc.)

Day 4-5: Implement core directives:
         - DEFM (quote-only, simpler than FCC)
         - Add DB/DW/DS/DC aliases
         - Test each directive

# Week 2: Segments and Basic Conditionals
Day 6-7: Implement M80 segments:
         - ASEG/CSEG/DSEG using vasm sections
         - Test multi-segment assembly

Day 8-10: Implement basic conditionals:
          - COND/ENDC (copy from 6809)
          - Test nested conditionals

# Week 3: Advanced Conditionals
Day 11-15: Implement M80 advanced conditionals:
           - IF/IFE (expression eval)
           - IF1/IF2 (pass tracking)
           - IFDEF/IFNDEF (symbol table lookup)
           - IFB/IFNB (argument testing)
           - IFIDN/IFDIF (string comparison)
           - ELSE/ENDIF
           - Test all variants + nesting

# Week 4-5: Testing and Polish
Day 16-20: Create comprehensive test suite:
           - Core directive tests
           - Segment tests
           - Conditional tests
           - Endianness tests
           - Performance tests

Day 21-25: Bug fixes, code review, polish
```

### 4. Critical Implementation Notes

**DEFW Endianness (MUST TEST DAY 1!):**
```asm
; Test case - CRITICAL!
  ORG $1000
  DEFW $1234
  END

; Expected output: 34 12 (little-endian for Z80)
; NOT: 12 34 (big-endian would be wrong!)
```

**Segment Management:**
- Use `new_section()` with `SEC_ABSOLUTE` flag
- ASEG: Absolute segment (org required)
- CSEG: Code segment (default, relocatable without org)
- DSEG: Data segment (separate area)

**Advanced Conditionals:**
- Extend existing conditional stack from 6809 EDTASM
- Use global `pass` variable for IF1/IF2
- Use `find_symbol()` for IFDEF/IFNDEF
- Use `strcmp()` for IFIDN/IFDIF

### 5. Build and Test Commands

```bash
# Build Phase 1
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80

# Test binary exists
./vasmz80_edtasm-m80 --help

# Run simple test
echo "  ORG \$1000" > test.asm
echo "  DEFB 1,2,3" >> test.asm
echo "  END" >> test.asm
./vasmz80_edtasm-m80 -Fbin -o test.bin test.asm

# Check output
hexdump -C test.bin
# Should show: 01 02 03

# Test DEFW endianness (CRITICAL!)
echo "  ORG \$1000" > test_defw.asm
echo "  DEFW \$1234" >> test_defw.asm
echo "  END" >> test_defw.asm
./vasmz80_edtasm-m80 -Fbin -o test_defw.bin test_defw.asm
hexdump -C test_defw.bin
# MUST show: 34 12 (little-endian!)
# If shows: 12 34 - BUG! Fix immediately!

# Run test suite (create as you go)
make CPU=z80 SYNTAX=edtasm-m80 test

# Check for memory leaks
valgrind --leak-check=full ./vasmz80_edtasm-m80 -Fbin test.asm

# Code coverage
gcc --coverage ...
make test
gcov syntax/edtasm-m80/syntax.c
```

---

## Phase 1 Directive List (24 directives)

### Core EDTASM (9 directives)
1. `ORG` - Set origin
2. `EQU` - Constant symbol
3. `DEFL` - Redefinable symbol (alias to SET)
4. `END` - End program
5. `DEFB` - Define bytes
6. `DEFW` - Define words (16-bit, little-endian!)
7. `DEFH` - Define halfwords (alias for DEFW)
8. `DEFS` - Reserve space
9. `DEFM` - Define message (quote-only)

### M80 Data Aliases (4 directives)
10. `DB` - Alias for DEFB
11. `DW` - Alias for DEFW
12. `DS` - Alias for DEFS
13. `DC` - Define constant

### M80 Segments (3 directives)
14. `ASEG` - Absolute segment
15. `CSEG` - Code segment
16. `DSEG` - Data segment

### Basic Conditionals (2 directives)
17. `COND` - Conditional assembly
18. `ENDC` - End conditional

### M80 Advanced Conditionals (12 directives)
19. `IF` - If expression non-zero
20. `IFE` - If expression equals zero
21. `IF1` - If pass 1
22. `IF2` - If pass 2+
23. `IFDEF` - If symbol defined
24. `IFNDEF` - If symbol not defined
25. `IFB` - If argument blank
26. `IFNB` - If argument not blank
27. `IFIDN` - If arguments identical
28. `IFDIF` - If arguments different
29. `ELSE` - Else clause
30. `ENDIF` - End conditional

**Total: 24 directives in Phase 1**

---

## File Structure

```
syntax/edtasm-m80/
├── syntax.c           # Main implementation (~2000-2500 lines)
├── syntax.h           # Interface (~30 lines)
├── syntax_errors.h    # Error messages (~50 lines)
└── README.md          # Basic docs (full docs in Phase 4)

tests/edtasm-m80/
├── test_directives.s       # Core directive tests
├── test_data.s             # DEFB/DEFW/DEFS/DEFM tests
├── test_aliases.s          # DB/DW/DS/DC tests
├── test_segments.s         # ASEG/CSEG/DSEG tests
├── test_conditionals.s     # COND/ENDC tests
├── test_m80_cond.s         # IF variants tests
├── test_endianness.s       # DEFW little-endian test (CRITICAL!)
├── test_nested_cond.s      # Nested conditionals
└── test_z80_instructions.s # Z80 instruction integration
```

---

## Code Standards (CRITICAL!)

**ANSI C90 Compliance:**
```c
/* ✅ GOOD - C90 compliant */
int i;
char *s;
i = 10;
s = malloc(100);

/* ❌ BAD - C99 feature */
int i = 10;  /* Declaration with initialization - NO! */
for (int j = 0; ...) /* Declaration in for loop - NO! */
// Comment /* NO C++ comments! */
```

**2-Space Indentation (vasm style):**
```c
/* ✅ GOOD - 2 spaces */
if (condition) {
  do_something();
  if (nested) {
    do_more();
  }
}

/* ❌ BAD - 4 spaces (ai-pack default) */
if (condition) {
    do_something();  /* TOO MUCH INDENT! */
}
```

**See `.ai/repo-overrides.md` for complete C90 rules!**

---

## Success Indicators (Weekly Checkpoints)

**End of Week 1:**
- ✅ Build works: `make CPU=z80 SYNTAX=edtasm-m80`
- ✅ Binary runs: `./vasmz80_edtasm-m80 --help`
- ✅ Core directives work: ORG, EQU, DEFL, END, DEFB
- ✅ **DEFW endianness correct** (34 12, not 12 34)
- ✅ Basic test passes

**End of Week 2:**
- ✅ All data directives work: DEFB/DEFW/DEFH/DEFS/DEFM
- ✅ Data aliases work: DB/DW/DS/DC
- ✅ Segments work: ASEG/CSEG/DSEG
- ✅ Multi-segment test passes
- ✅ Basic conditionals work: COND/ENDC

**End of Week 3:**
- ✅ All M80 conditionals work: IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF
- ✅ Nested conditionals work (10+ levels)
- ✅ Pass tracking works (IF1/IF2)
- ✅ Symbol testing works (IFDEF/IFNDEF)
- ✅ String comparison works (IFIDN/IFDIF)

**End of Week 4:**
- ✅ All 24 directives tested
- ✅ Test suite ≥30 test cases
- ✅ Code coverage ≥80%
- ✅ No memory leaks
- ✅ Performance ≥10,000 lines/second

**Week 5 (if needed):**
- ✅ Bug fixes
- ✅ Code review
- ✅ Polish
- ✅ Ready for Phase 2

---

## Common Pitfalls (Avoid These!)

### 1. ❌ Wrong Endianness
```asm
  DEFW $1234
; WRONG: 12 34 (big-endian)
; RIGHT: 34 12 (little-endian for Z80)
```
**Fix:** Test immediately on Day 1!

### 2. ❌ Keeping 6809-Specific Code
```c
/* WRONG - 6809 asterisk comment */
if (*s == '*' && column == 1)
  return 1;  /* This is a comment */
```
**Fix:** Remove all asterisk comment code!

### 3. ❌ Allowing Dollar Signs
```asm
F$Link  EQU  1  ; WRONG - no dollar signs in Z80 EDTASM!
```
**Fix:** Reject $ in identifier parsing

### 4. ❌ Using C99 Features
```c
/* WRONG - C99 */
for (int i = 0; i < 10; i++)

/* RIGHT - C90 */
int i;
for (i = 0; i < 10; i++)
```
**Fix:** Read `.ai/repo-overrides.md`!

### 5. ❌ Forgetting Case-Insensitive Default
```c
/* WRONG - case-sensitive default */
void syntax_init(void) {
  /* nocase not set */
}

/* RIGHT - case-insensitive default */
void syntax_init(void) {
  nocase = 1;  /* M80/EDTASM default */
}
```
**Fix:** Set `nocase=1` in `syntax_init()`

---

## Questions? Blockers?

**If you encounter issues:**

1. **Check architecture documents:**
   - `architecture-response.md` - Original architecture
   - `architecture-m80-assessment.md` - M80 additions

2. **Check 6809 EDTASM reference:**
   - `syntax/edtasm/syntax.c` - Working implementation
   - Similar patterns, different directives

3. **Consult with Architect:**
   - Create issue in task packet
   - Document blocker in 20-work-log.md
   - Request Architect consultation if needed

4. **Don't block on unknowns:**
   - Implement what you know first
   - Mark TODOs for uncertain areas
   - Test incrementally
   - Ask for clarification early

---

## Definition of Done (Phase 1)

Phase 1 is **DONE** when:

```
✅ All 24 directives implemented
✅ All acceptance criteria met (see 00-contract.md)
✅ Test suite passes (≥30 test cases)
✅ Code coverage ≥80%
✅ No compiler warnings
✅ No memory leaks
✅ Performance ≥10,000 lines/second
✅ DEFW outputs little-endian (critical!)
✅ Multi-segment assembly works
✅ Nested conditionals work
✅ Code reviewed
✅ Ready for Phase 2 (advanced macros)
```

---

## Timeline Reminder

**Phase 1:** 4-5 weeks (YOU ARE HERE)
**Phase 2:** 3-4 weeks (Advanced Macros + /CMD output)
**Phase 3:** 2-3 weeks (Control directives + polish)
**Phase 4:** 2 weeks (Documentation + validation)
**Total:** 11-13 weeks for complete EDTASM-M80 assembler

---

## Related Documentation

**In This Task Packet:**
- `00-contract.md` - Full Phase 1 contract
- `10-plan.md` - Implementation plan (fill this out as you work)
- `20-work-log.md` - Daily progress log
- `30-review.md` - Code review checklist
- `40-acceptance.md` - Final acceptance validation

**Parent Task (Requirements):**
- `../2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md`
- `../2026-01-08_z80-edtasm-research/architecture-m80-assessment.md`
- `../2026-01-08_z80-edtasm-research/SUMMARY.md`

**Reference:**
- `.ai/repo-overrides.md` - C90 rules (MUST READ!)
- `.ai-pack/roles/engineer.md` - Your role definition
- `.ai-pack/workflows/feature.md` - Feature workflow
- `syntax/edtasm/` - 6809 EDTASM reference

---

**Status:** ✅ READY TO START
**Engineer:** [Your Name] (update when you start)
**Start Date:** [Date] (update when you start)
**Target Completion:** 4-5 weeks from start date

**Good luck! 🚀**
