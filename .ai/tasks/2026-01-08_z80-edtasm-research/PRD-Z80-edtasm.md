# Product Requirements Document: Z80 EDTASM Syntax Module

**Feature:** TRS-80 Model III EDTASM Syntax Support for vasm
**Target Platform:** TRS-80 Model I/III/4 (Z80-based systems)
**CPU:** z80 (already available in vasm)
**Created:** 2026-01-08
**Status:** Requirements Definition Phase

---

## Problem Statement

**Problem:** Users who want to assemble vintage TRS-80 Model III Z80 assembly code using the EDTASM syntax currently have no modern cross-assembler that supports this classic Radio Shack assembler syntax.

**Impact:**
- **Vintage computing enthusiasts** cannot assemble legacy TRS-80 code on modern systems
- **Game developers** working on TRS-80 homebrew projects lack tooling
- **Preservation community** cannot maintain and build historical TRS-80 software
- **Educators** teaching Z80 assembly for TRS-80 platforms have limited options

**Current State:**
- vasm has excellent z80 CPU support but no EDTASM syntax module
- vasm has 6809 EDTASM syntax (for Color Computer) but it's incompatible with TRS-80 Z80
- Users must use vintage emulators or wine-wrapped DOS assemblers
- No cross-platform build pipeline exists for TRS-80 software development

**Opportunity:**
- Leverage existing z80 CPU module (already in vasm)
- Leverage existing 6809 EDTASM syntax module as architectural reference
- Provide modern, cross-platform TRS-80 development toolchain
- Enable preservation and new development for TRS-80 platform

---

## Target Users

**Primary Users:**
1. **Vintage Computing Enthusiasts** - Building/maintaining TRS-80 software
2. **Homebrew Developers** - Creating new games and utilities for TRS-80
3. **Software Preservationists** - Reassembling and documenting historical code
4. **Educators** - Teaching Z80 assembly language using TRS-80 context

**User Goals:**
- Assemble legacy TRS-80 EDTASM source code without modification
- Build multi-file TRS-80 projects on modern operating systems (Linux, macOS, Windows)
- Generate TRS-DOS compatible binary formats
- Use modern build tools (Make, shell scripts) with TRS-80 development

**User Context:**
- Working with original TRS-80 source code listings from magazines, books, archives
- Developing new software for TRS-80 emulators (xtrs, trs80gp, sdltrs)
- Building disk images for real TRS-80 hardware via modern interfaces
- Teaching Z80 programming in educational settings

---

## Product Requirements

### Functional Requirements

**FR-1: Core EDTASM Directive Set**
- **Description:** Support the complete Series 1 Editor/Assembler directive set
- **Acceptance Criteria:**
  - ✓ `ORG` - Set assembly origin address
  - ✓ `EQU` - Assign permanent value to symbol
  - ✓ `DEFL` - Define temporary/redefinable label
  - ✓ `END` - Mark program end, set entry point
  - ✓ `DEFB` - Define bytes (8-bit data)
  - ✓ `DEFW` - Define words (16-bit data, little-endian Z80 format)
  - ✓ `DEFH` - Define halfwords (alias for DEFW)
  - ✓ `DEFS` - Reserve storage space
  - ✓ `DEFM` - Define message strings
  - ✓ All directives case-insensitive by default
- **Priority:** P0 (Must-have)

**FR-2: Comment Syntax**
- **Description:** Support TRS-80 EDTASM comment conventions
- **Acceptance Criteria:**
  - ✓ Semicolon (`;`) indicates start of comment anywhere on line
  - ✓ NO asterisk-in-column-1 comment support (differs from 6809 EDTASM)
  - ✓ Comments preserved in listing output
  - ✓ Empty lines and comment-only lines allowed
- **Priority:** P0 (Must-have)

**FR-3: Identifier Rules**
- **Description:** Support TRS-80 EDTASM identifier conventions
- **Acceptance Criteria:**
  - ✓ Labels start with letter (A-Z, case-insensitive)
  - ✓ Labels can contain letters, digits, underscore
  - ✓ Labels up to 6 characters (original EDTASM limit) OR configurable limit
  - ✓ Case-insensitive by default (can distinguish LABEL vs label optionally)
  - ✓ Dollar sign NOT supported in identifiers (Z80 has no OS-9 equivalent)
- **Priority:** P0 (Must-have)

**FR-4: Columnar Source Format Support**
- **Description:** Support traditional EDTASM columnar layout (optional)
- **Acceptance Criteria:**
  - ✓ Labels in columns 1-6 (traditional)
  - ✓ Mnemonics in columns 9-15 (traditional)
  - ✓ Operands starting column 17+ (traditional)
  - ✓ Free-form layout also supported (modern style)
  - ✓ Tab-separated fields supported
  - ✓ Whitespace-separated fields supported (modern)
- **Priority:** P1 (Should-have)

**FR-5: Conditional Assembly**
- **Description:** Support EDTASM-PLUS conditional directives
- **Acceptance Criteria:**
  - ✓ `COND` expression / `ENDC` - Basic conditional assembly
  - ✓ Expression evaluation for condition (non-zero = true)
  - ✓ Nested conditionals supported
  - ✓ Conditional blocks can contain any valid assembly
- **Priority:** P1 (Should-have)

**FR-6: Listing Control Directives**
- **Description:** Support disk version listing control
- **Acceptance Criteria:**
  - ✓ `PAGE` - Start new listing page
  - ✓ `TITLE` text - Set page header (32 char limit)
  - ✓ `LIST` / `NOLIST` - Enable/disable listing
  - ✓ Directives affect vasm listing output (`-L` flag)
- **Priority:** P2 (Nice-to-have)

**FR-7: Include File Support**
- **Description:** Support `INCLUDE` directive for modular assembly
- **Acceptance Criteria:**
  - ✓ `INCLUDE filename` loads external source file
  - ✓ Multiple include paths via `-I` flag
  - ✓ Recursive includes supported (with depth limit)
  - ✓ TRS-DOS filename conventions recognized (8.3 format)
  - ✓ Unix/Linux/Windows path separators supported
- **Priority:** P1 (Should-have)

**FR-8: Macro System (EDTASM-PLUS)**
- **Description:** Support EDTASM-PLUS macro system with `#param` syntax
- **Acceptance Criteria:**
  - ✓ `MACRO` / `ENDM` define macros
  - ✓ Parameter syntax: `#P1`, `#P2`, ... `#PN` (up to 9 or 16 params)
  - ✓ `#$YM` unique ID substitution for local labels
  - ✓ Macro invocation with arguments
  - ✓ Macro expansion in listing (configurable)
  - ✓ No nested macro definitions (per EDTASM-PLUS spec)
- **Priority:** P1 (Should-have) - Essential for real-world code

**FR-9: Case Sensitivity Control**
- **Description:** Support both case-insensitive (default) and case-sensitive modes
- **Acceptance Criteria:**
  - ✓ Default: Case-insensitive (matches original EDTASM behavior)
  - ✓ `-case` flag: Enable case-sensitive mode (for modern mixed-case code)
  - ✓ Applies to labels, symbols, directives
  - ✓ Z80 mnemonics always case-insensitive (standard)
- **Priority:** P0 (Must-have)

**FR-10: Z80 Instruction Set**
- **Description:** Full Z80 instruction set support via existing z80 CPU module
- **Acceptance Criteria:**
  - ✓ All standard Z80 mnemonics (LD, ADD, JP, CALL, etc.)
  - ✓ Zilog syntax preferred (matches EDTASM conventions)
  - ✓ All addressing modes (immediate, direct, indexed, indirect)
  - ✓ Index registers (IX, IY) with displacements
  - ✓ Undocumented instructions (optional, via flag)
- **Priority:** P0 (Must-have) - Provided by existing z80 module

**FR-11: TRS-DOS Binary Output**
- **Description:** Generate TRS-DOS compatible binary formats
- **Acceptance Criteria:**
  - ✓ `-Fbin` flat binary output (memory image)
  - ✓ `-Fcmd` TRS-DOS /CMD executable format (with load address header)
  - ✓ Support for multiple ORG segments
  - ✓ Entry point specification via `END` directive
  - ✓ Compatible with TRS-DOS SYSTEM command
- **Priority:** P0 (Must-have) for `/CMD` format, P2 for advanced formats

### Non-Functional Requirements

**NFR-1: Performance**
- Assembly speed: ≥10,000 lines/second on modern hardware
- Multi-pass resolution: Up to 1500 passes (standard vasm)
- Memory footprint: <100MB for typical 64K program

**NFR-2: Compatibility**
- **Input:** Accept original TRS-80 EDTASM source code with minimal changes
- **Output:** Generate binaries loadable on TRS-80 emulators and real hardware
- **Syntax:** Match Series 1 Editor/Assembler (1981) and EDTASM-PLUS behavior
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
- README.md with syntax reference
- Example code demonstrating all directives
- Migration guide from original EDTASM
- Comparison table vs 6809 EDTASM syntax

### Out of Scope

**Explicitly NOT included:**

1. **M80 Advanced Features** - This PRD targets basic EDTASM, not the full M80 macro assembler
   - **Why:** M80 has 40+ directives, relocatable linking, advanced macros - separate project
   - **Future:** Could be added as "edtasm-m80" syntax variant later

2. **Integrated Editor** - Assembler-only, no text editor component
   - **Why:** Modern users have editors; EDTASM's editor was for limited 1980s systems

3. **TRS-DOS Filesystem Access** - No direct reading from .DSK disk images
   - **Why:** Use existing tools (trs80-tool, etc.) to extract files first

4. **SYSMACRO/ASM Library** - No built-in TRS-DOS system call macros
   - **Why:** Can be provided as separate include file by users

5. **Debugger Integration** - No Z-BUG or debug symbol table generation
   - **Why:** Use emulator debuggers; could be added later via DWARF support

6. **8080 Compatibility Mode** - Z80 only, not Intel 8080
   - **Why:** TRS-80 used Z80; 8080 mode already in z80 module if needed

7. **Model 4 Extended Features** - Focus on Model I/III Series 1 EDTASM
   - **Why:** Model 4 had different assemblers; keep scope manageable

---

## Success Metrics

**KPI 1: Syntax Compatibility Rate**
- **Target:** ≥95% of original EDTASM source code assembles without modification
- **Measurement:** Test suite of historical TRS-80 programs (games, utilities, OS-9 code)

**KPI 2: Binary Compatibility**
- **Target:** 100% byte-for-byte match with original EDTASM output for test cases
- **Measurement:** Diff against known-good binaries from original assembler

**KPI 3: User Adoption**
- **Target:** ≥10 GitHub stars or community mentions within 6 months
- **Measurement:** GitHub insights, forum mentions, blog posts

**KPI 4: Documentation Completeness**
- **Target:** All directives documented with examples
- **Measurement:** README.md covers 100% of FR-1 through FR-9

**KPI 5: Test Coverage**
- **Target:** 80-90% coverage of NEW syntax module code
- **Measurement:** Test cases for each directive, macro system, conditional assembly

---

## Dependencies

**Technical Dependencies:**
1. **Existing z80 CPU Module** (cpus/z80/) - Already available in vasm ✓
2. **vasm Core** (vasm.c, atom.c, expr.c, etc.) - Already available ✓
3. **6809 EDTASM Syntax Module** (syntax/edtasm/) - Reference architecture ✓
4. **vasm Build System** (Makefile, make.rules) - Already available ✓

**External Dependencies:**
- **TRS-80 Emulator** (xtrs, trs80gp, sdltrs) - For testing binaries
- **Test Code Archive** - Historical TRS-80 source code for validation
- **TRS-DOS Disk Tools** (trs80-tool) - For extracting/injecting files to .DSK images

**Business Dependencies:**
- User testing and feedback from TRS-80 community
- Documentation review by vintage computing experts

---

## Assumptions and Constraints

**Assumptions:**
1. Users have TRS-80 emulator or hardware for testing assembled code
2. Users can extract source code from .DSK images using existing tools
3. Legacy code may require minor formatting changes (tabs to spaces, etc.)
4. Case-insensitive default matches original behavior for 90% of code
5. Z80 CPU module in vasm is feature-complete and well-tested
6. Users prefer modern cross-platform assembler over vintage DOS tools

**Constraints:**
1. **ANSI C90 compliance** - Must work with vasm's strict C90 codebase
2. **2-space indentation** - Follow vasm project style (not ai-pack 4-space default)
3. **No breaking changes** - Must not affect existing vasm CPU/syntax modules
4. **Module isolation** - All changes in `syntax/edtasm-z80/` directory
5. **Build system** - Must integrate with existing Makefile/make.rules
6. **Binary name** - Output must be named `vasmz80_edtasm` per vasm conventions
7. **Memory limits** - Z80 address space is 16-bit (64K max)
8. **No TDD on legacy vasm** - Only new code requires test coverage per repo-overrides.md

---

## Implementation Phases

### Phase 1: Foundation (MVP)
**Deliverables:**
- Basic syntax module skeleton
- FR-1: Core directives (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM)
- FR-2: Comment syntax (semicolon only)
- FR-3: Identifier rules
- FR-9: Case sensitivity control
- FR-10: Z80 instruction integration (via existing module)
- Simple test cases

**Success Criteria:**
- `vasmz80_edtasm` binary builds successfully
- Can assemble "Hello World" equivalent TRS-80 program
- Generates correct binary for simple test case

### Phase 2: Disk Features
**Deliverables:**
- FR-7: INCLUDE directive support
- FR-6: Listing control (PAGE, TITLE, LIST/NOLIST)
- FR-11: TRS-DOS /CMD format output
- Expanded test suite

**Success Criteria:**
- Multi-file projects assemble correctly
- /CMD files load in TRS-80 emulator
- Listing output formatted properly

### Phase 3: Advanced Features
**Deliverables:**
- FR-5: Conditional assembly (COND/ENDC)
- FR-8: Macro system (MACRO/ENDM with #param syntax)
- FR-4: Columnar format parser (optional)
- Real-world test cases (historical programs)

**Success Criteria:**
- Complex macros expand correctly
- Conditional assembly works with nested blocks
- Historical TRS-80 programs assemble successfully

### Phase 4: Polish and Documentation
**Deliverables:**
- Comprehensive README.md
- Example programs demonstrating all features
- Migration guide from original EDTASM
- Comparison table vs 6809 EDTASM
- Performance optimization
- User acceptance testing with TRS-80 community

**Success Criteria:**
- ≥95% syntax compatibility with original EDTASM
- Documentation covers all features
- Positive community feedback
- KPIs met

---

## Risks and Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Historical documentation incomplete** | Medium | High | Research multiple sources; consult vintage computing forums |
| **Binary format differences** | Low | High | Test with real TRS-80 emulators; compare with original output |
| **Macro system complexity** | Medium | Medium | Study existing vasm macro infrastructure; start with simple cases |
| **Case sensitivity confusion** | Medium | Low | Document clearly; provide examples; make default match original |
| **Limited test code availability** | Medium | Medium | Request code from TRS-80 community; digitize magazine listings |
| **/CMD format undocumented** | Low | Medium | Reverse engineer from known good files; consult emulator docs |
| **Community adoption low** | Medium | Low | Announce on vintage computing forums; provide compelling examples |

---

## Technical Consultation Questions for Architect

1. **Architecture Approach:**
   - Should we fork the 6809 EDTASM module or start fresh?
   - How much code can be shared vs needs Z80-specific adaptations?

2. **Directive Implementation:**
   - DEFB vs FCB - implement as aliases or separate handlers?
   - DEFL (redefinable) - how to integrate with vasm's symbol table?

3. **Macro System:**
   - `#param` vs `\\param` - how to cleanly implement different parameter syntax?
   - `#$YM` unique ID - can we reuse existing vasm macro infrastructure?

4. **Output Format:**
   - TRS-DOS /CMD format - implement as new output module or extend binary output?
   - Where should /CMD code live - `syntax/edtasm-z80/` or `output_trsdos.c`?

5. **Case Sensitivity:**
   - Default case-insensitive - implement at parser level or symbol table level?
   - How does this interact with z80 CPU module?

6. **Build System:**
   - Binary naming: `vasmz80_edtasm` per convention?
   - Any conflicts with existing syntax modules?

---

## Comparison: TRS-80 vs Color Computer EDTASM

| Feature | TRS-80 Model III (Z80) | Color Computer (6809) |
|---------|------------------------|----------------------|
| **CPU** | Zilog Z80 | Motorola 6809E |
| **Data Byte** | `DEFB` | `FCB` |
| **Data Word** | `DEFW`, `DEFH` | `FDB` |
| **Reserve Space** | `DEFS` | `RMB` |
| **String** | `DEFM` | `FCC` (with delimiter) |
| **Redefine** | `DEFL` | `SET` |
| **Comment** | `;` only | `*` in col 1, `;` anywhere |
| **Macro Param** | `#P1` | `\\1` |
| **Macro Unique** | `#$YM` | `\\@` |
| **Macro Local** | Manual with `#$YM` | `\\.label` |
| **Case Default** | Insensitive | Sensitive (with `-nocase`) |
| **OS Integration** | TRS-DOS SYSMACRO | OS-9 system calls (`F$Link`) |
| **Dollar in ID** | No | Yes (for OS-9) |
| **Columnar** | Traditional (cols 1-6-9-17) | Free-form typical |

---

## References

**Historical Documentation:**
- Series 1 Editor/Assembler Manual (1981) - Radio Shack
- EDTASM-PLUS Manual (1979) - Microsoft
- Disk Editor/Assembler Manual (M80) - Microsoft

**Online Resources:**
- http://www.trs-80.org/edtasm.html
- https://archive.org/details/Editor_Assembler_Series_I_1981_Tandy
- https://archive.org/details/Editor_Assembler_Plus_1979_Microsoft_Cassette

**Technical Resources:**
- vasm documentation: doc/vasm.texi, doc/cpu_z80.texi
- Existing 6809 EDTASM: syntax/edtasm/README.md
- Z80 CPU module: cpus/z80/cpu.c

---

## Acceptance Criteria Summary

The z80 EDTASM syntax module is **COMPLETE** when:

✅ **Builds successfully:**
- `make CPU=z80 SYNTAX=edtasm` produces `vasmz80_edtasm` binary
- No compiler warnings with `-pedantic -std=c90`

✅ **Core functionality:**
- All P0 (must-have) functional requirements implemented
- All 9 core directives work correctly
- Z80 instructions assemble to correct opcodes
- Comment syntax matches EDTASM behavior

✅ **Compatibility:**
- ≥95% of historical TRS-80 EDTASM code assembles without changes
- Output binaries load and run correctly in TRS-80 emulators
- Case-insensitive mode matches original EDTASM behavior

✅ **Quality:**
- 80-90% test coverage of new code
- All tests pass
- No memory leaks or crashes
- Error messages clear and helpful

✅ **Documentation:**
- README.md complete with all directives documented
- Examples for every major feature
- Migration guide from original EDTASM
- Build and usage instructions

✅ **Community validation:**
- At least 2 historical TRS-80 programs assemble successfully
- Feedback from TRS-80 community is positive
- No major syntax incompatibilities reported

---

**Version:** 1.0
**Author:** Product Manager (Claude)
**Review Status:** Pending Architect consultation and user approval
**Next Steps:** Create epics and user stories, consult with Architect on technical approach
