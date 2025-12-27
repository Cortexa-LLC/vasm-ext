# Merlin Syntax Module for vasm

✅ **Status:** 100% Complete - Full Merlin 8/16/32 compatibility

Cross-platform assembler syntax module providing complete support for the Merlin assembler dialect used in Apple II and Apple IIgs development.

## Quick Start

### Building

```bash
cd /path/to/vasm
make CPU=6502 SYNTAX=merlin
```

This creates the `vasm6502_merlin` executable.

### Basic Usage

```bash
./vasm6502_merlin -Fbin -o output.bin source.asm
```

### Example Code

```asm
* Simple Merlin Program
        ORG   $8000

BELL    EQU   $FBDD
COUNT   =     5

START   LDX   #COUNT
:LOOP   JSR   BELL        ; Ring the bell
        DEX
        BNE   :LOOP
        RTS

        CHK               ; Checksum byte
        END
```

## Key Features

### 🏷️ Three-Tier Label System
- **Global labels**: `START`, `LOOP`, `DATA`
- **Local labels**: `:LOOP`, `:DONE` (scoped between globals, bidirectional)
- **Variable labels**: `]VAR` (generate unique names on each definition)

### 📝 Directives (No Dot Prefix)
- **Data**: `DFB`, `DA`, `DW`, `DDB`, `ADR`, `ADRL`, `DS`
- **Strings**: `ASC`, `DCI`, `INV`, `FLS`, `REV`, `STR`, `STRL`, `HEX`
- **Macros**: `MAC`...`<<<` (or `EOM`), `PMC`, `>>>`
- **Loops**: `LUP`...`--^`
- **Conditionals**: `DO`...`ELSE`...`FIN`
- **65816**: `MX`, `XC`, `LONGA`, `LONGI`
- **I/O**: `PUT`, `USE`, `REL`, `DSK`, `ENT`, `EXT`
- **Utility**: `CHK`, `DAT`, `ERR`

### 🎯 String Delimiter Semantics

```asm
TEXT1   ASC   "Hello"    ; High bit SET (delimiter < 0x27)
TEXT2   ASC   'Hello'    ; High bit CLEAR (delimiter >= 0x27)
TEXT3   DCI   "World"    ; Last char inverted
TEXT4   INV   "Test"     ; All chars high bit set
```

Delimiters < 0x27 (apostrophe) → high bit SET
Delimiters >= 0x27 → high bit CLEAR

### 🔧 Macro System

**Parameters:** `]0` (count), `]1`-`]8` (arguments)

**Separators:** `,` `/` `.` `-` space `(`

```asm
DELAY   MAC
        LDX   #]1
:LOOP   DEX
        BNE   :LOOP
        <<<               ; Or EOM, EM, ENDM

* Invoke macro
        DELAY 100         ; Direct
        PMC   DELAY,100   ; PMC prefix
        >>>   DELAY,100   ; >>> prefix
```

### 🚀 65816 Support

```asm
        XC                ; Enable 65816 mode
        MX    %00         ; 16-bit A and X/Y
        LONGA ON          ; 16-bit accumulator
        LONGI ON          ; 16-bit index registers

        REP   #$30        ; 16-bit mode
        LDA   #$1234      ; 16-bit load
```

## Documentation

📚 **[Complete Syntax Reference →](SYNTAX_CONVENTIONS.md)**

The SYNTAX_CONVENTIONS.md file contains the comprehensive reference guide with:
- All directives documented with examples
- Complete macro system details
- Expression operators and precedence
- Addressing modes and CPU features
- Real-world code examples

## Compatibility

### Supported
- ✅ All Merlin 8/16/32 directives
- ✅ Three-tier label system (global, `:local`, `]variable`)
- ✅ Macro system with `]0`-`]8` parameters
- ✅ String directives with delimiter semantics
- ✅ 65816 16-bit modes (MX, LONGA, LONGI)
- ✅ ProDOS environment (USE with prefix mapping)
- ✅ OMF output support (REL, ENT, EXT)

### Differences from Merlin 32
- **Line numbers**: Not supported (use labels instead)
- **Editor commands**: Ignored (e.g., `LIST`, `NEW`, `OLD`)
- **Interactive mode**: Command-line only (no REPL)
- **Checksum (CHK)**: Placeholder byte (full calculation requires output module)

## Environment Variables

```bash
# ProDOS prefix mapping for USE directive
export VASM_MERLIN_PREFIX_4="/path/to/macro/library"
```

Maps `USE 4/MACROS` → `/path/to/macro/library/MACROS`

## Testing

```bash
# Run test suite
make CPU=6502 SYNTAX=merlin test

# Individual test
./vasm6502_merlin -Fbin -o /tmp/test.bin tests/merlin/test_basic.asm
```

## Resources

- **User Manual**: Merlin 8/16 User's Manual (Roger Wagner Publishing, 1987)
- **Merlin 32**: https://www.brutaldeluxe.fr/products/crossdevtools/merlin/
- **Apple II Documentation**: https://archive.org/

## License

vasm core: (c) 2002-2025 Volker Barthelmann and Frank Wille
Merlin syntax module: (c) 2025 Bryan Woodruff, Cortexa LLC

See root LICENSE file for details.
