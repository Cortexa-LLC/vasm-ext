# Epics: Z80 EDTASM Syntax Module

**Project:** vasm-ext TRS-80 Model III EDTASM Support
**Created:** 2026-01-08
**Status:** Planning Phase

---

## Epic Overview

This document breaks down the Z80 EDTASM syntax module implementation into manageable epics, each representing a cohesive area of functionality.

---

## Epic 1: Foundation and Core Directives

**Title:** As a TRS-80 developer, I want basic EDTASM directive support so that I can assemble simple programs

**Description:**
Implement the foundational syntax module infrastructure and core EDTASM directives (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM). This epic establishes the basic module structure, parser, and data definition capabilities.

**User Stories:**
- US-001: Syntax module skeleton
- US-002: ORG directive
- US-003: EQU directive
- US-004: DEFL directive
- US-005: END directive
- US-006: DEFB directive
- US-007: DEFW/DEFH directives
- US-008: DEFS directive
- US-009: DEFM directive
- US-010: Case-insensitive mode
- US-011: Basic test suite

**Success Criteria:**
- `vasmz80_edtasm` binary builds successfully
- All 9 core directives implemented and tested
- Can assemble simple "Hello World" equivalent program
- Case-insensitive label/symbol handling works
- Test coverage ≥80% for new code

**Priority:** P0 (Must-have for MVP)
**Estimated Complexity:** XL

---

## Epic 2: Comment Syntax and Identifier Rules

**Title:** As a TRS-80 developer, I want proper EDTASM comment and identifier handling so that my legacy code assembles correctly

**Description:**
Implement TRS-80 EDTASM-specific comment syntax (semicolon only, NO asterisk-in-column-1) and identifier rules (alphanumeric + underscore, no dollar signs). This differs from 6809 EDTASM and requires careful implementation.

**User Stories:**
- US-012: Semicolon comment parsing
- US-013: Remove asterisk-in-column-1 comment support
- US-014: Identifier character validation
- US-015: Label length limits (6 chars traditional, configurable)
- US-016: Comment preservation in listing

**Success Criteria:**
- Semicolon comments work on any line
- Asterisk at column 1 treated as label/mnemonic, not comment
- Dollar signs in identifiers rejected (unlike 6809)
- 6-character label limit enforced (or configurable)
- Legacy TRS-80 code with comments assembles correctly

**Priority:** P0 (Must-have for MVP)
**Estimated Complexity:** M

---

## Epic 3: Z80 CPU Integration

**Title:** As a TRS-80 developer, I want full Z80 instruction support so that I can assemble all TRS-80 assembly code

**Description:**
Integrate the existing vasm z80 CPU module with the EDTASM syntax module. Ensure all Z80 mnemonics, addressing modes, and instruction encodings work correctly with EDTASM syntax conventions.

**User Stories:**
- US-017: Z80 CPU module integration
- US-018: Instruction parsing and encoding
- US-019: Addressing mode support (immediate, direct, indexed, indirect)
- US-020: Index register support (IX, IY with displacements)
- US-021: Test suite for Z80 instructions

**Success Criteria:**
- All standard Z80 instructions assemble correctly
- All addressing modes work (immediate #, direct, indexed (IX+d), indirect (HL))
- Instruction encodings match Z80 specification
- Test coverage for common instruction patterns
- Real TRS-80 code assembles with correct opcodes

**Priority:** P0 (Must-have for MVP)
**Estimated Complexity:** M (leveraging existing z80 module)

---

## Epic 4: TRS-DOS Binary Output

**Title:** As a TRS-80 developer, I want to generate TRS-DOS compatible binaries so that my code runs on real hardware and emulators

**Description:**
Implement TRS-DOS /CMD file format output. This is the standard executable format for TRS-DOS, with load address header and entry point. Also support flat binary output for ROM images.

**User Stories:**
- US-022: Flat binary output (-Fbin)
- US-023: TRS-DOS /CMD format research
- US-024: /CMD format implementation (-Fcmd)
- US-025: Multiple ORG segment handling
- US-026: Entry point from END directive
- US-027: /CMD format testing with emulators

**Success Criteria:**
- `-Fbin` produces flat binary memory images
- `-Fcmd` produces TRS-DOS /CMD executable files
- /CMD files load correctly in xtrs, trs80gp, sdltrs emulators
- Multiple ORG segments handled correctly
- END directive sets entry point in /CMD header
- Real TRS-80 hardware can load and execute /CMD files

**Priority:** P0 (Must-have for MVP)
**Estimated Complexity:** L

---

## Epic 5: Include File Support

**Title:** As a TRS-80 developer, I want to include external files so that I can build modular multi-file projects

**Description:**
Implement the INCLUDE directive for modular assembly. Support multiple include paths, recursive includes (with depth limits), and both TRS-DOS (8.3) and modern filename conventions.

**User Stories:**
- US-028: INCLUDE directive parsing
- US-029: File loading and path resolution
- US-030: Multiple include paths (-I flag)
- US-031: Recursive include support with depth limit
- US-032: TRS-DOS 8.3 filename handling
- US-033: Cross-platform path separator support
- US-034: Include file test suite

**Success Criteria:**
- `INCLUDE filename` loads external source file
- Multiple `-I` paths searched in order
- Recursive includes work (max depth 10-20)
- Both `INCLUDE MYLIB/ASM` (TRS-DOS) and `INCLUDE mylib.asm` (modern) work
- Error messages show correct filename and line numbers
- Multi-file projects assemble successfully

**Priority:** P1 (Should-have)
**Estimated Complexity:** M

---

## Epic 6: Conditional Assembly

**Title:** As a TRS-80 developer, I want conditional assembly directives so that I can build configurable code

**Description:**
Implement EDTASM-PLUS conditional assembly directives (COND/ENDC). Support expression evaluation, nested conditionals, and proper symbol resolution within conditional blocks.

**User Stories:**
- US-035: COND/ENDC directive parsing
- US-036: Expression evaluation for conditions
- US-037: Nested conditional support
- US-038: Symbol scope in conditional blocks
- US-039: Conditional assembly test suite

**Success Criteria:**
- `COND expression` / `ENDC` blocks work correctly
- Non-zero expression = true, zero = false
- Nested conditionals up to reasonable depth (10-20 levels)
- Symbols defined in false blocks don't leak
- Legacy code with conditionals assembles correctly

**Priority:** P1 (Should-have)
**Estimated Complexity:** M

---

## Epic 7: Macro System

**Title:** As a TRS-80 developer, I want macro support with #param syntax so that I can use EDTASM-PLUS macros

**Description:**
Implement the EDTASM-PLUS macro system with `#param` syntax (differs from 6809's `\\param`). Support parameter substitution, `#$YM` unique ID generation, macro expansion control, and listing integration.

**User Stories:**
- US-040: MACRO/ENDM directive parsing
- US-041: Macro definition storage
- US-042: Macro parameter parsing (#P1, #P2, etc.)
- US-043: #$YM unique ID generation
- US-044: Macro invocation and expansion
- US-045: Nested macro invocation (not definition)
- US-046: Macro listing control (*MLIST ON/OFF)
- US-047: Macro test suite with complex cases

**Success Criteria:**
- `MACRO` / `ENDM` define macros correctly
- Parameters `#P1` through `#PN` substitute correctly (up to 16 params)
- `#$YM` generates unique 4-char ID per expansion
- Macro invocations work with correct argument passing
- Nested macro calls work (but not nested definitions)
- Macro expansions appear in listing when enabled
- Real-world TRS-80 macros assemble successfully

**Priority:** P1 (Should-have - essential for real code)
**Estimated Complexity:** XL

---

## Epic 8: Listing Control

**Title:** As a TRS-80 developer, I want listing control directives so that I can generate formatted assembly listings

**Description:**
Implement disk version listing control directives (PAGE, TITLE, LIST/NOLIST). Integrate with vasm's listing output system to produce properly formatted listings matching EDTASM style.

**User Stories:**
- US-048: PAGE directive
- US-049: TITLE directive (32 char limit)
- US-050: LIST/NOLIST directives
- US-051: Listing format integration
- US-052: Listing control test suite

**Success Criteria:**
- `PAGE` starts new listing page
- `TITLE text` sets page header (max 32 chars)
- `LIST` / `NOLIST` enable/disable listing sections
- Listing output (`-L` flag) shows proper formatting
- Page headers, titles, and pagination work correctly

**Priority:** P2 (Nice-to-have)
**Estimated Complexity:** M

---

## Epic 9: Columnar Format Support

**Title:** As a TRS-80 developer, I want columnar format support so that I can assemble traditional fixed-column source code

**Description:**
Implement optional support for traditional EDTASM columnar format (labels cols 1-6, mnemonics cols 9-15, operands col 17+). This is optional since free-form layout should also work.

**User Stories:**
- US-053: Columnar format parser
- US-054: Column position detection
- US-055: Tab expansion for columnar format
- US-056: Free-form fallback
- US-057: Columnar format test suite

**Success Criteria:**
- Traditional columnar source assembles correctly
- Labels in columns 1-6 recognized
- Mnemonics in columns 9-15 recognized
- Operands starting column 17+ recognized
- Free-form (modern) layout also works
- Mixed columnar/free-form handled gracefully

**Priority:** P1 (Should-have for compatibility)
**Estimated Complexity:** M

---

## Epic 10: Documentation and Examples

**Title:** As a TRS-80 developer, I want comprehensive documentation so that I can learn and use the assembler effectively

**Description:**
Create comprehensive documentation including README, syntax reference, examples, migration guide, and comparison with 6809 EDTASM. Provide real-world example programs demonstrating all features.

**User Stories:**
- US-058: README.md with syntax reference
- US-059: Example programs (Hello World, macro demo, conditional demo)
- US-060: Migration guide from original EDTASM
- US-061: Comparison table vs 6809 EDTASM
- US-062: Build and usage instructions
- US-063: Troubleshooting guide
- US-064: Integration with TRS-80 emulators guide

**Success Criteria:**
- README.md covers all directives with examples
- At least 3 example programs provided
- Migration guide addresses common compatibility issues
- Comparison table clearly shows TRS-80 vs CoCo differences
- Build instructions work on Linux, macOS, Windows
- Documentation reviewed by TRS-80 community

**Priority:** P0 (Must-have for release)
**Estimated Complexity:** L

---

## Epic 11: Testing and Validation

**Title:** As a TRS-80 developer, I want comprehensive testing so that I can trust the assembler output

**Description:**
Create comprehensive test suite covering all directives, edge cases, error conditions, and real-world code. Validate against historical TRS-80 programs and achieve ≥95% syntax compatibility.

**User Stories:**
- US-065: Unit tests for each directive
- US-066: Integration tests for multi-file projects
- US-067: Macro system edge case tests
- US-068: Historical program test suite (magazine listings, games)
- US-069: Binary compatibility validation
- US-070: Error message quality tests
- US-071: Performance benchmarking

**Success Criteria:**
- 80-90% test coverage of new code
- All core functionality unit tested
- At least 5 historical TRS-80 programs assemble successfully
- Binary output matches original EDTASM (where available)
- Error messages clear and helpful
- Performance ≥10,000 lines/second
- No memory leaks or crashes

**Priority:** P0 (Must-have for quality)
**Estimated Complexity:** XL

---

## Epic Summary

| Epic | Priority | Complexity | User Stories | Dependencies |
|------|----------|------------|--------------|--------------|
| **1. Foundation and Core Directives** | P0 | XL | US-001 to US-011 | None |
| **2. Comment Syntax and Identifiers** | P0 | M | US-012 to US-016 | Epic 1 |
| **3. Z80 CPU Integration** | P0 | M | US-017 to US-021 | Epic 1 |
| **4. TRS-DOS Binary Output** | P0 | L | US-022 to US-027 | Epic 1 |
| **5. Include File Support** | P1 | M | US-028 to US-034 | Epic 1 |
| **6. Conditional Assembly** | P1 | M | US-035 to US-039 | Epic 1 |
| **7. Macro System** | P1 | XL | US-040 to US-047 | Epic 1, 5 |
| **8. Listing Control** | P2 | M | US-048 to US-052 | Epic 1 |
| **9. Columnar Format Support** | P1 | M | US-053 to US-057 | Epic 1, 2 |
| **10. Documentation and Examples** | P0 | L | US-058 to US-064 | All epics |
| **11. Testing and Validation** | P0 | XL | US-065 to US-071 | All epics |

---

## Implementation Sequence

**Phase 1 (MVP):** Epics 1, 2, 3, 4 - Foundation, comments, Z80 integration, binary output
**Phase 2 (Disk Features):** Epics 5, 8 - INCLUDE support, listing control
**Phase 3 (Advanced):** Epics 6, 7, 9 - Conditional assembly, macros, columnar format
**Phase 4 (Release):** Epics 10, 11 - Documentation, comprehensive testing

---

**Version:** 1.0
**Last Updated:** 2026-01-08
**Next Steps:** Create detailed user stories with Given-When-Then acceptance criteria
