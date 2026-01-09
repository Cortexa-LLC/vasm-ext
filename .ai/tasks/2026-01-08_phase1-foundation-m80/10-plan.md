# Implementation Plan

**Task ID:** [YYYY-MM-DD_task-name]
**Created:** [Date]
**Author:** [Agent Role]
**Plan Version:** 1.0

---

## Approach Summary

[High-level description of the implementation approach in 2-3 paragraphs]

**Key Technical Decisions:**
- [Decision 1 and rationale]
- [Decision 2 and rationale]
- [Decision 3 and rationale]

**Patterns to Use:**
- [Pattern 1] - [Where and why]
- [Pattern 2] - [Where and why]

---

## Critical Files Identified

### Files to Modify
```
1. path/to/file1.ext
   - Current: [What it does now]
   - Changes: [What will change]

2. path/to/file2.ext
   - Current: [What it does now]
   - Changes: [What will change]
```

### Files to Create
```
1. path/to/new-file1.ext
   - Purpose: [What it will do]
   - Rationale: [Why it's needed]

2. path/to/new-file2.ext
   - Purpose: [What it will do]
   - Rationale: [Why it's needed]
```

### Files to Read (for context)
```
- path/to/reference1.ext - [Why relevant]
- path/to/reference2.ext - [Why relevant]
```

---

## Step-by-Step Implementation Plan

### Phase 1: [Phase Name]
```
Step 1.1: [Description]
  - Action: [What to do]
  - Files: [Which files]
  - Tests: [What tests to add/modify]
  - Verification: [How to verify]

Step 1.2: [Description]
  - Action: [What to do]
  - Files: [Which files]
  - Tests: [What tests to add/modify]
  - Verification: [How to verify]
```

### Phase 2: [Phase Name]
```
Step 2.1: [Description]
  ...
```

### Phase 3: [Phase Name]
```
Step 3.1: [Description]
  ...
```

---

## Testing Strategy

### Unit Tests
```
□ [Component/function 1] - Test cases:
  - Happy path
  - Edge case 1
  - Edge case 2
  - Error case 1

□ [Component/function 2] - Test cases:
  - Happy path
  - Edge case 1
  - Error case 1
```

### Integration Tests
```
□ [Integration scenario 1]
  - Setup: [What to set up]
  - Execute: [What to execute]
  - Verify: [What to verify]

□ [Integration scenario 2]
  ...
```

### Acceptance Tests
```
□ [User scenario 1]
  - Given: [Initial state]
  - When: [User action]
  - Then: [Expected result]

□ [User scenario 2]
  ...
```

### Coverage Target
```
- Overall: 85%+
- Critical paths: 100%
- New code: 90%+
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ [Library/package name] - [Version] - [Purpose]
□ [Service/API] - [What it provides]
□ [Tool] - [What it's used for]
```

### Task Dependencies
```
□ [Task/feature] must be completed first
□ [Infrastructure] must be ready
□ [Permission/access] must be granted
```

### Knowledge Dependencies
```
□ Understanding of [concept/technology]
□ Familiarity with [pattern/framework]
□ Access to [documentation/resource]
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** [Description]
- **Probability:** [Low/Medium/High]
- **Impact:** [Low/Medium/High]
- **Mitigation:** [Strategy]
- **Contingency:** [Backup plan]

**Risk 2:** [Description]
- **Probability:** [Low/Medium/High]
- **Impact:** [Low/Medium/High]
- **Mitigation:** [Strategy]
- **Contingency:** [Backup plan]

### Integration Risks

**Risk 1:** [Description]
- **Mitigation:** [Strategy]

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. [Step to revert changes]
2. [Step to restore state]
3. [Step to verify rollback]
```

**Rollback Verification:**
```
✓ [System returns to previous state]
✓ [All tests pass]
✓ [No data loss]
✓ [Services operational]
```

### Git Rollback
```
# If changes committed
git revert <commit-hash>

# If not committed
git restore .

# Verify
npm test / pytest / cargo test
```

---

## Performance Considerations

### Expected Performance Impact
```
- [Operation 1]: [Expected impact and why]
- [Operation 2]: [Expected impact and why]
```

### Performance Targets
```
- [Metric 1]: [Target value]
- [Metric 2]: [Target value]
```

### Monitoring Plan
```
□ [Metric to monitor]
□ [Tool/method for monitoring]
□ [Alert threshold]
```

---

## Security Considerations

### Security Checklist
```
□ Input validation implemented
□ Output sanitization applied
□ Authentication checked
□ Authorization verified
□ Sensitive data encrypted
□ Secrets not in code
□ SQL injection prevented
□ XSS prevented
□ CSRF protection (if web)
```

### Security Review Points
```
- [Point 1 requiring security review]
- [Point 2 requiring security review]
```

---

## Alternative Approaches Considered

### Alternative 1: [Name]
**Pros:**
- [Pro 1]
- [Pro 2]

**Cons:**
- [Con 1]
- [Con 2]

**Why Not Chosen:**
[Rationale]

### Alternative 2: [Name]
**Pros:**
- [Pro 1]

**Cons:**
- [Con 1]

**Why Not Chosen:**
[Rationale]

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: [X] hours
- Step 1.1: [X] hours
- Step 1.2: [X] hours

Phase 2: [X] hours
- Step 2.1: [X] hours

Phase 3: [X] hours
- Step 3.1: [X] hours

Testing: [X] hours
Documentation: [X] hours

Total: [X] hours
```

---

## Success Metrics

### Completion Criteria
```
✓ All steps completed
✓ All tests passing
✓ Coverage targets met
✓ Performance acceptable
✓ Security validated
✓ Documentation complete
```

### Quality Metrics
```
- Test coverage: [Target %]
- Performance: [Target metrics]
- Code quality: [Linting/review score]
- Security: [No critical issues]
```

---

## Plan Approval

**Plan Status:** [Draft | Approved | Needs Revision]

**Reviewed By:**
- [ ] Author: [Name] [Date]
- [ ] User/Stakeholder: [Name] [Date]

**Revision History:**
```
v1.0 - [Date] - Initial plan
v1.1 - [Date] - [Changes made]
```

---

## Notes and Considerations

[Any additional notes, assumptions, or considerations not captured above]

---

**Plan Version:** 1.0
**Last Updated:** [Date]

---

## Usage Instructions

This template should be instantiated at: `.ai/tasks/YYYY-MM-DD_task-name/10-plan.md`

**When to create:**
- After contract established
- Before implementation begins
- During planning phase of workflow

**Who creates it:**
- Orchestrator (delegates planning)
- Worker (for assigned tasks)
- Plan agent (for complex tasks)

**Key principles:**
- Be specific and actionable
- Break into small steps
- Consider risks upfront
- Plan for testing
- Document alternatives
- Get approval before implementing
