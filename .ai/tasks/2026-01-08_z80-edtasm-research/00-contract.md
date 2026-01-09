# Task Contract

**Task ID:** 2026-01-08_z80-edtasm-research
**Created:** 2026-01-08
**Requestor:** User (bryanw)
**Assigned Role:** Product Manager
**Workflow:** Research / Product Definition

---

## Task Description

Research TRS-80 Model III EDTASM assembler syntax and prepare comprehensive product requirements for implementing a Z80 EDTASM syntax module in vasm.

### Background and Context

The vasm-ext project currently supports:
- 6809 EDTASM syntax for TRS-80 Color Computer (syntax/edtasm/)
- Z80 CPU module (cpus/z80/)

However, there is no EDTASM syntax support for TRS-80 Model I/III/4 (Z80-based systems). These systems used completely different syntax from the Color Computer despite sharing the "EDTASM" name.

The TRS-80 vintage computing community needs modern cross-platform tools to assemble legacy code and develop new software for this classic platform.

### Current State

**What exists:**
- vasm has excellent z80 CPU module
- vasm has 6809 EDTASM syntax module (Color Computer)
- TRS-80 developers must use vintage DOS assemblers or emulators
- No modern cross-platform TRS-80 EDTASM assembler exists

**What's missing:**
- Z80 EDTASM syntax module for TRS-80 Model I/III/4
- TRS-DOS /CMD executable output format
- Documentation for TRS-80 development with vasm

### Desired State

**Product Manager deliverables:**
- Comprehensive PRD (Product Requirements Document)
- Epics defining major feature areas
- User stories with acceptance criteria (JIRA-style)
- Technical consultation document for Architect review
- Task packet fully documented for future implementation

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ PRD completed covering all functional requirements
✓ 11 epics defined with success criteria
✓ 71 user stories created with Given-When-Then acceptance criteria
✓ Technical consultation document prepared for Architect
✓ Task packet structure complete with all templates
✓ Historical TRS-80 EDTASM research documented
✓ Comparison table: TRS-80 vs Color Computer EDTASM
✓ All questions for Architect identified and documented
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
✓ PRD document created (PRD-Z80-edtasm.md)
✓ Problem statement clearly defined
✓ Target users identified
✓ 11 functional requirements (FR-1 through FR-11) documented
✓ 5 non-functional requirements documented
✓ Success metrics defined (5 KPIs)
✓ Dependencies identified
✓ Out-of-scope items documented
✓ Implementation phases defined (4 phases)
```

### Deliverables Quality
```
✓ Epics document created (epics.md) with 11 epics
✓ User stories document created (user-stories.md) with detailed stories
✓ Each user story has Given-When-Then acceptance criteria
✓ User stories prioritized (P0/P1/P2)
✓ Complexity estimates provided (S/M/L/XL)
✓ Dependencies between stories identified
✓ Technical consultation document created
✓ All documents follow Product Manager role template standards
```

### Research Quality
```
✓ TRS-80 Model III EDTASM syntax researched
✓ Last available EDTASM version identified (Series 1, 1981)
✓ Key syntax differences from 6809 documented
✓ Directive set documented (DEFB, DEFW, DEFS, DEFM, etc.)
✓ Macro system researched (#param syntax vs \\param)
✓ Comment syntax documented (semicolon only, no asterisk)
✓ Sources cited with URLs
```

---

## Constraints and Dependencies

### Constraints
```
✓ Product Manager role - requirements definition only, no implementation
✓ No code changes in this task
✓ Research limited to publicly available historical documentation
✓ Must align with ai-pack framework (Product Manager role)
✓ Task packet structure must follow template standards
```

### Dependencies
```
✓ Web search for historical TRS-80 documentation
✓ Access to existing vasm codebase for reference
✓ 6809 EDTASM module as architectural reference
✓ Z80 CPU module documentation
✓ Archive.org for TRS-80 manuals
```

### Out of Scope
```
✗ Implementation (that's for Engineer role in future task)
✗ Technical architecture design (that's for Architect role)
✗ Code review or testing
✗ M80 advanced features (separate PRD if needed)
✗ TRS-80 Model 4 specific features (focus on Model I/III)
```

---

## Estimated Complexity

**Complexity:** Medium (for PM research and requirements phase)

**Rationale:**
- Number of files created: 5 (PRD, epics, user stories, technical consultation, contract)
- Lines of documentation: ~3000 lines total
- New concepts/patterns: Vintage TRS-80 assembler syntax research
- Research complexity: Medium (historical documentation, reverse engineering)
- Risk level: Low (pure research, no code changes)

---

## Resources and References

### Relevant Files
```
- syntax/edtasm/README.md - 6809 EDTASM reference implementation
- syntax/edtasm/syntax.c - 6809 EDTASM source code
- cpus/z80/cpu.c - Z80 CPU module
- doc/cpu_z80.texi - Z80 CPU documentation
- .ai-pack/roles/product-manager.md - PM role definition
```

### Documentation
```
- http://www.trs-80.org/edtasm.html - TRS-80 EDTASM overview
- https://archive.org/details/Editor_Assembler_Series_I_1981_Tandy - Series 1 Manual
- https://archive.org/details/Editor_Assembler_Plus_1979_Microsoft_Cassette - EDTASM-PLUS
- https://github.com/Konamiman/M80dotNet/blob/master/MACRO80.txt - M80 reference
```

### Examples
```
- syntax/edtasm/ - 6809 EDTASM implementation (architectural reference)
- Task agent research output - Comprehensive TRS-80 EDTASM research
```

---

## Assumptions

```
1. Historical TRS-80 documentation is available and sufficient for requirements
2. Series 1 Editor/Assembler (1981) is the appropriate target version
3. TRS-80 Model I/III are similar enough to share one syntax module
4. User community exists and will adopt modern cross-platform tooling
5. Z80 CPU module in vasm is complete and compatible
6. 6809 EDTASM module provides good architectural reference
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Historical documentation incomplete or inaccurate
   - Probability: Medium
   - Impact: High
   - Mitigation: Research multiple sources, consult vintage computing forums, validate with community

2. TRS-DOS /CMD format undocumented
   - Probability: Low
   - Impact: Medium
   - Mitigation: Reverse engineer from samples, test with emulators, consult emulator source code

3. Macro #param syntax ambiguous with immediate addressing
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Research Z80 syntax conventions, context-sensitive parsing, Architect consultation

4. Case sensitivity default may confuse users
   - Probability: Medium
   - Impact: Low
   - Mitigation: Clear documentation, good examples, match original EDTASM behavior

5. Limited test code availability
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Request code from TRS-80 community, digitize magazine listings, create synthetic tests
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: User (bryanw) - 2026-01-08
- [x] Agent: Product Manager (Claude) - 2026-01-08

**Changes to Contract:**
- Initial version created 2026-01-08
- All PM deliverables completed 2026-01-08

---

## Notes

**Deliverables Created:**
1. PRD-Z80-edtasm.md - Comprehensive 13-page Product Requirements Document
2. epics.md - 11 epics covering all feature areas
3. user-stories.md - 71 user stories with Given-When-Then acceptance criteria
4. technical-consultation.md - Questions and consultation notes for Architect
5. 00-contract.md - This file

**Key Findings from Research:**
- TRS-80 Model III used Series 1 Editor/Assembler (1981) as last EDTASM version
- Z80 syntax differs significantly from 6809: DEFB vs FCB, semicolon vs asterisk comments, #param vs \\param macros
- TRS-DOS /CMD format is the target executable format (needs research)
- Community has significant interest in vintage TRS-80 development tools

**Next Steps:**
1. User reviews PRD and provides feedback
2. Architect reviews technical consultation document
3. PM and Architect conduct joint refinement session
4. Architect creates technical design document
5. Engineer implements Phase 1 (MVP) based on PRD and design

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-08

---

## Task Complete

This Product Manager task is now complete. All deliverables have been created and documented in the task packet at:

`.ai/tasks/2026-01-08_z80-edtasm-research/`

Ready for user review and next phase (Architect consultation).
