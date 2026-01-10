# Task Contract

**Task ID:** 2026-01-09_phase2-completion
**Created:** 2026-01-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Complete the two remaining deferred Phase 2 criteria to achieve 100% Phase 2 acceptance:
1. LOCAL label replacement - automatic label substitution during macro expansion
2. & concatenation operator - token concatenation in preprocessing

### Background and Context

Phase 2 achieved 11/14 acceptance criteria (79%). Two features were deferred due to complexity vs. benefit trade-offs, with workarounds documented. User has requested completion of these features to achieve 100% Phase 2 acceptance and eliminate the need for workarounds.

### Current State

**LOCAL Labels:**
- Directive parsing works (syntax.c:389-437)
- Generates unique names (??0001, ??0002)
- Tracks labels per macro definition
- Does NOT substitute labels during expansion

**& Concatenation:**
- Not implemented
- Users must design macros without concatenation

### Desired State

**LOCAL Labels:**
- Automatic substitution during macro expansion
- Each invocation gets unique labels
- Works with nested macros
- No workarounds needed

**& Concatenation:**
- Token merging in preprocessing pass
- Works with macro parameters
- Handles all edge cases
- M80-compatible behavior

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ LOCAL label replacement works in all contexts
✓ & concatenation operator works in all contexts
✓ All existing tests still pass (22/22)
✓ All new tests pass (minimum 10)
✓ Zero compiler warnings
✓ Phase 2 acceptance: 14/14 (100%)
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements - LOCAL Labels
```
□ LOCAL labels are automatically replaced with unique names per invocation
□ Each macro invocation gets unique label names (??0001, ??0002, etc.)
□ Label references in macro body are correctly substituted
□ Multiple LOCAL labels in same macro all work correctly
□ Nested macros with LOCAL labels work correctly
□ Forward and backward references to LOCAL labels work
```

### Functional Requirements - & Concatenation
```
□ & operator concatenates adjacent tokens
□ Works with macro parameters (#P1&SUFFIX)
□ Works with literals (LABEL&123)
□ Handles whitespace correctly (spaces around & removed)
□ Multiple & in same line works (A&B&C)
□ & inside quoted strings is ignored
□ Works in all contexts (labels, operands, expressions)
```

### Quality Requirements
```
□ All existing tests passing (22/22)
□ All new tests passing (minimum 10)
□ Zero compiler warnings
□ ANSI C90 compliance
□ No memory leaks
□ Documentation complete
```

### Non-Functional Requirements
```
□ Performance acceptable (no significant slowdown)
□ Error handling robust
□ Code follows vasm style
□ Compatible with existing macros
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain ANSI C90 compliance
□ Must not break existing macro functionality
□ Must follow vasm coding conventions
□ Must integrate with existing symbol table
```

### Dependencies
```
□ Existing macro expansion infrastructure (syntax.c)
□ Existing #$YM unique ID generation
□ vasm symbol table and hash functions
□ Current LOCAL directive parsing code
```

### Out of Scope
```
✗ Changes to other syntax modules (mot, merlin, etc.)
✗ Additional macro features beyond LOCAL and &
✗ Optimization of macro expansion performance
✗ Support for non-M80 macro patterns
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 2-3 (syntax.c, tests)
- Lines of code estimate: ~300-400 lines
- New concepts/patterns: No (follow existing patterns)
- Integration complexity: Medium (symbol table integration)
- Risk level: Medium (affects core macro expansion)

---

## Resources and References

### Relevant Files
```
- syntax/edtasm-m80/syntax.c - Main implementation (LOCAL parsing at 389-437, expand_macro)
- syntax/edtasm-m80/syntax.h - Data structures (struct macro)
- tests/edtasm-m80/phase2/test_*.asm - Existing test patterns
- .ai/tasks/2026-01-09_phase2-macros-output/ - Phase 2 documentation
```

### Documentation
```
- .ai/tasks/2026-01-09_phase2-macros-output/UNMET_CRITERIA.md - Detailed analysis
- .ai/tasks/2026-01-09_phase2-macros-output/FINAL_ACCEPTANCE.md - Current status
- tests/edtasm-m80/phase2/README_CMD.md - Macro documentation
```

### Examples
```
- #$YM implementation - Unique ID generation pattern to follow
- expand_macro() function - Shows how macro bodies are processed
- M80 assembler manual - Token concatenation specification
```

---

## Assumptions

```
1. Existing #$YM implementation provides pattern for unique ID generation
2. Symbol table has APIs for label lookup and substitution
3. Macro expansion already processes lines sequentially
4. M80-compatible behavior is well-documented
5. Existing test infrastructure supports new test cases
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Symbol Table Integration Complexity
   - Probability: Medium
   - Impact: High
   - Mitigation: Study #$YM implementation, use existing symbol APIs

2. Breaking Existing Macros
   - Probability: Low
   - Impact: High
   - Mitigation: Run all 22 existing tests after each change

3. Edge Cases in & Operator
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Comprehensive test suite covering all cases

4. Performance Degradation
   - Probability: Low
   - Impact: Medium
   - Mitigation: Keep preprocessing minimal, benchmark if needed
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: User [2026-01-09]
- [x] Agent: Engineer [2026-01-09]

**Changes to Contract:**
[None yet]

---

## Notes

**Key Implementation Insights:**
- LOCAL: Follow #$YM pattern for unique ID generation
- LOCAL: Need label -> unique name mapping per invocation
- &: Preprocessing must happen before tokenization
- &: Must not process & inside quoted strings
- Both: Must maintain backward compatibility

**Testing Strategy:**
- Start with simple cases, build to complex
- Test each feature independently first
- Then test combined usage
- Verify no regressions in existing tests

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-09
