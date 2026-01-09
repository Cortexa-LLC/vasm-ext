# User Stories: Z80 EDTASM Syntax Module

**Project:** vasm-ext TRS-80 Model III EDTASM Support
**Created:** 2026-01-08
**Status:** Planning Phase

This document provides detailed user stories in JIRA-style format with Given-When-Then acceptance criteria.

---

## Epic 1: Foundation and Core Directives

### US-001: Syntax Module Skeleton

**Story:** As a developer, I want a basic syntax module skeleton so that I have the infrastructure to implement EDTASM directives

**Description:**
Create the foundational files and structure for the EDTASM Z80 syntax module, following vasm's syntax module conventions. This includes syntax.c, syntax.h, syntax_errors.h, and basic integration with the build system.

**Acceptance Criteria:**

Given the vasm build system
When I run `make CPU=z80 SYNTAX=edtasm`
Then the build succeeds and produces `vasmz80_edtasm` binary

Given the basic syntax module
When I run `./vasmz80_edtasm --help`
Then it displays help text identifying itself as "vasmz80_edtasm"

Given an empty source file
When I assemble it with `./vasmz80_edtasm -Fbin empty.asm`
Then it assembles without errors

**Technical Notes:**
- Create `syntax/edtasm-z80/` directory
- Files needed: syntax.c, syntax.h, syntax_errors.h
- Follow vasm C90 coding standards
- Use 2-space indentation per vasm style
- Reference `syntax/edtasm/` (6809) for structure patterns
- Integrate with Makefile: `SYNTAX_NAME = edtasm`, `CPUDIR = cpus/z80`

**Dependencies:** None

**Priority:** P0
**Complexity:** M

---

### US-002: ORG Directive

**Story:** As a TRS-80 developer, I want the ORG directive so that I can set the assembly origin address

**Description:**
Implement the ORG directive to set the current assembly origin. This is fundamental for TRS-80 programs that must load at specific memory addresses.

**Acceptance Criteria:**

Given a source file:
```asm
  ORG $0400
START LD A,$FF
```
When I assemble it
Then the code is assembled starting at address $0400

Given a source file with multiple ORG directives:
```asm
  ORG $1000
CODE1 LD A,1
  ORG $2000
CODE2 LD A,2
```
When I assemble it
Then CODE1 is at $1000 and CODE2 is at $2000

Given hex, decimal, and binary expressions
When I use `ORG $1000`, `ORG 4096`, `ORG %0001000000000000`
Then all three set the same origin ($1000)

**Technical Notes:**
- Parse ORG directive in `parse()` function
- Call vasm's `set_section()` or equivalent to change origin
- Support expressions (hex $, decimal, binary %, symbols)
- Handle section transitions properly

**Dependencies:** US-001

**Priority:** P0
**Complexity:** S

---

### US-003: EQU Directive

**Story:** As a TRS-80 developer, I want the EQU directive so that I can define constant symbols

**Description:**
Implement the EQU directive to assign permanent values to symbols. Once defined, EQU symbols cannot be redefined (unlike DEFL).

**Acceptance Criteria:**

Given a source file:
```asm
BUFSIZE EQU 256
MAXVAL  EQU $FF
  LD B,BUFSIZE
  LD A,MAXVAL
```
When I assemble it
Then BUFSIZE=256 and MAXVAL=255 are used correctly

Given an attempt to redefine an EQU symbol:
```asm
COUNT EQU 10
COUNT EQU 20  ; Error!
```
When I assemble it
Then an error is reported: "symbol already defined"

Given an expression:
```asm
BASE EQU $1000
OFFSET EQU BASE+256
```
When I assemble it
Then OFFSET evaluates to $1100

**Technical Notes:**
- Parse EQU directive
- Call `new_equate()` or equivalent to create constant symbol
- Mark symbol as non-redefinable
- Use vasm's expression evaluator

**Dependencies:** US-001

**Priority:** P0
**Complexity:** S

---

### US-004: DEFL Directive

**Story:** As a TRS-80 developer, I want the DEFL directive so that I can define redefinable symbols

**Description:**
Implement the DEFL directive to assign values to symbols that can be redefined later. This is like SET in 6809 EDTASM.

**Acceptance Criteria:**

Given a source file:
```asm
COUNT DEFL 0
COUNT DEFL COUNT+1
COUNT DEFL 10
  LD A,COUNT  ; Should be 10
```
When I assemble it
Then COUNT equals 10 and the LD instruction uses 10

Given a source file with both EQU and DEFL:
```asm
CONST EQU 5    ; Permanent
VAR   DEFL 1   ; Changeable
VAR   DEFL 2   ; OK to redefine
CONST EQU 6    ; Error!
```
When I assemble it
Then VAR=2 is valid but CONST redefinition causes error

**Technical Notes:**
- Parse DEFL directive
- Create symbol with redefinable flag
- Allow subsequent DEFL to update value
- Error if EQU symbol redefined

**Dependencies:** US-001, US-003

**Priority:** P0
**Complexity:** M

---

### US-006: DEFB Directive

**Story:** As a TRS-80 developer, I want the DEFB directive so that I can define byte data

**Description:**
Implement the DEFB directive to define 8-bit byte values. This is equivalent to FCB in 6809 EDTASM or DB in other assemblers.

**Acceptance Criteria:**

Given a source file:
```asm
  DEFB 1,2,3,4,5
```
When I assemble it
Then 5 bytes are output: 01 02 03 04 05

Given various formats:
```asm
  DEFB $FF      ; Hex
  DEFB 255      ; Decimal
  DEFB %11111111 ; Binary
  DEFB 'A'      ; ASCII character
```
When I assemble it
Then all output byte value $FF (255)

Given expressions:
```asm
BASE EQU 100
  DEFB BASE+10, BASE+20
```
When I assemble it
Then bytes output are 110 (0x6E) and 120 (0x78)

**Technical Notes:**
- Parse DEFB directive
- Support comma-separated list
- Evaluate each expression to 8-bit value
- Error if value >255
- Support character literals 'A'
- Call `new_data_atom()` or equivalent

**Dependencies:** US-001, US-003

**Priority:** P0
**Complexity:** M

---

### US-007: DEFW/DEFH Directives

**Story:** As a TRS-80 developer, I want DEFW/DEFH directives so that I can define 16-bit word data

**Description:**
Implement DEFW and DEFH directives to define 16-bit words. Z80 is little-endian, so low byte first. DEFH is an alias for DEFW.

**Acceptance Criteria:**

Given a source file:
```asm
  DEFW $1234
```
When I assemble it
Then 2 bytes are output: 34 12 (little-endian)

Given multiple words:
```asm
  DEFW $1000,$2000,$3000
```
When I assemble it
Then 6 bytes are output: 00 10 00 20 00 30

Given DEFH as alias:
```asm
  DEFH $ABCD
```
When I assemble it
Then 2 bytes are output: CD AB (same as DEFW)

Given label addresses:
```asm
START LD A,1
  DEFW START
```
When I assemble it
Then the word contains the address of START (little-endian)

**Technical Notes:**
- Parse DEFW and DEFH directives (DEFH is alias)
- Output 16-bit values little-endian (Z80 convention)
- Support expressions and label addresses
- Error if value >65535

**Dependencies:** US-001, US-003

**Priority:** P0
**Complexity:** M

---

### US-008: DEFS Directive

**Story:** As a TRS-80 developer, I want the DEFS directive so that I can reserve uninitialized space

**Description:**
Implement the DEFS directive to reserve a specified number of bytes. This is equivalent to RMB in 6809 EDTASM or DS in other assemblers.

**Acceptance Criteria:**

Given a source file:
```asm
BUFFER DEFS 256
```
When I assemble it
Then 256 bytes of space are reserved

Given an expression:
```asm
SIZE EQU 100
ARRAY DEFS SIZE*2
```
When I assemble it
Then 200 bytes are reserved

Given optional fill value:
```asm
  DEFS 10,$FF  ; If supported
```
When I assemble it
Then 10 bytes filled with $FF (if fill value supported)

**Technical Notes:**
- Parse DEFS directive with size expression
- Optional: support fill value (not in all EDTASM versions)
- Create SPACE atom or equivalent
- Advance location counter by size

**Dependencies:** US-001

**Priority:** P0
**Complexity:** S

---

### US-009: DEFM Directive

**Story:** As a TRS-80 developer, I want the DEFM directive so that I can define string data

**Description:**
Implement the DEFM directive to define ASCII message strings. This is equivalent to FCC in 6809 EDTASM.

**Acceptance Criteria:**

Given a source file:
```asm
MSG DEFM "Hello, World!"
```
When I assemble it
Then 13 ASCII bytes are output: 48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21

Given single quotes:
```asm
  DEFM 'Another string'
```
When I assemble it
Then the string is output as ASCII bytes

Given special characters inside quotes:
```asm
  DEFM "Text;with;semicolons"
```
When I assemble it
Then semicolons inside quotes are treated as string data, not comments

**Technical Notes:**
- Parse DEFM directive with quoted string
- Support both double quotes " and single quotes '
- Do NOT treat comment chars (;) inside quotes as comments
- Output ASCII bytes
- No automatic null terminator (add explicitly if needed)

**Dependencies:** US-001, US-012

**Priority:** P0
**Complexity:** M

---

### US-010: Case-Insensitive Mode

**Story:** As a TRS-80 developer, I want case-insensitive mode by default so that legacy code assembles correctly

**Description:**
Implement case-insensitive handling for labels, symbols, and directives to match original EDTASM behavior. Also provide optional case-sensitive mode via `-case` flag.

**Acceptance Criteria:**

Given case-insensitive mode (default):
```asm
MyLabel EQU 10
  LD A,mylabel  ; Should reference MyLabel
  ld a,MYLABEL  ; Should also reference MyLabel
```
When I assemble it
Then all three forms (MyLabel, mylabel, MYLABEL) reference the same symbol

Given case-sensitive mode (`-case` flag):
```asm
MyLabel EQU 10
mylabel EQU 20
  LD A,MyLabel  ; Should be 10
  LD A,mylabel  ; Should be 20
```
When I assemble with `./vasmz80_edtasm -case`
Then MyLabel and mylabel are distinct symbols

Given directives in various cases (default mode):
```asm
  org $1000
  ORG $2000
  Org $3000
```
When I assemble it
Then all three directive forms are recognized

**Technical Notes:**
- Use vasm's `nocase` flag or implement case-insensitive symbol table
- Default: case-insensitive (unlike 6809 EDTASM which defaults to sensitive)
- Add `-case` command-line flag for case-sensitive mode
- Apply to labels, symbols, directives
- Z80 mnemonics always case-insensitive (standard CPU module behavior)

**Dependencies:** US-001, US-003

**Priority:** P0
**Complexity:** M

---

## Epic 2: Comment Syntax and Identifier Rules

### US-012: Semicolon Comment Parsing

**Story:** As a TRS-80 developer, I want semicolon comments so that I can document my code

**Description:**
Implement semicolon (;) as the comment delimiter. Everything after ; on a line is a comment. This is the ONLY comment syntax for TRS-80 EDTASM (NO asterisk-in-column-1).

**Acceptance Criteria:**

Given a source file:
```asm
  LD A,$10  ; Load 16 into A
  LD B,$20  ; Load 32 into B
```
When I assemble it
Then text after semicolons is ignored

Given a comment-only line:
```asm
; This is a full-line comment
  LD A,1
```
When I assemble it
Then the comment line is ignored

Given an asterisk at the start of a line:
```asm
*LABEL LD A,1  ; Asterisk is part of label, NOT a comment!
```
When I assemble it
Then *LABEL is treated as a label (or error if invalid char)

**Technical Notes:**
- Parse semicolon as comment start in `parse()` function
- Implement `iscomment()` to check for semicolon ONLY
- Do NOT implement asterisk-in-column-1 comment (unlike 6809)
- Preserve comments in listing output

**Dependencies:** US-001

**Priority:** P0
**Complexity:** S

---

### US-013: Remove Asterisk-in-Column-1 Comment Support

**Story:** As a TRS-80 developer, I want asterisk-in-column-1 to NOT be a comment so that my TRS-80 code assembles correctly

**Description:**
Ensure that asterisk (*) in column 1 is NOT treated as a comment character. This is a key difference from 6809 EDTASM. Asterisk was EDTASM's editor command prompt, not a comment.

**Acceptance Criteria:**

Given a source file:
```asm
*ENTRY LD A,1
```
When I assemble it
Then *ENTRY is treated as a label (or error if asterisk not allowed in identifiers)

Given a 6809 EDTASM file with asterisk comments:
```asm
* This is a comment in 6809 EDTASM
  LD A,1
```
When I assemble it with z80 EDTASM
Then an error or warning is generated (asterisk not recognized as comment)

**Technical Notes:**
- Ensure `iscomment()` does NOT check for asterisk
- Asterisk may or may not be allowed in identifiers (check TRS-80 spec)
- Document this difference clearly in README
- Provide migration guidance for 6809 -> z80 code

**Dependencies:** US-001, US-012

**Priority:** P0
**Complexity:** S

---

### US-014: Identifier Character Validation

**Story:** As a TRS-80 developer, I want proper identifier rules so that valid labels are accepted and invalid ones rejected

**Description:**
Implement TRS-80 EDTASM identifier rules: start with letter, contain letters/digits/underscore, NO dollar signs (unlike 6809 which allows dollar for OS-9).

**Acceptance Criteria:**

Given valid identifiers:
```asm
START   EQU 1
MyLabel EQU 2
VAR_1   EQU 3
LABEL2  EQU 4
```
When I assemble it
Then all identifiers are accepted

Given invalid identifiers:
```asm
123BAD  EQU 1  ; Starts with digit - error!
$DOLLAR EQU 2  ; Dollar sign - error!
F$Link  EQU 3  ; OS-9 style - error!
```
When I assemble it
Then errors are reported for invalid identifiers

Given underscore and alphanumeric mix:
```asm
_PRIVATE   EQU 1
MY_LABEL_2 EQU 2
```
When I assemble it
Then both are accepted (if underscore allowed at start)

**Technical Notes:**
- Implement `isidstart()` and `isidchar()` functions
- `isidstart()`: A-Z, a-z, possibly underscore
- `isidchar()`: A-Z, a-z, 0-9, underscore
- NO dollar sign support (unlike 6809 EDTASM)
- Error message: "illegal identifier character"

**Dependencies:** US-001

**Priority:** P0
**Complexity:** S

---

## Epic 3: Z80 CPU Integration

### US-017: Z80 CPU Module Integration

**Story:** As a TRS-80 developer, I want Z80 instruction support so that I can assemble Z80 assembly code

**Description:**
Integrate the existing vasm z80 CPU module with the EDTASM syntax module. Ensure the syntax module properly calls CPU module functions for instruction parsing and encoding.

**Acceptance Criteria:**

Given a simple Z80 program:
```asm
  ORG $0400
  LD A,$FF
  LD B,A
  JP $0400
```
When I assemble it
Then correct Z80 opcodes are generated

Given the resulting binary
When I disassemble it
Then it matches the source instructions exactly

Given CPU-specific features (index registers, bit operations):
```asm
  LD (IX+5),A
  BIT 7,B
  RLC C
```
When I assemble it
Then correct Z80 opcodes with prefixes are generated

**Technical Notes:**
- Call z80 CPU module's `instruction()` function
- Parse mnemonic and operands according to EDTASM syntax
- Pass parsed instruction to CPU module for encoding
- Handle CPU module errors gracefully
- Test with cpus/z80/ test cases

**Dependencies:** US-001, US-002 (ORG)

**Priority:** P0
**Complexity:** M

---

## Epic 4: TRS-DOS Binary Output

### US-023: TRS-DOS /CMD Format Research

**Story:** As a TRS-80 developer, I need to understand /CMD format so that it can be implemented correctly

**Description:**
Research and document TRS-DOS /CMD executable file format. This is not implementation, just research and specification.

**Acceptance Criteria:**

Given research into TRS-DOS /CMD format
When the research is complete
Then a specification document is created covering:
- File header structure
- Load address encoding
- Transfer address (entry point) encoding
- Block structure for multiple segments
- File termination marker
- Any checksums or validation bytes

Given sample /CMD files
When they are analyzed
Then the format is reverse-engineered and documented

**Technical Notes:**
- Research TRS-DOS technical manuals
- Examine existing /CMD files with hex editor
- Consult TRS-80 emulator source code (xtrs, trs80gp)
- Document in task packet or separate spec file
- This is pure research - no code yet

**Dependencies:** None

**Priority:** P0 (for /CMD support)
**Complexity:** M

---

### US-024: /CMD Format Implementation

**Story:** As a TRS-80 developer, I want -Fcmd output so that I can generate TRS-DOS executable files

**Description:**
Implement TRS-DOS /CMD file format output as a new output module or extension of binary output.

**Acceptance Criteria:**

Given a simple Z80 program:
```asm
  ORG $5200
START LD A,$FF
  RET
  END START
```
When I assemble with `./vasmz80_edtasm -Fcmd -o test.cmd test.asm`
Then a /CMD file is generated with:
- Load address $5200
- Transfer address at START
- Correct header bytes
- Can load in TRS-80 emulator

Given the /CMD file
When I load it in xtrs or trs80gp emulator
Then it loads at correct address and executes from correct entry point

**Technical Notes:**
- Option 1: Create new `output_trsdos.c` module
- Option 2: Extend `output_bin.c` with /CMD support
- Parse END directive for entry point
- Generate /CMD header with load address and transfer address
- Support multiple ORG segments (multiple load blocks)
- Test with real TRS-80 emulators

**Dependencies:** US-001, US-002, US-005, US-017, US-023

**Priority:** P0
**Complexity:** L

---

## Epic 7: Macro System

### US-040: MACRO/ENDM Directive Parsing

**Story:** As a TRS-80 developer, I want to define macros so that I can reuse code patterns

**Description:**
Implement MACRO and ENDM directives to define macro templates. Parse macro name, parameters, and body.

**Acceptance Criteria:**

Given a macro definition:
```asm
DELAY MACRO #COUNT
      LD B,#COUNT
LOCAL DEC B
      JP NZ,LOCAL
      ENDM
```
When I parse it
Then the macro is stored with name "DELAY" and parameter "#COUNT"

Given a macro with multiple parameters:
```asm
ADDW MACRO #SRC1,#SRC2,#DEST
     LD HL,(#SRC1)
     LD DE,(#SRC2)
     ADD HL,DE
     LD (#DEST),HL
     ENDM
```
When I parse it
Then the macro is stored with 3 parameters

**Technical Notes:**
- Parse MACRO directive with name and parameter list
- Parameters use # prefix: #P1, #P2, etc.
- Store macro body as text or token stream
- Parse ENDM as macro terminator
- No nested macro definitions (per EDTASM-PLUS spec)
- Use vasm's macro infrastructure

**Dependencies:** US-001

**Priority:** P1
**Complexity:** L

---

### US-042: Macro Parameter Parsing (#P1, #P2, etc.)

**Story:** As a TRS-80 developer, I want #param syntax so that my EDTASM-PLUS macros work

**Description:**
Implement parameter substitution using #param syntax (different from 6809's \\param). When macro is invoked, #P1, #P2, etc. are replaced with actual arguments.

**Acceptance Criteria:**

Given a macro definition and invocation:
```asm
LOAD MACRO #REG,#VAL
     LD #REG,#VAL
     ENDM

     LOAD A,$FF  ; Invocation
```
When the macro expands
Then it generates: `LD A,$FF`

Given multiple parameters:
```asm
MOVE MACRO #FROM,#TO
     LD A,(#FROM)
     LD (#TO),A
     ENDM

     MOVE $1000,$2000
```
When the macro expands
Then it generates:
```asm
LD A,($1000)
LD ($2000),A
```

**Technical Notes:**
- Scan macro body for # followed by parameter name
- Replace with argument from macro invocation
- Support up to 16 parameters (or 9 minimum)
- Error if parameter not provided
- Be careful with # in hex numbers ($#FF should NOT be parameter)

**Dependencies:** US-040

**Priority:** P1
**Complexity:** M

---

### US-043: #$YM Unique ID Generation

**Story:** As a TRS-80 developer, I want #$YM unique IDs so that I can create local labels in macros

**Description:**
Implement #$YM special parameter that expands to a unique 4-character ID for each macro invocation, allowing unique local labels.

**Acceptance Criteria:**

Given a macro with local label:
```asm
WAIT MACRO
LOOP#$YM LD A,($4000)
         BIT 7,A
         JP Z,LOOP#$YM
         ENDM
```
When invoked twice:
```asm
     WAIT
     WAIT
```
Then two unique labels are generated (e.g., LOOP_0001, LOOP_0002)

Given nested macro invocations
When each macro uses #$YM
Then each invocation gets a unique ID (no collisions)

**Technical Notes:**
- Generate unique ID per macro expansion (4 chars, e.g., _0001, _0002)
- Replace #$YM in macro body during expansion
- Increment counter for each macro invocation
- Ensure uniqueness across entire assembly
- Format: underscores + 4 digits or alphanumeric

**Dependencies:** US-040, US-042

**Priority:** P1
**Complexity:** M

---

## Epic 10: Documentation and Examples

### US-058: README.md with Syntax Reference

**Story:** As a TRS-80 developer, I want comprehensive documentation so that I can learn and use the assembler

**Description:**
Create README.md with complete syntax reference covering all directives, examples, and usage instructions.

**Acceptance Criteria:**

Given the README.md file
When I read it
Then it covers:
- Overview and features
- Building instructions
- Basic usage examples
- All directives with syntax and examples (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM)
- Comment syntax
- Macro system
- Conditional assembly
- Output formats (-Fbin, -Fcmd)
- Command-line options
- Differences from 6809 EDTASM
- Examples section
- Troubleshooting

Given each directive section
When I read it
Then it includes:
- Syntax description
- Parameter explanation
- At least 1 working example
- Notes on behavior/edge cases

**Technical Notes:**
- Create syntax/edtasm-z80/README.md
- Follow structure of syntax/edtasm/README.md (6809 version)
- Use markdown formatting
- Include code examples with syntax highlighting
- Cross-reference related directives

**Dependencies:** All implementation epics

**Priority:** P0
**Complexity:** L

---

### US-060: Migration Guide from Original EDTASM

**Story:** As a TRS-80 developer, I want a migration guide so that I can adapt legacy code to vasm

**Description:**
Create migration guide documenting differences between original TRS-80 EDTASM and this implementation, with solutions for common issues.

**Acceptance Criteria:**

Given the migration guide
When I encounter legacy code issues
Then the guide provides solutions for:
- Case sensitivity differences
- Comment syntax differences from other assemblers
- Label length limits
- Directive differences (if any)
- Macro syntax differences
- Column format vs free-form
- Output format compatibility

Given common errors
When they occur
Then the guide explains the cause and solution

**Technical Notes:**
- Create MIGRATION.md or section in README.md
- Document known incompatibilities
- Provide before/after code examples
- Include command-line flag recommendations
- Reference TRS-80 community resources

**Dependencies:** All implementation epics, US-058

**Priority:** P0 (for adoption)
**Complexity:** M

---

## Priority Summary

**P0 (Must-have for MVP):**
- US-001 to US-011 (Epic 1: Foundation)
- US-012 to US-016 (Epic 2: Comments/Identifiers)
- US-017 to US-021 (Epic 3: Z80 Integration)
- US-022 to US-027 (Epic 4: Binary Output)
- US-058, US-060 to US-064 (Epic 10: Documentation - partial)
- US-065 to US-071 (Epic 11: Testing)

**P1 (Should-have for completeness):**
- US-028 to US-034 (Epic 5: INCLUDE)
- US-035 to US-039 (Epic 6: Conditional Assembly)
- US-040 to US-047 (Epic 7: Macros)
- US-053 to US-057 (Epic 9: Columnar Format)

**P2 (Nice-to-have):**
- US-048 to US-052 (Epic 8: Listing Control)

---

**Total User Stories:** 71
**Version:** 1.0
**Last Updated:** 2026-01-08
**Next Steps:** Begin implementation with Phase 1 (Epic 1-4) user stories
