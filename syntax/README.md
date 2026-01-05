# vasm Syntax Modules

## Overview

vasm's syntax modules are pluggable parsers that interpret different assembly language syntaxes. Each syntax module transforms source code into vasm's internal atom representation, which is then processed by CPU and output modules.

## Architecture

```
┌────────────────────────────────────────────────────────────────┐
│                        Syntax Module                           │
│                                                                │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  Required Functions (exported to vasm core)             │  │
│  ├─────────────────────────────────────────────────────────┤  │
│  │  • int init_syntax(void)                                │  │
│  │    - Initialize hash tables, set flags                  │  │
│  │    - Return 1 on success                                │  │
│  │                                                          │  │
│  │  • int syntax_args(char *arg)                           │  │
│  │    - Parse syntax-specific command-line arguments       │  │
│  │    - Return 1 if handled, 0 otherwise                   │  │
│  │                                                          │  │
│  │  • char *parse(void)                                    │  │
│  │    - Parse source lines                                 │  │
│  │    - Create atoms (labels, instructions, data)          │  │
│  │    - Handle directives and macros                       │  │
│  │    - Return next line or NULL at end                    │  │
│  │                                                          │  │
│  │  • int expand_macro(source *src, char **line,          │  │
│  │                     char *dest, int dlen)               │  │
│  │    - Expand macro parameters in line                    │  │
│  │    - Return number of chars written or -1 if overflow   │  │
│  │                                                          │  │
│  │  • char *skip_macro_arg(char *p)                        │  │
│  │    - Skip past one macro argument                       │  │
│  │    - Used for parsing macro invocations                 │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  Optional Functions                                      │  │
│  ├─────────────────────────────────────────────────────────┤  │
│  │  • void syntax_defsect(void)                            │  │
│  │    - Define default section name/type                   │  │
│  │                                                          │  │
│  │  • void set_internal_abs(char *name, taddr value)       │  │
│  │    - Define internal absolute symbols                   │  │
│  │                                                          │  │
│  │  • char *parse_cpu_special(char *s)                     │  │
│  │    - Handle CPU-specific syntax extensions              │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  Configuration Macros (in syntax.h)                     │  │
│  ├─────────────────────────────────────────────────────────┤  │
│  │  • ISIDSTART(c)    - Test if valid identifier start    │  │
│  │  • ISIDCHAR(c)     - Test if valid identifier char     │  │
│  │  • ISBADID(p,l)    - Test if identifier is invalid     │  │
│  │  • ISEOL(p)        - Test if at end of line            │  │
│  │  • MAXMACPARAMS    - Maximum macro parameters          │  │
│  │  • EXPSKIP()       - Skip whitespace in expressions    │  │
│  └─────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
```

## Data Flow

```
Source File
    │
    ├──► parse() ────────────────┐
    │    ├─ Read lines           │
    │    ├─ Tokenize             │
    │    ├─ Process macros       │
    │    └─ Handle directives    │
    │                            │
    ├──► expand_macro() ─────────┤
    │    ├─ Substitute params    │
    │    └─ Generate local labels│
    │                            │
    └──► Create Atoms ───────────┘
         ├─ LABEL atoms
         ├─ INSTRUCTION atoms
         ├─ DATA atoms
         ├─ SPACE atoms
         └─ DATADEF atoms
              ↓
         Atom Chain
              ↓
         CPU Module (encoding)
              ↓
         Core Engine (resolution)
              ↓
         Output Module (generation)
```

## Atom Types

The syntax module creates atoms representing source elements:

| Atom Type     | Purpose                    | Example                |
|---------------|----------------------------|------------------------|
| LABEL         | Symbol definition          | `start: lda #0`        |
| INSTRUCTION   | CPU instruction            | `lda #$42`             |
| DATA          | Raw byte data              | `FCB $01,$02,$03`      |
| SPACE         | Reserved space             | `RMB 16`               |
| DATADEF       | Data with expressions      | `FDB label+10`         |
| ROFFS         | Relative offset            | `rs.w 1`               |
| PRINTEXPR     | Debug/listing output       | `echo value`           |
| RORG          | Relocatable origin         | Used internally        |

## Available Syntax Modules

### Standard (std)
- GNU-as style syntax
- Generic syntax for all CPUs
- Directives: `.org`, `.byte`, `.word`, `.ascii`, etc.

### Motorola (mot)
- Classic Motorola 68k assembler syntax
- Compatible with Devpac and PhxAss
- Directives: `org`, `dc.b`, `ds.w`, `macro`, etc.
- Extensive macro system with `\1-\9` parameters

### MadMac (madmac)
- Atari MadMac assembler syntax
- Supports 6502, 68k, and Jaguar
- GPU/DSP specific directives

### Oldstyle (oldstyle)
- Classic 8-bit assembler syntax
- Simple, straightforward
- Suitable for small projects

### SCMASM (scmasm)
- S-C Macro Assembler 3.0/3.1 syntax
- Three-tier label system (global, `:N` private, `.N` local)
- Directives with `.` prefix (`.OR`, `.DA`, `.HS`)
- **Target:** Apple II 6502/65816
- **Documentation:** `scmasm/README.md`

### Merlin (merlin)
- Merlin 32 assembler syntax
- 65816 extended instruction support
- Automatic REP/SEP tracking for 16-bit modes
- MX, LONGA, LONGI directives
- **Target:** Apple II/IIgs
- **Documentation:** `merlin/README.md`

### EDTASM (edtasm)
- Disk EDTASM+ syntax
- OS-9 system support
- Double-backslash macro notation (`\\1-\\9`)
- **Target:** TRS-80 Color Computer, OS-9
- **Documentation:** `edtasm/README.md`

## Creating a New Syntax Module

### Step 1: Create Directory Structure

```bash
mkdir syntax/newsyntax
cd syntax/newsyntax
```

### Step 2: Create Required Files

**syntax.h** - Interface definitions:
```c
/* Identifier character tests */
#define ISIDSTART(x) (isalpha((unsigned char)(x)))
#define ISIDCHAR(x) (isalnum((unsigned char)(x))||((x)=='_'))
#define ISBADID(p,l) (0)
#define ISEOL(p) (*(p)=='\0'||*(p)==';')

/* Macro configuration */
#define MAXMACPARAMS 35
#define SKIP_MACRO_ARGNAME(p) (NULL)
```

**syntax_errors.h** - Error messages:
```c
"directive expected", ERROR,
"invalid operand", ERROR,
"syntax error", WARNING,
```

**syntax.c** - Implementation:
```c
#include "vasm.h"

const char *syntax_copyright = "vasm newsyntax syntax module 1.0 (c) 2025";

/* Required functions */
int init_syntax(void) {
    /* Initialize module */
    return 1;
}

int syntax_args(char *p) {
    /* Handle command-line args */
    return 0;
}

char *parse(void) {
    /* Parse source lines */
    char *line;
    while ((line = read_next_line())) {
        /* Process line, create atoms */
    }
    return NULL;
}

int expand_macro(source *src, char **line, char *d, int dlen) {
    /* Expand macro parameters */
    return 0;
}

char *skip_macro_arg(char *p) {
    /* Skip past macro argument */
    return p;
}
```

### Step 3: Add to Build System

The Makefile automatically includes any syntax module in `syntax/*/syntax.c`.

### Step 4: Build and Test

```bash
make CPU=6809 SYNTAX=newsyntax
./vasm6809_newsyntax -Fbin -o test.bin test.asm
```

## Directive Handling

Syntax modules typically maintain a directive table:

```c
struct {
    const char *name;
    unsigned flags;
    void (*func)(char *);
} directives[] = {
    "org",    0,      handle_org,
    "equ",    0,      handle_equ,
    "fcb",    0,      handle_fcb,
    "macro",  0,      handle_macro,
    "endm",   0,      handle_endm,
    /* ... */
};
```

Use hash tables for fast lookup:
```c
dirhash = new_hashtable(0x1000);
for (i = 0; i < dir_cnt; i++) {
    data.idx = i;
    add_hashentry(dirhash, directives[i].name, data, 1);
}
```

## Macro Expansion

Macros are expanded during parsing:

1. **Define macro**: Store name, parameters, and body
2. **Invoke macro**: Create source context with parameters
3. **Expand**: Replace parameter references with values
4. **Parse**: Process expanded lines

Example macro expansion:
```c
int expand_macro(source *src, char **line, char *d, int dlen) {
    char *s = *line;
    if (*s == '\\' && *(s+1) >= '1' && *(s+1) <= '9') {
        int pnum = *(s+1) - '0';
        if (pnum <= src->num_params) {
            int len = src->param_len[pnum-1];
            if (len < dlen) {
                memcpy(d, src->param[pnum-1], len);
                *line = s + 2;
                return len;
            }
        }
    }
    return 0;
}
```

## Symbol/Label Creation

Use vasm core functions to create symbols:

```c
/* Create label at current PC */
symbol *label = new_labsym(0, labname);

/* Create constant (EQU) */
symbol *sym = new_equate(name, expression);

/* Create absolute symbol (SET) */
symbol *sym = new_abs(name, expression);
```

## Error Reporting

Use syntax module error codes:

```c
syntax_error(error_num, ...);     /* Syntax-specific errors */
general_error(error_num, ...);    /* General vasm errors */
cpu_error(error_num, ...);        /* CPU-specific errors */
```

## Testing

Create test files in `tests/newsyntax/`:

```
tests/newsyntax/
├── test_basic.asm       # Basic functionality
├── test_macros.asm      # Macro expansion
├── test_conditionals.asm # IF/ELSE/ENDIF
└── test_data.asm        # Data directives
```

## Best Practices

1. **Use hash tables** for directive/mnemonic lookup
2. **Cache parsed values** to avoid re-parsing
3. **Validate early** - check syntax before creating atoms
4. **Error recovery** - continue parsing after errors when possible
5. **Memory management** - use vasm's `mymalloc()`, never call `free()`
6. **Documentation** - Create detailed README.md in module directory

## Common Pitfalls

1. **Case sensitivity** - Respect `-nocase` flag
2. **Expression parsing** - Use `parse_expr()` from vasm core
3. **Local labels** - Implement proper scoping
4. **Macro nesting** - Handle recursive expansion
5. **Line continuation** - Check for backslash at EOL
6. **String literals** - Handle escape sequences correctly

## Resources

- **vasm source code** - Study existing syntax modules
- **parse.h** - Core parsing functions
- **symbol.h** - Symbol table management
- **atom.h** - Atom creation functions
- **expr.h** - Expression evaluation
- **Official docs** - http://sun.hasenbraten.de/vasm/

## License

Syntax modules must comply with vasm's license. See main README.md for details.

## Support

For questions about syntax module development:
- Study existing modules (scmasm, merlin, edtasm are well-documented)
- Review vasm core interfaces in header files
- Test thoroughly with real-world code
