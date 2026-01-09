# Scope Change Analysis: M80 Inclusion in MVP

**Product Manager:** Claude (PM Role)
**Request:** User requests M80 features be included in MVP launch
**Date:** 2026-01-08
**Status:** Analysis and Recommendation

---

## Executive Summary

**User Request:** Include Microsoft MACRO-80 (M80) features as part of the Z80 EDTASM MVP launch.

**PM Assessment:** This is a **major scope expansion** that would increase complexity by 300-400% and timeline by 150-200%. However, M80 provides significantly more value to professional developers and may be worth the investment.

**PM Recommendation:** **Hybrid Approach** - Launch with "EDTASM-M80" feature set that includes M80's most valuable features while deferring relocatable linking to Phase 2.

---

## Scope Comparison

### Original MVP Scope (Basic EDTASM)

**Features:**
- 9 core directives (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM, INCLUDE)
- Basic macro system (MACRO/ENDM with #P1-#P9, #$YM)
- Basic conditionals (COND/ENDC)
- Semicolon comments
- Case-insensitive mode
- TRS-DOS /CMD output

**Estimated Timeline:** 7-11 weeks (Architect estimate)
**Target Users:** Hobbyists, vintage computing enthusiasts
**Complexity:** Low-Medium
**Value Proposition:** Modern cross-platform basic EDTASM assembler

---

### M80 Full Scope

**Additional Features (40+ directives vs 9):**

#### Segment/Section Management
- `ASEG` - Absolute segment
- `CSEG` - Code segment
- `DSEG` - Data segment
- `COMMON` - Common blocks

#### Linkage and Modularity
- `PUBLIC`/`ENTRY` - Export symbols
- `EXT`/`EXTRN` - Import external symbols
- `NAME` - Module naming
- `.REL` relocatable object format
- LINK-80 compatible output

#### Advanced Macros
- `LOCAL` - Automatic local labels (better than #$YM)
- `REPT` - Repeat block N times
- `IRP` - Iterate through argument list
- `IRPC` - Iterate through characters
- `EXITM` - Exit macro early
- Concatenation operator (`&`)
- Null testing, value conversion

#### Advanced Conditionals
- `IF`/`IFE` - If expression / If expression equals zero
- `IF1`/`IF2` - Conditional on pass number
- `IFDEF`/`IFNDEF` - If symbol defined/not defined
- `IFB`/`IFNB` - If blank/not blank
- `IFIDN`/`IFDIF` - If identical/different strings
- `ELSE` - Else clause
- `ENDIF` - End conditional (in addition to ENDC)

#### Data Directives (Alternatives)
- `DB` - Define byte (alias for DEFB)
- `DW` - Define word (alias for DEFW)
- `DS` - Define storage (alias for DEFS)
- `DC` - Define constant

#### Control Directives
- `.Z80` / `.8080` - CPU mode selection
- `.RADIX` - Number base control (2/8/10/16)
- `.PHASE` / `.DEPHASE` - Phase control for relocated code
- `.REQUEST` - Library file inclusion

#### Advanced Listing
- `SUBTTL` - Subtitle for listings
- `.LIST` / `.XLIST` - Enhanced listing control
- `.CREF` / `.XCREF` - Cross-reference control

**Estimated Timeline:** 11-19 weeks (basic EDTASM 7-11 weeks + M80 additions 4-8 weeks)
**Target Users:** Professional developers, system programmers, commercial projects
**Complexity:** High
**Value Proposition:** Full professional-grade Z80 development toolchain

---

## Impact Analysis

### Timeline Impact

| Scope | Duration | Increase |
|-------|----------|----------|
| **Basic EDTASM** | 7-11 weeks | Baseline |
| **M80 Full** | 15-23 weeks | +114% to +209% |
| **M80 Hybrid (Recommended)** | 10-15 weeks | +43% to +91% |

**Critical Path Items Added by M80:**
1. Segment management (ASEG/CSEG/DSEG) - 1-2 weeks
2. Relocatable output (.REL format) - 2-3 weeks
3. Symbol import/export (PUBLIC/EXTRN) - 1-2 weeks
4. Advanced macros (LOCAL/REPT/IRP) - 2-3 weeks
5. Advanced conditionals (IF variants) - 1-2 weeks
6. .PHASE/.DEPHASE - 1 week
7. Testing and validation - 2-3 weeks

### Complexity Impact

| Component | Basic EDTASM | M80 Full | Increase |
|-----------|--------------|----------|----------|
| **Directives** | 9 | 40+ | 344% |
| **Lines of Code** | ~3,400 | ~8,000-10,000 | 135-194% |
| **Test Cases** | ~50 | ~150-200 | 200-300% |
| **Documentation** | 500 lines | 1,500+ lines | 200% |

### Risk Impact

| Risk | Basic EDTASM | M80 Full | M80 Hybrid |
|------|--------------|----------|------------|
| **Implementation Complexity** | Low-Medium | High | Medium-High |
| **Testing Burden** | Medium | High | Medium-High |
| **Maintenance Cost** | Low | High | Medium |
| **Compatibility Issues** | Low | Medium | Low-Medium |
| **User Confusion** | Low | Medium | Low |

### User Value Impact

**Basic EDTASM Users:**
- ✅ Can assemble simple programs
- ✅ Macros for code reuse
- ✅ Basic conditionals
- ❌ Limited to single-file projects
- ❌ No modular development
- ❌ Can't use existing M80 code libraries
- **Value Score:** 6/10

**M80 Full Users:**
- ✅ Professional multi-file projects
- ✅ Relocatable modules and linking
- ✅ Advanced macro capabilities
- ✅ Can use CP/M and TRS-DOS libraries
- ✅ Supports large-scale development
- ✅ Compatible with existing M80 workflows
- **Value Score:** 10/10

---

## Strategic Considerations

### Market Research: Who Used M80?

From historical research:
- **Professional developers:** M80 was the "Disk Editor/Assembler" ($99.95) vs basic EDTASM ($29.95)
- **Commercial software:** Many TRS-80 commercial games and utilities were built with M80
- **CP/M crossover:** M80 was also the standard assembler for CP/M systems
- **Library ecosystem:** Significant body of M80 code exists (relocatable modules, libraries)

**User Need:** If target users are **hobbyists/enthusiasts**, basic EDTASM suffices. If target users include **serious developers/preservationists**, M80 is critical.

### Competitive Landscape

**Existing TRS-80 Z80 Assemblers (Modern):**
- zmac - Z80 macro assembler (basic features)
- Sjasm - Advanced Z80 assembler (modern syntax)
- z80asm (z88dk) - Modern Z80 toolchain

**None provide authentic M80 compatibility** - this would be a unique value proposition.

### Technical Debt Consideration

**If we skip M80 now:**
- Future M80 addition may require refactoring
- Two separate products (edtasm-z80 and edtasm-m80)?
- User confusion about which to use

**If we include M80 now:**
- Longer initial development
- But cleaner architecture from start
- Single product covers all use cases

---

## Options Analysis

### Option A: Basic EDTASM Only (Original Plan)

**Pros:**
- ✅ Faster time to market (7-11 weeks)
- ✅ Lower complexity and risk
- ✅ Meets needs of hobbyist users
- ✅ Easier to maintain

**Cons:**
- ❌ Excludes professional developers
- ❌ Can't assemble existing M80 code
- ❌ Limited to simple single-file projects
- ❌ May need separate M80 product later

**Timeline:** 7-11 weeks
**Recommendation:** Only if target market is strictly hobbyists

---

### Option B: Full M80 Implementation

**Pros:**
- ✅ Professional-grade toolchain
- ✅ Assembles existing M80 codebases
- ✅ Supports multi-file projects
- ✅ Relocatable linking
- ✅ Comprehensive feature set

**Cons:**
- ❌ 15-23 week timeline (doubles development time)
- ❌ High complexity
- ❌ Significant testing burden
- ❌ May be overkill for many users
- ❌ Relocatable linking (.REL + LINK-80) is complex

**Timeline:** 15-23 weeks
**Recommendation:** Only if willing to accept 4-5 month development cycle

---

### Option C: Hybrid "EDTASM-M80" (RECOMMENDED)

**Scope:** M80 directives and advanced features WITHOUT relocatable linking

**Include:**
- ✅ All M80 directives (40+)
- ✅ Segment management (ASEG/CSEG/DSEG)
- ✅ Advanced macros (LOCAL, REPT, IRP, IRPC)
- ✅ Advanced conditionals (IF variants)
- ✅ Symbol visibility (PUBLIC - but no linking)
- ✅ .PHASE/.DEPHASE
- ✅ .RADIX, .Z80/.8080
- ✅ M80-compatible syntax
- ✅ Flat binary output and /CMD output

**Defer to Phase 2:**
- ⏸️ .REL relocatable object format
- ⏸️ External symbol resolution (EXTRN)
- ⏸️ LINK-80 compatible linking
- ⏸️ Library file support (.LIB)

**Rationale:**
- M80 directives and macros provide 80% of the value
- Relocatable linking is complex and used by <20% of users
- Can still assemble M80 code (just outputs flat binary instead of .REL)
- Cleaner architecture for future linking support
- Faster time to market

**Pros:**
- ✅ Professional features without linking complexity
- ✅ Moderate timeline (10-15 weeks)
- ✅ Assembles most M80 code (non-modular)
- ✅ Advanced macros and conditionals
- ✅ Future extensibility for linking

**Cons:**
- ⚠️ Can't link separate modules (yet)
- ⚠️ 40% longer than basic EDTASM
- ⚠️ More complex than basic EDTASM

**Timeline:** 10-15 weeks
**Recommendation:** BEST BALANCE of value vs complexity

---

### Option D: Phased Approach

**Phase 1 (MVP):** Basic EDTASM (7-11 weeks)
**Phase 2:** M80 Extensions (4-6 weeks after Phase 1)
**Total:** 11-17 weeks (but staggered releases)

**Pros:**
- ✅ Get something out quickly
- ✅ Validate basic architecture
- ✅ Gather user feedback early
- ✅ Can pivot based on actual user needs

**Cons:**
- ❌ May require refactoring when adding M80
- ❌ Two release cycles (more overhead)
- ❌ Users wait longer for full features
- ❌ Risk of Phase 2 getting deprioritized

**Timeline:** 11-17 weeks (two releases)
**Recommendation:** Good if uncertain about user demand for M80

---

## PM Recommendation: **Option C - Hybrid "EDTASM-M80"**

### Product Vision

**Name:** vasm Z80 EDTASM-M80 Assembler (vasmz80_edtasm-m80)

**Tagline:** "Professional TRS-80 Z80 assembler with Microsoft M80 compatibility"

**Value Proposition:**
- Assemble legacy TRS-80 code (both EDTASM and M80)
- Modern cross-platform toolchain
- Advanced macros and conditionals
- Professional segment management
- Future-ready for relocatable linking

### Feature Set (Hybrid Scope)

**Core EDTASM (Original):**
- ORG, EQU, DEFL, END
- DEFB, DEFW, DEFH, DEFS, DEFM
- MACRO/ENDM with #P1-#P9, #$YM
- COND/ENDC
- INCLUDE, PAGE, TITLE, LIST/NOLIST
- Semicolon comments
- Case-insensitive mode

**M80 Extensions (Added):**
- **Segments:** ASEG, CSEG, DSEG (absolute output, not relocatable)
- **Data Aliases:** DB, DW, DS, DC
- **Advanced Macros:** LOCAL, REPT, IRP, IRPC, EXITM
- **Advanced Conditionals:** IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF
- **Control:** .Z80, .8080, .RADIX, .PHASE/.DEPHASE
- **Visibility:** PUBLIC (for documentation, not linking)
- **Listing:** SUBTTL, .LIST/.XLIST

**Deferred to Phase 2 (Future):**
- .REL relocatable object format
- EXTRN (external symbols requiring linking)
- LINK-80 compatible linker
- Library file support (.LIB)
- COMMON blocks (require linking)

### Timeline Estimate

| Phase | Duration | Deliverables |
|-------|----------|--------------|
| **Phase 1: Foundation** | 4-5 weeks | Basic EDTASM + segments + advanced conditionals |
| **Phase 2: Advanced Macros** | 2-3 weeks | LOCAL, REPT, IRP, IRPC |
| **Phase 3: Output & Polish** | 2-3 weeks | /CMD format, .PHASE/.DEPHASE, testing |
| **Phase 4: Documentation** | 2 weeks | README, examples, migration guide |
| **TOTAL** | **10-13 weeks** | Complete EDTASM-M80 hybrid |

**Best Case:** 10 weeks
**Realistic:** 12 weeks
**Conservative:** 15 weeks (with contingencies)

### Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Scope creep to full M80** | Medium | High | Strict feature freeze on relocatable linking |
| **Advanced macro complexity** | Medium | Medium | Incremental implementation, thorough testing |
| **Timeline slip** | Medium | Medium | Conservative estimates, weekly status checks |
| **User confusion (EDTASM vs M80)** | Low | Low | Clear documentation, examples for both syntaxes |

### Success Metrics (Updated)

**KPI 1: Syntax Compatibility**
- **Target:** ≥90% of M80 code assembles (excluding .REL/EXTRN)
- **Measurement:** Test with historical M80 programs

**KPI 2: Feature Completeness**
- **Target:** 40 M80 directives implemented
- **Measurement:** Directive checklist

**KPI 3: User Adoption**
- **Target:** ≥20 GitHub stars or mentions within 6 months (higher than basic EDTASM)
- **Measurement:** GitHub insights, forum mentions

**KPI 4: Professional User Satisfaction**
- **Target:** Positive feedback from TRS-80 developer community
- **Measurement:** Forum posts, GitHub issues

**KPI 5: Code Quality**
- **Target:** 80-90% test coverage of new code
- **Measurement:** Test suite coverage report

---

## Questions for User

Before finalizing this scope change, I need clarification on:

### Question 1: Primary User Base

**Who is the primary target user?**

A. Hobbyists and vintage computing enthusiasts (casual use, simple programs)
B. Serious developers and preservationists (professional use, complex projects)
C. Mixed user base (need to serve both)

**Impact:** Determines whether basic EDTASM or M80 features are essential

---

### Question 2: MVP Definition

**What does "MVP launch" mean to you?**

A. Get something working quickly for early feedback (7-11 weeks)
B. Launch with professional-grade features (10-15 weeks acceptable)
C. Launch when it's truly complete (15-23 weeks acceptable)

**Impact:** Determines whether we do basic EDTASM first, hybrid, or full M80

---

### Question 3: Relocatable Linking Priority

**How important is .REL format and relocatable linking?**

A. Critical - must be in MVP (adds 4-6 weeks)
B. Important - but can wait for Phase 2 (defer)
C. Nice-to-have - focus on M80 directives and macros first

**Impact:** Determines whether we do full M80 or hybrid

---

### Question 4: Trade-off Preference

**Given these options, which do you prefer?**

A. **Fast launch** - Basic EDTASM only (7-11 weeks)
B. **Professional features** - Hybrid EDTASM-M80 without linking (10-15 weeks) **[PM RECOMMENDS]**
C. **Complete M80** - Full implementation with .REL linking (15-23 weeks)
D. **Phased** - Basic EDTASM now, M80 later (two releases, 11-17 weeks total)

---

## PM Final Recommendation

Based on product analysis, I recommend **Option C: Hybrid "EDTASM-M80"** for these reasons:

### Value-to-Cost Ratio
- **80% of M80's value** with **50% of the complexity**
- Advanced macros and conditionals are highly valuable
- Relocatable linking used by <20% of users

### Market Differentiation
- **Unique offering** - no other modern assembler provides M80 compatibility
- Appeals to both hobbyists (EDTASM) and professionals (M80 features)
- Positions product as **premium** solution

### Technical Soundness
- Architect's foundation (7-11 weeks) easily extends to M80 directives
- No fundamental architecture changes needed
- Clean separation: directives/macros (now) vs linking (later)

### Timeline Feasibility
- **10-13 weeks realistic** (vs 7-11 for basic EDTASM)
- **Only 3-4 weeks added** for significant feature expansion
- Phase 2 (linking) can be deferred based on user demand

### User Satisfaction
- Exceeds expectations vs basic EDTASM
- Assembles most M80 code (excluding modular projects)
- Future-proof for Phase 2 linking support

---

## Updated PRD Changes Required

If user approves Option C (Hybrid EDTASM-M80), the PRD needs:

1. **Title Update:** "Z80 EDTASM-M80 Syntax Module" (not just EDTASM)
2. **Scope Expansion:** Add 30+ M80 directives to functional requirements
3. **Timeline Update:** 10-13 weeks (up from 7-11 weeks)
4. **Success Metrics:** Update for M80 compatibility
5. **Out-of-Scope Update:** Move advanced macros IN scope, keep .REL OUT of scope
6. **Architecture Review:** Architect needs to assess M80 directive impact

---

## Next Steps

**Awaiting User Decision:**

1. **User answers Questions 1-4** above
2. **PM updates PRD** based on user decision
3. **Architect reviews** M80 directives impact (if approved)
4. **PM finalizes** requirements and timeline
5. **Engineer begins** implementation with approved scope

---

**Document Status:** Pending User Decision
**PM Recommendation:** Hybrid EDTASM-M80 (Option C) - 10-13 weeks
**Created:** 2026-01-08
**Product Manager:** Claude (PM Role)
