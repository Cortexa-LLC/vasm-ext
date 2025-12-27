# Merlin Syntax Module for vasm

Cross-platform assembler syntax module supporting the Merlin 8/16/32 assembler dialect used in Apple II and Apple IIgs development.

## Features

### Label System

Three-tier label hierarchy matching Merlin semantics:

- **Global labels**: Standard identifiers (e.g., `START`, `MainLoop`)
- **Local labels**: `:LABEL` prefix - alphanumeric, bidirectional references, scoped between global labels
- **Anonymous labels**: `:` alone - forward (`:+`) and backward (`:-`) references
- **Variable label definitions**: `]LABEL EQU value` - creates unique internal names for each definition

### Directives

All standard Merlin directives supported (no dot prefix):

**Origin and Sections:**
- `ORG address` - Set origin
- `DUM address` / `DEND` - Dummy section (unallocated)
- `OBJ name` - Object file name
- `START` - Define entry point
- `END` - End of source

**Data Definition:**
- `DFB` / `DB` - Define byte(s)
- `DA` / `DW` - Define word(s) - 16-bit little-endian
- `DDB` - Define double-byte (big-endian word)
- `ADR` - Define address (16-bit)
- `ADRL` - Define long address (24-bit)
- `DS count` - Define storage (reserve bytes)

**String Directives:**
- `ASC "text"` - ASCII string (normal)
- `ASC 'text'` - ASCII with high bit set
- `DCI "text"` - Last character high bit inverted
- `INV "text"` - All characters high bit set
- `FLS "text"` - Flashing (alternating high bit)
- `REV "text"` - Reversed byte order
- `STR "text"` - String with 1-byte length prefix
- `STRL "text"` - String with 2-byte length prefix (little-endian)
- `HEX hexdigits` - Hex data bytes

**Symbol Definition:**
- `EQU value` - Equate symbol to value
- `= value` - Assignment (alternate syntax)

**Conditional Assembly:**
- `DO condition` / `IF condition` - Conditional block
- `ELSE` - Alternate block
- `FIN` - End conditional

**Macros:**
- `MAC` ... `<<<` - Define macro (also accepts `EOM`, `EM`, `ENDM`)
- `PMC name` - Call macro by name
- `>>> name` - Alternate macro invocation
- Direct invocation by name

**Loops:**
- `LUP count` ... `--^` - Loop block

**File Inclusion:**
- `PUT filename` - Include source file
- `USE [prefix/]filename` - Include macro library (supports ProDOS prefixes)

**Output Control:**
- `REL` - Generate relocatable code
- `DSK filename` - Set output filename
- `LST` / `LSTDO` / `LSTOFF` - Listing control

**Entry/External:**
- `ENT symbol` - Define entry point (exported symbol)
- `EXT symbol` - External reference (imported symbol)

**CPU Mode (65816):**
- `MX flags` - Set M and X flags (bit 0=X/Y size, bit 1=accumulator size)
- `XC` - Set CPU type
- `LONGA ON/OFF` - 16-bit accumulator mode
- `LONGI ON/OFF` - 16-bit index register mode

**Miscellaneous:**
- `ERR [message]` - Generate assembly error
- `CHK` - Insert checksum
- `DAT` - Insert date/time stamp

### Macro System

**Parameters:**
- `]0` - Parameter count
- `]1` through `]8` - Macro parameters (1-indexed)

**Parameter Separators:**
Comma (`,`), slash (`/`), period (`.`), hyphen (`-`), space, or open paren (`(`)

**Example:**
```asm
DELAY   MAC
        LDX   #]1
:WAIT   DEX
        BNE   :WAIT
        <<<

        DELAY 255    ; Invocation
```

### Number Formats

- **Decimal**: `123` (default)
- **Hexadecimal**: `$FF` or `$ABCD`
- **Binary**: `%11010011` (underscore separators allowed: `%1010_0101`)
- **Character**: `'A'` (high bit set) or `"A"` (normal ASCII)

### Expression Operators

**Byte Selection:**
- `<value` - Low byte
- `>value` - High byte
- `^value` - Bank byte (bits 16-23)

**Arithmetic:**
- `+` `-` `*` `/` - Standard operations
- Algebraic precedence with `{...}` braces

**Logical:**
- `&` - Bitwise AND
- `.` - Bitwise OR
- `!` - Bitwise XOR

**Comparison:**
- `<` `=` `>` - Comparison operators (return -1 for true, 0 for false)

### Special Features

**Tilde Prefix:**
Wrapper macros using `~` prefix (common in GS Toolbox code):
```asm
~CtlStartUp MAC
        <<<
```

**ProDOS Prefix Support:**
The `USE` directive supports Merlin 16+ ProDOS prefix notation:
```asm
        USE   4/Int.Macs    ; Load from Prefix 4 (MACRO.LIBRARY)
```

Set prefix paths via environment variables:
```bash
export VASM_MERLIN_PREFIX_4="path/to/macros/"
```

The USE directive automatically tries both `.S` and `.s` extensions.

## Building

```bash
cd /Users/bryanw/Projects/Vintage/tools/vasm
make CPU=6502 SYNTAX=merlin
```

This creates the executable: `vasm6502_merlin`

## Usage

### Basic Assembly

```bash
./vasm6502_merlin -Fbin -o output.bin source.asm
```

### With Macro Libraries

```bash
export VASM_MERLIN_PREFIX_4="macros/"
./vasm6502_merlin -Fbin -o output.bin source.asm
```

### Common Options

- `-Fbin` - Binary output format
- `-Fvobj` - vasm object format
- `-o filename` - Output filename
- `-L listing.lst` - Generate listing file
- `-nocase` - Case-insensitive symbols
- `-quiet` - Suppress informational messages

## Known Limitations

### 65816 CPU Support

Full Apple IIgs 65816 instruction set requires CPU module enhancements. The syntax module handles mode directives (MX, LONGA, LONGI) but instruction encoding depends on the 6502 CPU module.

Instructions like `PHK`, `PLB`, `PEA`, `PER` will produce "instruction not supported" errors until the CPU module is extended.

### OMF Output Format

Apple IIgs Object Module Format (OMF) output is planned but not yet implemented. Use `-Fbin` for binary output or `-Fvobj` for vasm's native format.

## Testing

### Test Suite

Located in `tests/merlin/`:

**Basic Tests:**
- `test_basic.asm` - Instructions, labels, data directives
- `test_labels.asm` - Global, local, and anonymous labels
- `test_macros.asm` - Macro definitions and invocations

**Real-World Tests:**
Merlin-16 macro libraries from the Merlin 16+ distribution (supermacs):
- `Ctl.Macs.S` - Control Manager macros (322 lines, 100+ macros) ✅
- `Desktop.Macs.S` - Desktop Manager macros ✅
- `Dialog.Macs.S` - Dialog Manager macros ✅
- `Int.Macs.S` - Integer math macros ✅
- `Menu.Macs.S` - Menu Manager macros ✅
- `QuickDraw.Macs.S` - QuickDraw II macros ✅

### Test Results

All available macro libraries assemble successfully. Example:

```bash
$ export VASM_MERLIN_PREFIX_4="tests/merlin/16/supermacs/"
$ ./vasm6502_merlin -Fbin -o /tmp/test.bin tests/merlin/16/supermacs/Ctl.Macs.S
vasm 2.0c (c) in 2002-2025 Volker Barthelmann
vasm 6502 cpu backend 0.10a (c) 2002,2006,2008-2012,2014-2020,2022 Frank Wille
vasm Merlin syntax module 1.0 (c) 2025 Bryan Woodruff
[322 lines, 0 passes, 0 bytes, 109 symbols]
```

**Cogito2 Test Case:**
The Apple IIgs game `Cogito2` successfully loads all required macro libraries but encounters 65816 CPU limitations:
```bash
$ ./vasm6502_merlin tests/merlin/16/Cogito2/cogito2.s
# Loads all 6 macro libraries successfully
# Error: PHK/PLB instructions not supported (requires 65816 CPU module)
```

## Syntax Differences from Merlin 32

### Compatible Features

- Directive names and syntax
- Three-tier label system
- Macro definition and invocation
- String directive semantics
- Expression operators
- ProDOS prefix notation

### Differences

1. **65816 Instructions**: Limited by vasm's 6502 CPU module - full 65816 instruction set pending
2. **OMF Output**: Not yet implemented - use binary or vobj output formats
3. **Case Sensitivity**: Use `-nocase` flag for case-insensitive assembly (Merlin default varies by version)

## Implementation Notes

### Core Modifications

To support the non-alphanumeric `<<<` macro end delimiter, the vasm core file `parse.c` was modified:

**Function:** `dirlist_match()` - Added special case before alphanumeric check:
```c
/* Special case for Merlin <<< delimiter (non-alphanumeric) */
if (s[0]=='<' && s[1]=='<' && s[2]=='<') {
  if ((isspace((unsigned char)*(s-1)) || *(s-1)=='\0') &&
      (isspace((unsigned char)s[3]) || s[3]=='\0')) {
    /* Search for "<<<" in the directive list */
    while (len = list->len) {
      if (len==3 && !strnicmp(list->name,"<<<",3))
        return list;
      list++;
    }
  }
  return NULL;
}
```

### Identifier Rules

**Start characters:** `A-Z`, `a-z`, `_`, `~` (tilde for wrapper macros)

**Continuation characters:** Alphanumeric, `_`, `.`, `~`

**Special prefixes:**
- `:` - Local or anonymous label
- `]` - Variable label or macro parameter

## References

- **Merlin 16+**: Original Apple IIgs assembler by Glen Bredon
- **Merlin 32**: Modern cross-platform version by Brutal Deluxe
  - Homepage: https://brutaldeluxe.fr/products/crossdevtools/merlin/
  - Documentation: Merlin32_v1.0.pdf
  - Source code: Available on Brutal Deluxe website

## Credits

**vasm**: Volker Barthelmann and Frank Wille (2002-2025)

**Merlin syntax module**: Bryan Woodruff, Cortexa LLC (2025)

**Based on**: SCMASM syntax module template

## License

vasm is freeware and part of the portable and retargetable VASM assembler.
The Merlin syntax module follows the same license terms.
