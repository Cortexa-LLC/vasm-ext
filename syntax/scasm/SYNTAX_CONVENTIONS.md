# S-C Macro Assembler (SCASM) Syntax Conventions

## Overview

The S-C Macro Assembler (SCASM) Version 2.0 is a professional macro assembler for the 6502 family of microprocessors, developed by S-C Software Corporation. It provides comprehensive support for:

- **6502** - Standard 6502 instruction set (default)
- **Sweet-16** - Apple's 16-bit interpreter
- **65C02** - CMOS 6502 with enhanced instruction set
- **65R02** - Rockwell 65C02 with bit manipulation instructions
- **65802** - 16-bit 6502 compatible processor
- **65816** - Full 16-bit 65xx series processor

**Platforms**: Apple II series (//e, //c, II+, IIe) with 48K or 64K RAM

**Key Features**:
- BASIC-style line numbering (0-65535)
- Macros with parameters
- Conditional assembly
- Multiple CPU modes
- 32-bit expression evaluation
- Local labels
- Memory protection
- Target file output
- Assembly listing generation

---

## Source Line Format

Each source line follows this general format:

```
[line_number] [label] [opcode] [operand] [;comment]
```

### Components

1. **Line Number** (optional): 0-65535, sorted automatically
2. **Label** (optional): Symbol definition or local label
3. **Opcode**: Instruction mnemonic or directive (with `.` prefix for directives)
4. **Operand**: Expression, addressing mode, or directive parameters
5. **Comment**: Text following `;` or entire line starting with `*` or `;`

### Examples

```assembly
1000 START    LDA #$00        ; Initialize accumulator
1010          STA $C000       ; Store to location
1020 *--------------------------------
1030          RTS             ; Return from subroutine
```

---

## Comments

Two comment styles are supported:

### Full Line Comments

Lines beginning with `*` or `;`:

```assembly
1000 * This is a comment line
1010 ; This is also a comment line
1020 *--------------------------------
```

### Inline Comments

Any text following the operand field is treated as a comment (no special character needed in standard cases):

```assembly
1000 START    LDA #$00    Clear accumulator
```

---

## Line Numbers

### Characteristics

- **Range**: 0 to 65535
- **Optional**: Not required but useful for editing
- **Auto-sorting**: Lines automatically sorted by number
- **Display format**:
  - Numbers < 10000: 4 digits with leading zeros
  - Numbers >= 10000: 5 digits

### Examples

```assembly
0100          .OR $0800
1000 START    LDA #$00
1010          STA COUNT
9999          RTS
10000         NOP
65535 END     BRK
```

---

## Labels

### Standard Labels

- Defined by appearing in the label field
- Cannot be redefined (unless using `.SE`)
- Used for addresses, constants, and symbols

```assembly
1000 START    LDA #$00
1010 COUNT    .DA $00
1020 MAXVAL   .EQ 255
```

### Local Labels (Numeric)

Format: `.N` where N is 0-9

**Important**: Local labels can ONLY be forward-referenced. They are redefined each time they appear.

```assembly
1000 LOOP     LDA TABLE,X
1010          BEQ .1          ; Forward reference to next .1
1020          JSR PROCESS
1030 .1       INX             ; Defines .1
1040          CPX #10
1050          BNE LOOP
1060          RTS
1070 .1       NOP             ; Redefines .1 (previous .1 no longer accessible)
```

---

## Numeric Constants

### Decimal

Standard decimal numbers:

```assembly
1000          LDA #100
1010          .DA 32767
```

### Hexadecimal

Prefix with `$`:

```assembly
1000          LDA #$FF
1010          STA $C000
1020          .DA $1234
```

### Binary

Prefix with `%`, optional `.` separators for readability:

```assembly
1000          LDA #%11001100
1010          AND #%01111111
1020          .DA %10000001
1030          LDA #%1.0000.1000          ; With separators
1040          AND #%0111.1111            ; More readable
```

### ASCII Character Literals

**Low bit zero** (normal ASCII): Use `'` (apostrophe)

```assembly
1000          LDA #'A          ; $41
1010          CMP #'Z          ; $5A
```

**High bit set**: Use `"` (quotation mark)

```assembly
1000          LDA #"A          ; $C1 (inverse/high-bit)
1010          CMP #"Z          ; $DA
```

Trailing quote/apostrophe is optional:

```assembly
1000          LDA #'X          ; Valid
1010          LDA #'X'         ; Also valid
1020          LDA #"X          ; Valid
1030          LDA #"X"         ; Also valid
```

---

## String Directives

Three string directives for text data:

### .AS - ASCII String (High Bit Clear)

Stores text with high bit clear (normal ASCII):

```assembly
1000 MSG      .AS /HELLO/      ; Use any delimiter
1010          .AS |WORLD|
1020          .AS *TEXT*
```

### .AZ - ASCII String with Zero Terminator

Like `.AS` but appends a `$00` byte:

```assembly
1000 CSTR     .AZ /C STRING/   ; Null-terminated
```

### .AT - ASCII String (High Bit Set)

Stores text with high bit set (inverse video on Apple II):

```assembly
1000 INVMSG   .AT /HELLO/      ; Each char has bit 7 set
```

### Delimiter Rules

- Use any non-alphanumeric character as delimiter
- Same delimiter must appear at start and end
- Common delimiters: `/`, `|`, `*`, `#`, `@`

```assembly
1000          .AS /TEXT/
1010          .AS |MORE TEXT|
1020          .AZ #LAST ONE#
```

---

## Data Definition

### .DA - Define Address/Data

Generates 8-bit, 16-bit, 24-bit, or 32-bit data values.

#### Syntax Operators

| Operator | Bits | Description |
|----------|------|-------------|
| `#expr` | 8 | Low byte only |
| `/expr` | 8 | Second byte (bits 8-15) |
| `expr` | 16 | Low 16 bits (2 bytes, high byte first) |
| `expr/256` | 16 | Middle 16 bits of 32-bit value |
| `expr/65536` | 16 | High 16 bits (2 bytes, high byte first) |
| `<expr` | 24 | Low 24 bits (3 bytes, high byte first) |
| `<expr/256` | 24 | High 24 bits (3 bytes, high byte first) |
| `>expr` | 32 | Full 32 bits (4 bytes, high byte first) |

#### Examples

```assembly
0800- 78       1020 BYTE     .DA #$12345678         ; Low byte
0801- 56       1030          .DA /$12345678         ; 2nd byte
0802- 56       1040          .DA #$12345678/256     ; 2nd byte (alternate)
0803- 34       1050          .DA #$12345678/65536   ; 3rd byte
0804- 12       1060          .DA /$12345678/65536   ; 4th byte

0806- 78 56    1090 WORD16   .DA $12345678          ; Low word (16 bits)
0808- 56 34    1100          .DA $12345678/256      ; Middle word
080A- 34 12    1110          .DA $12345678/65536    ; High word

080C- 78 56 34 1130 WORD24   .DA <$12345678         ; Low 24 bits
080F- 56 34 12 1140          .DA <$12345678/256     ; High 24 bits

0812- 78 56 34
0815- 12       1160 WORD32   .DA >$12345678         ; Full 32 bits

0816- 11 33 22
0819- 66 55 44
081C- AA 99 88
081F- 77       1180          .DA #$11,$2233,<$445566,>$778899AA
```

Multiple values can be comma-separated on one line, mixing sizes.

### .HS - Hexadecimal String

Define raw hexadecimal bytes:

```assembly
1000          .HS 01 02 03 04
1010          .HS A9 00 8D 00 C0    ; LDA #$00 : STA $C000
1020          .HS 60                ; RTS
```

Optional commas between byte pairs for readability (Version 2.0+):

```assembly
1000          .HS 01,02,03,04
1010          .HS A9,00,8D,00,C0
```

### .BS - Block Storage

Reserve memory space, optionally filled with specified value:

```assembly
.BS count[,fillbyte]
```

Examples:

```assembly
1000 BUFFER   .BS 32           ; 32 bytes filled with $00
1010 SYMBOL   .BS 4,$A0        ; 4 bytes filled with $A0
1020 VALUE    .BS 2,$FF        ; 2 bytes filled with $FF
```

- Default fill: `$00`
- Range: 0-32767 bytes (RANGE ERROR if outside)
- In memory: stores fill bytes
- On target file: writes fill bytes

---

## Expressions

### Expression Evaluation

- **32-bit** arithmetic (full 32-bit range)
- Evaluation order: operator precedence
- Current address: `*` (asterisk)

### Arithmetic Operators

| Operator | Operation | Example |
|----------|-----------|---------|
| `+` | Addition | `100+50` |
| `-` | Subtraction | `200-50` |
| `*` | Multiplication | `10*5` |
| `/` | Division | `100/4` |

### Boolean/Logical Operators (Version 2.0+)

| Operator | Operation | Example |
|----------|-----------|---------|
| `&` | Logical AND | `$12345678&$F0F0F0F0` = `$10305070` |
| `\|` | Logical OR | `$12345678\|$F0F0F0F0` = `$F2F4F6F8` |
| `~` | Logical XOR | `$12345678~$F0F0F0F0` = `$E2C4A688` |

### Operator Precedence

1. Parentheses `( )`
2. Multiplication `*`, Division `/`
3. Addition `+`, Subtraction `-`
4. Boolean AND `&`
5. Boolean OR `|`, XOR `~`

### Special Expression Forms

#### Byte Extraction

- `#expr` - Low byte (bits 0-7)
- `/expr` - Second byte (bits 8-15)
- `~expr` - Third byte (bits 16-23)
- `expr/256` - Shift right 8 bits
- `expr/65536` - Shift right 16 bits

#### Examples

```assembly
0800- AD 08 01  1010          LDA #%1.0000.1000       ; Binary with separators
0803- 29 7F     1020          AND ##%01111111         ; Low 8 bits
0805- 01 80     1030          .DA %1000000000000001   ; 16-bit binary
0807- 34 12     1040          .DA %0001.0010.0010.0100
```

---

## Addressing Modes

### 6502 Standard Addressing Modes

| Mode | Syntax | Example | Description |
|------|--------|---------|-------------|
| Implied | `opcode` | `RTS` | No operand |
| Accumulator | `opcode A` | `INC A` | Operate on A register |
| Immediate | `opcode #expr` | `LDA #$00` | Literal value |
| Zero Page | `opcode expr` | `LDA $80` | Page zero address |
| Zero Page,X | `opcode expr,X` | `LDA $80,X` | Zero page indexed by X |
| Zero Page,Y | `opcode expr,Y` | `LDX $80,Y` | Zero page indexed by Y |
| Absolute | `opcode expr` | `LDA $C000` | 16-bit address |
| Absolute,X | `opcode expr,X` | `LDA $C000,X` | Absolute indexed by X |
| Absolute,Y | `opcode expr,Y` | `LDA $C000,Y` | Absolute indexed by Y |
| Indirect | `opcode (expr)` | `JMP ($FFFC)` | Indirect address |
| Indexed Indirect | `opcode (expr,X)` | `LDA ($80,X)` | Pre-indexed indirect |
| Indirect Indexed | `opcode (expr),Y` | `LDA ($80),Y` | Post-indexed indirect |
| Relative | `opcode label` | `BNE LOOP` | Branch offset |

### Force Addressing Mode

Use `<`, `>`, or `>>` to force specific addressing mode:

| Prefix | Mode | Bytes | Example |
|--------|------|-------|---------|
| (none) | Auto | Varies | `LDA VALUE` |
| `<` | Zero Page | 2 | `LDA <VALUE` |
| `>` | Absolute | 3 | `LDA >VALUE` |
| `>>` | Long | 4 | `LDA >>VALUE` (65816 only) |

#### Examples

```assembly
1000 VALUE    .EQ $05

000800- A5 03     1030          LDA 3              ; Auto: zero page
000802- AD 03 00  1040          LDA >3             ; Force: absolute
000805- AF 03 00
        00        1050          LDA >>3            ; Force: long (65816)
000809- 4C 03 00  1060          JMP 3              ; Auto: absolute
00080C- 5C 03 00
        00        1070          JMP >>3            ; Force: long (65816)
000810- AD 34 12  1080          LDA $1234          ; Auto: absolute
000813- A5 34     1090          LDA <$1234         ; Force: zero page
```

---

## CPU Mode Selection

### .OP Directive

The `.OP` directive selects which instruction set to assemble:

```assembly
.OP 6502      ; Standard 6502 (default)
.OP SW16      ; Sweet-16
.OP 65C02     ; CMOS 65C02
.OP 65R02     ; Rockwell 65C02 with bit ops
.OP 65802     ; 65802 (16-bit)
.OP 65816     ; 65816 (16-bit, same as 65802)
```

### Detection Algorithm

The directive scans for key characters:
- No key chars → 6502 mode
- Contains "S" → Sweet-16 mode
- Contains "C" → 65C02 mode
- Contains "R" → 65R02 mode (Rockwell)
- Contains "8" → 65802/65816 mode

### Mode Inheritance

Each mode includes all instructions from its predecessors:
- **SW16**: Sweet-16 only
- **65C02**: 6502 + 65C02 enhancements
- **65R02**: 6502 + 65C02 + Rockwell bit operations
- **65802/65816**: 6502 + 65C02 + 16-bit enhancements

---

## 65C02 Enhanced Instructions

When `.OP 65C02` or higher is active:

### New Opcodes

```assembly
BRA reladdr         ; Branch Always
PHX                 ; Push X-register
PHY                 ; Push Y-register
PLX                 ; Pull X-register
PLY                 ; Pull Y-register
STZ zp              ; Store Zero
STZ zp,X            ; Store Zero indexed
STZ abs             ; Store Zero absolute
STZ abs,X           ; Store Zero absolute indexed
TRB zp              ; Test and Reset Bits
TSB zp              ; Test and Set Bits
```

### Enhanced Addressing Modes

Existing opcodes gain new addressing modes:

```assembly
BIT #val8           ; Test Bits immediate
BIT zp,X            ; Test Bits zero page indexed
BIT abs,X           ; Test Bits absolute indexed
INC                 ; Increment A-register (no operand)
DEC                 ; Decrement A-register (no operand)
JMP (abs,X)         ; Jump indexed indirect

; Direct Indirect mode (zp without ,Y):
ADC (zp)            ; Also: AND, CMP, EOR, LDA, ORA, SBC, STA
```

---

## 65R02 Rockwell Bit Operations

When `.OP 65R02` is active (includes all 65C02):

### Bit Manipulation Instructions

```assembly
SMB bit,zp          ; Set Memory Bit (bit = 0-7)
RMB bit,zp          ; Reset Memory Bit (bit = 0-7)
BBR bit,zp,reladdr  ; Branch if Bit Reset
BBS bit,zp,reladdr  ; Branch if Bit Set
```

These use 32 opcodes total (bit number encoded in opcode byte).

### Examples

```assembly
1000          SMB 0,$80       ; Set bit 0 of $80
1010          RMB 7,$81       ; Clear bit 7 of $81
1020          BBR 3,$82,NEXT  ; Branch if bit 3 of $82 is clear
1030          BBS 5,$83,LOOP  ; Branch if bit 5 of $83 is set
```

---

## 65816/65802 16-bit Extensions

When `.OP 65816` or `.OP 65802` is active:

### New Opcodes

#### Stack Operations
```assembly
PEA val16           ; Push 16-bit value on stack
PEI val8            ; Push 8-bit value on stack
PER longreladdr     ; Push 16-bit relative address
PHB                 ; Push Data Bank Register
PLB                 ; Pull Data Bank Register
PHD                 ; Push D-register
PLD                 ; Pull D-register
PHK                 ; Push Program Bank Register
```

#### Long Addressing & Branches
```assembly
BRL longreladdr     ; Branch Always (16-bit offset)
JML (abs)           ; Jump Long Absolute Indirect
JSL longabs         ; Jump to Subroutine Long (24-bit)
RTL                 ; Long Return from Subroutine
JMP longabs         ; Jump with 24-bit address
```

#### Block Move
```assembly
MVP longabs,longabs ; Block Move Up (previous/decrement)
MVN longabs,longabs ; Block Move Down (next/increment)
```

#### Register Transfers
```assembly
TCD                 ; Transfer C to D
TDC                 ; Transfer D to C
TCS                 ; Transfer C to S
TSC                 ; Transfer S to C
TXY                 ; Transfer X to Y
TYX                 ; Transfer Y to X
XBA                 ; Exchange B and A
XCE                 ; Exchange Carry with Emulation bit
```

#### Processor Control
```assembly
REP #val8           ; Reset Status Bits
SEP #val8           ; Set Status Bits
COP                 ; Co-Processor Interrupt
STP                 ; Stop Clock until RESET
WAI                 ; Wait for Interrupt
WDM                 ; Reserved (no operation)
```

### 16-bit Immediate Mode

Use `##` (double delimiter) for 16-bit immediate values:

| Syntax | Bits | Description |
|--------|------|-------------|
| `#expr` or `/expr` or `~expr` | 8 | Low/mid/high byte |
| `##expr` or `//expr` or `~~expr` | 16 | Low/mid/high word |

#### Examples

```assembly
1000          LDA #$12        ; 8-bit immediate (low byte)
1010          LDA /$12        ; 8-bit immediate (mid byte of 24-bit)
1020          LDA ~$12        ; 8-bit immediate (high byte of 24-bit)

1030          LDA ##$1234     ; 16-bit immediate (low word)
1040          LDA //$1234     ; 16-bit immediate (mid word of 32-bit)
1050          LDA ~~$1234     ; 16-bit immediate (high word of 32-bit)
```

**Note**: The opcode is the same for 8-bit and 16-bit immediate modes. The P-register M/X bits determine operation at runtime. The assembler syntax makes the programmer's intent explicit.

### New Addressing Modes

#### Stack-Relative
```assembly
ADC val8,S          ; Stack relative
```

#### Stack-Relative Indirect Indexed
```assembly
ADC (val8,S),Y      ; Stack relative indirect indexed by Y
```

#### Direct Indirect Long
```assembly
ADC >(zp)           ; Direct indirect long (24-bit address)
```

#### Indirect Indexed Long
```assembly
ADC >(zp),Y         ; Indirect indexed long (24-bit with Y)
```

#### Indexed Indirect Subroutine
```assembly
JSR (abs,X)         ; Jump to Subroutine indexed indirect
```

#### Examples

```assembly
1000 .OP 65816
1010          LDA ##$1234     ; 16-bit immediate
1020          LDA $123456     ; 24-bit long address
1030          LDA 5,S         ; Stack relative
1040          LDA (7,S),Y     ; Stack relative indirect indexed
1050          LDA >($80)      ; Direct indirect long
1060          LDA >($80),Y    ; Indirect indexed long
1070          JSR ($2000,X)   ; Indexed indirect subroutine
```

---

## Directives (Alphabetical)

All directives begin with `.` (period).

### .AS - ASCII String

Define ASCII string with high bit clear:

```assembly
.AS /text/
```

Example:
```assembly
1000 MSG      .AS /HELLO WORLD/
```

See [String Directives](#string-directives) for details.

---

### .AT - ASCII String (High Bit Set)

Define ASCII string with high bit set:

```assembly
.AT /text/
```

Example:
```assembly
1000 INVMSG   .AT /INVERSE TEXT/
```

See [String Directives](#string-directives) for details.

---

### .AZ - ASCII String with Zero Terminator

Define null-terminated ASCII string:

```assembly
.AZ /text/
```

Example:
```assembly
1000 CSTR     .AZ /NULL TERMINATED/
```

See [String Directives](#string-directives) for details.

---

### .BS - Block Storage

Reserve memory block with optional fill:

```assembly
.BS count[,fillbyte]
```

Examples:
```assembly
1000 BUFFER   .BS 256          ; 256 bytes of $00
1010 WORK     .BS 16,$FF       ; 16 bytes of $FF
```

- Default fill: `$00`
- Range: 0-32767 bytes
- RANGE ERROR if count negative or > 32767

---

### .DA - Define Address/Data

Generate data bytes (8/16/24/32-bit):

```assembly
.DA [#|/|<|>]expr[,expr,...]
```

See [Data Definition](#data-definition) for complete syntax.

Examples:
```assembly
1000          .DA $1234           ; 16-bit: 12 34
1010          .DA #$1234          ; 8-bit: 34
1020          .DA <$123456        ; 24-bit: 12 34 56
1030          .DA >$12345678      ; 32-bit: 12 34 56 78
```

---

### .DO / .ELSE / .FIN - Conditional Assembly

Conditional assembly block:

```assembly
.DO expression
    ; assembled if expression != 0
[.ELSE]
    ; assembled if expression == 0
.FIN
```

- Nesting: Up to 63 levels deep
- Expression evaluated as true (non-zero) or false (zero)

Examples:
```assembly
1000 DEBUG    .EQ 1
1010          .DO DEBUG
1020          JSR TRACE
1030          .FIN

1100          .DO VERSION-2
1110          JSR NEWCODE
1120          .ELSE
1130          JSR OLDCODE
1140          .FIN
```

---

### .DUMMY / .ED - Dummy Section

Define dummy section (no object code generated):

```assembly
.DUMMY
    ; lines assemble but generate no code
.ED
```

- Useful for data structure definitions
- Useful for syntax checking without code generation
- `.DUMMY` saves origin, `.ED` restores it
- `.OR` within dummy section only affects dummy section

Special feature: If `.TF` appears in dummy section, the listing is written to the target file!

Example:
```assembly
0800- 34 12    1000 ADDR     .DA LABEL
               1010 *-------------------------
               1020          .DUMMY
               1030          .OR $1234
1234- AD 00 08 1040 LABEL    LDA ADDR
1237-          1050 NEXT     .BS 1
1238-          1060 AGAIN    .BS 1
               1070          .ED
               1080 *-------------------------
0802- AD 35 12 1090          LDA NEXT
```

---

### .EM - End Macro

End macro definition:

```assembly
.MA MACRONAME
    ; macro body
.EM
```

See [.MA](#ma---macro-definition) for details.

---

### .ENDU - End Loop

End `.LU` loop:

```assembly
.LU count
    ; repeated lines
.ENDU
```

See [.LU](#lu---loop) for details.

---

### .EP - End Phase

End phase section:

```assembly
.PH address
    ; assembled at address but stored elsewhere
.EP
```

See [.PH](#ph--ep---phase-assembly) for details.

---

### .EQ - Equate

Define symbol with fixed value:

```assembly
label .EQ expression
```

- Symbol cannot be redefined
- Use `.SE` for redefinable symbols

Examples:
```assembly
1000 MAXVAL   .EQ 255
1010 IOPORT   .EQ $C000
1020 OFFSET   .EQ BUFFER+16
```

---

### .FIN - Finish Conditional

End `.DO` conditional block:

```assembly
.DO expression
    ; code
.FIN
```

See [.DO](#do--else--fin---conditional-assembly) for details.

---

### .HS - Hexadecimal String

Define raw hexadecimal bytes:

```assembly
.HS hexdigits [,hexdigits...]
```

- Two hex digits per byte
- Optional commas between pairs (Version 2.0+)

Examples:
```assembly
1000          .HS 01 02 03 04
1010          .HS A9,00,8D,00,C0      ; With commas
1020          .HS 4C0008              ; JMP $0800
```

---

### .IN - Include File

Include another source file:

```assembly
.IN filename
```

- Inserts contents of file at this point
- Can be nested
- File must be on accessible disk

Example:
```assembly
1000          .IN MACROS
1010          .IN SUBROUTINES
```

---

### .LIST - Listing Control

Control assembly listing output:

```assembly
.LIST [ON|OFF|CON|COFF]
```

| Option | Effect |
|--------|--------|
| `ON` | Enable listing |
| `OFF` | Disable listing |
| `CON` | Show conditional lines (excluded by .DO) |
| `COFF` | Hide conditional lines (default) |

Examples:
```assembly
1000          .LIST OFF        ; Stop listing
1010          ; (these lines not in listing)
1020          .LIST ON         ; Resume listing
1030          .LIST CON        ; Show .DO excluded lines
1040          .DO 0
1050          NOP              ; This will appear in listing
1060          .FIN
```

---

### .LU / .ENDU - Loop

Repeat assembly of lines:

```assembly
.LU count
    ; lines to repeat
.ENDU
```

- Count: Number of times to repeat
- Useful for generating tables or repetitive code

Example:
```assembly
1000 INDEX    .SE 0
1010          .LU 8
1020          .DA INDEX*256
1030 INDEX    .SE INDEX+1
1040          .ENDU
```

This generates:
```
.DA 0*256
.DA 1*256
.DA 2*256
...
.DA 7*256
```

---

### .MA / .EM - Macro Definition

Define a macro:

```assembly
.MA MACRONAME
    ; macro body
    ; use \0 through \9 for parameters
.EM
```

Invoke macro:
```assembly
MACRONAME [param0][,param1][,param2]...[,param9]
```

- Parameters referenced as `\0`, `\1`, `\2`, ..., `\9` (up to 10 parameters)
- Parameters are text substitution
- Macros can contain any assembly code

Examples:

**Definition:**
```assembly
1000          .MA MOVE
1010          LDA \0
1020          STA \1
1030          .EM
```

**Invocation:**
```assembly
1100          MOVE SOURCE,DEST
```

**Expands to:**
```assembly
          LDA SOURCE
          STA DEST
```

**More complex example:**
```assembly
1000 COUNT    .SE 0
1010          .MA GENTAB
1020 TAB\0    .DA COUNT
1030 COUNT    .SE COUNT+1
1040          .EM

1100          .LU 5
1110          GENTAB \COUNT
1120          .ENDU
```

Generates:
```
TAB0    .DA 0
TAB1    .DA 1
TAB2    .DA 2
TAB3    .DA 3
TAB4    .DA 4
```

---

### .OP - Select CPU Mode

Select instruction set:

```assembly
.OP 6502        ; Standard 6502 (default)
.OP SW16        ; Sweet-16
.OP 65C02       ; 65C02
.OP 65R02       ; Rockwell 65C02
.OP 65802       ; 65802 (16-bit)
.OP 65816       ; 65816 (16-bit)
```

See [CPU Mode Selection](#cpu-mode-selection) for details.

Example:
```assembly
1000          .OP 6502
1010          LDA #$00        ; Standard 6502
1020
1030          .OP 65816       ; Switch to 65816 mode
1040          LDA ##$1234     ; 16-bit immediate now available
1050          JSL $012345     ; Long addressing
```

---

### .OR - Origin

Set assembly origin address:

```assembly
.OR expression
```

- Sets both assembly address and target address
- All subsequent code assembled at this address
- Can be used multiple times

Example:
```assembly
1000          .OR $0800
1010 START    LDA #$00
1020          STA $C000

2000          .OR $2000
2010 DATA     .DA $1234
```

---

### .PH / .EP - Phase Assembly

Assemble code to run at different address than stored:

```assembly
.PH execution_address
    ; code assembled for execution_address
    ; but stored at current target address
.EP
```

- `.PH` sets origin for assembly but not target storage
- `.EP` resyncs origin with target
- Useful for relocating code at runtime

Example:
```assembly
0800- AD 03 9D  1030 START    LDA DATA
0803- 60        1040          RTS
                1050 *-------------------------
                1060          .PH $9000
9000- A9 08     1070 PATCH    LDA #ADDR
9002- 60        1080          RTS
9003-           1090 DATA     .BS 1
                1100          .EP
                1110 *-------------------------
0808- 00 90     1120 ADDR     .DA PATCH
```

Object code stored at `$0800-$0809`, but PATCH assembled for execution at `$9000`.

Memory after assembly:
```
$0800: AD 03 9D 60 A9 08 60 00 00 90
```

---

### .SE - Set Equate (Redefinable)

Define redefinable symbol:

```assembly
label .SE expression
```

- Symbol can be redefined by subsequent `.SE` directives
- Useful for counters in macros and loops
- Initial definition can be `.EQ` or `.SE`

Example:
```assembly
1000 COUNT    .SE 0
1010          .LU 10
1020 ENTRY    .DA COUNT*4
1030 COUNT    .SE COUNT+1
1040          .ENDU
```

Generates:
```
ENTRY    .DA 0
ENTRY    .DA 4
ENTRY    .DA 8
...
ENTRY    .DA 36
```

---

### .TF - Target File

Specify target file for object code:

```assembly
.TF filename
```

- Subsequent object code written to this file
- Can appear multiple times to switch output files
- Special: If in `.DUMMY` section, writes listing instead of object code

Example:
```assembly
1000          .OR $0800
1010          .TF BOOT
1020          ; (code written to file BOOT)

2000          .TF MAIN
2010          ; (code written to file MAIN)
```

---

## Memory Protection

The assembler protects several memory ranges during assembly:

### Protected Ranges (Default)

- `$001F-$02FF` - System area
- `$03D0-$07FF` - DOS buffers
- Symbol tables
- Assembler code itself
- DOS

### Override Protection

Use these addresses to un-protect a specific range:

| Address ($1000 based) | Address ($0000 based) | Description |
|-----------------------|-----------------------|-------------|
| `$1024-$1025` | `$D024-$D025` | Low address of un-protected range (lo,hi) |
| `$1026-$1027` | `$D026-$D027` | High address of un-protected range (lo,hi) |

Example (assembly language):
```assembly
1000          LDA #$00           ; Low byte of start
1010          STA $D024
1020          LDA #$08           ; High byte of start ($0800)
1030          STA $D025
1040          LDA #$FF           ; Low byte of end
1050          STA $D026
1060          LDA #$0F           ; High byte of end ($0FFF)
1070          STA $D027
```

This un-protects `$0800-$0FFF` for assembly.

---

## User Vectors

Several vectors allow custom extensions:

| Address ($1000) | Address ($0000) | Description |
|-----------------|-----------------|-------------|
| `$1000` | `$0000` | Hard entry: `JMP HARD.INIT` |
| `$1003` | `$0003` | Soft entry: `JMP SOFT` |
| `$1006` | `$0006` | USR vector: `JMP SOFT` |
| `$1009` | `$0009` | PRT vector: `JMP SOFT` |
| `$100C` | `$D00C` | Esc-U vector: `JMP ROL.ERR` |
| `$100F` | `$D00F` | "." command vector: `JMP SOFT` |
| `$1012` | `$0012` | Object byte vector: `JMP STORE.OBJECT.BYTE` |
| `$1015` | `$0015` | .US vector: `JMP COMMENT` |

### PRT Command

User-defined command for printer or custom output. Vectors to `$1009` (`$0009`). Default: does nothing.

### USR Command

User-defined command for custom extensions. Vectors to `$1006` (`$0006`). Default: does nothing.

### Esc-U Command

Custom escape command during line input. When `Esc-U` typed (or `OpenApple-U` on //e), vectors to `$100C` (`$D00C`). On entry: input buffer at `$200`, character count in X-register. Exit with `RTS`.

### "." Command

Custom single-character command. When `.` typed as first character of command line, vectors to `$100F` (`$D00F`) on RETURN. Allows custom command interpreter.

### Object Code Emission Vector

Intercept each byte of object code. Vector at `$1012` (`$0012`). Called for each byte during pass 2. Allows custom handling (serial output, encryption, etc.).

---

## Editor Commands

### EDIT Command

Edit an existing line:

```
:EDIT linenumber
```

Shorthand: `Ctrl-E`

#### Edit Mode Keys

| Key | Function |
|-----|----------|
| `Space` | Accept character, move to next |
| `Delete` / `Left Arrow` | Backspace |
| `Ctrl-A` | Insert mode (type new characters) |
| `Ctrl-C` | Change character, advance |
| `Ctrl-D` | Delete character |
| `Ctrl-K` | Kill to end of line |
| `Ctrl-Q` | Quote next character (literal) |
| `Ctrl-T` | Skip to next tab stop |
| `Tab` / `Ctrl-I` | Clear to next tab stop |
| `Ctrl-X` | Abort edit |
| `Return` | Accept line |

---

### COPY Command

Copy lines to new location:

```
:COPY start,end,target
```

- Copies lines `start` through `end` before line `target`
- Copied lines assigned the `target` line number
- After copy, prompts: `DELETE ORIGINAL?`
  - `Y` = move operation (deletes originals)
  - Any other key = copy operation (keeps originals)
- Use `RENUMBER` after copying

---

### RENUMBER Command

Renumber source lines:

```
:RENUMBER [start[,increment]]
```

Shorthand: `:REN`

- Default start: 1000
- Default increment: 10
- Renumbers all lines in memory

---

### Shorthand Commands

#### Ctrl-C - Quick CATALOG

Type `Ctrl-C` in column 1 to automatically expand to `CATALOG`.

#### Esc-L - Auto-LOAD

Type `Esc-L` (or `OpenApple-L` on older //e) in column 1, then filename, to automatically load file.

#### Esc-S - Auto-SAVE

Type `Esc-S` (or `OpenApple-S` on older //e) in column 1. Requires special comment in first 10 lines with format `*HHHHHHSAVE filename` (where H is Ctrl-H).

---

## Examples

### Example 1: Simple Program

```assembly
1000 *---------------------------------
1010 * SIMPLE EXAMPLE
1020 *---------------------------------
1030          .OR $0800
1040
1050 START    LDA #$00         ; Clear accumulator
1060          STA COUNT        ; Initialize counter
1070
1080 LOOP     INC COUNT        ; Increment counter
1090          LDA COUNT
1100          CMP #10          ; Check for limit
1110          BNE LOOP         ; Continue if not done
1120
1130          RTS              ; Return
1140
1150 COUNT    .BS 1            ; Counter variable
```

---

### Example 2: Using Local Labels

```assembly
1000          .OR $0800
1010
1020 SEARCH   LDX #0           ; Initialize index
1030 .1       LDA TABLE,X      ; Get table entry
1040          BEQ .2           ; Found zero, exit
1050          CMP TARGET       ; Compare with target
1060          BEQ .3           ; Found match
1070          INX              ; Next entry
1080          BNE .1           ; Loop if not overflow
1090 .2       LDA #$FF         ; Not found
1100          RTS
1110 .3       TXA              ; Found at X
1120          RTS
1130
1140 TABLE    .DA 1,5,9,15,22,0
1150 TARGET   .BS 1
```

---

### Example 3: Macro with Parameters

```assembly
1000 *---------------------------------
1010 * DEFINE MOVE MACRO
1020 *---------------------------------
1030          .MA MOVE
1040          LDA \0
1050          STA \1
1060          .EM
1070
1080 *---------------------------------
1090 * USE MACRO
1100 *---------------------------------
1110          .OR $0800
1120
1130 START    MOVE #$00,COUNT   ; Expands to LDA #$00 : STA COUNT
1140          MOVE SOURCE,DEST  ; Expands to LDA SOURCE : STA DEST
1150          RTS
1160
1170 COUNT    .BS 1
1180 SOURCE   .BS 1
1190 DEST     .BS 1
```

---

### Example 4: Conditional Assembly

```assembly
1000 *---------------------------------
1010 * CONDITIONAL ASSEMBLY DEMO
1020 *---------------------------------
1030 DEBUG    .EQ 1            ; Set to 0 to disable debug
1040 VERSION  .EQ 2            ; Version 1 or 2
1050
1060          .OR $0800
1070
1080 START    LDA #$00
1090          STA COUNT
1100
1110          .DO DEBUG
1120          JSR TRACE        ; Only if DEBUG=1
1130          .FIN
1140
1150          .DO VERSION-2
1160          JSR NEWCODE      ; Only if VERSION=2
1170          .ELSE
1180          JSR OLDCODE      ; Only if VERSION<>2
1190          .FIN
1200
1210          RTS
1220
1230 COUNT    .BS 1
```

---

### Example 5: 65816 16-bit Code

```assembly
1000 *---------------------------------
1010 * 65816 16-BIT EXAMPLE
1020 *---------------------------------
1030          .OP 65816
1040
1050          .OR $8000
1060
1070 START    CLC              ; Clear carry
1080          XCE              ; Switch to native mode
1090
1100          REP #$30         ; 16-bit A,X,Y
1110
1120          LDA ##$1234      ; 16-bit immediate
1130          STA $2000        ; Store word
1140
1150          LDX ##$4000      ; 16-bit index
1160          LDA $010000,X    ; Long addressing (24-bit)
1170
1180          JSL $020000      ; Long subroutine call
1190
1200          SEP #$30         ; Back to 8-bit
1210
1220          SEC              ; Set carry
1230          XCE              ; Back to emulation mode
1240
1250          RTS
```

---

## Quick Reference

### Common Operations

```assembly
; Define constants
MAXVAL  .EQ 255
BUFFER  .EQ $2000

; Define data
COUNT   .BS 1              ; 1 byte
TABLE   .DA $1000          ; 2 bytes (10 00)
STRING  .AS /HELLO/        ; 5 bytes
BYTES   .HS 01,02,03,04    ; 4 bytes

; Set origin
        .OR $0800

; Conditional assembly
        .DO DEBUG
        JSR TRACE
        .FIN

; Macros
        .MA MOVE
        LDA \0
        STA \1
        .EM

; Use macro
        MOVE #$00,COUNT

; Select CPU
        .OP 65816
        LDA ##$1234        ; 16-bit immediate

; Force addressing
        LDA <$1234         ; Force zero page
        LDA >$05           ; Force absolute
        LDA >>$1234        ; Force long (65816)

; Local labels
LOOP    LDA TABLE,X
        BEQ .1
        INX
        BNE LOOP
.1      RTS
```

---

## Compatibility Notes

### Key Differences from Merlin

| Feature | SCMASM | Merlin |
|---------|---------|---------|
| **Directives** | `.OR`, `.DA`, `.MA` (dot prefix) | `ORG`, `DA`, `MAC` (no dot) |
| **Local Labels** | `.N` (numeric, forward-only) | `:LABEL` (alphanumeric, bidirectional) |
| **Variable Labels** | `.SE` (re-definable) | `]LABEL` (mutable, backward-ref) |
| **Macros** | `.MA`...`.EM` | `MAC`...`<<<` |
| **Line Numbers** | Supported | Not supported |
| **Quote Semantics** | `"` high-bit SET, `'` clear | `"` high-bit SET, `'` clear (SAME!) |

---

## Conclusion

The S-C Macro Assembler Version 2.0 provides a comprehensive, professional development environment for the 6502 family of processors. With support for multiple CPU variants, powerful macro capabilities, conditional assembly, and extensive customization options, it remains a capable tool for Apple II assembly language development.

This reference documents all syntax conventions and features as described in the official Version 2.0 manual from S-C Software Corporation (1984).
