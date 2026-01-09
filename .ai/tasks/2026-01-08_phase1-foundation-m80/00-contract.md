# Task Contract: Phase 1 - Foundation + M80 Basics

**Task ID:** 2026-01-08_phase1-foundation-m80
**Created:** 2026-01-08
**Requestor:** Product Manager (from PRD v2.0)
**Assigned Role:** Engineer
**Workflow:** Feature Implementation (Phase 1 of 4)
**Parent Task:** 2026-01-08_z80-edtasm-research (Requirements Definition)

---

## Task Description

Implement Phase 1 (Foundation + M80 Basics) of the Z80 EDTASM-M80 syntax module for vasm. This phase establishes the foundational syntax module infrastructure and implements core EDTASM directives plus essential M80 extensions (segments and advanced conditionals).

### Background and Context

**Product:** Z80 EDTASM-M80 Assembler (vasmz80_edtasm-m80)
- **Purpose:** Modern cross-platform assembler for TRS-80 Model I/III/4 (Z80)
- **Target Users:** Professional TRS-80 developers, preservationists, enthusiasts
- **Unique Value:** First modern assembler with authentic Microsoft M80 compatibility

**Project Status:**
- ✅ Requirements complete (PRD v2.0)
- ✅ Architecture validated (Architect approved)
- ✅ User approved Hybrid M80 scope (10-13 weeks)
- → **NOW: Phase 1 implementation** (4-5 weeks)

**Phase 1 Scope:** Foundation infrastructure + core directives + M80 segments/conditionals
- This phase delivers a working assembler for basic EDTASM + M80 segment management
- Does NOT include: advanced macros (Phase 2), .PHASE/.DEPHASE (Phase 3)
- Subsequent phases will build on this foundation

### Current State

**What exists:**
- vasm core infrastructure (vasm.c, atom.c, expr.c, symbol.c, etc.)
- Z80 CPU module (cpus/z80/) - mature, well-tested
- 6809 EDTASM syntax module (syntax/edtasm/) - reference implementation
- Build system (Makefile, make.rules)
- Comprehensive requirements (PRD v2.0, architecture assessments)

**What's missing:**
- Z80 EDTASM-M80 syntax module (syntax/edtasm-m80/)
- M80-specific directive handlers
- TRS-DOS /CMD output module (deferred to Phase 2)
- Tests for new code

### Desired State

**At Phase 1 completion:**
- Working `vasmz80_edtasm-m80` assembler binary
- Can assemble basic EDTASM programs (9 core directives)
- Can assemble M80 programs using segments (ASEG/CSEG/DSEG)
- Can assemble M80 programs using advanced conditionals (10 IF variants)
- All Phase 1 directives tested and working
- Flat binary output (`-Fbin`) functional
- Foundation ready for Phase 2 (advanced macros)

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ vasmz80_edtasm-m80 binary builds successfully with no warnings
✓ All Phase 1 directives implemented and working (24 directives)
✓ Can assemble basic EDTASM programs (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM)
✓ Can assemble M80 data aliases (DB, DW, DS, DC)
✓ Can assemble M80 segment management (ASEG, CSEG, DSEG)
✓ Can assemble basic conditionals (COND/ENDC)
✓ Can assemble M80 advanced conditionals (IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF)
✓ Semicolon comment syntax works (no asterisk-in-column-1)
✓ Identifier rules enforced (no dollar signs)
✓ Case-insensitive mode by default (nocase=1)
✓ Z80 instructions assemble correctly via existing CPU module
✓ Flat binary output (-Fbin) generates correct output
✓ Multi-segment programs generate correct flat binaries
✓ Phase 1 test suite passes (30-40 test cases minimum)
✓ Code coverage ≥80% for new syntax module code
✓ No memory leaks (valgrind clean)
✓ Performance ≥10,000 lines/second
```

---

## Acceptance Criteria

[Full acceptance criteria included in file - truncated here for brevity]

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-08
**Phase 1 Duration:** 4-5 weeks
**Next Phase:** Phase 2 (Advanced Macros + Output) - 3-4 weeks
