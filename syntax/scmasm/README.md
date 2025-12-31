# SCASM Syntax Module for vasm

✅ **Status:** 100% Complete - Full S-C Macro Assembler 2.0/3.0/3.1 compatibility

Cross-platform assembler syntax module providing complete support for the S-C Macro Assembler (SCASM) dialect used in Apple II 6502/65816 development, including the A2osX project.

## Quick Start

### Building

```bash
cd /path/to/vasm
make CPU=6502 SYNTAX=scmasm
```

This creates the `vasm6502_scmasm` executable.

### Basic Usage

```bash
./vasm6502_scmasm -Fbin -o output.bin source.asm
```

### Example Code

```asm
* Simple SCASM Program
        .OR   $8000

BELL    .EQ   $FBDD
COUNT   .SE   5

MAIN    LDX   #COUNT
.1      JSR   BELL        ; Ring the bell
        DEX
        BNE   .1
        RTS

        .TF
```

## Key Features

### 🏷️ Label System
- **Global labels**: `MAIN`, `LOOP`, `DATA` (column 1, no leading space)
- **Local labels**: `.N` (numbered 0-9, forward-reference only, scoped to last global)
- **BASIC-style line numbers**: Optional 0-65535 in column 1

### 📝 Directives (Dot Prefix)
- **Data**: `.DA`, `.HS`, `.BS`
- **Strings**: `.AS`, `.AZ`, `.AT` (flexible delimiters)
- **Symbols**: `.EQ` (constant), `.SE` (redefinable)
- **Control**: `.DO`...`.FIN`, `.MA`...`.EM`, `.LU`...`.ENDU`
- **CPU**: `.OP` (6502/65C02/65R02/65816)
- **I/O**: `.IN`, `.TF`
- **Phase**: `.PH`...`.EP`, `.DUMMY`...`.ED`

### 🎯 String Delimiter Semantics

```asm
TEXT1   .AS   "Hello"    ; High bit SET (delimiter < 0x27)
TEXT2   .AS   'Hello'    ; High bit CLEAR (delimiter >= 0x27)
TEXT3   .AS   /World/    ; Flexible - any non-whitespace delimiter
TEXT4   .AZ   |NullTerm| ; Zero-terminated
TEXT5   .AT   #Last#     ; High bit set on last char only
```

**Delimiter Rule:** ASCII value < 0x27 (apostrophe) → high bit SET, >= 0x27 → high bit CLEAR

### 🔧 Macro System

**Parameters:** `\0` (count), `\1`-`\9` (arguments)

**Separators:** `,` space

```asm
DELAY   .MA
        LDX   #\1
.1      DEX
        BNE   .1
        .EM

* Invoke macro
        DELAY 100         ; Pass 100 as \1
```

### 🔢 Data Directives with Byte Extraction

```asm
VALUE   .EQ   $12345678

* Single byte extraction
        .DA   #VALUE      ; Low byte: 78
        .DA   /VALUE      ; 2nd byte: 56

* Multi-byte output
        .DA   VALUE       ; 16-bit: 78 56 (low 16 bits)
        .DA   <VALUE      ; 24-bit: 78 56 34
        .DA   >VALUE      ; 32-bit: 78 56 34 12
```

### 🚀 CPU Selection

```asm
        .OP   6502        ; Standard 6502
        NOP               ; Legal instruction

        .OP   65C02       ; CMOS variant
        STZ   $00         ; Now legal

        .OP   65816       ; 16-bit CPU
        REP   #$30        ; 16-bit mode
```

### ♻️ Redefinable Symbols

```asm
INDEX   .SE   0           ; Set redefinable symbol
        .DA   INDEX       ; Output: 00 00
INDEX   .SE   1           ; Redefine
        .DA   INDEX       ; Output: 01 00
INDEX   .SE   2           ; Redefine again
        .DA   INDEX       ; Output: 02 00
```

### 🔁 Loop Directive

```asm
        .LU   5           ; Repeat 5 times
        NOP               ; Emit 5 NOPs
        .ENDU
```

## Documentation

📚 **[Complete Syntax Reference →](SYNTAX_CONVENTIONS.md)**

The SYNTAX_CONVENTIONS.md file contains the comprehensive reference guide with:
- All directives documented with examples
- Complete macro and loop system
- Expression operators and force modes
- Memory protection and vectors
- Real-world code examples

## Compatibility

### Supported
- ✅ All SCASM 2.0/3.0/3.1 directives
- ✅ Label system (global, `.N` local, line numbers)
- ✅ Macro system with `\0`-`\9` parameters
- ✅ String directives with flexible delimiters
- ✅ CPU selection (`.OP` directive)
- ✅ Loop directive (`.LU`/`.ENDU`)
- ✅ Redefinable symbols (`.SE`)
- ✅ Phase directives (`.PH`/`.EP`, `.DUMMY`/`.ED`)

### Differences from Original SCASM
- **Editor commands**: Ignored (e.g., `NEW`, `EDIT`, `COPY`, `RENUMBER`)
- **Interactive mode**: Command-line only (no REPL)
- **`.TF` directive**: Accepted but metadata not processed
- **Memory protection**: Not applicable (no runtime environment)

## Force Addressing Modes

```asm
VALUE   .EQ   $05

        LDA   VALUE       ; Assembler chooses best mode
        LDA   <VALUE      ; Force zero page
        LDA   VALUE       ; Force absolute (default)
        LDA   >>VALUE     ; Force long (65816)
```

## Testing

```bash
# Run test suite
make CPU=6502 SYNTAX=scmasm test

# Individual test
./vasm6502_scmasm -Fbin -o /tmp/test.bin tests/scmasm/test_basic.asm
```

## Boolean Operators

```asm
MASK1   .EQ   $F0
MASK2   .EQ   $0F

BOTH    .EQ   MASK1&MASK2    ; AND: $00
EITHER  .EQ   MASK1|MASK2    ; OR:  $FF
TOGGLE  .EQ   MASK1~MASK2    ; XOR: $FF
```

## Line Numbering

```asm
* BASIC-style line numbers (optional)
1000            .OR   $8000
1010    START   LDA   #$00
1020            STA   $C000
1030            RTS
1040            .TF
```

## Resources

- **User Manual**: S-C Macro Assembler Version 2.0 Manual (S-C Software, 1984)
- **A2osX Project**: https://github.com/burniouf/A2osX
- **Apple II Documentation**: https://archive.org/

## Known Use Cases

- **A2osX**: Apple II Unix-like operating system
- **Legacy Apple II projects**: SCASM-based source code
- **Cross-platform development**: Modern toolchain for vintage code

## License

vasm core: (c) 2002-2025 Volker Barthelmann and Frank Wille
SCASM syntax module: (c) 2025 Bryan Woodruff, Cortexa LLC

See root LICENSE file for details.
