# vasm - Portable and Retargetable Assembler

**Version:** 2.0e (based on vasm 2.0e pre-release with SCMASM extensions)
**Authors:** Volker Barthelmann, Frank Wille
**SCMASM Module:** Bryan Woodruff, Cortexa LLC (2025)

## Overview

vasm is a portable and retargetable assembler to create linkable objects in various formats or absolute code. Multiple CPU, syntax, and output modules can be selected.

**Key Features:**
- Supports many common directives/pseudo-opcodes (syntax-dependent)
- CPU-specific extensions
- Code optimizations (shortest branch instructions, addressing modes)
- Relaxations (branch to jump conversion when necessary)
- Macro support, include directives, conditional assembly, local symbols
- Cross-platform (Unix/Linux, macOS, Windows, Amiga, Atari)

## Supported CPUs

- **6502 family** - 6502, 65C02, 65CE02, DTV, 45GS02, HU6280, 65802, 65816
- **6800 family** - 6800, 6801, 6803, 68HC11
- **6809 family** - 6809, 6309, Turbo9, Konami-2, 68HC12
- **80x86 family** - IA32 8/16/32 bit (AT&T syntax)
- **ARM** - ARMv1 to ARMv4, including THUMB mode
- **C16x/ST10**
- **ColdFire** - All V2, V3, V4, V4e models
- **HANS** - 32-bit FPGA CPU
- **Jaguar RISC** - GPU and DSP instruction sets
- **M680x0 family** - M6888x, M68851, CPU32, Apollo 68080
- **PDP-11** - includes EIS, FIS
- **PowerPC** - POWER, 40x, 440, 460, 6xx, 7xx, 7xxx, 860, Book-E, e300, e500
- **QNICE** - 16-bit FPGA CPU
- **Raspberry Pi VideoCore IV**
- **Sony SPC700**
- **SunPlus μnSP** - ISA V1.0 and V1.1
- **TR3200** - Virtual CPU from 0x10c
- **Z80 family** - Z80, 8080, 8085, GBZ80, 64180, RCM2/3/4k

## Syntax Modules

- **std** - Standard (GNU-as style) syntax
- **mot** - Motorola/Freescale 68k syntax (Devpac compatible)
- **madmac** - Atari MadMac syntax (6502, 68k, Jaguar)
- **oldstyle** - Old 8-bit style syntax
- **scmasm** - S-C Macro Assembler 3.0/3.1 syntax *(new in this fork)*

## Output Formats

- **a.out** - JRISC, M68k, x86
- **Amiga hunk** - M68k
- **Binary** - Apple DOS, Atari COM, Commodore PRG, Oric TAP (6502)
- **Atari TOS** - M68k executable
- **Atari DRI** - M68k object format
- **C #define** - All CPUs
- **C256 Foenix PGX/PGZ** - 65816
- **COFF** - ARM, M68k, PowerPC, x86
- **EHF** - Extended hunk format (PowerPC)
- **ELF** - ARM, JRISC, M68k, PowerPC, x86
- **GST** - Object file format
- **HANS** - HANS project output
- **Intel-Hex** - All CPUs
- **MOS Paper Tape** - All CPUs
- **Motorola S-Record** - All CPUs
- **o65** - Relocatable format (6502, 65816)
- **Raw binary** - All CPUs
- **Sharp X68000 Xfile/Zfile** - M68k
- **Tandy Color Computer** - Dragon DOS binaries (6809)
- **Test/Debug** - All CPUs (default)
- **VOBJ** - Proprietary versatile object format (all CPUs)
- **Wozmon** - Monitor format (6502)

## Building vasm

The build system uses Make with modular CPU and syntax selection. Assembled binaries are named `vasm<CPU>_<SYNTAX>`.

### Building for 6502 with SCMASM syntax:

```bash
make CPU=6502 SYNTAX=scmasm
```

This creates `vasm6502_scmasm`.

### Building for 6502 with oldstyle syntax:

```bash
make CPU=6502 SYNTAX=oldstyle
```

This creates `vasm6502_oldstyle`.

### Building for M68k with Motorola syntax:

```bash
make CPU=m68k SYNTAX=mot
```

This creates `vasmm68k_mot`.

### Clean build:

```bash
make CPU=6502 SYNTAX=scmasm clean
make CPU=6502 SYNTAX=scmasm
```

### Running Tests

Tests require Python 3 and are optional (for validation only).

```bash
# Build and test SCMASM syntax module
make CPU=6502 SYNTAX=scmasm test
```

The test target will:
- Check if Python 3 is available
- Run buffer safety tests (14 tests)
- Run CPU validation tests (20 tests)
- Skip gracefully if Python 3 not found

**Note:** Tests only run for SCMASM syntax module. Other syntax modules don't have automated tests.

### All available combinations:

Any CPU module can be combined with any syntax module that makes sense for that architecture:

```bash
make CPU=<cpu> SYNTAX=<syntax>
```

Where:
- `<cpu>` = 6502, 6800, 6809, arm, c16x, hans, jagrisc, m68k, pdp11, ppc, qnice, spc700, tr3200, unsp, vidcore, x86, z80
- `<syntax>` = std, mot, madmac, oldstyle, scmasm, test

## Basic Usage

```bash
# Assemble to binary
vasm6502_scmasm -Fbin -o output.bin source.s

# Assemble to ELF object
vasmm68k_mot -Felf -o output.o source.s

# With listing file
vasm6502_scmasm -Fbin -L listing.txt -o output.bin source.s

# Case-insensitive mode
vasm6502_scmasm -nocase -Fbin -o output.bin source.s

# Include path
vasm6502_scmasm -I./include -Fbin -o output.bin source.s
```

## Common Options

- `-F<format>` - Select output format (bin, elf, hunk, srec, ihex, etc.)
- `-o <file>` - Output file name
- `-L <file>` - Generate listing file
- `-I<path>` - Add include path
- `-nocase` - Case-insensitive symbol/macro names
- `-nosym` - Don't include symbol information
- `-quiet` - Suppress warnings
- `-w` - Show all warnings
- `-pic` - Generate position-independent code
- `-x` - Show error messages with source line
- `-Dname=value` - Define symbol

## Documentation

- **Official vasm docs:** http://sun.hasenbraten.de/vasm/release/vasm.pdf
- **vasm website:** http://sun.hasenbraten.de/vasm/
- **SCMASM syntax:** See `syntax/scmasm/README.md`
- **Test suite:** See `tests/README.md`

## Directory Structure

```
vasm/
├── README.md              # This file
├── CLAUDE.md             # Claude Code guidance
├── .gitignore            # Git exclusions
├── Makefile              # Unix/Linux/macOS build
├── make.rules            # Common build rules
├── vasm.c                # Main assembler driver
├── atom.c, expr.c, ...   # Core components
├── cpus/                 # CPU modules
│   ├── 6502/
│   ├── m68k/
│   └── ...
├── syntax/               # Syntax modules
│   ├── scmasm/          # SCMASM syntax (new)
│   ├── oldstyle/
│   ├── mot/
│   └── ...
├── output_*.c           # Output format modules
├── doc/                 # Documentation (Texinfo)
└── tests/               # Test suites
    ├── scmasm/          # SCMASM tests
    └── oldstyle/        # Oldstyle tests
```

## Building A2osX Projects

The SCMASM syntax module includes support for **A2osX build metadata extraction**, allowing automated builds that replicate the ProDOS file structure:

### Using Makefile Targets (Easiest)

```bash
# Build all A2osX binaries with Makefile convenience target
make CPU=6502 SYNTAX=scmasm A2OSX_DIR=/path/to/A2osX build-a2osx

# Verbose output
make CPU=6502 SYNTAX=scmasm A2OSX_DIR=/path/to/A2osX build-a2osx-verbose
```

### Using Build Script Directly

```bash
# Single file build
python3 build_a2osx.py source.s -o ./build

# Build entire directory (e.g., A2osX BIN/)
python3 build_a2osx.py -d /path/to/A2osX/BIN -o ./build

# Verbose output
python3 build_a2osx.py -v source.s
```

The build script:
- ✓ Extracts `.TF` (target binary path) from source
- ✓ Extracts `TEXT` (source listing path) from source
- ✓ Creates directory structure (`BIN/`, `SRC/`, etc.)
- ✓ Invokes vasm with correct output paths
- ✓ Handles A2osX editor directives (`NEW`, `AUTO`, `TEXT`)

## SCMASM Syntax Module (New)

This fork includes a complete implementation of the S-C Macro Assembler (SCMASM) 3.0/3.1 syntax for Apple II 6502/65816 development.

**Key SCMASM Features:**
- Three-tier label system (global, private `:N`, local `.N`)
- Directives with `.` prefix (`.OR`, `.DA`, `.HS`, `.AS`, `.OP`, etc.)
- Comments: `*` in column 1, `;` inline
- Flexible string delimiters
- CPU type switching mid-assembly (`.OP`)
- Conditional assembly (`.DO`/`.ELSE`/`.FIN`)

**Documentation:** See `syntax/scmasm/README.md`
**Tests:** See `tests/scmasm/README.md`

**Build and use:**
```bash
make CPU=6502 SYNTAX=scmasm
./vasm6502_scmasm -Fbin -o program.bin source.s
```

## Platform-Specific Makefiles

- `Makefile` - Default Unix/Linux/macOS (gcc)
- `Makefile.Win32` - Windows (Visual Studio)
- `Makefile.68k` - Amiga 68k
- `Makefile.OS4` - AmigaOS 4
- `Makefile.MOS` - MorphOS
- `Makefile.TOS` - Atari TOS
- `Makefile.MiNT` - Atari MiNT

## License

vasm is copyright © 2002-2025 Volker Barthelmann and Frank Wille.

This archive may be redistributed without modifications and used for non-commercial purposes.

An exception for commercial usage is granted, provided that the target CPU is M68k and the target OS is AmigaOS. Resulting binaries may be distributed commercially without further licensing.

In all other cases you need written consent from the authors.

**SCMASM syntax module:** Copyright © 2025 Bryan Woodruff, Cortexa LLC. Same license terms as vasm core.

Certain modules may fall under additional copyrights.

## Contact

**vasm core:**
- Volker Barthelmann - vb@compilers.de
- Frank Wille - frank@phoenix.owl.de

**SCMASM syntax module:**
- Bryan Woodruff, Cortexa LLC - bryanw@cortexa.com

## Links

- **Official vasm:** http://sun.hasenbraten.de/vasm/
- **vbcc compiler suite:** http://sun.hasenbraten.de/vbcc/
- **Volker's vasm page:** http://www.compilers.de/vasm.html
- **SCMASM documentation:** https://www.txbobsc.com/scsc/scassembler/
- **A2osX project:** https://github.com/burniouf/A2osX (SCMASM examples)

## Recent Changes

### vasm 2.0e (SCMASM fork - Dec 2025)
- **NEW:** Complete SCMASM 3.0/3.1 syntax module implementation
- Three-tier label system (global, `:N` private, `.N` local)
- SCMASM directives (`.OR`, `.DA`, `.HS`, `.AS`, `.AZ`, `.AT`, `.OP`, `.MA`/`.EM`, `.PH`/`.EP`, `.DUMMY`/`.ED`, etc.)
- Full macro system with `>MACRONAME`/`_MACRONAME` invocation and `]1`-`]9` parameters
- Flexible string delimiters for `.AS`/`.AZ`/`.AT`
- CPU type selection with `.OP` directive
- Conditional assembly (`.DO`/`.ELSE`/`.FIN`)
- Phase assembly (`.PH`/`.EP`) and dummy sections (`.DUMMY`/`.ED`)
- Binary include (`.INB`)
- Comprehensive test suite (19 test files, all passing)
- Full documentation
- Based on vasm 2.0e pre-release (includes upstream -nocase fix)

### vasm 2.0e upstream (pre-release - Nov 2025)
- Fixed -nocase option for symbols (was broken in 2.0d)
- New AOF (ARM/Acorn Object Format) output module for ARM/3DO
- ARM: Automatic literal pools with '=' operand prefix for LDR
- ARM: Alternative SWI immediate operand with '#'
- Fixed alignments > 32 bit in TOS and xfile output modules

See official release notes at http://sun.hasenbraten.de/vasm/ for complete history.

## Contributing

When contributing to the SCMASM module or other components:

1. Follow the existing code style (C90, 2-space indents)
2. Add tests to `tests/scmasm/` or `tests/oldstyle/`
3. Update relevant README files
4. Test with multiple target CPUs if applicable
5. Document new features in appropriate `.texi` files

## Building Documentation

```bash
cd doc
texi2pdf vasm.texi          # Creates vasm.pdf
makeinfo vasm.texi          # Creates vasm.info
texi2html vasm.texi         # Creates HTML docs
```

## Troubleshooting

**Problem:** `make: *** No rule to make target...`
**Solution:** Ensure you're specifying both CPU and SYNTAX: `make CPU=6502 SYNTAX=scmasm`

**Problem:** Undefined reference to `set_cpu_type`
**Solution:** Rebuild both CPU and syntax modules: `make CPU=6502 SYNTAX=scmasm clean && make CPU=6502 SYNTAX=scmasm`

**Problem:** Labels not recognized
**Solution:** Check syntax - SCMASM uses `.N` for local labels, `:N` for private labels

**Problem:** Directive not recognized
**Solution:** SCMASM directives require `.` prefix (`.OR` not `ORG`, `.DA` not `DW`)

See `CLAUDE.md` for detailed development guidance when using Claude Code.
