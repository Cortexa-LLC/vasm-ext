# Technical Consultation Notes: Z80 EDTASM Syntax Module

**Feature:** TRS-80 Model III EDTASM Syntax Support
**PM:** Product Manager (Claude)
**Consulting with:** Architect Role
**Created:** 2026-01-08
**Status:** Pending Architect Review

---

## Purpose

This document presents the Z80 EDTASM PRD to the Architect role for technical feasibility assessment, architecture recommendations, and identification of implementation constraints.

---

## Summary of Requirements

**Product:** Z80 EDTASM syntax module for vasm to support TRS-80 Model I/III/4 assembly
**Target Users:** Vintage computing enthusiasts, homebrew developers, preservationists
**Key Value:** Enable modern cross-platform assembly of TRS-80 Z80 code with EDTASM syntax

### Core Requirements (P0):
1. Core directives: ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM
2. Semicolon-only comment syntax (NO asterisk-in-column-1)
3. Case-insensitive mode by default
4. Full Z80 instruction set (via existing z80 CPU module)
5. TRS-DOS /CMD executable output format
6. Flat binary output

### Important Requirements (P1):
7. INCLUDE directive for multi-file projects
8. COND/ENDC conditional assembly
9. MACRO/ENDM with #param syntax (NOT \\param like 6809)
10. #$YM unique ID generation for macro local labels
11. Columnar format support (optional)

### Nice-to-Have Requirements (P2):
12. Listing control (PAGE, TITLE, LIST/NOLIST)

---

## Technical Consultation Questions

### 1. Architecture Approach

**Question:** Should we fork the existing 6809 EDTASM module (`syntax/edtasm/`) or start fresh for the Z80 version?

**Context:**
- Existing 6809 EDTASM is ~2500 lines, mature, tested
- Shares: module structure, conditional assembly, INCLUDE, listing control
- Differs: directives (DEFB vs FCB), comment syntax (semicolon vs asterisk), macro syntax (#param vs \\param)

**Options:**

**Option A: Fork and Modify**
- **Pros:**
  - Reuse module infrastructure, conditional assembly, INCLUDE handling
  - Proven patterns from working code
  - Faster initial development
- **Cons:**
  - 6809-specific code must be removed/adapted
  - Risk of carrying over 6809 assumptions
  - Directory naming: `syntax/edtasm/` (6809) vs `syntax/edtasm-z80/`?

**Option B: Clean Implementation**
- **Pros:**
  - No 6809 baggage
  - Cleaner, more maintainable code
  - Clear separation of concerns
- **Cons:**
  - Longer development time
  - Must re-implement common features (conditionals, INCLUDE)
  - May miss proven patterns from 6809 version

**PM Recommendation:** Option A (Fork and Modify)
- Faster time to MVP
- Leverage proven conditional/INCLUDE implementations
- Can refactor common code into shared utilities later if needed

**Architect Decision Needed:**
1. Fork or clean implementation?
2. If fork: What's the directory structure? `syntax/edtasm-z80/` or `syntax/edtasm_z80/`?
3. How to handle shared code (if any) between 6809 and Z80 versions?

---

### 2. Directive Implementation

**Question:** How should we implement DEFB vs FCB - as aliases or separate handlers?

**Context:**
- TRS-80 EDTASM: DEFB, DEFW, DEFH, DEFS, DEFM
- 6809 EDTASM: FCB, FDB, FCC, RMB

These are semantically equivalent (different names, same functionality):
- DEFB = FCB (define bytes)
- DEFW = FDB (define words)
- DEFS = RMB (reserve memory bytes)
- DEFM = FCC (define message/string) *but different delimiter handling

**Options:**

**Option A: Separate Directive Handlers**
- Each directive has its own handler function
- More explicit, easier to customize behavior
- Larger directive table

**Option B: Alias Mapping**
- Map DEFB->FCB handler, DEFW->FDB handler, etc.
- Smaller code, reuse logic
- Must ensure semantic compatibility (especially DEFM/FCC delimiter differences)

**PM Recommendation:** Option B (Alias Mapping) for simple directives, separate for DEFM
- DEFB/DEFW/DEFH/DEFS can alias to 6809 handlers
- DEFM needs separate handler (no flexible delimiter like FCC's /.../)

**Architect Decision Needed:**
1. Alias vs separate handlers?
2. Can we safely reuse 6809 data directive handlers?
3. What about DEFM - same as FCC or simpler (quote-only)?

---

**Question:** How should DEFL (redefinable symbols) integrate with vasm's symbol table?

**Context:**
- DEFL allows symbol redefinition (like SET in 6809 EDTASM)
- EQU creates permanent symbols (cannot redefine)
- vasm has symbol flags - need "redefinable" flag

**Options:**

**Option A: Use Existing SET Implementation**
- Map DEFL to vasm's SET directive handler
- Reuse existing redefinable symbol logic

**Option B: Custom DEFL Handler**
- Implement DEFL-specific semantics
- More control over behavior

**PM Recommendation:** Option A (Use SET Implementation)
- DEFL and SET are semantically identical
- Avoid duplicating logic

**Architect Decision Needed:**
1. Can we safely alias DEFL to SET?
2. Are there any DEFL-specific semantics that differ from SET?

---

### 3. Macro System

**Question:** How to implement #param syntax cleanly when 6809 uses \\param?

**Context:**
- TRS-80 EDTASM: `#P1`, `#P2`, ... `#PN`, `#$YM`
- 6809 EDTASM: `\\1`, `\\2`, ... `\\9`, `\\@`, `\\.label`
- vasm has macro infrastructure - need to adapt parameter syntax

**Challenges:**
- # is also used in hex numbers (#$1000 might be ambiguous)
- #$YM is special "unique ID" parameter (like \\@ in 6809)
- Need to distinguish #param from #immediate addressing or #hex

**Options:**

**Option A: Parse # Context-Sensitively**
- In macro body: # followed by letter/identifier = parameter
- In instructions: # followed by digit/$ = immediate/hex value
- Context: "Is this inside a macro definition?"

**Option B: Require Space After #**
- # followed immediately by identifier = parameter
- # followed by number/$ = immediate/hex
- Use whitespace as disambiguator

**PM Recommendation:** Option A (Context-Sensitive)
- # is parameter ONLY inside macro definitions
- Outside macros, # is immediate addressing (if Z80 uses # for immediate)
- Check if Z80 even uses # for immediate (might not!)

**Architect Decision Needed:**
1. Does Z80 assembler syntax use # for immediate addressing?
2. If yes, how to disambiguate #param from #immediate in macro bodies?
3. Can we reuse vasm's macro infrastructure with different param syntax?
4. How to implement #$YM unique ID generation?

---

### 4. Output Format

**Question:** Should TRS-DOS /CMD format be a new output module or extend binary output?

**Context:**
- vasm has output modules: output_bin.c, output_elf.c, etc.
- /CMD format is TRS-DOS-specific executable with header
- Structure: [header bytes][load address][length][data][transfer address]

**Options:**

**Option A: New Output Module (output_trsdos.c or output_cmd.c)**
- **Pros:**
  - Clean separation
  - Follows vasm convention (one format = one module)
  - Can add other TRS-DOS formats later (CAS, etc.)
- **Cons:**
  - More files to maintain
  - Might be overkill for simple format

**Option B: Extend output_bin.c**
- **Pros:**
  - Simpler, fewer files
  - /CMD is essentially binary with header
  - Could use flag: `-Fbin -trsdos-cmd`
- **Cons:**
  - Mixes different output formats
  - Less modular

**Option C: Syntax Module Handles Output**
- **Pros:**
  - EDTASM-specific, keeps it with syntax module
  - END directive directly generates /CMD header
- **Cons:**
  - Unusual pattern in vasm
  - Output modules are separate from syntax modules

**PM Recommendation:** Option A (New Output Module)
- Cleanest architecture
- Consistent with vasm conventions
- Enable with `-Fcmd` flag
- Can be reused by other syntax modules if needed

**Architect Decision Needed:**
1. Where should /CMD format code live?
2. What should the output module be named? (output_trsdos.c? output_cmd.c?)
3. How to pass entry point from END directive to output module?
4. Should we support other TRS-DOS formats in same module (CAS cassette)?

---

### 5. Case Sensitivity

**Question:** Should case-insensitive mode be implemented at parser level or symbol table level?

**Context:**
- TRS-80 EDTASM: Case-insensitive by default (unlike 6809 which is case-sensitive)
- Need: Default case-insensitive, optional case-sensitive via `-case` flag
- vasm has `nocase` global flag

**Options:**

**Option A: Symbol Table Level (Use vasm's nocase flag)**
- **Pros:**
  - Leverage existing infrastructure
  - Global case handling
  - Works for all symbols
- **Cons:**
  - Affects entire assembly, not syntax-specific
  - Default behavior reversed from 6809 EDTASM

**Option B: Parser Level**
- **Pros:**
  - Syntax-module-specific control
  - Can enforce EDTASM-specific rules
- **Cons:**
  - More implementation work
  - Must normalize identifiers before symbol table lookup

**PM Recommendation:** Option A (Symbol Table Level)
- Set `nocase=1` by default in syntax module init
- Override with `-case` command-line flag
- Simplest implementation

**Architect Decision Needed:**
1. Parser vs symbol table level case handling?
2. How to set default `nocase=1` for this syntax module?
3. Does `-case` flag already exist, or do we need to add it?
4. Any interaction issues with z80 CPU module (mnemonics)?

---

### 6. Build System Integration

**Question:** Binary naming and build system integration - any conflicts?

**Context:**
- Naming convention: `vasm<CPU>_<SYNTAX>`
- Proposed: `vasmz80_edtasm`
- Existing: `vasm6809_edtasm` (for 6809 EDTASM)

**Potential Issues:**
- Same SYNTAX name ("edtasm") but different CPUs
- Makefile: `make CPU=z80 SYNTAX=edtasm` - how to distinguish from 6809?
- Directory structure: `syntax/edtasm/` vs `syntax/edtasm-z80/`?

**Options:**

**Option A: Different Syntax Names**
- 6809: `vasm6809_edtasm` (existing, keep as is)
- Z80: `vasmz80_edtasm-z80` (syntax name = "edtasm-z80")
- **Pros:** No ambiguity, clear distinction
- **Cons:** Longer name, inconsistent with naming convention

**Option B: Same Syntax Name, Different Directories**
- Syntax name: "edtasm" for both
- Directories: `syntax/edtasm/` (6809), `syntax/edtasm-z80/` (z80)
- **Pros:** Clean binary names, CPU distinguishes them
- **Cons:** Build system might need adjustment

**Option C: Unified EDTASM Module**
- One syntax module supports both CPUs
- Detects CPU at build time, adapts syntax
- **Pros:** DRY, shared code
- **Cons:** Complex, might not work (too many differences)

**PM Recommendation:** Option B (Same Syntax Name, Different Directories)
- Binary: `vasmz80_edtasm` (clean, matches convention)
- Directory: `syntax/edtasm-z80/` (or `syntax/edtasm_z80/`)
- Let CPU distinguish 6809 vs Z80 versions

**Architect Decision Needed:**
1. What's the official syntax module name? "edtasm" or "edtasm-z80"?
2. Directory name: `syntax/edtasm-z80/` or other?
3. Any Makefile changes needed to support same syntax name, different dirs?
4. Does vasm support CPU-specific syntax variants already?

---

### 7. Z80 CPU Module Integration

**Question:** Any known issues with z80 CPU module that might affect EDTASM syntax?

**Context:**
- z80 CPU module exists, supports Z80/8080/GBZ80/64180
- Multiple syntax modes: Zilog (default), Intel 8080, z80asm
- EDTASM uses Zilog syntax (standard Z80 mnemonics)

**Concerns:**
- Does z80 module expect specific syntax patterns?
- Are there any operand parsing requirements?
- How to disable Intel 8080 mode (ensure Zilog syntax)?

**PM Request:**
- Confirm z80 module works with EDTASM syntax patterns
- Identify any operand syntax incompatibilities
- Guidance on z80 module configuration for EDTASM

**Architect Decision Needed:**
1. Any known z80 CPU module limitations for EDTASM?
2. What flags/init are needed for Zilog syntax mode?
3. How to parse operands for z80 instructions?
4. Test cases available in z80 module?

---

## Architecture Constraints

**Known Constraints:**
1. **C90 compliance** - Must compile with `-std=c90 -pedantic`
2. **2-space indentation** - Follow vasm style (per repo-overrides.md)
3. **Module isolation** - No changes to vasm core or other modules
4. **Build system** - Must integrate with existing Makefile/make.rules
5. **No TDD on legacy vasm** - Only new syntax module code requires tests
6. **Memory limits** - Z80 is 16-bit address space (64K max)

**Platform Requirements:**
- Unix, Linux, macOS, Windows
- Cross-platform build (Make, C90 compiler)

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Macro #param syntax ambiguity** | Medium | High | Context-sensitive parsing, check if Z80 uses # |
| **/CMD format undocumented** | Low | Medium | Reverse engineer from samples, test with emulators |
| **Case sensitivity confusion** | Medium | Low | Clear documentation, good defaults |
| **6809 code reuse assumptions** | Medium | Medium | Careful review, test TRS-80 specific behavior |
| **Build system naming conflicts** | Low | Low | Choose clear directory/syntax names |

---

## Recommended Next Steps

**After Architect Review:**

1. **Architect assesses technical feasibility**
   - Review proposed architecture approaches
   - Identify technical constraints or blockers
   - Recommend implementation patterns

2. **Joint refinement session (PM + Architect)**
   - Discuss trade-offs for open questions
   - Agree on architecture approach
   - Document final decisions

3. **PM updates PRD with technical constraints**
   - Incorporate Architect's feedback
   - Update requirements if needed
   - Document architecture decisions

4. **Architect creates technical design**
   - Module structure and file organization
   - API specifications (directive handlers, etc.)
   - Data structures
   - Integration approach

5. **Engineer implements Phase 1 (MVP)**
   - Foundation and core directives (Epic 1)
   - Comment syntax (Epic 2)
   - Z80 integration (Epic 3)
   - Binary output (Epic 4, at least -Fbin)

---

## Questions for Architect

**Critical Questions (Must Answer):**
1. Fork 6809 EDTASM or clean implementation?
2. Directory/syntax naming: `syntax/edtasm-z80/` or other?
3. How to handle #param macro syntax (context-sensitive parsing)?
4. Where does /CMD format code live (output module or syntax module)?
5. Case-insensitive default - implementation approach?

**Important Questions (Should Answer):**
6. Can we alias DEFB/DEFW/DEFS to 6809 handlers?
7. How to implement DEFL (use SET handler)?
8. How to implement #$YM unique ID generation?
9. Any z80 CPU module limitations or requirements?

**Nice to Know:**
10. Best practices for syntax module directory structure?
11. Recommendations for test organization?
12. Performance considerations for macro expansion?

---

**Status:** Awaiting Architect Review
**Next Action:** Architect conducts technical feasibility assessment
**PM Contact:** Product Manager (Claude)
**Date Created:** 2026-01-08
