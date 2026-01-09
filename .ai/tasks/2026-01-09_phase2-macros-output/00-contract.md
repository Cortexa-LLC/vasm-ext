# Task Contract: Phase 2 - Advanced Macros + Output

**Task ID:** 2026-01-09_phase2-macros-output
**Created:** 2026-01-09
**Requestor:** Product Manager (from PRD v2.0)
**Assigned Role:** Engineer
**Workflow:** Feature Implementation (Phase 2 of 4)

---

## Task Description

Implement Phase 2 (Advanced Macros + Output) of the Z80 EDTASM-M80 syntax module for vasm. This phase adds sophisticated macro capabilities and TRS-DOS /CMD output format support.

### Background and Context

**Project Status:**
- ✅ Phase 1 complete (Foundation + M80 Basics) - 24 directives working
- ✅ All Phase 1 tests passing (85+ tests)
- ✅ DEFW endianness verified (little-endian)
- → **NOW: Phase 2 implementation** (3-4 weeks)

**Phase 2 Scope:** Advanced macro system + TRS-DOS /CMD output format
- Professional-grade macro capabilities (LOCAL, REPT, IRP, IRPC)
- Native TRS-DOS executable output
- Does NOT include: .PHASE/.DEPHASE (Phase 3), full documentation (Phase 4)

### Current State

**What exists (from Phase 1):**
- Working `vasmz80_edtasm-m80` assembler binary
- 24 directives: Core EDTASM + M80 segments + advanced conditionals
- Case-insensitive mode by default
- Z80 CPU integration
- Flat binary output (`-Fbin`)
- Basic MACRO/ENDM support (inherited from 6809 EDTASM)
- Comprehensive Phase 1 test suite (85+ tests)

**What's missing:**
- Basic EDTASM macros (#P1-#P9, #$YM syntax)
- M80 advanced macros (LOCAL, REPT, IRP, IRPC, EXITM)
- M80 concatenation operator (&)
- INCLUDE directive
- TRS-DOS /CMD output module
- Tests for Phase 2 features

### Desired State

**At Phase 2 completion:**
- Complex M80 macros expand correctly
- LOCAL labels generate unique identifiers
- REPT/IRP/IRPC iterate through blocks
- Concatenation operator (&) works
- INCLUDE directive brings in external files
- TRS-DOS /CMD files load in emulators
- Multi-segment programs assemble to /CMD format
- All Phase 2 directives tested and working

---

## Success Criteria

```
✓ Basic EDTASM macros work (#P1-#P9, #$YM)
✓ M80 LOCAL directive generates unique labels per invocation
✓ M80 REPT directive repeats blocks N times
✓ M80 IRP directive iterates through comma-separated lists
✓ M80 IRPC directive iterates through string characters
✓ M80 EXITM directive exits macros early
✓ Concatenation operator (&) joins tokens
✓ INCLUDE directive processes external files
✓ TRS-DOS /CMD output module implemented
✓ /CMD files verified in TRS-80 emulator
✓ Phase 2 test suite passes (40-50 tests)
✓ Code coverage ≥80% for new code
✓ No memory leaks
✓ Performance ≥10,000 lines/second
```

---

## Acceptance Criteria

### Phase 2 Directives (11 features)

**Basic EDTASM Macros:**
1. MACRO/ENDM with #P1-#P9 parameters
2. #$YM unique macro invocation ID

**M80 Advanced Macros:**
3. LOCAL - Declare local labels
4. REPT - Repeat block N times
5. IRP - Iterate through list
6. IRPC - Iterate through string
7. EXITM - Exit macro early
8. & - Concatenation operator

**File Inclusion:**
9. INCLUDE - Include external file

**Output Format:**
10. TRS-DOS /CMD output module
11. Multi-segment /CMD support

### Functional Requirements
```
✓ Macros expand with #P1-#P9 syntax
✓ #$YM generates unique IDs per invocation
✓ LOCAL labels unique across invocations
✓ REPT repeats blocks correctly
✓ IRP iterates through lists
✓ IRPC iterates through strings
✓ EXITM terminates expansion
✓ & concatenates tokens
✓ INCLUDE processes files recursively
✓ /CMD files have correct structure
✓ /CMD supports multiple segments
✓ /CMD sets entry point correctly
```

### Quality Requirements
```
✓ All Phase 2 tests passing (40-50 tests)
✓ Code coverage ≥80%
✓ No compiler warnings (-std=c90 -pedantic)
✓ No memory leaks (valgrind clean)
✓ Performance acceptable
```

---

## Implementation Plan

### Week 1: Basic Macros + INCLUDE (5-7 days)
- Day 1-2: EDTASM macro syntax (#P1-#P9, #$YM)
- Day 3-4: INCLUDE directive with path resolution
- Day 5: Testing

### Week 2: M80 Advanced Macros (5-7 days)
- Day 6-7: LOCAL directive
- Day 8: REPT directive
- Day 9-10: IRP/IRPC directives
- Day 11: EXITM & concatenation (&)

### Week 3: /CMD Output (5-7 days)
- Day 12-14: output_cmd.c module
- Day 15-16: Multi-segment support
- Day 17: Emulator testing

### Week 4: Testing & Polish (3-5 days)
- Day 18-20: Comprehensive testing (40-50 tests)
- Day 21-22: Bug fixes & polish

---

## Constraints and Dependencies

### Constraints
```
✓ ANSI C90 compliance (no C99 features)
✓ 2-space indentation (vasm style)
✓ Must work on all platforms (Unix, Windows, Amiga)
✓ Little-endian for Z80
```

### Dependencies
```
✓ Phase 1 complete (prerequisite)
✓ vasm core infrastructure
✓ Z80 CPU module
✓ TRS-80 emulator for testing (trs80gp or sdltrs)
```

### Out of Scope
```
✗ .PHASE/.DEPHASE (Phase 3)
✗ .RADIX directive (Phase 3)
✗ Full documentation (Phase 4)
✗ .REL relocatable format (future)
✗ LINK-80 linker (future)
```

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- Files affected: 2-3 (syntax.c, new output_cmd.c, tests)
- Lines of code estimate: ~800-1000 new lines
- New concepts: Macro parameter transformation, /CMD format
- Integration complexity: Medium
- Risk level: Medium

**Critical Path:** Advanced macros (LOCAL, REPT, IRP) - Most complex features

---

## Resources and References

### Architecture Guidance
- `.ai/tasks/2026-01-08_z80-edtasm-research/architecture-m80-assessment.md` (lines 246-513)
- Phase 2 effort breakdown: lines 999-1031

### Reference Implementations
- `syntax/edtasm/syntax.c` - Existing macro infrastructure
- `syntax/mot/syntax.c` - May have REPT/IRP implementations

### Documentation
- `.ai/repo-overrides.md` - C90 rules
- PRD v2.0 - Full requirements

---

## Risk Assessment

### Identified Risks
```
1. Macro complexity - LOCAL labels interaction
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Incremental implementation, TDD

2. /CMD format compatibility
   - Probability: Low
   - Impact: High
   - Mitigation: Test with real emulator early

3. Performance with deep macro nesting
   - Probability: Low
   - Impact: Low
   - Mitigation: Profile if needed
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Product Manager [2026-01-09]
- [ ] Engineer: [Name] [Date] - Sign when starting

**Ready to Start:** YES - All prerequisites met

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-09
**Target Duration:** 3-4 weeks
**Previous Phase:** Phase 1 (COMPLETE)
**Next Phase:** Phase 3 (Control directives)
