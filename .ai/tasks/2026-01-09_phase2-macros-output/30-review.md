# Review Report

**Task ID:** [YYYY-MM-DD_task-name]
**Review Date:** [Date]
**Reviewer:** [Name/Role]
**Review Type:** [Code Review | Quality Review | Security Review | Performance Review]

---

## Review Summary

**Overall Assessment:** [Approved | Approved with Suggestions | Changes Requested | Rejected]

**Summary:**
[Brief 2-3 sentence summary of review findings]

---

## Files Reviewed

```
□ path/to/file1.ext - [Reviewed thoroughly]
□ path/to/file2.ext - [Reviewed thoroughly]
□ path/to/file3.ext - [Spot checked]
□ path/to/test1.test.ext - [Reviewed thoroughly]
```

**Total Files:** [X]
**Lines Changed:** +[X] / -[X]

---

## Test Results Verification

### Test Execution
```
✓ All tests passing: [X/X]
✓ Coverage: [X]% (target: 80-90%)
✓ Coverage of new code: [X]%
✓ Critical paths covered: [X]%
```

### Test Quality Assessment
```
✓ Tests are meaningful
✓ Tests verify behavior (not implementation)
✓ Tests are independent
✓ Tests are repeatable
✓ Edge cases tested
✓ Error paths tested
```

**Test Issues (if any):**
```
□ [Test issue 1]
□ [Test issue 2]
```

---

## Standards Compliance Check

### Formatting and Style
```
✓ Consistent formatting (spaces, not tabs)
✓ Follows language-specific style guide
✓ Naming conventions followed
✓ No commented-out code
✓ File organization appropriate
```

**Issues:** [None | See findings below]

---

### Design Principles
```
□ Single Responsibility Principle
□ Open-Closed Principle
□ Liskov Substitution Principle
□ Interface Segregation Principle
□ Dependency Inversion Principle
□ DRY (Don't Repeat Yourself)
□ YAGNI (You Aren't Gonna Need It)
```

**Issues:** [None | See findings below]

---

### Code Quality
```
□ Functions/methods focused and small
□ Classes have clear responsibilities
□ Appropriate abstractions
□ Low coupling, high cohesion
□ No code smells
□ Complexity reasonable
□ Error handling appropriate
```

**Issues:** [None | See findings below]

---

## Architecture Consistency

### Pattern Consistency
```
✓ Follows established patterns
✓ Layer boundaries respected
✓ Dependencies in correct direction
✓ Separation of concerns maintained
```

### Integration
```
✓ Integrates cleanly with existing code
✓ No breaking changes (or documented)
✓ Backward compatibility maintained (if required)
✓ API changes documented
```

**Issues:** [None | See findings below]

---

## Security Review

### Security Checklist
```
□ Input validation implemented
□ Output sanitization applied
□ Authentication checked (if applicable)
□ Authorization verified (if applicable)
□ Sensitive data encrypted
□ Secrets not in code
□ SQL injection prevented
□ XSS prevented
□ CSRF protection (if web)
```

**Security Issues:** [None | See critical findings below]

---

## Performance Review

### Performance Impact
```
Assessed: [Yes | No | N/A]

Impact: [None | Minimal | Acceptable | Concerning]

Metrics:
- [Metric 1]: [Value] (baseline: [X], target: [Y])
- [Metric 2]: [Value] (baseline: [X], target: [Y])
```

**Performance Issues:** [None | See findings below]

---

## Findings

### Critical Findings
**Must fix before approval**

```
[C1] [Title]
Location: file.ext:42
Severity: Critical
Issue: [Clear description]
Impact: [What could go wrong]
Recommendation: [How to fix]

[C2] [Title]
...
```

**Total Critical:** [X]

---

### Major Findings
**Should fix before approval**

```
[M1] [Title]
Location: file.ext:108
Severity: Major
Issue: [Clear description]
Impact: [Effect on quality/maintainability]
Recommendation: [How to improve]

[M2] [Title]
...
```

**Total Major:** [X]

---

### Minor Findings
**Consider for improvement**

```
[m1] [Title]
Location: file.ext:205
Severity: Minor
Issue: [Description]
Suggestion: [Optional improvement]

[m2] [Title]
...
```

**Total Minor:** [X]

---

## Positive Observations

**What Was Done Well:**
```
✓ [Good practice 1]
✓ [Good practice 2]
✓ [Good practice 3]
```

**Highlights:**
```
- [Particularly well-implemented aspect]
- [Good design decision]
- [Excellent test coverage for X]
```

---

## Documentation Review

### Code Documentation
```
□ Public APIs documented
□ Complex logic explained
□ Non-obvious decisions documented
□ Examples provided (where helpful)
□ Comments accurate and current
```

**Issues:** [None | See findings]

---

### Change Documentation
```
□ Commit messages clear
□ Work log complete
□ Breaking changes documented
□ Migration guide (if needed)
□ User-facing docs updated
```

**Issues:** [None | See findings]

---

## Recommended Actions

### Must Do (Blocking Approval)
```
1. [Action 1] - Addresses [Finding ID]
2. [Action 2] - Addresses [Finding ID]
3. [Action 3] - Addresses [Finding ID]
```

### Should Do (Strongly Recommended)
```
1. [Action 1] - Addresses [Finding ID]
2. [Action 2] - Addresses [Finding ID]
```

### Could Do (Nice to Have)
```
1. [Action 1] - Addresses [Finding ID]
2. [Action 2] - Addresses [Finding ID]
```

---

## Review Decision

**Decision:** [Approved | Approved with Suggestions | Changes Requested | Rejected]

### Rationale
[Explanation of decision]

### Conditions for Approval
```
IF changes requested:
  □ [Condition 1 must be met]
  □ [Condition 2 must be met]
  □ Re-review required: [Yes/No]

IF approved with suggestions:
  - Suggestions are non-blocking
  - Consider for future improvements
```

---

## Follow-Up Required

```
□ Re-review after changes: [Yes/No]
□ Security review: [Yes/No/Already Done]
□ Performance testing: [Yes/No/Already Done]
□ User acceptance testing: [Yes/No/Already Done]
```

---

## Reviewer Notes

### Review Process
```
Time spent: [X] hours
Review method: [Detailed line-by-line | Focused on changes | Spot check]
Tools used: [Linters, coverage tools, etc.]
```

### Additional Comments
[Any additional context, observations, or recommendations not captured above]

---

## References

**Standards Applied:**
- [Engineering Standards](../../quality/engineering-standards.md)
- [Code Review Checklist](../../quality/clean-code/06-code-review-checklist.md)
- [Language Guidelines](../../quality/clean-code/lang-*.md)
- [Design Principles](../../quality/clean-code/01-design-principles.md)

**Related Reviews:**
- [Link to previous review]
- [Link to related component review]

---

## Review Sign-Off

**Reviewed By:** [Name]
**Role:** [Reviewer/Lead]
**Date:** [Date]
**Signature:** [Digital signature or confirmation]

---

**Review Version:** 1.0
**Last Updated:** [Date]

---

## Usage Instructions

This template should be instantiated at: `.ai/tasks/YYYY-MM-DD_task-name/30-review.md`

**When to create:**
- After implementation complete
- Before final acceptance
- During review phase of workflow

**Who creates it:**
- Reviewer role (primary)
- Worker role (self-review)
- Orchestrator role (coordinates review)

**Key principles:**
- Be objective and constructive
- Provide specific, actionable feedback
- Explain the "why" behind findings
- Acknowledge good work
- Focus on code, not person
- Use severity appropriately
- Provide examples and suggestions
