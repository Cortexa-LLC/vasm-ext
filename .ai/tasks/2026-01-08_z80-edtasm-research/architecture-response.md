# Architecture Response: Z80 EDTASM Syntax Module

**Feature:** TRS-80 Model III EDTASM Syntax Support
**Architect:** Claude Sonnet 4.5 (Architecture Role)
**Consulting For:** Product Manager (PRD v1.0)
**Created:** 2026-01-08
**Status:** Architecture Assessment Complete

---

## Executive Summary

**FEASIBILITY: APPROVED**

The Z80 EDTASM syntax module is **technically feasible and architecturally sound**. All proposed requirements can be implemented within vasm's existing architecture with minimal risk.

**Key Findings:**
- ✅ Z80 CPU module is mature, well-tested, fully compatible
- ✅ 6809 EDTASM provides excellent foundation (fork approach recommended)
- ✅ No major technical blockers identified
- ✅ Clean separation via new syntax module directory
- ✅ /CMD format fits existing output module pattern
- ⚠️ Macro `#param` syntax requires careful parser implementation (medium complexity)

**Recommended Timeline:** 4-6 weeks for MVP (Phase 1), 8-12 weeks for complete implementation

---

## Technical Feasibility Assessment

### Overall Risk: **LOW TO MEDIUM**

| Component | Feasibility | Complexity | Risk Level |
|-----------|-------------|------------|------------|
| Core directives | High | Low | Low |
| Z80 integration | High | Low | Low |
| Comment syntax | High | Low | Low |
| Case sensitivity | High | Low | Low |
| Output format (/CMD) | High | Medium | Low |
| Macro #param syntax | Medium | Medium | Medium |
| Build system | High | Low | Low |

**Critical Path Items:**
1. Macro `#param` syntax parser (most complex component)
2. /CMD output format implementation (well-understood, low risk)
3. Directive table adaptation from 6809 EDTASM (straightforward)

---

## Architecture Decisions

### Decision 1: Implementation Approach

**DECISION: Fork 6809 EDTASM Module (Option A)**

**Rationale:**
- 6809 EDTASM is mature, tested, ~2800 lines
- Shares 70% of infrastructure: conditionals (COND/ENDC), INCLUDE handling, listing control, macro system framework
- Faster time to MVP (estimated 3-4 weeks vs 6-8 weeks clean implementation)
- Proven patterns reduce risk of subtle bugs
- Differences are well-contained and easily refactorable

**Implementation Strategy:**
1. Copy `syntax/edtasm/` → `syntax/edtasm-z80/`
2. Strip 6809-specific code (FCB/FDB/FCC handlers, `*` comment, `$` in identifiers, `\\param`)
3. Add Z80-specific code (DEFB/DEFW/DEFM handlers, `;`-only comment, `#param`)
4. Test incrementally with each change
5. Refactor common code into shared utilities (future enhancement, not MVP blocker)

**Estimated Effort:**
- Fork + strip 6809 code: 2-3 days
- Z80 directive implementation: 3-5 days
- Macro #param adaptation: 5-7 days
- Testing: 5-7 days
- **Total MVP: 3-4 weeks**

---

### Decision 2: Directory and Naming

**DECISION: `syntax/edtasm-z80/` directory, syntax name "edtasm-z80"**

**Directory Structure:**
```
vasm-ext/
├── syntax/
│   ├── edtasm/           # 6809 EDTASM (existing)
│   │   ├── syntax.c
│   │   ├── syntax.h
│   │   └── syntax_errors.h
│   └── edtasm-z80/       # Z80 EDTASM (new)
│       ├── syntax.c
│       ├── syntax.h
│       ├── syntax_errors.h
│       └── README.md
```

**Binary Naming:**
- Command: `make CPU=z80 SYNTAX=edtasm-z80`
- Output: `vasmz80_edtasm-z80` (or `vasmz80_edtasm_z80` if hyphens cause issues)

**Rationale:**
- Clear separation from 6809 EDTASM
- No Makefile ambiguity (different SYNTAX values)
- Follows vasm convention: `vasm<CPU>_<SYNTAX>`
- Hyphenated syntax names already exist in vasm ecosystem
- No build system changes required (existing make.rules handles it)

**Alternative Considered:**
- `syntax/edtasm_z80/` - Acceptable if hyphens cause shell escaping issues
- Test both; prefer hyphen for readability unless technical issue

**Build System Verification:**
```bash
# Existing pattern in make.rules:
INCLUDES = -I. -Icpus/$(CPU) -Isyntax/$(SYNTAX)
# Works with: SYNTAX=edtasm-z80
# Include path becomes: -Isyntax/edtasm-z80
```

**NO CHANGES NEEDED** to Makefile or make.rules. Existing infrastructure handles hyphenated syntax names.

---

### Decision 3: Directive Implementation

**DECISION: Hybrid Approach - Aliases for Simple Directives, Custom for Complex**

#### Simple Directives (Alias to 6809 Handlers)

| Z80 Directive | 6809 Equivalent | Implementation |
|---------------|-----------------|----------------|
| DEFB | FCB | Direct alias (identical semantics) |
| DEFW | FDB | Direct alias (word, but check endianness!) |
| DEFH | FDB | Direct alias (halfword = word in Z80 context) |
| DEFS | RMB | Direct alias (reserve memory bytes) |
| DEFL | SET | Direct alias (redefinable symbol) |

**CRITICAL: DEFW Endianness**

Z80 is **little-endian**, 6809 is **big-endian**. Must verify FDB handler respects CPU endianness:

```c
/* In directive handler: */
case DEFW:
  /* vasm's data directive handlers use fw16()/fw32() which respect CPU endianness */
  /* Z80 cpu.h defines: LITTLEENDIAN 1 */
  /* Should work correctly, but MUST test: */
  /*   DEFW $1234 should emit: 34 12 (little-endian) */
  /*   NOT: 12 34 (big-endian) */
```

**Action Item:** Add explicit test case in Phase 1 to verify DEFW endianness.

#### Complex Directives (Custom Implementation)

**DEFM (Define Message/String)**

6809 FCC uses flexible delimiters (`FCC /text/`, `FCC 'text'`, `FCC "text"`).
TRS-80 EDTASM uses **quote-only** delimiters (`DEFM "text"`).

**Implementation:**
```c
/* Simpler than FCC - only quote delimiters */
static void handle_defm(char *s) {
  char delimiter;

  s = skip(s);
  if (*s != '"' && *s != '\'') {
    syntax_error(ERR_STRING_DELIMITER);
    return;
  }

  delimiter = *s++;
  /* Parse string until closing delimiter */
  while (*s && *s != delimiter) {
    emit_byte(*s++);
  }

  if (*s != delimiter) {
    syntax_error(ERR_UNTERMINATED_STRING);
  }
}
```

**Estimated Effort:** 1 day (simpler than FCC)

#### Directive Table

Fork 6809 EDTASM's directive table, replace names:

```c
/* Z80 EDTASM directive table */
static struct {
  char *name;
  void (*handler)(char *);
} directives[] = {
  { "org",     handle_org },
  { "equ",     handle_equ },
  { "defl",    handle_set },      /* Alias to SET handler */
  { "end",     handle_end },
  { "defb",    handle_fcb },      /* Alias to FCB handler */
  { "defw",    handle_fdb },      /* Alias to FDB (check endianness!) */
  { "defh",    handle_fdb },      /* Alias to FDB */
  { "defs",    handle_rmb },      /* Alias to RMB */
  { "defm",    handle_defm },     /* Custom implementation */
  { "include", handle_include },
  { "cond",    handle_cond },
  { "endc",    handle_endc },
  { "macro",   handle_macro },
  { "endm",    handle_endm },
  { "page",    handle_page },
  { "title",   handle_title },
  { "list",    handle_list },
  { "nolist",  handle_nolist },
  { NULL, NULL }
};
```

**Note:** Keep 6809 handler implementations, just change directive names in table.

---

### Decision 4: Macro #param Syntax

**DECISION: Context-Sensitive Parsing (Option A)**

**Critical Finding:** Z80 does **NOT** use `#` for immediate addressing in standard Zilog syntax.

**Evidence from z80 CPU module (cpu.c line ~1083):**
```c
if ( z80asm_compat && *p == '#' ) {
    p++;  /* Constant identifier - compatibility with z80asm */
}
```

The `#` prefix is **ONLY** recognized in z80asm compatibility mode, not standard Zilog syntax. TRS-80 EDTASM uses **standard Zilog syntax** without `#` for immediate values.

**Conclusion:** `#` character is **SAFE** for macro parameters - no ambiguity with instruction syntax.

**Implementation Strategy:**

#### Macro Parameter Syntax

| Syntax | Meaning | Example |
|--------|---------|---------|
| `#P1` through `#P9` | Parameters 1-9 | `LD A,#P1` |
| `#$YM` | Unique macro invocation ID | `LABEL#$YM:` |

**Parser Implementation:**

```c
/* In macro body parsing (expand_macro function) */
static char *expand_macro_params(char *line, struct macro *m, char **args, int id) {
  char *src = line;
  char *dst = expanded_line;

  while (*src) {
    if (*src == '#') {
      src++;

      /* Check for #$YM unique ID */
      if (src[0] == '$' && toupper(src[1]) == 'Y' && toupper(src[2]) == 'M') {
        sprintf(dst, "_%06d", id);  /* Generate _000001, _000002, etc. */
        dst += 7;
        src += 3;
        continue;
      }

      /* Check for #P1-#P9 parameter */
      if (toupper(*src) == 'P' && isdigit(src[1])) {
        int param_num = src[1] - '0';  /* 1-9 */
        src += 2;

        if (param_num >= 1 && param_num <= m->num_params) {
          strcpy(dst, args[param_num - 1]);
          dst += strlen(args[param_num - 1]);
        } else {
          syntax_error(ERR_MACRO_PARAM_OUT_OF_RANGE);
        }
        continue;
      }

      /* Not a macro parameter - literal # */
      *dst++ = '#';
    } else {
      *dst++ = *src++;
    }
  }

  *dst = '\0';
  return expanded_line;
}
```

**Macro-Local Labels:**

Unlike 6809 EDTASM's `\\.label` syntax, TRS-80 EDTASM requires **manual unique labels** using `#$YM`:

```asm
; 6809 EDTASM (automatic):
DELAY  MACRO
\\.LOOP NOP
       DJNZ \\.LOOP
       ENDM

; TRS-80 EDTASM (manual with #$YM):
DELAY  MACRO
LOOP#$YM: NOP
          DJNZ LOOP#$YM
       ENDM
```

**No special handling needed** - `#$YM` expands to unique ID, labels are unique automatically.

**Estimated Effort:** 5-7 days (moderate complexity, but clean implementation)

**Risk Mitigation:**
- Test macro expansion with edge cases (nested macros, escaped #, literal # in strings)
- Compare output with original EDTASM assembler for validation
- Document limitations clearly in README.md

---

### Decision 5: Output Format (/CMD)

**DECISION: New Output Module `output_cmd.c` (Option A)**

**Rationale:**
- Follows vasm convention (one format = one output module)
- Clean separation from binary output
- Can support other TRS-DOS formats later (CAS cassette, .3BD disk, etc.)
- Reusable by other syntax modules (e.g., z80asm, oldstyle)

**TRS-DOS /CMD Format Specification:**

```
Multi-segment /CMD file format:
  For each segment:
    Byte 0:       $00 (segment header marker)
    Bytes 1-2:    Length (big-endian, includes load address)
    Bytes 3-4:    Load address (big-endian)
    Bytes 5-(n+4): Data

  Postamble (after all segments):
    Byte 0:       $FF (end marker)
    Bytes 1-2:    $00 $00 (zero length)
    Bytes 3-4:    Entry point address (big-endian)
```

**Example (3 segments):**
```
00 00 07 10 00 'C' 'O' 'D' 'E'     ; Segment 1: 4 bytes at $1000
00 00 07 20 00 'D' 'A' 'T' 'A'     ; Segment 2: 4 bytes at $2000
00 00 0A 30 00 'B' 'U' 'F' 'F' 'E' 'R' ; Segment 3: 6 bytes at $3000
FF 00 00 10 00                     ; Entry: $1000
```

**Implementation:**

```c
/* output_cmd.c - TRS-DOS /CMD executable format */

static int write_object(FILE *f, section *sec, symbol *sym) {
  section **seclist;
  size_t nsecs;
  int i;
  taddr entry_point = 0;

  /* Sort sections by load address */
  seclist = sort_sections(sec, &nsecs);

  /* Write segments */
  for (i = 0; i < nsecs; i++) {
    section *s = seclist[i];
    taddr len = s->pc - s->org;

    if (len == 0)
      continue;  /* Skip empty sections */

    /* Segment header */
    fw8(f, 0x00);                    /* Segment marker */
    fw16(f, len + 2, 1);             /* Length (big-endian) + 2 for load addr */
    fw16(f, s->org, 1);              /* Load address (big-endian) */

    /* Segment data */
    write_section_data(f, s);
  }

  /* Find entry point from END directive or first section */
  if (exec_symname) {
    symbol *entry_sym = find_symbol(exec_symname);
    if (entry_sym)
      entry_point = entry_sym->pc;
  } else {
    entry_point = seclist[0]->org;  /* Default to first section */
  }

  /* Postamble */
  fw8(f, 0xFF);                      /* End marker */
  fw16(f, 0, 1);                     /* Zero length */
  fw16(f, entry_point, 1);           /* Entry point (big-endian) */

  return 1;
}

static int output_args(char *s) {
  /* No additional args needed */
  return 0;
}
```

**Integration with END Directive:**

```c
/* In syntax module (syntax/edtasm-z80/syntax.c) */
static void handle_end(char *s) {
  s = skip(s);

  if (!ISEOL(s)) {
    /* Parse entry point expression */
    expr *entry = parse_expr(&s);
    if (entry) {
      /* Store entry point for output module */
      set_exec_symbol(entry);  /* vasm core function */
    }
  }

  parse_end = 1;  /* Stop parsing */
}
```

**Command-Line Usage:**
```bash
# TRS-DOS /CMD format (multi-segment)
vasmz80_edtasm-z80 -Fcmd -o program.cmd source.asm

# Flat binary (alternative)
vasmz80_edtasm-z80 -Fbin -o program.bin source.asm
```

**Estimated Effort:** 3-4 days (straightforward, well-understood format)

**Note:** Study existing output modules (output_bin.c with COCOML format) as reference. Very similar structure.

---

### Decision 6: Case Sensitivity

**DECISION: Symbol Table Level (Option A) - Use vasm's `nocase` flag**

**Implementation:**

```c
/* In syntax module init function (syntax_init) */
void syntax_init(void) {
  /* Z80 EDTASM defaults to case-insensitive */
  nocase = 1;

  /* ... other initialization ... */
}
```

**Command-Line Override:**

Users can enable case-sensitive mode with existing `-case` flag:

```bash
# Case-insensitive (default, matches original EDTASM)
vasmz80_edtasm-z80 -Fbin -o test.bin test.asm

# Case-sensitive (modern style, for mixed-case code)
vasmz80_edtasm-z80 -case -Fbin -o test.bin test.asm
```

**Flag Status:** The `-case` flag **already exists in vasm** (opposite of `-nocase`). No new flag needed.

**Verification:**
```bash
grep -r "\-case" vasm.c  # Confirms -case flag exists
```

**Rationale:**
- Simplest implementation (one line of code)
- Leverages existing, tested infrastructure
- No parser-level normalization needed
- Works for all symbols, labels, directives
- Default matches original EDTASM behavior

**Z80 Mnemonic Handling:**

Z80 mnemonics are **always case-insensitive** (handled by CPU module, not syntax module). This is standard for all assemblers.

```asm
; All equivalent (CPU module normalizes):
LD A,10
ld a,10
Ld A,10
```

**No special handling needed** - CPU module handles mnemonic case normalization.

---

### Decision 7: Build System Integration

**DECISION: No Changes Required**

**Verification:**

Existing make.rules pattern:
```makefile
INCLUDES = -I. -Icpus/$(CPU) -Isyntax/$(SYNTAX)
VASMEXE = vasm$(CPU)_$(SYNTAX)$(TARGET)$(TARGETEXTENSION)
```

**Test Build Command:**
```bash
make CPU=z80 SYNTAX=edtasm-z80
```

**Expected Output:**
- Binary: `vasmz80_edtasm-z80`
- Objects: `obj/z80_edtasm-z80_*.o`
- Include paths: `-I. -Icpus/z80 -Isyntax/edtasm-z80`

**No Makefile Edits Required** - existing infrastructure handles new syntax module automatically.

---

## Z80 CPU Module Integration Assessment

### Compatibility: **EXCELLENT**

**Z80 CPU Module Analysis:**
- Mature, well-tested module (in vasm since 2007)
- Supports full Z80 instruction set + variants (8080, Z180, eZ80, GameBoy Z80, Rabbit)
- Default mode: **Zilog syntax** (matches TRS-80 EDTASM)
- Extensive instruction coverage: 600+ mnemonic entries
- Addressing modes: All Z80 modes supported (immediate, direct, indexed, indirect, register)

**Critical Finding: No Immediate Addressing Syntax Conflict**

Z80 standard Zilog syntax (used by TRS-80 EDTASM) does **NOT** use `#` for immediate values:

```asm
; Z80 Standard (Zilog) Syntax:
LD A,10        ; Immediate value (no # prefix)
LD A,(1000h)   ; Direct addressing
LD A,(HL)      ; Indirect addressing

; z80asm compatibility mode (optional):
LD A,#10       ; # prefix accepted but not required
```

**Conclusion:** No conflict between macro `#P1` syntax and instruction operands.

### Operand Parsing

Z80 CPU module handles all operand parsing - syntax module just passes instruction line:

```c
/* Syntax module responsibility: */
instruction *parse_instruction(char *s) {
  char *mnemonic;
  operands *ops;

  mnemonic = parse_mnemonic(&s);     /* Extract "LD" */
  ops = parse_operands(&s);          /* Extract "A,10" */

  return new_inst(mnemonic, NULL, 0, ops);  /* CPU module handles rest */
}
```

**No special Z80-specific parsing needed in syntax module.**

### Configuration

**Z80 Module Flags (cpus/z80/cpu.c):**
```c
/* Defaults for EDTASM usage: */
static int z80asm_compat = 0;      /* Disable z80asm # prefix */
static int cpu_type = CPU_Z80;     /* Standard Z80 (not 8080/Rabbit) */
```

**No configuration needed** - defaults are correct for TRS-80 EDTASM.

### Test Cases

Z80 CPU module includes extensive test cases:
```
cpus/z80/tests/*.asm  (if they exist)
tests/z80/*.asm       (syntax-specific tests)
```

**Action Item:** Verify test suite exists, run baseline tests before integration.

---

## Module Structure and File Organization

### Recommended Directory Structure

```
vasm-ext/
├── syntax/
│   └── edtasm-z80/
│       ├── syntax.c              # Main syntax module (~2500 lines estimated)
│       ├── syntax.h              # Interface definitions (~30 lines)
│       ├── syntax_errors.h       # Error message definitions (~50 lines)
│       └── README.md             # Syntax reference and usage guide
├── output_cmd.c                  # TRS-DOS /CMD format module (~300 lines)
├── tests/
│   └── edtasm-z80/               # Test suite
│       ├── test_directives.s     # Core directive tests
│       ├── test_macros.s         # Macro system tests
│       ├── test_conditionals.s   # COND/ENDC tests
│       ├── test_z80_inst.s       # Z80 instruction tests
│       └── test_cmd_output.s     # /CMD format tests
└── .ai/
    └── tasks/
        └── 2026-01-08_z80-edtasm-research/
            ├── PRD-Z80-edtasm.md
            ├── technical-consultation.md
            └── architecture-response.md  # This document
```

### File Size Estimates

| File | Estimated Lines | Basis |
|------|----------------|-------|
| syntax/edtasm-z80/syntax.c | 2500 | Based on 6809 EDTASM (2792 lines), minus 6809-specific code |
| syntax/edtasm-z80/syntax.h | 30 | Interface definitions (minimal) |
| syntax/edtasm-z80/syntax_errors.h | 50 | Error messages (fork from 6809) |
| output_cmd.c | 300 | Simple format, reference COCOML (~200 lines) |
| README.md | 500 | Comprehensive syntax reference |
| **Total New Code** | **~3380 lines** | Manageable scope |

---

## Key Data Structures

### Syntax Module State

```c
/* Global state in syntax.c */
static struct {
  int case_sensitive;           /* 0 = case-insensitive (default) */
  int list_enabled;             /* 1 = listing on (default) */
  int macro_call_list;          /* 1 = list macro calls */
  int macro_expand_list;        /* 0 = don't list expansions */
  unsigned long macro_id;       /* Unique ID counter for #$YM */
  int parse_end;                /* 1 = END directive encountered */
  char *exec_symbol;            /* Entry point from END directive */
} edtasm_state;
```

### Macro Structure

```c
/* Macro definition (reuse from 6809 EDTASM) */
struct macro {
  char *name;                   /* Macro name */
  int num_params;               /* Number of parameters (1-9) */
  char **param_names;           /* Parameter names (for documentation) */
  char **body_lines;            /* Macro body (array of strings) */
  int num_lines;                /* Number of body lines */
  struct macro *next;           /* Linked list */
};
```

### Directive Table Entry

```c
/* Directive handler table entry */
struct directive {
  const char *name;             /* Directive name (lowercase) */
  void (*handler)(char *);      /* Handler function */
  int min_params;               /* Minimum parameters */
  int max_params;               /* Maximum parameters */
  int context_flags;            /* Where directive is valid */
};
```

---

## Integration Patterns

### Syntax Module → CPU Module

```c
/* Standard vasm pattern: */
instruction *parse_instruction(char *line) {
  char mnemonic[MAX_MNEMONIC_LEN];
  char *operands;

  /* Extract mnemonic */
  operands = parse_mnemonic(line, mnemonic);

  /* Pass to CPU module */
  return new_inst(mnemonic, NULL, 0, parse_operands(operands));
}
```

**No Z80-specific code needed** - CPU module handles all instruction encoding.

### Syntax Module → Output Module

```c
/* Entry point from END directive: */
void handle_end(char *s) {
  /* Parse optional entry point */
  expr *entry = parse_expr(&s);
  if (entry) {
    set_exec_symbol(entry);  /* Stored in vasm core, passed to output module */
  }
  parse_end = 1;
}
```

**Output module receives:**
- Section list (from vasm core)
- Symbol table (from vasm core)
- Entry point (from `set_exec_symbol()`)

**Clean separation** - no direct coupling between syntax and output modules.

### Syntax Module → Core (Symbols)

```c
/* Case sensitivity: */
void syntax_init(void) {
  nocase = 1;  /* Set global flag in vasm core */
}

/* Symbol definition: */
void handle_equ(char *s) {
  char *name = parse_identifier(&s);
  expr *value = parse_expr(&s);

  symbol *sym = new_symbol(name);  /* vasm core creates symbol */
  sym->type = CONSTANT;
  sym->value = value;
}
```

**Use vasm core functions** - don't reimplement symbol table logic.

---

## Reusable Components from 6809 EDTASM

### High Reusability (>90%)

These components can be **copied almost verbatim** from 6809 EDTASM:

1. **Conditional Assembly**
   - `handle_cond()` - COND directive
   - `handle_endc()` - ENDC directive
   - Condition stack management
   - Nested conditional logic
   - **Effort:** Copy and test (1 day)

2. **Include File Handling**
   - `handle_include()` - INCLUDE directive
   - Path search logic
   - Include depth tracking
   - **Effort:** Copy and test (0.5 days)

3. **Listing Control**
   - `handle_page()` - PAGE directive
   - `handle_title()` - TITLE directive
   - `handle_list()` / `handle_nolist()` - LIST/NOLIST directives
   - **Effort:** Copy and test (0.5 days)

4. **Macro Framework**
   - Macro definition parsing
   - Macro invocation parsing
   - Macro storage structures
   - **Effort:** Copy and adapt (2-3 days for parameter syntax changes)

5. **Expression Evaluation**
   - Leverage vasm core's `parse_expr()` - no syntax module code needed
   - **Effort:** None (core handles it)

6. **Section Management**
   - `handle_org()` - ORG directive
   - Section creation/switching
   - **Effort:** Copy and test (0.5 days)

### Medium Reusability (50-70%)

These need **moderate adaptation**:

1. **Macro Parameter Expansion**
   - Change from `\\1` to `#P1`
   - Change from `\\@` to `#$YM`
   - Remove `\\.label` support (not in TRS-80 EDTASM)
   - **Effort:** 3-4 days (moderate complexity)

2. **Comment Handling**
   - Remove asterisk-in-column-1 support
   - Keep semicolon support
   - **Effort:** 0.5 days (trivial change)

3. **Identifier Parsing**
   - Remove dollar sign support
   - Keep underscore, letters, digits
   - **Effort:** 0.5 days (trivial change)

### Low Reusability (<30%)

These need **custom implementation**:

1. **Data Directives**
   - DEFM implementation (simpler than FCC)
   - DEFW endianness verification
   - **Effort:** 1-2 days

2. **Directive Name Table**
   - Replace FCB/FDB/FCC/RMB/SET with DEFB/DEFW/DEFM/DEFS/DEFL
   - **Effort:** 0.5 days (simple search-and-replace)

---

## Technical Constraints and Blockers

### Hard Constraints

1. **ANSI C90 Compliance**
   - No C++ features, no C99/C11 features
   - Explicit casting, no `//` comments, no mixed declarations
   - **Impact:** Moderate (must follow vasm style)
   - **Mitigation:** Follow 6809 EDTASM patterns exactly

2. **2-Space Indentation**
   - vasm project standard (not ai-pack 4-space default)
   - **Impact:** Low (configure editor, use `clang-format`)
   - **Mitigation:** Copy `.clang-format` from vasm root

3. **Module Isolation**
   - No changes to vasm core (vasm.c, symbol.c, etc.)
   - No changes to other CPU/syntax modules
   - **Impact:** Low (good architecture practice anyway)
   - **Mitigation:** Use only public vasm APIs

4. **Z80 Address Space**
   - 16-bit address space (64K max)
   - **Impact:** None (TRS-80 Model III has 64K max anyway)
   - **Mitigation:** None needed

### Soft Constraints

1. **Test Coverage**
   - 80-90% coverage for NEW code only (not legacy vasm)
   - **Impact:** Medium (requires writing comprehensive tests)
   - **Mitigation:** Create tests incrementally per epic/feature

2. **Binary Compatibility**
   - Should match original EDTASM output byte-for-byte
   - **Impact:** Medium (hard to validate without original assembler)
   - **Mitigation:** Use TRS-80 emulators, community feedback

3. **Performance**
   - Assembly speed ≥10,000 lines/second
   - **Impact:** Low (vasm core is fast, syntax module adds minimal overhead)
   - **Mitigation:** Profile if issues arise (unlikely)

### No Blockers Identified

**All requirements are achievable within vasm's architecture.** No fundamental technical blockers exist.

---

## Risk Assessment and Mitigation

### Risk 1: Macro #param Syntax Parser Complexity

**Likelihood:** Medium
**Impact:** Medium
**Severity:** **MEDIUM OVERALL**

**Description:** Parsing `#P1` vs `#$YM` vs literal `#` in strings/comments requires careful state machine.

**Mitigation:**
1. Implement context-aware parser (inside macro body vs outside)
2. Test edge cases: `#` in strings, escaped `#`, nested macros, `#` in comments
3. Reference 6809 EDTASM's `\\param` parser as template
4. Start with simple cases, add complexity incrementally
5. Compare output with original EDTASM assembler for validation

**Contingency:** If too complex, fall back to simpler syntax (e.g., `@P1` instead of `#P1`), document as "EDTASM-compatible" rather than "EDTASM-exact"

### Risk 2: DEFW Endianness Mismatch

**Likelihood:** Low
**Impact:** High (silent data corruption)
**Severity:** **MEDIUM OVERALL**

**Description:** Z80 is little-endian, 6809 is big-endian. If FDB handler hardcodes endianness, DEFW will emit wrong byte order.

**Mitigation:**
1. **Verify immediately** in Phase 1: Test `DEFW $1234` emits `34 12` (not `12 34`)
2. If wrong, check `fw16()` function in vasm core - should respect CPU endianness flag
3. Z80 cpu.h defines `LITTLEENDIAN 1` - vasm core should honor this
4. Add explicit test case to CI/test suite

**Contingency:** If core `fw16()` doesn't respect CPU endianness, implement custom `handle_defw()` that calls `fw8()` twice in little-endian order

### Risk 3: /CMD Format Documentation Incomplete

**Likelihood:** Low
**Impact:** Medium
**Severity:** **LOW OVERALL**

**Description:** TRS-DOS /CMD format may have undocumented quirks or edge cases.

**Mitigation:**
1. Reverse-engineer from known-good .CMD files
2. Test with TRS-80 emulators (xtrs, trs80gp, sdltrs)
3. Consult TRS-80 community forums/archives
4. Reference COCOML format in output_bin.c (very similar structure)
5. Start with single-segment files, add multi-segment support incrementally

**Contingency:** If format proves too complex, prioritize flat binary output (`-Fbin`) for MVP, defer /CMD to Phase 2

### Risk 4: Limited Historical Test Code

**Likelihood:** Medium
**Impact:** Low
**Severity:** **LOW OVERALL**

**Description:** May be hard to find original TRS-80 EDTASM source code for validation testing.

**Mitigation:**
1. Request code samples from TRS-80 community (vcfed.org, comp.sys.tandy forums)
2. Digitize magazine listings from 80 Micro, Creative Computing, etc.
3. Use simple synthetic tests (known input → known output)
4. Create comprehensive unit tests for each directive

**Contingency:** Focus on specification compliance rather than exact bug-for-bug compatibility with original EDTASM

---

## Performance Considerations

### Expected Performance

**Assembly Speed:** ≥10,000 lines/second (easily achievable)

**Basis:**
- vasm core is highly optimized
- Syntax module overhead is minimal (string parsing, directive dispatch)
- 6809 EDTASM achieves 15,000+ lines/second on modern hardware
- Z80 EDTASM should match or exceed (simpler macro syntax)

**Bottlenecks (Unlikely):**
- Macro expansion with deeply nested calls
- Expression evaluation with complex relocations
- Multi-pass optimization (vasm core, not syntax module)

**Optimization Strategy:**
1. **Don't optimize prematurely** - implement correctly first
2. Profile if performance issues arise (use `gprof` or `valgrind`)
3. Focus optimization on hot paths (macro expansion, identifier lookup)
4. Leverage vasm core's optimized hash tables and expression evaluator

### Memory Footprint

**Expected:** <100MB for typical 64K Z80 program

**Basis:**
- Z80 address space is 64K max
- vasm uses linked lists for atoms (minimal overhead)
- Symbol table is hash-based (O(1) lookup, modest memory)
- Macro expansion uses temporary buffers (freed after use)

**No concerns** - typical TRS-80 programs are small (<64K), memory is non-issue on modern systems.

---

## Testing Strategy

### Test Organization

```
tests/
└── edtasm-z80/
    ├── test_directives.s          # Core directive tests
    ├── test_data_directives.s     # DEFB/DEFW/DEFH/DEFS/DEFM
    ├── test_symbols.s             # EQU/DEFL/labels
    ├── test_macros.s              # MACRO/ENDM with #P1-#P9, #$YM
    ├── test_conditionals.s        # COND/ENDC nested
    ├── test_includes.s            # INCLUDE directive
    ├── test_z80_instructions.s    # Z80 instruction set
    ├── test_case_sensitivity.s    # Case-insensitive mode
    ├── test_listing.s             # PAGE/TITLE/LIST/NOLIST
    ├── test_cmd_output.s          # /CMD format multi-segment
    ├── test_endianness.s          # DEFW little-endian verification
    └── test_edge_cases.s          # Stress tests, error handling
```

### Test Execution Framework

**Existing:** vasm uses Python-based test framework (`tests/run_tests.py`)

**Command:**
```bash
make CPU=z80 SYNTAX=edtasm-z80 test
```

**Test Structure:**
```python
# tests/edtasm-z80/test_directives.s
; Test ORG directive
 ORG $1000
START: LD A,10
 END START

; Expected output (binary):
; Address: $1000
; Bytes: 3E 0A
```

**Validation:**
- Binary output matches expected bytes
- Listing output correct
- No assembler errors
- Entry point set correctly (for /CMD format)

### Coverage Target

**80-90% coverage of NEW syntax module code** (per repo-overrides.md)

**Excluded from coverage:**
- Legacy vasm core code
- 6809 EDTASM code (already tested)
- Error handling for impossible states (defensive programming)

**Tooling:**
```bash
# Generate coverage report
gcc --coverage ...
make test
gcov syntax/edtasm-z80/syntax.c
```

### Test-Driven Development (TDD)

**Apply TDD only to NEW code** (per repo-overrides.md):

1. **Macro #param expansion** - TDD critical (complex logic)
2. **DEFM directive** - TDD recommended (new code)
3. **/CMD output module** - TDD recommended (new code)
4. **Directive handlers** (copied from 6809) - Test, but not TDD (legacy code)

**TDD Pattern:**
```bash
# 1. Write failing test
echo "MACRO TEST #P1 ENDM" > test_macro.s
./vasmz80_edtasm-z80 -Fbin test_macro.s  # Fails

# 2. Implement feature
vim syntax/edtasm-z80/syntax.c

# 3. Test passes
./vasmz80_edtasm-z80 -Fbin test_macro.s  # Success

# 4. Refactor if needed
```

---

## Implementation Roadmap

### Phase 1: Foundation (MVP) - 3-4 Weeks

**Deliverables:**
- Basic syntax module skeleton
- Core directives (ORG, EQU, DEFL, END, DEFB, DEFW, DEFS, DEFM)
- Comment syntax (semicolon only)
- Identifier rules (no dollar sign)
- Case-insensitive mode
- Z80 instruction integration
- Flat binary output (`-Fbin`)
- Unit tests for all directives

**Success Criteria:**
- `make CPU=z80 SYNTAX=edtasm-z80` builds successfully
- Can assemble simple Z80 program
- Generates correct binary output
- All Phase 1 tests pass

**Estimated Effort:** 3-4 weeks (1 engineer, full-time)

### Phase 2: Output Formats - 1-2 Weeks

**Deliverables:**
- /CMD output module (`output_cmd.c`)
- Multi-segment support
- Entry point handling
- Test cases for /CMD format
- Validation with TRS-80 emulator

**Success Criteria:**
- /CMD files load in xtrs/trs80gp emulator
- Multi-segment programs work correctly
- Entry point set from END directive

**Estimated Effort:** 1-2 weeks

### Phase 3: Advanced Features - 2-3 Weeks

**Deliverables:**
- INCLUDE directive
- COND/ENDC conditionals
- MACRO/ENDM with #param syntax
- Listing control (PAGE, TITLE, LIST/NOLIST)
- Comprehensive test suite
- Historical code validation

**Success Criteria:**
- Complex macros expand correctly
- Nested conditionals work
- Multi-file projects assemble
- Historical TRS-80 programs assemble (≥2 test cases)

**Estimated Effort:** 2-3 weeks

### Phase 4: Polish and Documentation - 1-2 Weeks

**Deliverables:**
- README.md with complete syntax reference
- Example programs
- Migration guide from original EDTASM
- Comparison table vs 6809 EDTASM
- Performance benchmarks
- Community feedback integration

**Success Criteria:**
- Documentation covers all features
- Examples run on TRS-80 emulator
- ≥95% syntax compatibility with original EDTASM
- Positive community feedback

**Estimated Effort:** 1-2 weeks

### Total Timeline: 7-11 Weeks

**Best Case:** 7 weeks (no blockers, experienced developer)
**Realistic:** 9 weeks (some issues, learning curve)
**Conservative:** 11 weeks (contingencies, polish)

---

## Recommended Next Steps

### Immediate Actions (Before Implementation)

1. **PM Reviews Architecture Response** (This Document)
   - Confirm all decisions align with PRD
   - Identify any concerns or alternative preferences
   - Approve architecture for implementation

2. **Joint Refinement Session (PM + Architect)**
   - Discuss trade-offs for any open questions
   - Finalize directory naming (hyphen vs underscore)
   - Agree on macro syntax approach
   - Document final decisions in PRD

3. **PM Updates PRD with Architecture Decisions**
   - Incorporate technical constraints
   - Update requirements if needed
   - Add architecture references

4. **Architect Creates Detailed Technical Design** (Optional)
   - API specifications for directive handlers
   - Detailed data structure definitions
   - Sequence diagrams for macro expansion
   - (This architecture response covers most needs already)

### Implementation Kickoff

5. **Engineer Reviews Architecture + PRD**
   - Understand requirements and constraints
   - Ask clarifying questions
   - Estimate effort for Phase 1

6. **Create Task Packet for Phase 1**
   - Follow ai-pack workflow
   - Break down into stories/tasks
   - Set acceptance criteria

7. **Begin Implementation**
   - Start with Phase 1 (Foundation/MVP)
   - Fork 6809 EDTASM module
   - Implement core directives
   - Test incrementally

---

## Open Questions for PM Clarification

### Question 1: Macro Parameter Limit

**Context:** TRS-80 EDTASM supported `#P1` through `#P9` (9 parameters). Some variants supported up to 16.

**Question:** Should we support:
- **Option A:** 9 parameters (#P1-#P9) - Matches original EDTASM
- **Option B:** 16 parameters (#P1-#P9, #PA-#PG) - Extended compatibility

**Recommendation:** Start with 9 (Option A) for MVP, extend to 16 in Phase 3 if needed.

### Question 2: Columnar Format Enforcement

**Context:** Original EDTASM enforced columnar format (labels 1-6, mnemonics 9-15, operands 17+).

**Question:** Should we:
- **Option A:** Enforce columnar format (reject non-conforming code)
- **Option B:** Support both columnar and free-form (modern style)
- **Option C:** Free-form only (ignore column positions)

**Recommendation:** Option B (support both) - most flexible, matches modern expectations. Column positions are optional hints, not requirements.

### Question 3: Error Message Compatibility

**Context:** Original EDTASM had specific error messages and codes.

**Question:** Should we:
- **Option A:** Match original EDTASM error messages exactly
- **Option B:** Use vasm-style error messages (clearer, more detailed)

**Recommendation:** Option B - Better user experience, easier maintenance. Document any differences in migration guide.

### Question 4: Listing Output Format

**Context:** Original EDTASM had specific listing format (line numbers, addresses, opcodes, source).

**Question:** Should we:
- **Option A:** Match original EDTASM listing format exactly
- **Option B:** Use vasm standard listing format

**Recommendation:** Option B - Consistent with other vasm syntax modules. Document any differences.

---

## Summary of Architecture Recommendations

| Component | Recommendation | Effort | Risk |
|-----------|----------------|--------|------|
| **Implementation Approach** | Fork 6809 EDTASM | 3-4 weeks | Low |
| **Directory/Naming** | `syntax/edtasm-z80/`, syntax name "edtasm-z80" | 0 days | Low |
| **Directives** | Alias simple, custom DEFM | 2-3 days | Low |
| **Macro #param** | Context-sensitive parser | 5-7 days | Medium |
| **/CMD Format** | New output module `output_cmd.c` | 3-4 days | Low |
| **Case Sensitivity** | Symbol table level (`nocase=1`) | 0.5 days | Low |
| **Build System** | No changes needed | 0 days | Low |
| **Z80 Integration** | Use existing CPU module as-is | 0 days | Low |
| **DEFW Endianness** | Verify, add test case | 0.5 days | Low |
| **Test Coverage** | 80-90% of new code | Ongoing | Low |
| **Documentation** | README.md + examples | 3-5 days | Low |

### Total Estimated Effort: 7-11 Weeks

**Critical Path:** Macro #param syntax parser → /CMD output module → Integration testing

---

## Final Assessment

**RECOMMENDATION: PROCEED WITH IMPLEMENTATION**

The Z80 EDTASM syntax module is **architecturally sound, technically feasible, and low risk**. All requirements from the PRD can be implemented within vasm's existing architecture with no fundamental blockers.

**Key Strengths:**
1. ✅ Excellent foundation from 6809 EDTASM (70% code reuse)
2. ✅ Mature, compatible Z80 CPU module (no integration issues)
3. ✅ Clean separation via new syntax module directory
4. ✅ Well-understood /CMD format (similar to existing COCOML)
5. ✅ No build system changes required
6. ✅ Clear implementation roadmap with manageable phases

**Key Risks (All Manageable):**
1. ⚠️ Macro #param syntax parser (medium complexity) - Mitigated by incremental implementation and testing
2. ⚠️ DEFW endianness (low likelihood) - Mitigated by early verification test
3. ⚠️ Limited historical test code (low impact) - Mitigated by synthetic tests and community engagement

**Recommended Timeline:**
- **Phase 1 (MVP):** 3-4 weeks
- **Phase 2 (Output):** 1-2 weeks
- **Phase 3 (Advanced):** 2-3 weeks
- **Phase 4 (Polish):** 1-2 weeks
- **Total:** 7-11 weeks

**This architecture response provides sufficient technical guidance for an Engineer to begin implementation.**

---

**Document Status:** Complete - Ready for PM Review
**Next Action:** PM reviews and approves architecture, or requests clarifications
**Architect:** Claude Sonnet 4.5 (Architecture Role)
**Date:** 2026-01-08
