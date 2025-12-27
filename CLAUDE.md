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

## Bug Fixes Applied

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
