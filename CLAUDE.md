# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

vasm is a portable and retargetable assembler written in ANSI C. It supports multiple CPU architectures, assembly syntax styles, and output formats through a modular plugin architecture. The final assembler binary is named `vasm<CPU>_<SYNTAX>` (e.g., `vasm6502_oldstyle`).

## Building vasm

### Standard Build Process

vasm uses Make with a modular build system. Each assembler variant is built by specifying a CPU module and syntax module:

```bash
# Build for a specific CPU and syntax combination
make CPU=6502 SYNTAX=oldstyle

# Clean build artifacts
make CPU=6502 SYNTAX=oldstyle clean

# Build vobjdump utility
make CPU=6502 SYNTAX=oldstyle
```

The resulting executable will be named `vasm<CPU>_<SYNTAX>` (e.g., `vasm6502_oldstyle`).

### Build System Details

- **Makefile**: Platform-specific settings (compiler, flags, target OS)
- **make.rules**: Common build rules for all platforms
- **obj/**: Build artifacts stored as `obj/<CPU>_<SYNTAX>_*.o`
- Object files are prefixed with the CPU and syntax combination to allow parallel builds

### Available Modules

**CPU Modules** (in `cpus/`):
- 6502, 6800, 6809, arm, c16x, hans, jagrisc, m68k, pdp11, ppc, qnice, spc700, test, tr3200, unsp, vidcore, x86

**Syntax Modules** (in `syntax/`):
- oldstyle, std, mot, madmac, test

### Platform-Specific Makefiles

- `Makefile`: Default Unix/Linux/macOS (uses gcc)
- `Makefile.Win32`: Windows with Visual Studio
- `Makefile.68k`, `Makefile.OS4`, `Makefile.MOS`: Amiga variants
- `Makefile.TOS`, `Makefile.MiNT`: Atari
- Others for specific platforms

## Architecture

### Module System

vasm's architecture is based on three types of pluggable modules:

1. **CPU Modules** (`cpus/<cpu>/`)
   - Define instruction sets and addressing modes
   - Encode machine instructions
   - Handle CPU-specific optimizations
   - Required files: `cpu.c`, `cpu.h`, `cpu_errors.h`, optional `opcodes.h`

2. **Syntax Modules** (`syntax/<syntax>/`)
   - Parse assembly language syntax
   - Handle directives (sections, data definitions, macros)
   - Process source line structure
   - Required files: `syntax.c`, `syntax.h`, `syntax_errors.h`

3. **Output Modules** (`output_*.c` in root)
   - Generate object file formats
   - All output modules compiled into every executable
   - Selected at runtime with `-F<format>` option
   - Formats: elf, aout, bin, vobj, hunk, tos, xfile, srec, ihex, o65, gst, woz, pap, hans, coff, aof, test

### Core Components

The assembler core (in root directory) provides:

- **vasm.c**: Main driver and multi-pass resolver
  - Performs up to 1500 passes (MAXPASSES) to resolve all symbols and optimize instructions
  - First 200 passes (FASTOPTPHASE) optimize all instructions simultaneously
  - Subsequent passes optimize one instruction per pass for safety

- **atom.c/h**: Atomic objects representing source elements
  - Atoms are the fundamental unit: LABEL, DATA, INSTRUCTION, SPACE, DATADEF, etc.
  - Instructions contain: opcode, qualifiers, operands, extensions

- **symbol.c/h**, **symtab.c/h**: Symbol table management
  - Hash-based symbol tables
  - Support for local/global scope, imports/exports
  - Section-relative and absolute symbols

- **expr.c/h**: Expression evaluation
  - 128-bit expression support
  - Complex expressions with relocations
  - Supports arithmetic, logical, and bitwise operations

- **parse.c/h**: General parsing utilities
- **source.c/h**: Source file management and line reading
- **reloc.c/h**: Relocation handling
- **error.c/h**: Error reporting system with error codes defined in `*_errors.h` files
- **listing.c/h**: Assembly listing generation
- **dwarf.c/h**: DWARF debug information support
- **supp.c/h**: Support functions and utilities
- **hugeint.c/h**: Large integer arithmetic
- **cond.c/h**: Conditional assembly

### Data Flow

1. Source files read by **source.c**
2. Lines parsed by syntax module
3. Creates atom chain (labels, instructions, data)
4. CPU module encodes instructions
5. Multi-pass resolution optimizes and resolves symbols
6. Output module writes object file

### Section Management

Sections are the basic organizational unit:
- Each section has: name, attributes, alignment, origin, atoms
- Sections contain linked list of atoms
- Support for absolute sections (ORG) and relative sections
- Section flags: ABSOLUTE, UNALLOCATED, IN_RORG, NEAR/FAR_ADDRESSING, etc.

### Include Paths

The build system uses include paths:
- `-I.` (root directory for core headers)
- `-Icpus/<CPU>` (CPU module headers)
- `-Isyntax/<SYNTAX>` (syntax module headers)

Each CPU and syntax module must provide their required interface headers.

## Compiler Requirements

- ANSI C90 compliant compiler
- Compilation flags: `-std=c90 -O2 -pedantic`
- Platform macros: `-DUNIX`, `-DAMIGA`, `-DATARI`, `-DMSDOS`, or `-D_WIN32`

## Documentation

- `doc/*.texi`: Texinfo documentation source files
- `doc/vasm.texi`: Main manual
- `doc/cpu_*.texi`: CPU-specific documentation
- `doc/output_*.texi`: Output format documentation
- `doc/interface.texi`: Module interface documentation

Build documentation:
```bash
cd doc
texi2dvi --pdf vasm.texi  # Creates vasm.pdf
texi2html -split=chapter -nosec_nav -frames vasm.texi  # Creates HTML docs
```

## Merlin Syntax Module

The Merlin syntax module (`syntax/merlin/`) provides compatibility with Apple II Merlin assembler syntax, including support for 65816 extended instructions.

### 65816 16-bit Immediate Mode

The Merlin syntax module properly tracks 65816 register sizes for 16-bit immediate mode instructions.

**Directives that set register sizes:**
- `MX %00` - 16-bit A and X/Y
- `MX %01` - 16-bit A, 8-bit X/Y
- `MX %10` - 8-bit A, 16-bit X/Y
- `MX %11` - 8-bit A and X/Y (default)
- `LONGA ON/OFF` - Set accumulator to 16-bit/8-bit
- `LONGI ON/OFF` - Set index registers to 16-bit/8-bit

**Automatic REP/SEP tracking:**

The assembler automatically tracks `REP` and `SEP` instructions to update register sizes:
```asm
 xc
 xc              ; Enable 65816 mode
 rep $30         ; Sets 16-bit A and X/Y (tracked automatically)
 ldx #$1234      ; Assembled as 3 bytes: A2 34 12
 sep $20         ; Sets 8-bit A, X/Y unchanged (tracked)
 lda #$56        ; Assembled as 2 bytes: A9 56
 xc off          ; Resets to 8-bit mode
```

**XC OFF behavior:**

The `xc off` directive resets both the CPU type to 6502 AND the register sizes back to 8-bit. This ensures subsequent code is assembled correctly for 6502/65C02.

### Binary Size Difference vs. snap Assembler

**Important:** vasm produces larger binaries than Adam Green's `snap` assembler for code containing 65816 instructions. This is by design and is the correct behavior.

| Assembler | 65816 Code Handling | IIgs Compatible |
|-----------|---------------------|-----------------|
| **vasm**  | Outputs actual 65816 machine code | ✓ Yes |
| **snap**  | Outputs RTS ($60) placeholders | ✗ No |

**Example - getparam function:**
```asm
getparam
 lda IIGS
 beq ]rts        ; Skip on non-IIgs
 clc
 xce             ; Enter native mode
 rep $30         ; 16-bit registers
 ldx #$0C03      ; IIgs toolbox call
 hex 22,00,00,E1 ; JSL $E10000
 ...
```

**snap output (~16 bytes):**
```
60 60 60 60 60 60 60 60 22 00 00 e1 60 60 60 60
   ^^^^^^^^^^^^^^^^^^^ RTS placeholders ^^^^^^^^^^^
```

**vasm output (~18 bytes):**
```
18 fb c2 30 48 5a a2 03 0c 22 00 00 e1 68 38 fb a8 60
^^ actual 65816 instructions ^^
```

**Why this matters:**
- On Apple IIe/IIc: The 65816 code is skipped via `beq ]rts` - both work
- On Apple IIgs: snap's RTS placeholders immediately return without executing the IIgs-specific code; vasm's code executes correctly

**Disk layout implications:**

When building disk images (e.g., Prince of Persia), the binary sizes will differ from snap-built versions. Disk layout files must be adjusted to accommodate the correct (larger) binary sizes. This is a one-time adjustment and ensures proper IIgs compatibility.

## EDTASM Syntax Module

The EDTASM syntax module (`syntax/edtasm/`) provides compatibility with Disk EDTASM+ assembler for TRS-80 Color Computer 6809/6309 development.

### Features

- **Comment handling**: Asterisk (`*`) in column 1, semicolon (`;`) anywhere
- **Case sensitivity**: Case-sensitive by default, `-nocase` flag for case-insensitive mode
- **Dollar signs in identifiers**: Support for OS-9 system calls (`F$Link`, `I$Read`, `E$EOF`, etc.)
- **Data directives**: FCB, FDB, FCC (with flexible delimiters), RMB
- **Conditional assembly**: COND/ENDC, IFEQ/IFNE/IFGT/IFGE/IFLT/IFLE, ELSE, ENDIF
- **Macro system**: MACRO/ENDM with double-backslash notation
  - `\\1` through `\\9` - Macro parameters 1-9
  - `\\@` - Unique ID for this invocation (generates `_nnnnnn`)
  - `\\.label` - Local label unique to this macro invocation
- **Sections**: SECTION, ORG
- **Listing control**: OPT, PAGE, TITLE, LIST, NOLIST
- **OS-9 support**: MOD directive, system call definitions

### Build

```bash
make CPU=6809 SYNTAX=edtasm
```

This produces: `vasm6809_edtasm`

### Usage

```bash
# Case-sensitive mode (default)
./vasm6809_edtasm -Fbin -o program.bin source.asm

# Case-insensitive mode (per EDTASM+ specification)
./vasm6809_edtasm -nocase -Fbin -o program.bin source.asm

# With OS-9 include files
./vasm6809_edtasm -I./include -Fbin -o program.bin source.asm
```

### Example

```asm
* EDTASM+ example for TRS-80 Color Computer
        INCLUDE os9.d           ; OS-9 definitions

* Macro with parameters and local labels
WAITKEY MACRO
        LDA     #'?'
        JSR     PUTCHAR
\\.WAIT JSR     GETCHAR         ; Local label
        BEQ     \\.WAIT         ; Branch to local
        ENDM

* Data directives
        ORG     $1000
START   FCB     1,2,3           ; Bytes
        FDB     $1234,$5678     ; Words
MSG     FCC     /Hello!/        ; String (flexible delimiter)
BUF     RMB     256             ; Reserve space

* Conditional assembly
DEBUG   EQU     1
        COND    DEBUG
        JSR     DEBUGOUT        ; Only if DEBUG != 0
        ENDC

* Macro invocation
        WAITKEY                 ; Expands with unique labels
        WAITKEY                 ; Each gets different local labels

* OS-9 system calls
        LDA     #READ
        LDX     #FILENAME
        OS9     I$Open          ; Dollar sign in identifier
        BCS     ERROR

        END     START
```

### Architecture Details

The EDTASM module was built by forking the Motorola syntax module and adapting it:

**Key modifications:**
1. **Comment handling** - Added asterisk-in-column-1 check in `iscomment()`
2. **Identifier characters** - Added dollar sign to `isidchar()` for OS-9 system calls
3. **Macro expansion** - Modified `expand_macro()` for double-backslash notation:
   - Changed from `\1` to `\\1` (requires checking for two consecutive backslashes)
   - Added `\\@` for unique ID generation
   - Added `\\.label` for macro-local labels using `make_local_label()`
4. **Directive table** - Simplified from 200+ directives to 46 EDTASM essentials
5. **Case handling** - Made case-sensitive by default, added `-nocase` flag
6. **Removed mot-specific code** - Stripped PhxAss/Devpac compatibility, alignment options

**Files:**
- `syntax/edtasm/syntax.c` - Main implementation (~2520 lines)
- `syntax/edtasm/syntax.h` - Interface definitions (~25 lines)
- `syntax/edtasm/syntax_errors.h` - Error messages (inherited from mot)

### OS-9 Support

**Current Status:**
- ✓ MOD directive recognized and parsed
- ✓ OS-9 system call identifiers (F$Link, I$Read, etc.)
- ✓ OS-9 include file (os9.d) with system definitions
- ✓ OS9 macro for generating SWI2 + FCB system calls
- ⚠ OS-9 module header generation is a stub (future enhancement)

**Future Enhancement:**

Proper OS-9 module format support requires:
- Module header sync bytes ($87CD)
- Module size calculation
- Header parity byte
- Module CRC generation
- Module type/language/attributes encoding

This will enable direct generation of loadable OS-9 modules without post-processing.

### Test Cases

Successfully assembles real-world OS-9 code:

**Invaders09** - Classic Space Invaders for OS-9 (https://github.com/barberd/Invaders09)
- 41,000+ lines of 6809 assembly
- Heavy use of macros with parameters
- OS-9 system calls throughout
- Conditional assembly
- Complex data structures

### Differences from Original EDTASM

1. **Case sensitivity default**: Original EDTASM+ was case-insensitive. This implementation defaults to case-sensitive for compatibility with existing mixed-case code. Use `-nocase` for traditional behavior.

2. **Expression evaluator**: Uses vasm's powerful 128-bit expression evaluator with complex relocations, more capable than original EDTASM.

3. **Macro system**: Based on vasm's robust macro infrastructure with proper nesting and recursion support.

4. **MOD directive**: Currently a stub. Full OS-9 module header generation planned for future release.

### Documentation

- **Detailed syntax reference**: `syntax/edtasm/README.md`
- **OS-9 include file**: Example in Invaders09 project
- **Original EDTASM manual**: Available from CoCo community archives

## Bug Fixes Applied

### Fixed: 65816 16-bit Immediate Mode (Merlin syntax)

**Bug**: The Merlin syntax module was not properly handling 65816 16-bit immediate mode. After `rep $30`, instructions like `ldx #$0C03` were assembled as 2 bytes instead of 3 bytes.

**Root Cause**:
1. MX/LONGA/LONGI directives used values 1/2 instead of 8/16 for register sizes
2. The CPU module's `asize`/`xsize` variables weren't updated when directives were processed
3. REP/SEP instructions weren't tracked to update register sizes
4. `xc off` didn't reset register sizes back to 8-bit

**Fix Applied**:

1. Added `set_65816_sizes()` and `get_65816_sizes()` to CPU module (`cpus/6502/cpu.c`):
```c
void set_65816_sizes(int a_size, int x_size)
{
  asize = a_size;
  xsize = x_size;
  cpu_opts_init(NULL);
}
```

2. Updated MX/LONGA/LONGI handlers to call `set_65816_sizes()` with correct 8/16 values

3. Added REP/SEP tracking in instruction parsing (`syntax/merlin/syntax.c:3640-3661`):
```c
if (cpu_type & WDC65816) {
  flags = parse_constexpr(&p);
  get_65816_sizes(&cur_asize, &cur_xsize);
  if (is_rep) {
    new_asize = (flags & 0x20) ? 16 : cur_asize;
    new_xsize = (flags & 0x10) ? 16 : cur_xsize;
  } else {
    new_asize = (flags & 0x20) ? 8 : cur_asize;
    new_xsize = (flags & 0x10) ? 8 : cur_xsize;
  }
  set_65816_sizes(new_asize, new_xsize);
}
```

4. Added 8-bit reset in `xc off` handler

**Test cases**: `tests/merlin/test_65816_*.s`

### Fixed: -nocase Flag Not Working (symbol.c)

**Bug**: The `-nocase` flag was completely broken due to a hash table mismatch in `symbol.c`.

**Root Cause**:
- `add_symbol()` stored symbols using case-insensitive hash (`hashcode_nc`) when `-nocase` was enabled
- `find_symbol()` always searched using case-sensitive hash (`hashcode`), ignoring the `nocase` flag
- Result: Symbols stored in one bucket, searched for in a different bucket = all labels undefined

**Fix Applied** (symbol.c:139-151):
```c
symbol *find_symbol(const char *name)
{
  hashdata data;
  if (nocase) {
    if (!find_name_nc(symhash,name,&data))
      return 0;
  }
  else {
    if (!find_name(symhash,name,&data))
      return 0;
  }
  return data.ptr;
}
```

Now `find_symbol()` correctly uses case-insensitive search when `-nocase` is enabled.

**Status**: This fix is now included in upstream vasm 2.0e (pre-release). Our implementation matches the official fix.

## Version and History

- Current version tracked in `vasm.c` (search for `_VER`)
- Release history in `history` file
- Copyright: (c) 2002-2025 Volker Barthelmann and Frank Wille
