# Implementation Plan - Phase 2 Completion

**Task ID:** 2026-01-09_phase2-completion
**Created:** 2026-01-09
**Author:** Engineer
**Plan Version:** 1.0

---

## Approach Summary

This plan implements the two remaining deferred Phase 2 features: LOCAL label replacement and & concatenation operator. The implementation follows existing vasm patterns and integrates with the current macro expansion infrastructure.

**Key Insight:** The codebase analysis reveals that:
1. LOCAL label infrastructure exists (lines 89-437 in syntax.c) but labels aren't substituted
2. The expand_macro() function (lines 2794-2900) already handles #P1-#P9 and #$YM substitution
3. Adding LOCAL substitution and & concatenation can follow these existing patterns

**Technical Decisions:**
- **LOCAL Labels:** Store LOCAL label names in macro definition, generate unique names per invocation during expansion, substitute in expand_macro()
- **& Concatenation:** Add preprocessing pass before tokenization, merge tokens around &, skip & inside strings
- **Storage:** Use hash table to map macro* to LOCAL label lists (avoids modifying core vasm structures)

**Patterns to Use:**
- **#$YM pattern** - Use src->id for unique ID generation per invocation (line 2850)
- **IRP variable pattern** - Check identifiers during expansion and substitute (lines 2801-2825)
- **String skipping** - Use skip_string() to avoid processing & inside quoted strings

---

## Critical Files Identified

### Files to Modify

```
1. syntax/edtasm-m80/syntax.c (current: ~3600 lines)
   - Current: LOCAL parsing works, but no substitution during expansion
   - Current: No preprocessing for & concatenation
   - Changes:
     - Add LOCAL label hash table mapping (macro* -> local_label list)
     - Enhance expand_macro() to substitute LOCAL labels (add ~50 lines)
     - Add preprocess_concatenation() function (~100 lines)
     - Integrate preprocessing into parse() function
     - Add helper functions for hash table management (~50 lines)
   - Total additions: ~200-250 lines

2. syntax/edtasm-m80/syntax_errors.h
   - Current: Error messages 0-49 defined
   - Changes: Add error for & concatenation edge cases (if needed)
   - Total additions: ~5 lines
```

### Files to Create

```
1. tests/edtasm-m80/phase2/test_local_basic.asm (NEW)
   - Purpose: Test basic LOCAL label replacement
   - Content: Simple macro with one LOCAL label

2. tests/edtasm-m80/phase2/test_local_multiple.asm (NEW)
   - Purpose: Test multiple LOCAL labels in same macro

3. tests/edtasm-m80/phase2/test_local_nested.asm (NEW)
   - Purpose: Test nested macros with LOCAL labels

4. tests/edtasm-m80/phase2/test_local_invocations.asm (NEW)
   - Purpose: Verify each invocation gets unique labels

5. tests/edtasm-m80/phase2/test_local_references.asm (NEW)
   - Purpose: Test forward/backward references to LOCAL labels

6. tests/edtasm-m80/phase2/test_concat_basic.asm (NEW)
   - Purpose: Test simple & concatenation

7. tests/edtasm-m80/phase2/test_concat_params.asm (NEW)
   - Purpose: Test & with macro parameters

8. tests/edtasm-m80/phase2/test_concat_multiple.asm (NEW)
   - Purpose: Test multiple & in same line

9. tests/edtasm-m80/phase2/test_concat_literals.asm (NEW)
   - Purpose: Test & with numbers and literals

10. tests/edtasm-m80/phase2/test_concat_strings.asm (NEW)
    - Purpose: Verify & inside strings is NOT processed

11. tests/edtasm-m80/phase2/test_local_concat.asm (NEW)
    - Purpose: Integration test with both features

Total new tests: 11 minimum
```

### Files to Read (for context)

```
- parse.h - struct macro definition (lines 24-37)
- vasm.h - struct source definition (need to understand src->id, src->macro)
- syntax/edtasm-m80/syntax.c:2794-2900 - expand_macro() function (reference implementation)
- syntax/edtasm-m80/syntax.c:340-371 - LOCAL label helper functions
```

---

## Step-by-Step Implementation Plan

### Phase 1: LOCAL Label Substitution

**Goal:** Make LOCAL labels automatically substitute with unique names during macro expansion

```
Step 1.1: Add LOCAL label storage for macro definitions
  - Action: Create hash table to map macro* to struct local_label* lists
  - Files: syntax/edtasm-m80/syntax.c (add after line 100)
  - Code:
    - Add macro_local_labels hash table (use vasm's hash functions)
    - Add store_macro_locals(macro *m, struct local_label *list) function
    - Add get_macro_locals(macro *m) function
  - Tests: None (internal infrastructure)
  - Verification: Compiles without warnings

Step 1.2: Store LOCAL labels when macro is defined
  - Action: In ENDM handler, associate current_local_labels with the macro
  - Files: syntax/edtasm-m80/syntax.c (modify ENDM handling)
  - Code:
    - Find handle_endm() or equivalent
    - Before clearing current_local_labels, call store_macro_locals()
    - Create copy of list for storage (don't free original yet)
  - Tests: test_local_basic.asm (just parse, don't expand yet)
  - Verification: Macro definition completes, labels stored

Step 1.3: Generate unique names during macro invocation
  - Action: Create per-invocation LOCAL label map
  - Files: syntax/edtasm-m80/syntax.c (add to source structure or parallel map)
  - Code:
    - Create invocation_local_labels hash or linked list
    - For each LOCAL label in macro, generate new unique name (??nnnn)
    - Use local_label_counter++ for unique IDs
    - Store in src->local_labels_map or parallel structure
  - Tests: test_local_basic.asm (add expansion check)
  - Verification: Each invocation generates different unique names

Step 1.4: Substitute LOCAL labels in expand_macro()
  - Action: Add LOCAL label checking and substitution
  - Files: syntax/edtasm-m80/syntax.c (expand_macro function, line 2794)
  - Code:
    ```c
    /* After IRP variable check, before #P1-#P9 check: */

    /* Check if this identifier is a LOCAL label */
    if (src->macro && ISIDSTART(*s)) {
      char *labname = s;
      int lablen = 0;
      while (ISIDCHAR(*s)) {
        s++;
        lablen++;
      }

      /* Look up in current invocation's LOCAL label map */
      char *unique = find_invocation_local(src, labname, lablen);
      if (unique != NULL) {
        /* Substitute with unique name */
        if (dlen >= strlen(unique)) {
          strcpy(d, unique);
          nc = strlen(unique);
          *line = s;  /* Advance past identifier */
          return nc;
        }
        else
          return -1;  /* Out of space */
      }
      s = labname;  /* Reset if no match */
    }
    ```
  - Tests: test_local_basic.asm, test_local_multiple.asm
  - Verification: LOCAL labels replaced correctly, unique per invocation

Step 1.5: Test comprehensive LOCAL label scenarios
  - Action: Run all LOCAL tests
  - Files: All test_local_*.asm files
  - Tests:
    - test_local_basic.asm - Single LOCAL label
    - test_local_multiple.asm - Multiple LOCAL labels
    - test_local_nested.asm - Nested macros
    - test_local_invocations.asm - Multiple invocations
    - test_local_references.asm - Forward/backward refs
  - Verification: All 5 LOCAL tests pass
```

---

### Phase 2: & Concatenation Operator

**Goal:** Implement token concatenation via & operator in preprocessing

```
Step 2.1: Implement preprocessing function
  - Action: Create preprocess_concatenation() function
  - Files: syntax/edtasm-m80/syntax.c (add before parse())
  - Code:
    ```c
    /* Preprocess line to handle & concatenation operator
     * Merges tokens around & (e.g., "FOO&BAR" -> "FOOBAR")
     * Skips & inside quoted strings
     * Returns preprocessed line in static buffer
     */
    static char *preprocess_concatenation(char *line) {
      static char preprocessed[MAXLINESIZE];
      char *s = line;
      char *d = preprocessed;
      int in_string = 0;
      char string_delim = 0;

      while (*s && (d - preprocessed) < MAXLINESIZE - 1) {
        /* Track string state */
        if (*s == '"' || *s == '\'') {
          if (!in_string) {
            in_string = 1;
            string_delim = *s;
          }
          else if (*s == string_delim) {
            in_string = 0;
          }
          *d++ = *s++;
          continue;
        }

        /* Inside string: copy as-is */
        if (in_string) {
          *d++ = *s++;
          continue;
        }

        /* Check for & concatenation operator */
        if (*s == '&') {
          s++;  /* Skip & */
          s = skip(s);  /* Skip trailing whitespace */
          /* Don't add & or whitespace to output */
          continue;
        }

        /* Regular character */
        *d++ = *s++;
      }
      *d = '\0';
      return preprocessed;
    }
    ```
  - Tests: None yet (internal function)
  - Verification: Compiles without warnings

Step 2.2: Integrate preprocessing into parse()
  - Action: Call preprocess_concatenation() before tokenization
  - Files: syntax/edtasm-m80/syntax.c (in parse() function)
  - Code:
    - Find parse() function
    - Before line is tokenized, add:
      ```c
      /* Preprocess & concatenation if present */
      if (strchr(line, '&') != NULL) {
        line = preprocess_concatenation(line);
      }
      ```
  - Tests: test_concat_basic.asm
  - Verification: Simple concatenation works (FOO&BAR -> FOOBAR)

Step 2.3: Handle edge cases
  - Action: Add special case handling
  - Files: syntax/edtasm-m80/syntax.c (enhance preprocess_concatenation)
  - Code:
    - Multiple & in sequence (A&B&C)
    - & at line boundaries
    - Whitespace around &
    - & in comments (should be ignored)
  - Tests:
    - test_concat_multiple.asm (A&B&C)
    - test_concat_params.asm (#P1&SUFFIX)
    - test_concat_literals.asm (LABEL&123)
  - Verification: All edge cases handled correctly

Step 2.4: Test comprehensive & concatenation scenarios
  - Action: Run all & tests
  - Files: All test_concat_*.asm files
  - Tests:
    - test_concat_basic.asm - Simple concatenation
    - test_concat_params.asm - With macro parameters
    - test_concat_multiple.asm - Multiple & operators
    - test_concat_literals.asm - With numbers/literals
    - test_concat_strings.asm - Verify strings not processed
  - Verification: All 5 & tests pass
```

---

### Phase 3: Integration and Testing

**Goal:** Verify everything works together and no regressions

```
Step 3.1: Integration test
  - Action: Test LOCAL and & together
  - Files: test_local_concat.asm (NEW)
  - Content:
    ```asm
    ; Test LOCAL labels with & concatenation
    MAKEFUNC MACRO
      LOCAL LOOP
    #P1&_START:
    LOOP:
      LD A,(HL)
      INC HL
      DJNZ LOOP
      RET
    ENDM

    MAKEFUNC FOO    ; Should generate FOO_START and ??0001
    MAKEFUNC BAR    ; Should generate BAR_START and ??0002
    ```
  - Tests: test_local_concat.asm
  - Verification: Both features work together

Step 3.2: Regression testing
  - Action: Run ALL existing Phase 2 tests
  - Files: tests/edtasm-m80/phase2/test_*.asm
  - Tests: All 22 existing tests
  - Verification: 22/22 pass (100% pass rate maintained)

Step 3.3: Build verification
  - Action: Full clean build
  - Commands:
    ```bash
    make CPU=z80 SYNTAX=edtasm-m80 clean
    make CPU=z80 SYNTAX=edtasm-m80
    ```
  - Verification:
    - Zero compiler warnings
    - ANSI C90 compliance
    - Binary created successfully

Step 3.4: Performance check
  - Action: Verify no significant performance degradation
  - Method: Assemble large file (e.g., all tests concatenated)
  - Verification: Assembly time < 2x original (preprocessing overhead)
```

---

## Testing Strategy

### Unit Tests (Individual Features)

**LOCAL Label Tests (5 tests):**
```
□ test_local_basic.asm - Single LOCAL label
  - Happy path: One macro, one LOCAL label, two invocations
  - Verify: Each invocation gets different unique name (??0001, ??0002)

□ test_local_multiple.asm - Multiple LOCAL labels
  - Happy path: One macro with 3 LOCAL labels
  - Verify: All three labels get unique names per invocation

□ test_local_nested.asm - Nested macros
  - Happy path: Macro A calls Macro B, both have LOCAL labels
  - Verify: Both sets of LOCAL labels get unique names

□ test_local_invocations.asm - Multiple invocations
  - Happy path: Same macro invoked 5 times
  - Verify: Each invocation gets unique label set

□ test_local_references.asm - Forward/backward refs
  - Happy path: LOCAL label defined after reference
  - Verify: Both forward and backward references work
```

**& Concatenation Tests (5 tests):**
```
□ test_concat_basic.asm - Simple concatenation
  - Happy path: FOO&BAR -> FOOBAR
  - Edge case: Whitespace around & removed

□ test_concat_params.asm - With macro parameters
  - Happy path: #P1&SUFFIX with param=FOO -> FOOSUFFIX
  - Edge case: Multiple params concatenated

□ test_concat_multiple.asm - Multiple & in line
  - Happy path: A&B&C -> ABC
  - Edge case: A&&B (double &)

□ test_concat_literals.asm - With numbers/literals
  - Happy path: LABEL&123 -> LABEL123
  - Edge case: 0x&FF (hex literals)

□ test_concat_strings.asm - Strings not processed
  - Happy path: FCC "A&B" -> keeps & literally
  - Error case: Verify & in strings untouched
```

### Integration Tests (1 test)

```
□ test_local_concat.asm - Both features together
  - Setup: Macro with LOCAL labels and & concatenation
  - Execute: Multiple invocations
  - Verify:
    - LOCAL labels unique per invocation
    - & concatenation produces correct identifiers
    - Both features don't interfere
```

### Acceptance Tests (Existing Test Suite)

```
□ Run all 22 existing Phase 2 tests
  - Given: Clean build of assembler
  - When: Assemble all test_*.asm files
  - Then: All 22 tests pass (100%)

□ Verify no regressions
  - Given: New features implemented
  - When: Run full test suite
  - Then: No new failures, same output as before
```

### Coverage Target

```
- Overall: 100% (all features tested)
- Critical paths: 100% (LOCAL substitution, & preprocessing)
- New code: 100% (all new functions have tests)
- Edge cases: 100% (all known edge cases covered)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
□ vasm core - Macro expansion infrastructure
□ vasm hash table functions - For macro -> LOCAL label mapping
□ vasm memory management (mymalloc/myfree) - For allocations
□ ANSI C90 compiler - gcc/clang with -std=c90
```

### Task Dependencies

```
□ Phase 2 baseline complete - All existing tests passing
□ Build system working - make CPU=z80 SYNTAX=edtasm-m80
□ Test infrastructure - Ability to run .asm tests
```

### Knowledge Dependencies

```
□ Understanding of expand_macro() function (syntax.c:2794-2900)
□ Understanding of LOCAL label infrastructure (syntax.c:89-437)
□ Understanding of vasm source structure (source.h)
□ Understanding of macro structure (parse.h:24-37)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** LOCAL label substitution breaks nested macros
- **Probability:** Medium
- **Impact:** High
- **Mitigation:** Study #$YM implementation which already handles nesting (src->id is unique per invocation)
- **Contingency:** If nesting breaks, use invocation stack to track label scopes

**Risk 2:** & concatenation conflicts with other & uses
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Only process & outside quoted strings, in comments context
- **Contingency:** Add escape sequence (&&) if needed

**Risk 3:** Performance degradation from preprocessing
- **Probability:** Low
- **Impact:** Low
- **Mitigation:** Only preprocess lines that contain & (strchr check)
- **Contingency:** If slow, add caching or lazy preprocessing

**Risk 4:** Memory leaks from LOCAL label storage
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Carefully manage allocation/deallocation, use existing patterns
- **Contingency:** Add cleanup function called at end of assembly

### Integration Risks

**Risk 1:** Breaking existing macro expansion
- **Mitigation:** Run all 22 existing tests after each change, verify no regressions

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git stash (save work in progress)
2. git reset --hard HEAD (revert to last commit)
3. make CPU=z80 SYNTAX=edtasm-m80 clean && make CPU=z80 SYNTAX=edtasm-m80
4. Run all 22 existing tests to verify clean state
```

**Rollback Verification:**
```
✓ All 22 existing tests pass
✓ Zero compiler warnings
✓ Binary size unchanged from baseline
✓ No memory leaks (valgrind if needed)
```

### Git Rollback

```
# If changes committed but broken
git revert <commit-hash>

# If not committed yet
git restore syntax/edtasm-m80/syntax.c
git clean -fd tests/edtasm-m80/phase2/test_local_*.asm
git clean -fd tests/edtasm-m80/phase2/test_concat_*.asm

# Verify
make CPU=z80 SYNTAX=edtasm-m80 clean && make CPU=z80 SYNTAX=edtasm-m80
cd tests/edtasm-m80/phase2 && ./run_tests.sh
```

---

## Performance Considerations

### Expected Performance Impact

```
- LOCAL label substitution: Negligible (same pattern as #P1-#P9, happens only in macros)
- & concatenation: Low (only preprocesses lines with &, simple string operation)
- Overall: < 5% slowdown for macro-heavy files, negligible for normal files
```

### Performance Targets

```
- Assembly time: < 110% of baseline (max 10% slowdown)
- Memory usage: < 105% of baseline (LOCAL label storage overhead)
- Macro expansion: Same performance as #$YM (proven fast)
```

### Monitoring Plan

```
□ Benchmark before/after with large test file
□ Profile if slowdown > 10%
□ Check for memory leaks with valgrind
```

---

## Security Considerations

### Security Checklist

```
□ Input validation: Identifier length checks (prevent buffer overflow)
□ Output sanitization: Use snprintf instead of sprintf
□ Memory bounds: Check dlen before writing to destination buffer
□ No unsafe string functions: strncpy/strncat instead of strcpy/strcat
□ Buffer overflow prevention: Check buffer sizes in preprocess_concatenation()
```

### Security Review Points

```
- expand_macro() buffer handling (line 2794)
- preprocess_concatenation() buffer handling
- LOCAL label name generation (bounded to ??9999 format)
```

---

## Alternative Approaches Considered

### Alternative 1: Store LOCAL labels in macro structure

**Pros:**
- Cleaner design (labels belong to macro)
- No separate hash table needed

**Cons:**
- Requires modifying core vasm struct macro (parse.h)
- Would affect all syntax modules
- Breaks vasm plugin architecture

**Why Not Chosen:**
Violates vasm's architecture principle of not modifying core structures for syntax-specific features

### Alternative 2: Implement & as a token operator

**Pros:**
- More integrated with parser
- Could optimize token merging

**Cons:**
- Much more complex implementation
- Would require modifying tokenization phase
- Higher risk of breaking existing code

**Why Not Chosen:**
Preprocessing approach is simpler, lower risk, and sufficient for M80 compatibility

### Alternative 3: Use #$YM for LOCAL labels (keep workaround)

**Pros:**
- Zero implementation effort
- Already works

**Cons:**
- Not M80-compatible syntax
- User requested completion of these features
- Would leave acceptance at 11/14 instead of 13/14

**Why Not Chosen:**
User explicitly requested these features be completed, wants 100% Phase 2 acceptance

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: LOCAL Label Substitution (4-5 hours)
- Step 1.1: Hash table infrastructure (1 hour)
- Step 1.2: Store labels at ENDM (30 min)
- Step 1.3: Generate unique names (1 hour)
- Step 1.4: Substitute in expand_macro() (1.5 hours)
- Step 1.5: Comprehensive testing (1 hour)

Phase 2: & Concatenation Operator (2-3 hours)
- Step 2.1: Implement preprocessing (1 hour)
- Step 2.2: Integrate into parse() (30 min)
- Step 2.3: Handle edge cases (1 hour)
- Step 2.4: Comprehensive testing (30 min)

Phase 3: Integration and Testing (1-2 hours)
- Step 3.1: Integration test (30 min)
- Step 3.2: Regression testing (30 min)
- Step 3.3: Build verification (15 min)
- Step 3.4: Performance check (15 min)

Testing: 1-2 hours (included above)
Documentation: 30 min

Total: 8-10 hours (1-2 days)
```

---

## Success Metrics

### Completion Criteria

```
✓ LOCAL label replacement implemented and working
✓ & concatenation operator implemented and working
✓ All new tests passing (minimum 11 tests)
✓ All existing tests still passing (22/22)
✓ Zero compiler warnings
✓ Documentation updated
✓ FINAL_ACCEPTANCE.md updated to 13/14 (93%)
```

### Quality Metrics

```
- Test coverage: 100% (all features tested)
- Pass rate: 100% (33/33 tests passing)
- Performance: < 110% of baseline
- Code quality: ANSI C90 compliant, no warnings
- Memory: No leaks (valgrind clean)
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Author: Engineer [2026-01-09]
- [x] User/Stakeholder: Approved via "let's complete the first 2 remaining deferred criteria" [2026-01-09]

**Revision History:**
```
v1.0 - 2026-01-09 - Initial plan based on codebase analysis
```

---

## Notes and Considerations

**Key Implementation Insights:**

1. **LOCAL Labels - Invocation vs. Definition:**
   - The tricky part is that LOCAL labels are declared during macro DEFINITION
   - But unique names must be generated during macro INVOCATION
   - Solution: Store list of label NAMES with macro, generate unique IDs per invocation

2. **& Concatenation - Preprocessing Order:**
   - Must happen BEFORE macro parameter expansion (#P1-#P9)
   - Otherwise #P1&SUFFIX would try to concatenate the literal text "#P1" before substitution
   - Actually NO - must happen AFTER parameter expansion
   - Correct order: Expand parameters first, then process &
   - Need to add second preprocessing pass in expand_macro()

3. **Nesting and Recursion:**
   - Both features must work with nested macros
   - LOCAL labels: Each nesting level gets own unique IDs (use src->id)
   - & concatenation: Works at each expansion level independently

4. **Testing Philosophy:**
   - Start simple (one label, one &)
   - Build to complex (nested, multiple, edge cases)
   - Test features independently first
   - Then test integration
   - Always verify no regressions

5. **Code Style:**
   - Follow existing vasm patterns (see #$YM implementation)
   - Use mymalloc/myfree for allocations
   - Use mystrdup for string copies
   - Match 2-space indentation
   - No C++ features (C90 only)

**Correction to Timeline:**
After analyzing expand_macro(), I realize & concatenation needs to happen DURING expansion, not in parse(). This changes the implementation location but not the complexity.

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-09
**Status:** Ready for implementation
