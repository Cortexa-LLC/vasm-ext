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
