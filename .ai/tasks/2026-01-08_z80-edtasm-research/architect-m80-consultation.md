# Architect Consultation: M80 Scope Impact Assessment

**From:** Product Manager (Claude)
**To:** Architect Role
**Date:** 2026-01-08
**Subject:** M80 Hybrid Scope - Architecture Impact Assessment Request
**Status:** Awaiting Architect Response

---

## Context

**User Decision:** Approved Option B (Hybrid EDTASM-M80) - 10-13 weeks timeline

**Scope Change:** Expanded from basic EDTASM (9 directives) to EDTASM + M80 hybrid (40+ directives)

**Previous Architecture Review:** Completed for basic EDTASM scope (architecture-response.md)

**New Requirement:** Assess architecture impact of M80 additions

---

## M80 Additions to Original Scope

### New Directives Added (30+ directives)

#### Data Directive Aliases
- `DB` (alias for DEFB)
- `DW` (alias for DEFW)
- `DS` (alias for DEFS)
- `DC` (define constant)

#### Segment Management
- `ASEG` - Absolute segment
- `CSEG` - Code segment
- `DSEG` - Data segment

#### Advanced Conditionals (10 new IF variants)
- `IF` expression
- `IFE` expression
- `IF1` - Pass 1 only
- `IF2` - Pass 2+ only
- `IFDEF` symbol
- `IFNDEF` symbol
- `IFB` argument (if blank)
- `IFNB` argument (if not blank)
- `IFIDN` arg1,arg2 (if identical)
- `IFDIF` arg1,arg2 (if different)
- `ELSE` clause
- `ENDIF` (in addition to ENDC)

#### Advanced Macros
- `LOCAL` - Automatic unique local labels
- `REPT` count - Repeat block
- `IRP` param,<list> - Iterate list
- `IRPC` param,string - Iterate characters
- `EXITM` - Exit macro early
- Concatenation operator (`&`)
- Special comment (`;;`)
- Literal character (`!`)

#### Control Directives
- `.Z80` - Enable Z80 instructions
- `.8080` - Restrict to 8080
- `.RADIX` base - Set number base
- `.PHASE` address - Phase code
- `.DEPHASE` - End phase

#### Symbol Visibility (Documentation Only)
- `PUBLIC` symbol
- `ENTRY` symbol (alias)
- `NAME` module

#### Listing Extensions
- `SUBTTL` - Subtitle
- `.LIST` / `.XLIST` (M80 variants)
- `.CREF` / `.XCREF` (placeholder)

### Deferred to Phase 2 (Not in MVP)
- `.REL` relocatable object format
- `EXTRN` external symbols
- `COMMON` blocks
- LINK-80 linker
- Library files

---

## Specific Questions for Architect

### Question 1: Timeline Impact Assessment

**Original Estimate (Basic EDTASM):** 7-11 weeks (from architecture-response.md)

**PM Estimate (M80 Hybrid):** 10-13 weeks (+3-4 weeks)

**Question:** Do you agree with the 3-4 week addition for M80 features?

**Breakdown Request:**
- Data aliases (DB/DW/DS/DC): ___ days?
- Segment management (ASEG/CSEG/DSEG): ___ days?
- Advanced conditionals (10 IF variants): ___ days?
- Advanced macros (LOCAL/REPT/IRP/IRPC): ___ days?
- Control directives (.PHASE/.DEPHASE/.RADIX): ___ days?
- Testing and validation: ___ days?

**Total M80 Addition:** ___ weeks?
**New Total Timeline:** ___ weeks?

---

### Question 2: Directive Table Architecture

**Original:** 9 directives (linear search acceptable)
**New:** 40+ directives

**Question:** Should we optimize directive lookup?

**Options:**
- **A.** Keep linear search (simple, sufficient for 40 items)
- **B.** Use hash table (faster, more complex)
- **C.** Binary search (sorted table, moderate complexity)

**Recommendation Needed:** Which approach for 40+ directives?

---

### Question 3: Segment Management Implementation

**New Feature:** ASEG/CSEG/DSEG for code organization

**Questions:**
1. Should we use vasm's existing section mechanism or custom tracking?
2. How to handle interactions with ORG directive?
3. Multiple segments in /CMD output - architecture implications?
4. DSEG (data segment) - separate from code or just labeling?

**Complexity Estimate:** ___ days for segment management?

---

### Question 4: Advanced Macro Architecture

**LOCAL Directive:**
- **Purpose:** Automatic unique labels (better than manual #$YM)
- **Question:** Implementation approach?
  - A. Counter-based (LOCAL_0001, LOCAL_0002, etc.)
  - B. Hash-based (LOCAL_A4F3, etc.)
  - C. Macro name + counter (MYMACRO_0001, etc.)
- **Recommendation:** Which approach?

**REPT Directive:**
- **Purpose:** Repeat block N times
- **Question:** Implementation complexity?
  - Simple loop duplication?
  - Special handling needed?
- **Effort Estimate:** ___ days?

**IRP/IRPC Directives:**
- **Purpose:** Iterate through list/string
- **Question:** String parsing strategy?
- **Complexity:** Low/Medium/High?
- **Effort Estimate:** ___ days?

**Concatenation Operator (&):**
- **Purpose:** Token pasting (e.g., `PREFIX&SUFFIX`)
- **Question:** Implementation approach?
  - Parse-time concatenation?
  - Expansion-time?
- **Effort Estimate:** ___ days?

**Total Advanced Macros Effort:** ___ days?

---

### Question 5: Advanced Conditionals Architecture

**10 IF Variants vs 1 COND:**
- Original: COND/ENDC (simple expression evaluation)
- M80: IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF

**Questions:**
1. Can we reuse COND infrastructure for all IF variants?
2. Pass tracking (IF1/IF2) - how to integrate with vasm's multi-pass?
3. String comparison (IFIDN/IFDIF) - new code or existing utilities?
4. Blank testing (IFB/IFNB) - implementation strategy?

**Complexity Assessment:**
- IF/IFE: ___ (similar to COND?)
- IF1/IF2: ___ (pass tracking complexity?)
- IFDEF/IFNDEF: ___ (symbol table lookup)
- IFB/IFNB: ___ (argument parsing)
- IFIDN/IFDIF: ___ (string comparison)
- ELSE: ___ (extends existing logic)

**Total Advanced Conditionals Effort:** ___ days?

---

### Question 6: .PHASE/.DEPHASE Implementation

**Purpose:** Assemble code for execution at different address than load address

**Example:**
```asm
        ORG $1000      ; Load at $1000
        .PHASE $8000   ; But executes at $8000
CODE    JP ROUTINE     ; Generates JP $8006 (not $1006)
ROUTINE NOP
        .DEPHASE
```

**Questions:**
1. How does .PHASE interact with vasm's section management?
2. Does this require two location counters (load address + exec address)?
3. Complexity estimate?

**Effort Estimate:** ___ days for .PHASE/.DEPHASE?

---

### Question 7: Overall Architecture Impact

**Original Architecture (Basic EDTASM):**
- Fork 6809 EDTASM (~70% code reuse)
- Add Z80-specific directives
- Modify macro syntax (#param)
- Modify comment syntax (semicolon only)

**M80 Additions Impact:**
- Does M80 scope change the fork approach? (Still valid?)
- Any fundamental architecture changes needed?
- New data structures required?
- Risk level change from Low-Medium to Medium-High?

**Assessment Needed:**
- Architecture approach still valid? (Yes/No/Modify?)
- Any new architectural concerns?
- Risk level for M80 hybrid: Low/Medium/High?

---

### Question 8: Testing Impact

**Original:** ~50 test cases for basic EDTASM
**M80 Additions:** Need ~100-150 additional test cases

**Questions:**
1. Testing effort estimate for M80 features?
2. Any special testing concerns (macros, conditionals, segments)?
3. How to validate against original M80 behavior?

**Testing Effort Estimate:** ___ days?

---

### Question 9: Code Size Estimate

**Original Estimate:**
- syntax/edtasm-z80/syntax.c: ~2500 lines
- output_cmd.c: ~300 lines
- Total: ~3400 lines

**M80 Additions:**
- Advanced macro handlers: +___ lines?
- Advanced conditional handlers: +___ lines?
- Segment management: +___ lines?
- Control directives: +___ lines?

**New Total Estimate:** ~___ lines?

---

### Question 10: Implementation Phase Breakdown

**Original Phases (Basic EDTASM):**
1. Foundation (MVP): 3-4 weeks
2. Output Format: 1-2 weeks
3. Advanced Features: 2-3 weeks
4. Polish: 1-2 weeks

**M80 Hybrid Phases (Proposed):**
1. Foundation + M80 Basics: 4-5 weeks
2. Advanced Macros + Output: 2-3 weeks
3. Control Directives + Polish: 2-3 weeks
4. Documentation + Validation: 2 weeks

**Question:** Do these phases make sense? Any reordering recommended?

**Phase-by-Phase Assessment:**
- Phase 1 effort: ___ weeks?
- Phase 2 effort: ___ weeks?
- Phase 3 effort: ___ weeks?
- Phase 4 effort: ___ weeks?

**Total:** ___ weeks realistic estimate?

---

## Priority Questions (Must Answer)

1. **Total timeline for M80 hybrid:** ___ weeks?
2. **Risk level:** Low / Medium / High?
3. **Architecture approach change needed:** Yes / No?
4. **Most complex M80 feature:** LOCAL / REPT / IRP / .PHASE / other?
5. **Critical path item:** Which M80 feature gates others?

---

## Architect Deliverable Request

Please provide updated architecture assessment document covering:

1. ✅ **Timeline Impact** - Detailed breakdown of M80 additions (+3-4 weeks realistic?)
2. ✅ **Complexity Assessment** - Each M80 feature group (macros, conditionals, segments)
3. ✅ **Architecture Changes** - Any modifications to original architecture-response.md
4. ✅ **Risk Assessment** - Updated risk matrix for M80 hybrid scope
5. ✅ **Implementation Phases** - Revised phase breakdown with effort estimates
6. ✅ **Directive Table** - Recommendation for 40+ directive lookup optimization
7. ✅ **Data Structures** - Any new structures needed for M80 features
8. ✅ **Code Size Estimate** - Updated LOC estimate for M80 hybrid
9. ✅ **Testing Strategy** - Testing approach for M80 features
10. ✅ **Critical Path** - What's the longest/most complex M80 addition?

---

## Background Documents

For context, please review:

1. **Original PRD:** PRD-Z80-edtasm.md (basic EDTASM scope)
2. **Updated PRD:** PRD-Z80-edtasm-m80-v2.md (M80 hybrid scope)
3. **Original Architecture:** architecture-response.md (basic EDTASM assessment)
4. **M80 Analysis:** scope-change-analysis-m80.md (PM's scope change analysis)
5. **M80 Reference:** MACRO80.txt (M80 directive reference)

---

## Expected Output

Please create: **architecture-m80-assessment.md**

Including:
- Detailed timeline breakdown (10-13 weeks realistic?)
- Feature-by-feature complexity estimates
- Updated risk assessment
- Implementation recommendations
- Any architecture concerns or blockers

---

**Status:** Awaiting Architect Response
**Priority:** High (blocks implementation planning)
**Requested By:** Product Manager
**Date:** 2026-01-08
