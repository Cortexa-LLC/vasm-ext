# Phase 2 Implementation: Z80 EDTASM-M80 Advanced Macros + Output

**Task Packet:** 2026-01-09_phase2-macros-output
**Phase:** 2 of 4 (Advanced Macros + Output)
**Duration:** 3-4 weeks
**Role:** Engineer
**Status:** Ready to Start
**Prerequisites:** Phase 1 COMPLETE (24 directives working)

---

## Quick Start for Engineer

### 1. Read These Documents (Priority Order)

**Essential Reading (MUST READ FIRST):**
1. `00-contract.md` (this task packet) - Your Phase 2 contract
2. `../2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md` - Full requirements
3. `../2026-01-08_z80-edtasm-research/architecture-m80-assessment.md` (lines 246-513) - Phase 2 guidance

**Reference Documentation:**
4. `syntax/edtasm/syntax.c` - Basic macro infrastructure (MACRO/ENDM)
5. `syntax/mot/syntax.c` - May have REPT/IRP implementations
6. `output_bin.c` - Binary output module reference
7. `.ai/repo-overrides.md` - C90 specific rules (CRITICAL!)

### 2. Understand the Scope

**What You're Building (Phase 2):**
- ✅ Basic EDTASM macros with #P1-#P9 parameters and #$YM unique IDs
- ✅ M80 advanced macros: LOCAL, REPT, IRP, IRPC, EXITM
- ✅ M80 concatenation operator (&)
- ✅ INCLUDE directive with path resolution
- ✅ TRS-DOS /CMD output format module
- ✅ Multi-segment /CMD support with entry point

**What You're NOT Building (Later Phases):**
- ❌ .PHASE/.DEPHASE - Phase 3
- ❌ .RADIX directive - Phase 3
- ❌ Full documentation - Phase 4
- ❌ .REL relocatable format - Future
- ❌ LINK-80 linker - Future

### 3. Implementation Strategy (from Architect)

```bash
# Week 1: Basic Macros + INCLUDE (5-7 days)
Day 1-2:   Implement EDTASM macro syntax:
           - #P1-#P9 parameter substitution
           - #$YM unique macro invocation ID
           - Test basic macro expansion

Day 3-4:   Implement INCLUDE directive:
           - File path resolution (current dir, include paths)
           - Recursive include detection
           - Line number tracking across files
           - Test nested includes

Day 5:     Testing:
           - Macros with parameters
           - Multiple invocations with unique IDs
           - Include files with macros
           - Error cases (missing files, recursion)

# Week 2: M80 Advanced Macros (5-7 days)
Day 6-7:   Implement LOCAL directive:
           - Generate unique labels per invocation
           - Scope management
           - Test with nested macros

Day 8:     Implement REPT directive:
           - Block repetition N times
           - Parameter substitution within blocks
           - Test nested REPT

Day 9-10:  Implement IRP/IRPC directives:
           - IRP: Iterate through comma-separated list
           - IRPC: Iterate through string characters
           - Variable substitution in blocks

Day 11:    Implement EXITM and & operator:
           - EXITM: Early macro exit
           - &: Token concatenation
           - Test combined with other macros

# Week 3: /CMD Output (5-7 days)
Day 12-14: Implement output_cmd.c module:
           - /CMD file structure (header, load blocks, transfer)
           - Single-segment output
           - Entry point setting
           - Test basic /CMD files

Day 15-16: Multi-segment /CMD support:
           - Handle ASEG/CSEG/DSEG
           - Multiple non-contiguous load blocks
           - Proper segment ordering
           - Test multi-segment programs

Day 17:    Emulator testing:
           - Test /CMD files in trs80gp or sdltrs
           - Verify load addresses
           - Verify entry point execution
           - Fix any compatibility issues

# Week 4: Testing & Polish (3-5 days)
Day 18-20: Comprehensive testing:
           - 40-50 test cases for all Phase 2 features
           - Macro edge cases (nesting, recursion)
           - /CMD format validation
           - Performance testing

Day 21-22: Bug fixes & polish:
           - Address any failures
           - Code review
           - Memory leak checking
           - Performance optimization if needed
```

### 4. Critical Implementation Notes

**EDTASM Macro Parameter Syntax:**
```asm
; #P1 through #P9 - Parameters 1-9
; #$YM - Unique invocation ID

PRINT MACRO
  LD A,#P1      ; First parameter
  CALL PUTCHAR
  DEFM #P2      ; Second parameter
LOCAL#$YM:      ; Unique label for this invocation
  JP LOCAL#$YM
ENDM

; Invocation 1:
  PRINT 'A',"Hello"
; Expands to:
;   LD A,'A'
;   CALL PUTCHAR
;   DEFM "Hello"
; LOCAL000001:
;   JP LOCAL000001

; Invocation 2:
  PRINT 'B',"World"
; LOCAL000002:  (different ID!)
;   JP LOCAL000002
```

**LOCAL Label Generation:**
```asm
; M80 LOCAL directive
DELAY MACRO
  LOCAL LOOP,DONE    ; Declare local labels
LOOP:
  DEC A
  JR NZ,LOOP         ; Safe - unique per invocation
DONE:
  RET
ENDM

; Implementation: Generate ??0000, ??0001, etc.
```

**REPT Block Repetition:**
```asm
; Repeat block N times
  REPT 3
  NOP
  NOP
  ENDM
; Expands to:
;   NOP
;   NOP
;   NOP
;   NOP
;   NOP
;   NOP
```

**IRP/IRPC Iteration:**
```asm
; IRP - iterate through list
  IRP ITEM,<A,B,C>
  LD A,ITEM
  ENDM
; Expands to:
;   LD A,A
;   LD A,B
;   LD A,C

; IRPC - iterate through string
  IRPC CHAR,ABC
  DEFB '&CHAR'
  ENDM
; Expands to:
;   DEFB 'A'
;   DEFB 'B'
;   DEFB 'C'
```

**TRS-DOS /CMD Format:**
```
Offset  Size  Description
------  ----  -----------
0x00    1     Object code flag (0x01 = load block, 0x02 = transfer)
0x01    2     Block length (little-endian, includes this header)
0x03    2     Load address (little-endian)
0x05    N     Object code bytes
...     ...   (additional load blocks)
LAST    1     Transfer address flag (0x02)
LAST+1  2     Transfer block length (0x0002)
LAST+3  2     Entry point address (little-endian)
```

**Load Block Structure:**
```c
/* output_cmd.c pseudo-code */
for each section {
  if (section has atoms) {
    write_byte(0x01);                    /* Load block flag */
    write_word_le(length + 5);           /* Block length */
    write_word_le(section->org);         /* Load address */
    write_bytes(section->data, length);  /* Code/data */
  }
}
write_byte(0x02);                        /* Transfer flag */
write_word_le(0x0002);                   /* Transfer block length */
write_word_le(entry_point);              /* Entry address */
```

### 5. Build and Test Commands

```bash
# Build Phase 2
cd /Users/bryanw/Projects/Vintage/tools/vasm-ext
make CPU=z80 SYNTAX=edtasm-m80

# Test macro expansion
cat > test_macro.asm << 'EOF'
  ORG $8000

; Test basic EDTASM macros
PRINT MACRO
  LD A,#P1
  CALL $0033
ENDM

  PRINT 'A'
  PRINT 'B'
  END
EOF

./vasmz80_edtasm-m80 -Fbin -o test_macro.bin test_macro.asm

# Test INCLUDE directive
mkdir -p include
cat > include/common.inc << 'EOF'
; Common definitions
PUTCHAR EQU $0033
GETCHAR EQU $0049
EOF

cat > test_include.asm << 'EOF'
  ORG $8000
  INCLUDE "include/common.inc"
  LD A,'X'
  CALL PUTCHAR
  END
EOF

./vasmz80_edtasm-m80 -Iinclude -Fbin -o test_include.bin test_include.asm

# Test /CMD output
cat > test_cmd.asm << 'EOF'
  ASEG
  ORG $5200

START:
  LD A,$00
  LD ($3C00),A
  RET

  END START
EOF

./vasmz80_edtasm-m80 -Fcmd -o test.cmd test_cmd.asm
hexdump -C test.cmd
# Should show /CMD structure:
# 01 xx xx 00 52 ... 02 02 00 00 52

# Test in TRS-80 emulator (if available)
# trs80gp -model 3 -disk1 test.dsk
# Then in emulator: LOAD "TEST" (M) EXEC

# Run test suite
python tests/edtasm-m80/run_phase2_tests.py

# Memory leak check
valgrind --leak-check=full ./vasmz80_edtasm-m80 -Fcmd test_cmd.asm
```

---

## Phase 2 Feature List (11 features)

### Basic EDTASM Macros (2 features)
1. **MACRO/ENDM with #P1-#P9** - Parameter substitution
2. **#$YM** - Unique macro invocation ID

### M80 Advanced Macros (6 features)
3. **LOCAL** - Declare local labels (unique per invocation)
4. **REPT** - Repeat block N times
5. **IRP** - Iterate through comma-separated list
6. **IRPC** - Iterate through string characters
7. **EXITM** - Exit macro early
8. **&** - Concatenation operator

### File Inclusion (1 feature)
9. **INCLUDE** - Include external file

### Output Format (2 features)
10. **TRS-DOS /CMD output module** - Native executable format
11. **Multi-segment /CMD support** - ASEG/CSEG/DSEG in one file

---

## File Structure

```
syntax/edtasm-m80/
├── syntax.c           # Extended with macro features (~2800-3000 lines)
├── syntax.h           # Interface (~30 lines)
└── syntax_errors.h    # Extended error messages (~60 lines)

output_cmd.c           # NEW - TRS-DOS /CMD output module (~400-500 lines)
output_cmd.h           # NEW - Interface (~20 lines)

tests/edtasm-m80/
├── phase2/
│   ├── test_macro_params.asm       # #P1-#P9 substitution
│   ├── test_macro_id.asm           # #$YM unique IDs
│   ├── test_local.asm              # LOCAL directive
│   ├── test_rept.asm               # REPT directive
│   ├── test_irp.asm                # IRP directive
│   ├── test_irpc.asm               # IRPC directive
│   ├── test_exitm.asm              # EXITM directive
│   ├── test_concat.asm             # & operator
│   ├── test_include.asm            # INCLUDE directive
│   ├── test_nested_include.asm     # Recursive includes
│   ├── test_cmd_single.asm         # Single-segment /CMD
│   ├── test_cmd_multi.asm          # Multi-segment /CMD
│   └── test_cmd_entry.asm          # Entry point setting
└── run_phase2_tests.py
```

---

## Code Standards (CRITICAL!)

**ANSI C90 Compliance:**
```c
/* ✅ GOOD - C90 compliant */
int i;
char *buffer;
macro_context *ctx;

i = 10;
buffer = malloc(100);
ctx = current_macro;

/* ❌ BAD - C99 features */
int i = 10;  /* Declaration with initialization - NO! */
for (int j = 0; ...) /* Declaration in for loop - NO! */
macro_context *ctx = current_macro;  /* Combined declaration - NO! */
```

**Macro Expansion Safety:**
```c
/* ✅ GOOD - Prevent infinite recursion */
#define MAX_MACRO_DEPTH 100
static int macro_depth = 0;

void expand_macro(macro *m) {
  if (macro_depth >= MAX_MACRO_DEPTH) {
    syntax_error("macro nesting too deep");
    return;
  }
  macro_depth++;
  /* ... expansion ... */
  macro_depth--;
}

/* ✅ GOOD - Track include files */
#define MAX_INCLUDE_DEPTH 20
static int include_depth = 0;

void process_include(char *filename) {
  if (include_depth >= MAX_INCLUDE_DEPTH) {
    syntax_error("include nesting too deep");
    return;
  }
  include_depth++;
  /* ... include processing ... */
  include_depth--;
}
```

**See `.ai/repo-overrides.md` for complete C90 rules!**

---

## Success Indicators (Weekly Checkpoints)

**End of Week 1:**
- ✅ MACRO/ENDM with #P1-#P9 works
- ✅ #$YM generates unique IDs per invocation
- ✅ INCLUDE directive processes files
- ✅ Include path resolution works (-I flag)
- ✅ Recursive include detection works
- ✅ Basic macro tests pass

**End of Week 2:**
- ✅ LOCAL generates unique labels
- ✅ REPT repeats blocks correctly
- ✅ IRP iterates through lists
- ✅ IRPC iterates through strings
- ✅ EXITM exits macros early
- ✅ & concatenates tokens
- ✅ All advanced macro tests pass

**End of Week 3:**
- ✅ output_cmd.c module implemented
- ✅ Single-segment /CMD files work
- ✅ Multi-segment /CMD files work
- ✅ Entry point setting works
- ✅ /CMD files load in TRS-80 emulator
- ✅ /CMD format tests pass

**End of Week 4:**
- ✅ All Phase 2 features tested (40-50 tests)
- ✅ Macro edge cases handled (nesting, recursion limits)
- ✅ /CMD format validated
- ✅ Code coverage ≥80% for new code
- ✅ No memory leaks
- ✅ Performance ≥10,000 lines/second
- ✅ Ready for Phase 3

---

## Common Pitfalls (Avoid These!)

### 1. ❌ Macro Recursion Without Depth Limit
```c
/* WRONG - No depth check */
void expand_macro(macro *m) {
  expand_macro(m);  /* Infinite recursion! */
}
```
**Fix:** Always track and limit macro_depth!

### 2. ❌ Include File Circular Reference
```asm
; file_a.asm
  INCLUDE "file_b.asm"

; file_b.asm
  INCLUDE "file_a.asm"  ; WRONG - circular!
```
**Fix:** Track included files and detect cycles!

### 3. ❌ Wrong /CMD Block Length
```c
/* WRONG - Doesn't include header bytes */
write_word_le(data_length);  /* Too short! */

/* RIGHT - Include full block length */
write_word_le(data_length + 5);  /* 1-byte flag + 2-byte len + 2-byte addr */
```
**Fix:** /CMD block length includes the header!

### 4. ❌ Big-Endian /CMD Addresses
```c
/* WRONG - Big-endian */
write_word(section->org);  /* 52 00 instead of 00 52 */

/* RIGHT - Little-endian */
write_word_le(section->org);  /* Z80 is little-endian! */
```
**Fix:** Always use write_word_le() for Z80!

### 5. ❌ Not Handling Empty Parameters
```asm
  MACRO TEST
    LD A,#P1    ; What if P1 is empty?
  ENDM

  TEST          ; No parameters - crashes!
```
**Fix:** Check for empty parameters and handle gracefully!

### 6. ❌ LOCAL Without Unique Generation
```c
/* WRONG - Same label every time */
sprintf(label, "LOCAL");

/* RIGHT - Unique per invocation */
sprintf(label, "??%06d", unique_id++);
```
**Fix:** Generate truly unique labels!

---

## Testing Strategy

### Unit Tests (Syntax Module)
```bash
# Test individual macro features
./vasmz80_edtasm-m80 tests/edtasm-m80/phase2/test_macro_params.asm
./vasmz80_edtasm-m80 tests/edtasm-m80/phase2/test_local.asm
./vasmz80_edtasm-m80 tests/edtasm-m80/phase2/test_rept.asm
```

### Integration Tests (/CMD Output)
```bash
# Test /CMD file structure
./vasmz80_edtasm-m80 -Fcmd -o test.cmd tests/edtasm-m80/phase2/test_cmd_multi.asm
hexdump -C test.cmd

# Verify structure:
# - Starts with 0x01 (load block)
# - Correct block lengths
# - Little-endian addresses
# - Ends with 0x02 (transfer block)
```

### Emulator Tests (Real Hardware Verification)
```bash
# Create test disk (if tools available)
# Copy test.cmd to TRS-80 disk image
# Load in trs80gp or sdltrs emulator
# Verify program loads and executes at correct address
```

### Regression Tests
```bash
# Ensure Phase 1 still works
python tests/edtasm-m80/run_tests.py

# All Phase 1 tests should still pass!
```

---

## TRS-DOS /CMD Format Reference

### File Structure
```
+--------+--------+--------+--------+--------+--------+-----+
| 0x01   | LEN_LO | LEN_HI | ADR_LO | ADR_HI | DATA.. | ... |  (Load block 1)
+--------+--------+--------+--------+--------+--------+-----+
| 0x01   | LEN_LO | LEN_HI | ADR_LO | ADR_HI | DATA.. | ... |  (Load block 2)
+--------+--------+--------+--------+--------+--------+-----+
| ...    | ...    | ...    | ...    | ...    | ...    | ... |  (More blocks)
+--------+--------+--------+--------+--------+--------+-----+
| 0x02   | 0x02   | 0x00   | ENT_LO | ENT_HI |              |  (Transfer block)
+--------+--------+--------+--------+--------+              +
```

### Block Types
- **0x01** - Load block (code/data to load)
- **0x02** - Transfer block (entry point)
- **0x04** - REL load block (not used in Phase 2)

### Example: Single Segment
```asm
  ORG $5200
START:
  LD A,$00
  LD ($3C00),A
  RET
  END START
```

**Generated /CMD:**
```
01 0A 00 00 52   ; Load block: length=10, address=$5200
3E 00 32 00 3C   ; Code: LD A,$00 / LD ($3C00),A
C9               ; Code: RET
02 02 00 00 52   ; Transfer block: entry=$5200
```

### Example: Multiple Segments
```asm
  ASEG
  ORG $5000
CODE:
  CALL $6000
  RET

  DSEG
  ORG $6000
DATA:
  NOP
  RET

  END CODE
```

**Generated /CMD:**
```
01 08 00 00 50   ; Load block 1: $5000
CD 00 60 C9      ; CALL $6000 / RET

01 07 00 00 60   ; Load block 2: $6000
00 C9            ; NOP / RET

02 02 00 00 50   ; Transfer: entry=$5000
```

---

## Definition of Done (Phase 2)

Phase 2 is **DONE** when:

```
✅ All 11 Phase 2 features implemented
✅ All acceptance criteria met (see 00-contract.md)
✅ Test suite passes (40-50 test cases)
✅ Code coverage ≥80% for new code
✅ No compiler warnings
✅ No memory leaks
✅ Performance ≥10,000 lines/second
✅ Macros expand correctly with #P1-#P9 and #$YM
✅ LOCAL labels unique per invocation
✅ REPT/IRP/IRPC iterate correctly
✅ INCLUDE directive processes files recursively
✅ /CMD files have correct structure
✅ /CMD files load and execute in TRS-80 emulator
✅ Multi-segment /CMD files work
✅ Code reviewed
✅ Ready for Phase 3 (control directives)
```

---

## Timeline Reminder

**Phase 1:** 4-5 weeks (COMPLETE)
**Phase 2:** 3-4 weeks (YOU ARE HERE)
**Phase 3:** 2-3 weeks (Control directives)
**Phase 4:** 2 weeks (Documentation + validation)
**Total:** 11-13 weeks for complete EDTASM-M80 assembler

---

## Related Documentation

**In This Task Packet:**
- `00-contract.md` - Full Phase 2 contract
- `10-plan.md` - Implementation plan (fill this out as you work)
- `20-work-log.md` - Daily progress log
- `30-review.md` - Code review checklist
- `40-acceptance.md` - Final acceptance validation

**Phase 1 (Foundation):**
- `../2026-01-08_phase1-foundation-m80/00-contract.md` - What was built
- `../2026-01-08_phase1-foundation-m80/40-acceptance.md` - Phase 1 validation

**Requirements:**
- `../2026-01-08_z80-edtasm-research/PRD-Z80-edtasm-m80-v2.md` - Full PRD
- `../2026-01-08_z80-edtasm-research/architecture-m80-assessment.md` - Architecture

**Reference:**
- `.ai/repo-overrides.md` - C90 rules (MUST READ!)
- `.ai-pack/roles/engineer.md` - Your role definition
- `.ai-pack/workflows/feature.md` - Feature workflow

**TRS-DOS References:**
- TRS-80 Disk System Owner's Manual (for /CMD format)
- EDTASM/MRAS Manual (for macro syntax)
- Microsoft M80 Manual (for advanced macros)

---

**Status:** ✅ READY TO START (Phase 1 complete)
**Engineer:** [Your Name] (update when you start)
**Start Date:** [Date] (update when you start)
**Target Completion:** 3-4 weeks from start date

**Prerequisites Met:**
- ✅ Phase 1 complete (24 directives working)
- ✅ Test suite passing (85+ tests)
- ✅ Basic macro infrastructure exists (MACRO/ENDM)
- ✅ vasm build system understood

**Good luck! 🚀**
