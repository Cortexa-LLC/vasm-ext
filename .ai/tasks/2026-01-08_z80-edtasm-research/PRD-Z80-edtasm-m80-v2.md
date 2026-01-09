# Product Requirements Document: Z80 EDTASM-M80 Syntax Module

**Feature:** TRS-80 Model I/III/4 EDTASM + Microsoft M80 Syntax Support
**Target Platform:** TRS-80 Model I/III/4 (Z80-based systems)
**CPU:** z80 (already available in vasm)
**Created:** 2026-01-08
**Updated:** 2026-01-08 (Scope expanded to include M80)
**Status:** Requirements Definition Phase - User Approved Hybrid M80 Scope
**Version:** 2.0 (Hybrid EDTASM-M80)

---

## Problem Statement

**Problem:** Users who want to assemble vintage TRS-80 Model III Z80 assembly code currently have no modern cross-assembler that supports both basic EDTASM syntax AND professional Microsoft MACRO-80 (M80) features.

**Impact:**
- **Vintage computing enthusiasts** cannot assemble legacy TRS-80 code on modern systems
- **Professional developers** working on TRS-80 projects lack advanced macro and conditional capabilities
- **Preservation community** cannot maintain and build historical M80-based software
- **Educators** teaching Z80 assembly lack modern tooling with professional features

**Current State:**
- vasm has excellent z80 CPU support but no EDTASM or M80 syntax module
- vasm has 6809 EDTASM syntax (for Color Computer) but it's incompatible with TRS-80 Z80
- Users must use vintage DOS assemblers, wine-wrapped tools, or limited modern alternatives
- No cross-platform build pipeline exists for professional TRS-80 development

**Opportunity:**
- Leverage existing z80 CPU module (already in vasm)
- Leverage existing 6809 EDTASM syntax module as architectural reference
- Provide modern, cross-platform TRS-80 development toolchain
- **UNIQUE VALUE:** First modern assembler with authentic M80 compatibility
- Enable both hobbyist and professional TRS-80 development

---

## Target Users

**Primary Users:**
1. **Professional TRS-80 Developers** - Building complex multi-file projects with advanced macros
2. **Software Preservationists** - Reassembling and documenting historical M80-based code
3. **Vintage Computing Enthusiasts** - Building/maintaining TRS-80 software (hobbyist level)
4. **Educators** - Teaching Z80 assembly language using TRS-80 context with professional tools

**User Goals:**
- Assemble legacy TRS-80 code (both basic EDTASM and M80) without modification
- Use advanced M80 macro capabilities (LOCAL, REPT, IRP)
- Use professional segment management (ASEG/CSEG/DSEG)
- Build multi-file TRS-80 projects on modern operating systems
- Generate TRS-DOS compatible binary formats
- Use modern build tools (Make, shell scripts) with TRS-80 development

**User Context:**
- Working with original TRS-80 source code from magazines, books, commercial software
- Developing new software for TRS-80 emulators (xtrs, trs80gp, sdltrs)
- Building disk images for real TRS-80 hardware
- Teaching Z80 programming with professional tooling
- Maintaining/extending historical M80-based codebases

---

## Product Requirements

### Functional Requirements

**FR-1: Core EDTASM Directive Set**
- **Description:** Support the complete Series 1 Editor/Assembler directive set
- **Acceptance Criteria:**
  - ✓ `ORG` - Set assembly origin address
  - ✓ `EQU` - Assign permanent value to symbol
  - ✓ `DEFL` - Define temporary/redefinable label (M80 compatible)
  - ✓ `END` - Mark program end, set entry point
  - ✓ `DEFB` - Define bytes (8-bit data)
  - ✓ `DEFW` - Define words (16-bit data, little-endian Z80 format)
  - ✓ `DEFH` - Define halfwords (alias for DEFW)
  - ✓ `DEFS` - Reserve storage space
  - ✓ `DEFM` - Define message strings
  - ✓ All directives case-insensitive by default
- **Priority:** P0 (Must-have)

**FR-2: M80 Data Directive Aliases**
- **Description:** Support M80's alternative data directive names
- **Acceptance Criteria:**
  - ✓ `DB` - Define byte (alias for DEFB)
  - ✓ `DW` - Define word (alias for DEFW)
  - ✓ `DS` - Define storage (alias for DEFS)
  - ✓ `DC` - Define constant
  - ✓ All aliases work identically to EDTASM equivalents
- **Priority:** P0 (Must-have for M80 compatibility)

**FR-3: M80 Segment Management**
- **Description:** Support M80 segment directives for code organization
- **Acceptance Criteria:**
  - ✓ `ASEG` - Absolute segment (sets absolute addressing mode)
  - ✓ `CSEG` - Code segment (default segment)
  - ✓ `DSEG` - Data segment (separate data area)
  - ✓ Segments output as absolute addresses (not relocatable in MVP)
  - ✓ Multiple segments can coexist in same source file
  - ✓ Each segment maintains separate location counter
- **Priority:** P0 (Must-have for M80 compatibility)
- **Note:** Relocatable .REL format deferred to Phase 2

**FR-4: Comment Syntax**
- **Description:** Support TRS-80 EDTASM comment conventions
- **Acceptance Criteria:**
  - ✓ Semicolon (`;`) indicates start of comment anywhere on line
  - ✓ NO asterisk-in-column-1 comment support (differs from 6809 EDTASM)
  - ✓ Comments preserved in listing output
  - ✓ Empty lines and comment-only lines allowed
- **Priority:** P0 (Must-have)

**FR-5: Identifier Rules**
- **Description:** Support TRS-80 EDTASM/M80 identifier conventions
- **Acceptance Criteria:**
  - ✓ Labels start with letter (A-Z, case-insensitive)
  - ✓ Labels can contain letters, digits, underscore, question mark
  - ✓ No length limit (modern improvement over 6-char EDTASM limit)
  - ✓ Case-insensitive by default (can distinguish LABEL vs label optionally)
  - ✓ Dollar sign NOT supported in identifiers (Z80 has no OS-9 equivalent)
- **Priority:** P0 (Must-have)

**FR-6: Basic Conditional Assembly (EDTASM)**
- **Description:** Support EDTASM-PLUS conditional directives
- **Acceptance Criteria:**
  - ✓ `COND` expression / `ENDC` - Basic conditional assembly
  - ✓ Expression evaluation for condition (non-zero = true)
  - ✓ Nested conditionals supported
  - ✓ Conditional blocks can contain any valid assembly
- **Priority:** P1 (Should-have)

**FR-7: M80 Advanced Conditional Assembly**
- **Description:** Support M80's comprehensive conditional directive set
- **Acceptance Criteria:**
  - ✓ `IF` expression - Assemble if expression non-zero
  - ✓ `IFE` expression - Assemble if expression equals zero
  - ✓ `IF1` - Assemble only on pass 1
  - ✓ `IF2` - Assemble only on pass 2 (and later)
  - ✓ `IFDEF` symbol - Assemble if symbol defined
  - ✓ `IFNDEF` symbol - Assemble if symbol not defined
  - ✓ `IFB` argument - Assemble if argument blank
  - ✓ `IFNB` argument - Assemble if argument not blank
  - ✓ `IFIDN` arg1,arg2 - Assemble if arguments identical (string compare)
  - ✓ `IFDIF` arg1,arg2 - Assemble if arguments different
  - ✓ `ELSE` - Else clause for all IF variants
  - ✓ `ENDIF` - End conditional (in addition to ENDC)
  - ✓ Nested conditionals up to reasonable depth (20 levels)
- **Priority:** P0 (Must-have for M80 compatibility)

**FR-8: Basic Macro System (EDTASM-PLUS)**
- **Description:** Support EDTASM-PLUS macro system with #param syntax
- **Acceptance Criteria:**
  - ✓ `MACRO` / `ENDM` define macros
  - ✓ Parameter syntax: `#P1`, `#P2`, ... `#P9` (9 parameters minimum)
  - ✓ `#$YM` unique ID substitution for local labels
  - ✓ Macro invocation with arguments
  - ✓ Macro expansion in listing (configurable)
  - ✓ No nested macro definitions (per EDTASM-PLUS spec)
- **Priority:** P1 (Should-have for basic EDTASM compatibility)

**FR-9: M80 Advanced Macro System**
- **Description:** Support M80's professional macro capabilities
- **Acceptance Criteria:**
  - ✓ `LOCAL` directive - Automatic unique local labels (better than #$YM)
  - ✓ `REPT` count - Repeat block N times
  - ✓ `IRP` param,<list> - Iterate through comma-separated list
  - ✓ `IRPC` param,string - Iterate through string characters
  - ✓ `EXITM` - Exit macro expansion early
  - ✓ Concatenation operator (`&`) for token pasting
  - ✓ Special comment syntax (`;;`) for macro-only comments
  - ✓ Literal character entry (`!`) for escaping special characters
  - ✓ Macro nesting (invocation inside macros, not definition)
  - ✓ Up to 16 parameters (#P1 through #PG or similar extension)
- **Priority:** P0 (Must-have for M80 compatibility)

**FR-10: Include File Support**
- **Description:** Support `INCLUDE` directive for modular assembly
- **Acceptance Criteria:**
  - ✓ `INCLUDE filename` loads external source file
  - ✓ Multiple include paths via `-I` flag
  - ✓ Recursive includes supported (with depth limit)
  - ✓ TRS-DOS filename conventions recognized (8.3 format)
  - ✓ Unix/Linux/Windows path separators supported
- **Priority:** P1 (Should-have)

**FR-11: M80 Control Directives**
- **Description:** Support M80 control directives for assembly customization
- **Acceptance Criteria:**
  - ✓ `.Z80` - Enable Z80 instruction set
  - ✓ `.8080` - Restrict to 8080 instruction set
  - ✓ `.RADIX` base - Set default number base (2/8/10/16)
  - ✓ `.PHASE` address - Phase code for relocated execution
  - ✓ `.DEPHASE` - End phased assembly
  - ✓ Directives affect subsequent assembly
- **Priority:** P1 (Should-have for M80 compatibility)

**FR-12: M80 Symbol Visibility**
- **Description:** Support M80 symbol visibility directives (documentation only in MVP)
- **Acceptance Criteria:**
  - ✓ `PUBLIC` symbol - Mark symbol as public (documentation, not linking in MVP)
  - ✓ `ENTRY` symbol - Alias for PUBLIC
  - ✓ `NAME` module - Set module name (documentation)
  - ✓ Symbols tracked but not used for linking in MVP
  - ✓ Prepares architecture for Phase 2 linking support
- **Priority:** P2 (Nice-to-have for documentation purposes)
- **Note:** `EXTRN` (external symbols) deferred to Phase 2 (requires linking)

**FR-13: Case Sensitivity Control**
- **Description:** Support both case-insensitive (default) and case-sensitive modes
- **Acceptance Criteria:**
  - ✓ Default: Case-insensitive (matches original EDTASM/M80 behavior)
  - ✓ `-case` flag: Enable case-sensitive mode (for modern mixed-case code)
  - ✓ Applies to labels, symbols, directives
  - ✓ Z80 mnemonics always case-insensitive (standard)
- **Priority:** P0 (Must-have)

**FR-14: Z80 Instruction Set**
- **Description:** Full Z80 instruction set support via existing z80 CPU module
- **Acceptance Criteria:**
  - ✓ All standard Z80 mnemonics (LD, ADD, JP, CALL, etc.)
  - ✓ Zilog syntax preferred (matches EDTASM/M80 conventions)
  - ✓ All addressing modes (immediate, direct, indexed, indirect)
  - ✓ Index registers (IX, IY) with displacements
  - ✓ Undocumented instructions (optional, via flag)
- **Priority:** P0 (Must-have) - Provided by existing z80 module

**FR-15: TRS-DOS Binary Output**
- **Description:** Generate TRS-DOS compatible binary formats
- **Acceptance Criteria:**
  - ✓ `-Fbin` flat binary output (memory image)
  - ✓ `-Fcmd` TRS-DOS /CMD executable format (with load address header)
  - ✓ Support for multiple ORG segments
  - ✓ Support for multiple ASEG/CSEG/DSEG segments (absolute output)
  - ✓ Entry point specification via `END` directive
  - ✓ Compatible with TRS-DOS SYSTEM command
- **Priority:** P0 (Must-have)

**FR-16: Listing Control Directives**
- **Description:** Support disk version listing control
- **Acceptance Criteria:**
  - ✓ `PAGE` - Start new listing page
  - ✓ `TITLE` text - Set page header (32 char limit)
  - ✓ `SUBTTL` text - Set subtitle (M80 extension)
  - ✓ `LIST` / `NOLIST` - Enable/disable listing
  - ✓ `.LIST` / `.XLIST` - M80 variants of LIST/NOLIST
  - ✓ `.CREF` / `.XCREF` - Cross-reference control (placeholder in MVP)
  - ✓ Directives affect vasm listing output (`-L` flag)
- **Priority:** P2 (Nice-to-have)

### Non-Functional Requirements

**NFR-1: Performance**
- Assembly speed: ≥10,000 lines/second on modern hardware
- Multi-pass resolution: Up to 1500 passes (standard vasm)
- Memory footprint: <100MB for typical 64K program

**NFR-2: Compatibility**
- **Input:** Accept original TRS-80 EDTASM and M80 source code with minimal changes
- **Output:** Generate binaries loadable on TRS-80 emulators and real hardware
- **Syntax:** Match Series 1 Editor/Assembler (1981) and M80 (Microsoft) behavior
- **Extensions:** Support modern enhancements (long labels, free-form) via flags

**NFR-3: Platform Support**
- Unix, Linux, macOS, Windows (via vasm's cross-platform C90 code)
- ANSI C90 compliance
- No platform-specific dependencies

**NFR-4: Error Reporting**
- Clear error messages with line numbers
- Filename and line context in multi-file projects
- Error codes matching vasm conventions
- Warnings for deprecated/ambiguous syntax

**NFR-5: Documentation**
- README.md with comprehensive syntax reference
- Example code demonstrating all directives
- Migration guide from original EDTASM and M80
- Comparison table: EDTASM vs M80 vs 6809 EDTASM

### Out of Scope (Deferred to Phase 2)

**Explicitly NOT included in MVP:**

1. **M80 Relocatable Object Format (.REL)**
   - **Why:** Complex format requiring linker support - 20% of users need this
   - **Future:** Phase 2 will add .REL output format

2. **External Symbol Resolution (EXTRN)**
   - **Why:** Requires linking multiple modules together
   - **Future:** Phase 2 will add with .REL support

3. **LINK-80 Compatible Linker**
   - **Why:** Separate tool, not assembler responsibility
   - **Future:** Phase 2 or separate linker project

4. **Library File Support (.LIB)**
   - **Why:** Requires linker infrastructure
   - **Future:** Phase 2 with linking support

5. **COMMON Blocks**
   - **Why:** Requires relocatable linking
   - **Future:** Phase 2 with .REL format

6. **Integrated Editor**
   - **Why:** Modern users have editors; EDTASM's editor was for limited 1980s systems
   - **Future:** Not planned

7. **TRS-DOS Filesystem Access**
   - **Why:** Use existing tools (trs80-tool, etc.) to extract files first
   - **Future:** Not planned

8. **SYSMACRO/ASM Library**
   - **Why:** Can be provided as separate include file by users
   - **Future:** Not planned (community-provided)

9. **Debugger Integration**
   - **Why:** Use emulator debuggers; could be added later via DWARF support
   - **Future:** Possible Phase 3

10. **8080 Compatibility Mode**
    - **Why:** TRS-80 used Z80; 8080 mode already in z80 module if needed via .8080
    - **Future:** Already available via CPU module

---

## Success Metrics

**KPI 1: M80 Syntax Compatibility Rate**
- **Target:** ≥90% of M80 source code assembles without modification (excluding EXTRN/linking features)
- **Measurement:** Test suite of historical M80 programs from TRS-80 archives

**KPI 2: Binary Compatibility**
- **Target:** 100% byte-for-byte match with original M80 output for non-relocatable code
- **Measurement:** Diff against known-good binaries from original assembler

**KPI 3: User Adoption**
- **Target:** ≥20 GitHub stars or community mentions within 6 months (higher than basic EDTASM)
- **Measurement:** GitHub insights, forum mentions, blog posts, TRS-80 community feedback

**KPI 4: Documentation Completeness**
- **Target:** All 40+ directives documented with examples
- **Measurement:** README.md covers 100% of FR-1 through FR-16

**KPI 5: Test Coverage**
- **Target:** 80-90% coverage of NEW syntax module code
- **Measurement:** Test cases for each directive, macro system, conditional assembly, M80 features

**KPI 6: Professional Developer Satisfaction**
- **Target:** Positive feedback from professional TRS-80 developers
- **Measurement:** Forum posts, GitHub issues, community testimonials

---

## Dependencies

**Technical Dependencies:**
1. **Existing z80 CPU Module** (cpus/z80/) - Already available in vasm ✓
2. **vasm Core** (vasm.c, atom.c, expr.c, etc.) - Already available ✓
3. **6809 EDTASM Syntax Module** (syntax/edtasm/) - Reference architecture ✓
4. **vasm Build System** (Makefile, make.rules) - Already available ✓

**External Dependencies:**
- **TRS-80 Emulator** (xtrs, trs80gp, sdltrs) - For testing binaries
- **M80 Test Code Archive** - Historical TRS-80 M80 source code for validation
- **TRS-DOS Disk Tools** (trs80-tool) - For extracting/injecting files to .DSK images

**Business Dependencies:**
- User testing and feedback from TRS-80 community
- Documentation review by vintage computing experts
- Validation with historical M80 codebases

---

## Assumptions and Constraints

**Assumptions:**
1. Users have TRS-80 emulator or hardware for testing assembled code
2. Users can extract source code from .DSK images using existing tools
3. Legacy M80 code may require minor formatting changes (tabs to spaces, etc.)
4. Case-insensitive default matches original behavior for 90% of code
5. Z80 CPU module in vasm is feature-complete and well-tested
6. Users prefer modern cross-platform assembler over vintage DOS tools
7. Most M80 users don't need relocatable linking (single-file or flat multi-segment projects)
8. Advanced macro features (LOCAL, REPT, IRP) provide significant value

**Constraints:**
1. **ANSI C90 compliance** - Must work with vasm's strict C90 codebase
2. **2-space indentation** - Follow vasm project style (not ai-pack 4-space default)
3. **No breaking changes** - Must not affect existing vasm CPU/syntax modules
4. **Module isolation** - All changes in `syntax/edtasm-m80/` directory
5. **Build system** - Must integrate with existing Makefile/make.rules
6. **Binary name** - Output must be named `vasmz80_edtasm-m80` per vasm conventions
7. **Memory limits** - Z80 address space is 16-bit (64K max)
8. **No TDD on legacy vasm** - Only new code requires test coverage per repo-overrides.md
9. **Timeline** - Target 10-13 weeks for MVP (user approved Hybrid M80 scope)

---

## Implementation Phases

### Phase 1: Foundation + M80 Basics (4-5 weeks)

**Deliverables:**
- Basic syntax module skeleton
- FR-1: Core EDTASM directives (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM)
- FR-2: M80 data directive aliases (DB, DW, DS, DC)
- FR-3: M80 segment management (ASEG, CSEG, DSEG)
- FR-4: Comment syntax (semicolon only)
- FR-5: Identifier rules
- FR-6: Basic conditionals (COND/ENDC)
- FR-7: M80 advanced conditionals (IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF)
- FR-13: Case sensitivity control
- FR-14: Z80 instruction integration (via existing module)
- Unit tests for Phase 1 features

**Success Criteria:**
- `vasmz80_edtasm-m80` binary builds successfully
- Can assemble EDTASM programs
- Can assemble M80 programs using segments and advanced conditionals
- Generates correct binary for simple test cases
- All Phase 1 tests pass

### Phase 2: Advanced Macros + Output (2-3 weeks)

**Deliverables:**
- FR-8: Basic EDTASM macros (MACRO/ENDM with #P1-#P9, #$YM)
- FR-9: M80 advanced macros (LOCAL, REPT, IRP, IRPC, EXITM, concatenation)
- FR-10: INCLUDE directive support
- FR-15: TRS-DOS /CMD format output (output_cmd.c module)
- Multi-segment binary generation
- Entry point handling
- Expanded test suite

**Success Criteria:**
- Complex M80 macros expand correctly
- LOCAL labels work correctly
- REPT/IRP/IRPC iterate properly
- /CMD files load in TRS-80 emulator
- Multi-segment programs work correctly
- INCLUDE directive handles multi-file projects

### Phase 3: Control Directives + Polish (2-3 weeks)

**Deliverables:**
- FR-11: M80 control directives (.Z80, .8080, .RADIX, .PHASE/.DEPHASE)
- FR-12: Symbol visibility (PUBLIC/ENTRY/NAME - documentation only)
- FR-16: Listing control (PAGE, TITLE, SUBTTL, LIST/NOLIST, .LIST/.XLIST)
- Real-world test cases (historical M80 programs)
- Performance optimization
- Comprehensive test suite

**Success Criteria:**
- .PHASE/.DEPHASE work correctly for relocated code
- .RADIX affects number parsing
- Historical M80 programs assemble successfully (≥3 test cases)
- Listing output formatted properly
- Performance ≥10,000 lines/second

### Phase 4: Documentation + Validation (2 weeks)

**Deliverables:**
- Comprehensive README.md
- Example programs demonstrating all features
- Migration guide from original EDTASM and M80
- Comparison table: EDTASM vs M80 vs 6809 EDTASM
- Performance benchmarks
- User acceptance testing with TRS-80 community

**Success Criteria:**
- ≥90% M80 syntax compatibility
- Documentation covers all 40+ directives
- Examples run on TRS-80 emulator
- Positive community feedback
- All KPIs met

---

## Risks and Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **M80 macro complexity** | Medium | High | Incremental implementation, extensive testing, LOCAL simplifies vs #$YM |
| **Scope creep to full .REL** | Medium | High | Strict feature freeze, document Phase 2 roadmap clearly |
| **Timeline slip** | Medium | Medium | Conservative estimates (10-13 weeks), weekly progress checks |
| **DEFW endianness** | Low | High | Test early, verify little-endian output (Z80 convention) |
| **Historical documentation gaps** | Medium | Medium | Research multiple sources, consult vintage computing forums |
| **Limited M80 test code** | Medium | Medium | Request code from TRS-80 community, digitize magazine listings |
| **User confusion (EDTASM vs M80 syntax)** | Low | Low | Clear documentation, examples for both syntaxes, feature matrix |

---

## Technical Consultation Questions for Architect (UPDATED)

### New Questions for M80 Scope:

1. **M80 Directive Table Size:**
   - 40+ directives vs 9 in basic EDTASM
   - Impact on directive lookup performance?
   - Should we use hash table instead of linear search?

2. **Advanced Macro Implementation:**
   - LOCAL: How to generate unique labels? (Counter? Hash?)
   - REPT: Simple loop or need special handling?
   - IRP/IRPC: String parsing strategy?
   - Concatenation (&): Implementation approach?

3. **Segment Management:**
   - ASEG/CSEG/DSEG: Use vasm sections or custom tracking?
   - How to handle multiple segments in /CMD output?
   - Any conflicts with ORG directive?

4. **Advanced Conditionals:**
   - 10 IF variants vs 1 COND - architecture impact?
   - Pass tracking (IF1/IF2) - how to integrate with vasm's multi-pass?
   - String comparison (IFIDN/IFDIF) - implementation strategy?

5. **.PHASE/.DEPHASE:**
   - How does this interact with vasm's section management?
   - Implementation complexity estimate?

6. **Effort Estimate Update:**
   - Original estimate: 7-11 weeks for basic EDTASM
   - M80 hybrid adds: 3-4 weeks?
   - Total realistic: 10-13 weeks?

---

## Comparison: EDTASM vs M80 vs 6809 EDTASM

| Feature | TRS-80 EDTASM | TRS-80 M80 | Color Computer 6809 |
|---------|---------------|------------|---------------------|
| **CPU** | Zilog Z80 | Zilog Z80 | Motorola 6809E |
| **Complexity** | Basic | Professional | Basic |
| **Directives** | 9 core | 40+ | ~15 |
| **Data Byte** | `DEFB` | `DEFB`, `DB` | `FCB` |
| **Data Word** | `DEFW`, `DEFH` | `DEFW`, `DW` | `FDB` |
| **Reserve Space** | `DEFS` | `DEFS`, `DS` | `RMB` |
| **String** | `DEFM` | `DEFM` | `FCC` (with delimiter) |
| **Redefine** | `DEFL` | `DEFL` | `SET` |
| **Comment** | `;` only | `;` only | `*` in col 1, `;` anywhere |
| **Macro Param** | `#P1` | `#P1` | `\\1` |
| **Macro Unique** | `#$YM` | N/A (use LOCAL) | `\\@` |
| **Macro Local** | Manual with `#$YM` | `LOCAL` directive | `\\.label` |
| **Conditionals** | `COND/ENDC` | 10 IF variants + ELSE | `COND/ENDC` |
| **Segments** | No | `ASEG/CSEG/DSEG` | No |
| **Advanced Macros** | No | `LOCAL/REPT/IRP/IRPC` | No |
| **Case Default** | Insensitive | Insensitive | Sensitive (with `-nocase`) |
| **Relocatable** | No | .REL format | No |
| **Linking** | No | LINK-80 | No |
| **Target Market** | Hobbyists ($30) | Professionals ($100) | Hobbyists |

---

## References

**Historical Documentation:**
- Series 1 Editor/Assembler Manual (1981) - Radio Shack
- EDTASM-PLUS Manual (1979) - Microsoft
- Disk Editor/Assembler Manual (M80) - Microsoft MACRO-80
- M80 Reference: https://github.com/Konamiman/M80dotNet/blob/master/MACRO80.txt

**Online Resources:**
- http://www.trs-80.org/edtasm.html - TRS-80 EDTASM overview
- https://archive.org/details/Editor_Assembler_Series_I_1981_Tandy - Series 1 Manual
- https://archive.org/details/Editor_Assembler_Plus_1979_Microsoft_Cassette - EDTASM-PLUS
- https://en.wikipedia.org/wiki/Microsoft_MACRO-80 - M80 Overview

**Technical Resources:**
- vasm documentation: doc/vasm.texi, doc/cpu_z80.texi
- Existing 6809 EDTASM: syntax/edtasm/README.md
- Z80 CPU module: cpus/z80/cpu.c
- Architecture Response: .ai/tasks/2026-01-08_z80-edtasm-research/architecture-response.md
- M80 Scope Analysis: .ai/tasks/2026-01-08_z80-edtasm-research/scope-change-analysis-m80.md

---

## Acceptance Criteria Summary

The Z80 EDTASM-M80 syntax module is **COMPLETE** when:

✅ **Builds successfully:**
- `make CPU=z80 SYNTAX=edtasm-m80` produces `vasmz80_edtasm-m80` binary
- No compiler warnings with `-pedantic -std=c90`

✅ **Core functionality:**
- All P0 (must-have) functional requirements implemented
- All 40+ directives work correctly
- Z80 instructions assemble to correct opcodes
- Comment syntax matches EDTASM/M80 behavior

✅ **M80 Compatibility:**
- ≥90% of M80 code assembles without changes (excluding EXTRN/linking)
- Advanced macros (LOCAL, REPT, IRP, IRPC) work correctly
- Advanced conditionals (10 IF variants) work correctly
- Segment management (ASEG/CSEG/DSEG) works correctly
- .PHASE/.DEPHASE works for relocated code

✅ **Compatibility:**
- Output binaries load and run correctly in TRS-80 emulators
- Case-insensitive mode matches original EDTASM/M80 behavior
- /CMD format files work on real hardware and emulators

✅ **Quality:**
- 80-90% test coverage of new code
- All tests pass
- No memory leaks or crashes
- Error messages clear and helpful

✅ **Documentation:**
- README.md complete with all 40+ directives documented
- Examples for every major feature (EDTASM and M80)
- Migration guide from original EDTASM and M80
- Comparison table showing EDTASM vs M80 differences
- Build and usage instructions

✅ **Community validation:**
- At least 3 historical M80 programs assemble successfully
- Feedback from TRS-80 community is positive
- No major syntax incompatibilities reported

---

**Version:** 2.0 (Hybrid EDTASM-M80)
**Author:** Product Manager (Claude)
**Review Status:** User approved Hybrid M80 scope (Option B)
**Architecture Review:** Pending - Architect needs to assess M80 impact
**Next Steps:**
1. Architect reviews M80 scope impact
2. PM incorporates architecture feedback
3. Engineer begins Phase 1 implementation

**Timeline:** 10-13 weeks (User approved, Architect estimate pending)
