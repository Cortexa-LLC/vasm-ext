# Merlin 8/16 Syntax Conventions

## Overview

Merlin 8/16 is a comprehensive macro assembler system for the Apple IIgs or 128K IIe/IIc, supporting multiple microprocessors:
- **Merlin 8**: 6502, 65C02, and 65802 (128K IIe/IIc)
- **Merlin 16**: 6502, 65C02, 65802, and 65816 (Apple IIgs)

This document describes the complete syntax conventions for Merlin 8/16 assembly language source code, extracted from the Merlin 8/16 User's Manual. It serves as a comprehensive reference guide for all features documented in the manual.

---

## Source Line Format

A source line has four fields separated by spaces:

```
LABEL OPCODE OPERAND ;COMMENT
```

### Field Structure

1. **Label Field** (optional)
   - Starts in column 1 (no leading space)
   - Maximum 26 characters (Merlin 16) or 13 characters (Merlin 8)
   - Must begin with character >= ':' (ASCII $3A)
   - Cannot contain characters < '0' (ASCII $30)
   - Periods (.) not allowed (reserved for logical OR operator)

2. **Opcode Field** (required)
   - Preceded by one space if no label, or space after label
   - Standard 6502/65C02/65816 opcodes OR pseudo-opcodes OR macro names
   - Only first 3 characters examined (except MAC, DEND, and some others)

3. **Operand Field** (optional)
   - Preceded by one space after opcode
   - Expression, address, immediate data, or directive parameter
   - Maximum combined operand+comment length: 64 characters

4. **Comment Field** (optional)
   - Preceded by semicolon (;) after operand, or space after operand then semicolon
   - Rest of line is comment text
   - Comment-only lines begin with asterisk (*) or semicolon (;)

### Field Spacing Rules

- **One space** separates fields
- Editor automatically tabs to next field when space is pressed
- Fields are tabbed positions (configurable)
- Number of spaces between fields not important for assembler (only for listing)

### Example Lines

```asm
* DEMO PROGRAM 1
         ORG   $8000
BELL     EQU   $FBDD
START    JSR   BELL        ; RING THE BELL
DONE     RTS
```

---

## Comments

### Comment Styles

1. **Full-line comment with asterisk**
   ```asm
   * THIS IS A COMMENT LINE
   ```

2. **Full-line comment with semicolon** (tabbed to comment field)
   ```asm
   ; THIS IS ALSO A COMMENT
   ```

3. **End-of-line comment**
   ```asm
   START    LDA   #$00      ; INITIALIZE ACCUMULATOR
   ```

4. **Blank lines** are allowed and treated as `SKP 1`

---

## Labels

Merlin supports a three-tier label system: Global labels, Local labels, and Variables.

### Global Labels

- Normal labels not beginning with `:` or `]`
- Visible throughout entire program
- Must be unique across the entire source
- Examples: `START`, `LOOP`, `DONE`
- Case-sensitive: `START`, `Start`, `start` are different labels

```asm
START    LDA   #0
         JSR   SUBROUTINE
DONE     RTS
```

### Local Labels (begin with colon `:`)

- Begin with colon character (`:`)
- Attached to the last global label
- Scope extends from previous global label to next global label
- Same local label name can be reused under different global labels
- Maximum 26 characters (Merlin 16) or 13 characters (Merlin 8)
- **Do not save space in symbol table** (unlike variables)

```asm
START    LDY   #0
         LDX   #0
:LOOP    LDA   (JUNK),Y    ; :LOOP is local to START
         STA   (JUNKDEST),Y
         INY
         CPY   #100
         BNE   :LOOP       ; Branch back to :LOOP in line 3

LOOP2    LDY   #0
:LOOP    LDA   (STUFF),Y   ; :LOOP now local to LOOP2
         STA   (STUFFDEST),Y
         INY
         CPY   #100
         BNE   :LOOP       ; Branch to :LOOP in line 9
         RTS
```

**Restrictions:**
- Cannot be used inside macros
- Cannot label MAC, ENT, or EXT pseudo-ops
- Cannot be EQUated
- First label in program cannot be local
- Can be used for forward and backward branching

### Variables (begin with right bracket `]`)

- Begin with right bracket character (`]`)
- Can be redefined as often as needed
- **Save space in symbol table** (unlike global/local labels)
- Primarily designed for use in macros (]1 through ]8 are macro parameters)
- Forward reference impossible - must be defined before use
- Can be used for backwards branching with repeated label names
- Good programming practice: use for passing data, not branch points

```asm
]A       =     0            ; Initialize variable ]A
         LUP   $FF          ; Loop 255 times
]A       =     ]A+1         ; Increment ]A
         DFB   ]A           ; Define byte with value of ]A
         --^                ; End loop
```

**Macro Parameter Variables:**
- `]0`: Number of parameters (Merlin 16 only)
- `]1` through `]8`: Macro parameters

```asm
MOVE     MAC               ; Define MOVE macro
         LDA   ]1           ; Load from first parameter
         STA   ]2           ; Store to second parameter
         <<<                ; End macro definition

         MOVE  $06;$07      ; Use macro: moves $06 to $07
```

**Variables as backward branch targets:**

```asm
         LDY   #0
]JLOOP   LDA   TABLE,Y      ; First loop
         BEQ   NOGOOD
         JSR   DOIT
         INY
         BNE   ]JLOOP       ; Branch to first ]JLOOP (line 2)

NOGOOD   LDX   #-1
]JLOOP   INX                ; Redefine ]JLOOP - second loop
         STA   DATA,X
         LDA   TBL2,X
         BNE   ]JLOOP       ; Branch to second ]JLOOP (line 8)
```

---

## Numeric Constants

### Decimal Numbers

Default number format - no prefix required:

```asm
         LDA   #100         ; Decimal 100
         DFB   10,20,30     ; Three decimal bytes
         DA    1,10,100,1000,10000
```

### Hexadecimal Numbers

Prefix with dollar sign (`$`):

```asm
         LDA   #$FF         ; Hex FF (255 decimal)
         ORG   $8000        ; Origin at hex 8000
START    EQU   $1000        ; Equate to hex 1000
         DFB   $0A,$14,$1E  ; Hex bytes
```

Leading zeros optional: `$64` same as `$0064`

### Binary Numbers

Prefix with percent sign (`%`):

```asm
         LDA   #%11111111   ; Binary 11111111 (255 decimal)
         ORA   #%10000000   ; Set high bit
         AND   #%01111111   ; Clear high bit
         DFB   %1100100     ; Binary byte
```

Leading zeros optional: `%1100100` same as `%01100100`

### Equivalence Examples

These four numbers are equivalent:
- Decimal: `100`
- Hex: `$64`
- Binary: `%1100100`
- Binary: `%01100100`

### ASCII Character Constants

Single or double quotes create ASCII values:

```asm
         LDA   #"A"         ; ASCII 'A' with high bit SET ($C1)
         LDA   #'A'         ; ASCII 'A' with high bit CLEAR ($41)
         CMP   #"0"         ; Compare with ASCII '0' (high bit set)
         DFB   "K"-"A"+1    ; Arithmetic with ASCII values
```

**Quote behavior:**
- Double quote (`"`): High bit SET (negative ASCII, $80-$FF)
- Single quote (`'`): High bit CLEAR (positive ASCII, $00-$7F)
- Quote can enclose single character: `'A'` or `"A"`
- Or precede character: `'A` or `"A`

---

## String Directives

### General Notes on String Delimiters

**Delimiter effects on high bit:**
- Delimiter with ASCII < `'` (apostrophe, $27): high bit SET
- Delimiter with ASCII >= `'`: high bit CLEAR
- Common delimiters for negative ASCII: `" ! # $ % &`
- Common delimiters for positive ASCII: `' ( ) + ?`

**Examples:**

```asm
         ASC   "HELLO"      ; Negative ASCII (hi bit set)
         ASC   !HELLO!      ; Negative ASCII
         ASC   #HELLO#      ; Negative ASCII
         ASC   &HELLO&      ; Negative ASCII
         ASC   'HELLO'      ; Positive ASCII (hi bit clear)
         ASC   (HELLO(      ; Positive ASCII
```

**Embedded quotes:**

```asm
         ASC   !ENTER "HELLO"!     ; String with embedded quotes (negative)
         ASC   'ENTER "HELLO"'     ; String with embedded quotes (positive)
```

### ASC - ASCII String

Stores ASCII string with high bit controlled by delimiter:

```asm
         ASC   "HELLO"      ; Negative ASCII: C8 C5 CC CC CF
         ASC   'HELLO'      ; Positive ASCII: 48 45 4C 4C 4F
         ASC   "Bye,Bye",8D ; String plus hex bytes
         ASC   'Hi',00      ; Positive ASCII with terminator
```

**Trailing hex bytes:**
- Hex data can follow string (no spaces allowed)
- Formats: `"string"878D00` or `"string",87,8D,00`

### DCI - Dextral Character Inverted

Same as ASC but **last character** has inverted high bit:

```asm
         DCI   "STRING"     ; All negative except last "G"
         DCI   'STRING'     ; All positive except last "G"
         DCI   'Hello',878D ; Only 'o' has inverted bit (not trailing bytes)
```

**Note:** When hex suffix added, bit inversion still on last character of string, not trailing bytes.

### INV - Inverse Text

Stores string in inverse video format:

```asm
         INV   "STOP!"      ; Inverse characters
         INV   'END',8D     ; Positive inverse with CR
```

### FLS - Flashing Text

Stores string in flashing format:

```asm
         FLS   "The End"    ; Flashing characters
         FLS   'Flash',00   ; Flashing with null terminator
         FLS   'The End',8D00  ; Flashing with trailing bytes
```

### REV - Reverse String

Stores string **backwards** in memory:

```asm
         REV   "DISK VOLUME"  ; Result: EMULOV KSID
         REV   'Insert'       ; Result: tresnI
```

**Note:** REV does **not** accept trailing hex bytes (unlike other string directives).

### STR - String with Length Byte

Stores string with **leading length byte** (Pascal/ProDOS style):

```asm
         STR   "/PATH/"     ; Result: 06 2F 50 41 54 48 2F
         STR   "HI"         ; Result: 02 C8 C9
         STR   'HI'         ; Result: 02 48 49
         STR   'HI',8D      ; Result: 02 48 49 8D (length doesn't count trailing bytes)
```

**Purpose:** Primarily for ProDOS pathname/string handling.

**Note:** Trailing hex bytes, if any, are **not counted in length byte**. Appending hex bytes not recommended for STR.

### STRL - String with Length Word (Merlin 16)

Similar to STR but with **2-byte length** prefix:

```asm
         STRL  "LONGSTRING"  ; Two-byte length prefix (low, high)
```

### HEX - Raw Hexadecimal Data

Define raw hex bytes (**no $ prefix** needed):

```asm
         HEX   0102030F     ; Four bytes: 01 02 03 0F
         HEX   FD,ED,C0     ; Commas optional: FD ED C0
         HEX   AABBCCDD     ; Multiple bytes
         HEX   CACFCSD9     ; Example from manual
```

**Rules:**
- **No `$` prefix** (will cause error)
- Must use **two hex digits** per byte (use `0F` not `F`)
- Commas optional between bytes
- Maximum 64 characters per line
- Odd number of digits generates error
- Ending in comma generates error

---

## Data Definition Directives

### DFB / DB - Define Byte

Define single bytes (8-bit values):

```asm
         DFB   10           ; Decimal 10: 0A
         DFB   $10          ; Hex 10: 10
         DB    >$FDED+2     ; High byte: FD
         DFB   LAB          ; Low byte of label
         DFB   $34,100,LAB-LAB2,%011,>LAB1-LAB2
```

**Operators:**
- `#` prefix: optional, ignored for DFB
- `<` prefix: low byte (default if omitted)
- `>` prefix: high byte of expression
- Multiple values separated by **commas** (no spaces allowed)
- Accepts arithmetic expressions

**Example with all features:**

```asm
         DFB   $34,100,LAB-LAB2,%011,>LAB1-LAB2
```

Produces: `34 64 DE 0B 09` (assuming LAB1=$81A2, LAB2=$77C4)

### DA / DW - Define Address/Word

Define 16-bit values (**low byte first**):

```asm
         DA    $FDF0        ; Result: F0 FD
         DA    10,$300      ; Result: 0A 00 00 03
         DW    LAB1,LAB2    ; Two addresses
```

**DA and DW are synonymous.**

Accepts multiple data separated by commas: `DA 1,10,100`

### DDB - Define Double-Byte

Define 16-bit values (**high byte first**):

```asm
         DDB   $FDED+1      ; Result: FD EE
         DDB   10,$300      ; Result: 00 0A 03 00
         DDB   1,10,100     ; Multiple values
```

Accepts multiple data: `DDB 1,10,100`

### ADR - Define Long Address (Merlin 16 - 3 bytes)

Define 24-bit addresses (low, high, bank):

```asm
         ADR   $01FDF0      ; Result: F0 FD 01
         ADR   10,$020300   ; Result: 0A 00 00 00 03 02
         ADR   LAB1,LAB2    ; Multiple 24-bit addresses
```

**Byte order:** Low byte, high byte, bank byte

Accepts multiple data: `ADR 1,10,100`

### ADRL - Define Long Address (Merlin 16 - 4 bytes)

Define 32-bit addresses (low, high, bank, high word high byte):

```asm
         ADRL  $01FDF0      ; Result: F0 FD 01 00
         ADRL  10,$020300   ; Result: 0A 00 00 00 00 03 02 00
```

**Byte order:** Low byte, high byte, bank byte, high word high byte

**Choice between ADR and ADRL:**
- Use ADR for indirect pointers: `JSR [PTR]`
- Use ADRL for direct access: `LDA LABEL` / `LDA LABEL+2`

### DS - Define Storage

Reserve and optionally fill memory space:

```asm
         DS    10           ; Reserve 10 bytes (zeroed)
         DS    10,$80       ; Reserve 10 bytes filled with $80
         DS    \            ; Fill to next page boundary (zeros)
         DS    \$80         ; Fill to next page with $80
         DS    \expression  ; Fill to page with expression value
         DS    -1           ; Back up one byte
```

**Forms:**

1. **DS expression**: Zeroes out space if positive
   ```asm
   DS    10               ; Zero 10 bytes
   ```

2. **DS expr1,expr2**: Fill expr1 bytes with low byte of expr2
   ```asm
   DS    10,$80           ; Fill 10 bytes with $80
   ```
   If expr2 omitted or negative, fills with 0.

3. **DS \** or **DS \expression**: Fill to next page boundary
   ```asm
   DS    \                ; Fill to page with zeros
   DS    \$80             ; Fill to page with $80
   DS    \1               ; Fill to page with $01
   ```

**Negative values:** Back up object pointer
```asm
         DS    -1           ; Back up one byte
```

**Cannot use `ORG *-1` to back up; use `DS -1` instead.**

**REL files:** `DS \` causes linker to load next file at page boundary and fill with specified byte.

---

## Expressions

### Primitive Expressions

Building blocks for expressions:

1. **Labels**: Any defined label
2. **Numbers**: Decimal, hex (`$`), or binary (`%`)
3. **ASCII characters**: Preceded or enclosed by quotes
4. **Program Counter**: Asterisk (`*`) = current address

**ASCII value examples:**

```asm
         LDA   #"A"         ; Value $C1 (high bit set)
         LDA   #'A'         ; Value $41 (high bit clear)
         DFB   'A'          ; Same as DFB $41
```

**Current address example:**

```asm
HERE     =     *            ; Label equals current address
         BNE   *+5          ; Branch 5 bytes forward
```

### Operators

**Arithmetic operators:**
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Integer division

**Logical operators:**
- `&` Logical AND
- `.` Logical OR
- `!` Exclusive OR (XOR)

**Expression part operators:**
- `<` Low byte of expression (default)
- `>` High byte of expression
- `^` Bank byte of expression (65816)

### Expression Examples

```asm
LABEL1-LABEL2              ; Difference between labels
2*LABEL+$231               ; Arithmetic expression
1234+%10111                ; Decimal plus binary
"K"-"A"+1                  ; ASCII arithmetic (result: 11)
"0"!LABEL                  ; XOR with ASCII '0'
LABEL&$7F                  ; Mask off high bit (clear bit 7)
*-2                        ; Two bytes before current address
LABEL.%10000000            ; Set high bit with OR
$1000/2                    ; Divide by 2 (result: $0800)
```

**Number format examples:**

```asm
         LDA   #01          ; Decimal 1
         LDA   #$20         ; Hex 32 (decimal)
         LDA   #"A"         ; ASCII 'A' ($C1)
```

### Operator Precedence

**IMPORTANT: All operations evaluated LEFT TO RIGHT** (no operator precedence):

```asm
2+3*5                      ; Evaluates as (2+3)*5 = 25
                           ; NOT as 2+(3*5) = 17
```

**No parentheses for precedence** - rewrite expression if needed:

```asm
; Want 2+(3*5)?
]TEMP    =     3*5         ; First multiply
         DFB   2+]TEMP     ; Then add
```

### Special Expression Uses

**Comparison simulations** (for DO conditional):

```asm
         DO    LABEL1/LABEL2    ; Result 0 if LABEL1 < LABEL2, non-zero otherwise
         DO    LABEL1/5         ; DO if LABEL1 >= 5
         DO    LABEL1-1/-1      ; DO if LABEL1 <> 0
         DO    5-1/LABEL1       ; DO if LABEL1 < 5
         DO    LABEL1/6         ; DO if LABEL1 > 5
         DO    LABEL1-5-1/-1    ; DO only if LABEL1 = 5
         DO    LABEL1-LAB EL2   ; DO if not equal
```

**Control character generation:**

```asm
         CMP   #$9F&"A"     ; Control-A ($81)
```

Where `$9F AND $C1 = $81` (Control-A)

**Checking for equality in loop:**

```asm
         DO    LAB1-5-1/-1  ; DO only if LAB1 equals 5
```

### Parentheses in Expressions

Parentheses retrieve **value from memory** (indirect addressing), not for precedence:

```asm
         DO    ($300)           ; Use value at memory location $300
         DFB   ($300)           ; Byte at $300
```

**Restricted to certain pseudo-ops** - not for changing precedence.

**Example from manual:**

```asm
         DO    ($300)           ; Assemble if byte at $300 is non-zero
```

### Curly Braces `{}` (Merlin 16)

Force expression evaluation before macro substitution:

```asm
         LDA   {]1+]2}          ; Evaluate ]1+]2 before substitution
```

Used primarily in advanced macro programming.

### Example: Using Expressions for Table Building

```asm
COMPARE  LDX   #EODATA-DATA-1  ; Table size
LOOP     CMP   DATA,X           ; Compare
         BEQ   FOUND            ; Found
         DEX
         BPL   LOOP
         JMP   REJECT           ; Not found
DATA     HEX   CACFCSD9         ; Data table
EODATA   EQU   *                ; End of data
```

Adding/deleting DATA automatically adjusts loop counter.

---

## Addressing Modes

### Immediate Mode

Load a **value** (not an address) - prefix with `#`:

```asm
         LDA   #100             ; Load value 100 (decimal)
         LDA   #$64             ; Load value $64 (hex)
         LDA   #%01100100       ; Load binary value
         LDX   #3               ; Load immediate value 3
```

**CRITICAL:** Forgetting `#` for immediate values is the number-one cause of bugs!

```asm
         LDA   #$25             ; Load VALUE $25
         LDA   $25              ; Load FROM ADDRESS $25
```

**65816 Immediate Modes:**

Size depends on M/X status bits:

- `#expression`: Low byte (8-bit mode) or low word (16-bit mode)
- `#<expression`: Low byte (8-bit) or low word (16-bit)
- `#>expression`: High byte (8-bit) or high word (16-bit)
- `#^expression`: Bank byte (always 8-bit)

**Example:**

```asm
         MX    %00              ; 16-bit mode
         LDA   #$1234           ; Loads 16-bit value $1234
         MX    %11              ; 8-bit mode
         LDA   #$12             ; Loads 8-bit value $12
```

### Absolute and Zero Page

Assembler **automatically selects zero page** when address is $00-$FF:

```asm
         LDA   $10              ; Zero page (2 bytes: A5 10)
         LDA   $1000            ; Absolute (3 bytes: AD 00 10)
```

**Difference:**
- Zero page: 2 bytes, faster execution
- Absolute: 3 bytes, can address anywhere in 64K

### Force Absolute (Non-Zero Page) Addressing

Add any character except 'D' (Merlin 8) or 'L' (Merlin 16) after opcode:

```asm
         LDA   $10              ; Zero page: A5 10
         LDA:  $10              ; Forced absolute: AD 10 00
         LDAX  $10              ; Any suffix forces absolute (except D/L)
```

**Reason for 'D' exception:** DEND opcode
**Reason for 'L' exception:** Long addressing (LDAL, STAL, etc.)

### Indexed Addressing

```asm
         LDA   $1000,X          ; Absolute indexed by X
         LDA   $10,X            ; Zero page indexed by X
         LDA   $1000,Y          ; Absolute indexed by Y
         LDA   $10,Y            ; Zero page indexed by Y (limited opcodes)
```

### Indirect Addressing

```asm
         JMP   ($1000)          ; Indirect jump
         JMP   ($10)            ; Zero page indirect jump (65C02)
         LDA   ($10),Y          ; Indirect indexed
         LDA   ($10,X)          ; Indexed indirect
```

**Note:** Indexed indirect modes only allow zero page expression. Assembler gives error if address > $FF.

### Long Addressing (65816)

For 24-bit addressing on 65816:

```asm
         LDA   expression       ; One byte (low)
         LDA   <expression      ; One byte (low) - explicit
         LDA   >expression      ; Two bytes (low word)
         PEA   ^expression      ; Two bytes (high word)
         LDA   !expression      ; Three bytes (complete 24-bit address)
         LDAL  expression       ; Three bytes (L forces long)
```

**Forcing long addressing:**

1. **Add 'L' as 4th character:**
   ```asm
   LDAL  $1234            ; Long absolute load
   STAL  $1234            ; Long absolute store
   ADCL  $1234            ; Long absolute ADC
   ORAL  $1234            ; Long absolute OR
   ```

2. **Use '!' operator:**
   ```asm
   LDA   !$1234           ; Force long addressing
   ```

**Long opcodes in Merlin 16:**
- ADC, AND, CMP, EOR, LDA, ORA, SBC, STA accept 'L' suffix
- JML and JSL always assembled in long form
- JSR and JMP always assembled in short (64K) form

### Stack Relative (65816)

```asm
         LDA   1,S              ; Stack relative
         LDA   3,S              ; Stack relative
         LDA   (1,S),Y          ; Stack relative indirect indexed
         LDA   (3,S),Y          ; Stack relative indirect indexed
```

### Direct Page Indirect Long (65816)

```asm
         LDA   [$10]            ; DP indirect long
         LDA   [$10],Y          ; DP indirect long indexed
```

### Accumulator Mode

No operand required for operations on accumulator:

```asm
         LSR                    ; Shift accumulator right
         ASL                    ; Shift accumulator left
         ROL                    ; Rotate accumulator left
         ROR                    ; Rotate accumulator right
         INC                    ; Increment accumulator (65C02)
         DEC                    ; Decrement accumulator (65C02)
```

**Note:** Some assemblers require 'A' operand; Merlin does not.

**Incorrect (other assemblers):** `LSR A`
**Correct (Merlin):** `LSR`

### Block Move (65816)

```asm
         MVP   $01,$02          ; Move positive (source, dest banks)
         MVN   $01,$02          ; Move negative (source, dest banks)
```

---

## Directives by Category

### Origin and Sections

#### ORG - Set Origin

Establish address where program runs and BLOAD address:

```asm
         ORG   $8000            ; Start code at $8000
         ORG   START+END        ; Origin at expression value
         ORG                    ; Re-ORG (return to previous address)
```

**Rules:**
- Establishes where program runs
- **First ORG** sets BLOAD address
- Subsequent ORGs create new code segments
- `ORG` with no operand re-establishes previous address pointer
- Cannot use `ORG *-1` to back up; use `DS -1` instead

**Default:** $8000

**Multiple ORG Example:**

```asm
         ORG   $1000            ; First segment
         LDY   #0
         JSR   MOVE             ; MOVE not listed yet
         JMP   CONTINUE         ; CONTINUE not listed yet

         ORG   $300             ; Separate segment
PAGE3    STA   MAINZP           ; This code at $300
         JSR   COUT
         STA   AUXZP
         RTS

         ORG                    ; Return to $1000 series
CONTINUE LDA   #"A"             ; Resumes at $1012
         JSR   PAGE3
```

**REL files:** First ORG not allowed. ORG sets segments to absolute addresses.

#### DUM / DEND - Dummy Section

Define labels **without generating object code**:

```asm
IOBADRS  =     $B7EB
         DUM   IOBADRS          ; Start dummy section at $B7EB
IOBTYPE  DFB   1                ; Labels defined
IOBSLOT  DFB   $60              ; but no code
IOBDRV   DFB   1                ; generated
IOBVOL   DFB   0
IOBTRCK  DFB   0
IOBSECT  DFB   0
         DS    2                ; Pointer to DCT
IOBBUF   DA    0
         DA    0
IOBCMD   DFB   1
IOBERR   DFB   0
ACTVOL   DFB   0
PREVSL   DFB   0
PREVDR   DFB   0
         DEND                   ; End dummy section

START    LDA   #$60             ; Real code resumes
         STA   IOBSLOT          ; Can use dummy labels
```

**Purpose:** Define I/O block, data structure, or memory map labels.

**Behavior:**
- Text output shows addresses as if code generated
- No actual object code produced
- DEND re-establishes ORG to value before DUM
- Can re-ORG within dummy section with another DUM

#### OBJ - Set Object Code Address

```asm
         OBJ   $4000            ; Set object code memory division
         OBJ   START            ; Use label value
```

**Range:** $4000 to $BFE0
**Purpose:** Set division between symbol table and object code in memory
**Default:** $8000

**Use:** Rarely needed. Mainly for:
- Testing routine in memory then returning to assembler
- DOS 3.3 Merlin 8 only (ProDOS doesn't support this workflow)

**Notes:**
- OBJ out of range prevents object code save
- ProDOS: Ignore OBJ; save source, save object, quit to BASIC
- REL files: OBJ is disregarded
- DSK files: Can set OBJ to $BFE0 to maximize symbol table space

**Merlin 16:** Symbol table range printed at end of assembly (helps determine if new OBJ needed)

---

### Symbols and Constants

#### EQU or = - Equate

Define constant label values:

```asm
START    EQU   $1000            ; Define constant
CHAR     EQU   "A"              ; ASCII value ($C1)
PTR      =     *                ; Current address
LABEL    =     55               ; Decimal value
BELL     EQU   $FBDD            ; ROM routine address
HOME     EQU   $FC58            ; ROM routine
```

**Two forms:**
```asm
LABEL    EQU   expression       ; Standard form
LABEL    =     expression       ; Alternative form
```

**Immediate vs. Address:**

```asm
LABEL    EQU   #$25             ; Immediate value
         LDA   LABEL            ; Loads #$25 (immediate)

LABEL    EQU   $25              ; Address value
         LDA   LABEL            ; Loads from address $25
```

**CRITICAL POINT:** Including `#` in EQU makes huge difference!

**Restrictions:**
- Cannot redefine EQUated labels (use variables `]` for redefinable)
- Cannot EQU a local label (`:`)
- Cannot EQU after label used, if it would create zero page reference
- Labels are case-sensitive: `START`, `Start`, `start` all different

**Forward reference issue:**

```asm
LABEL    LDA   HEN              ; Forward ref to HEN
HEN      DFB   $00
         DFB   $01
LEN      EQU   *-LABEL          ; ILLEGAL - can create bad code
```

**Solution:**

```asm
         LDA   #END-LABEL       ; Immediate mode OK
LABEL    DFB   $00
         DFB   $01
END      EQU   *                ; Now legal
```

**Case sensitivity example:**

```asm
START    EQU   $1000            ; Upper case
Start    EQU   $2000            ; Mixed case - different label!
start    EQU   $3000            ; Lower case - different label!
```

---

### Conditional Assembly

#### DO - Do If True

Assemble block if expression non-zero:

```asm
FLAG     =     1
         DO    FLAG             ; Assemble if FLAG <> 0
         LDA   #"A"             ; This assembled
         ELSE
         LDA   #"B"             ; This not assembled
         FIN
```

**Comparison tests using division and subtraction:**

```asm
         DO    0                ; Assembly OFF
         DO    1                ; Assembly ON
         DO    LABEL            ; If LABEL <> 0 then ON
         DO    LAB1/LAB2        ; If LAB1 >= LAB2 then ON (else OFF)
         DO    LAB1-LAB2        ; If LAB1 <> LAB2 then ON
         DO    LABEL-1          ; If LABEL <> 0 (when LABEL is 0 or 1)
         DO    LABEL-5-1/-1     ; If LABEL = 5 (equality test)
         DO    LABEL1/5         ; If LABEL1 >= 5
         DO    5-1/LABEL1       ; If LABEL1 < 5
         DO    LABEL1/6         ; If LABEL1 > 5
```

**How division comparison works:**
- Result is 0 if numerator < denominator
- Result is non-zero if numerator >= denominator
- `DO 0` means OFF
- `DO` anything else means ON

**Nesting:** Up to 8 levels deep

**Effect on labels:**
- Assembler doesn't recognize labels in DO OFF areas (except macro names)
- Use for ROM/RAM versions, debug/release, feature selection

**Example:**

```asm
DEBUG    =     1                ; Debug flag
         DO    DEBUG
         JSR   PRINTREGS        ; Debug code
         FIN
```

#### IF - If Character Match

Test **first character** of variable (typically macro parameter):

```asm
         IF    (,]1             ; If ]1 starts with "("
         IF    ",]TEMP          ; If ]TEMP starts with quote
         IF    "=]1             ; Alternative syntax (= not examined)
```

**Purpose:** Determine addressing mode or string delimiter in macro.

**Syntax:** First and third characters checked; middle character (usually comma) ignored.

**Merlin 16 MX Flag Testing:**

Test processor M and X status bits:

```asm
         IF    MX/2             ; If M is short (8-bit)
         IF    MX/2-1           ; If M is long (16-bit)
         IF    MX&1             ; If X is short (8-bit)
         IF    MX&1-1           ; If X is long (16-bit)
         IF    MX/3             ; If both M and X short
         IF    MX!3/3           ; If both M and X long
         IF    MX-2/-1          ; If M long, X short
         IF    MX-3/-1          ; If M short, X long
         IF    MX+1&3           ; If either M or X or both long
         IF    MX               ; If either M or X or both short
```

**MX values:**
- 0: M=16-bit, X=16-bit
- 1: M=16-bit, X=8-bit
- 2: M=8-bit, X=16-bit
- 3: M=8-bit, X=8-bit (Emulation mode)

**Use in macros:** Determine register sizes for proper code generation.

#### ELSE - Invert Condition

Inverts assembly condition of last DO/IF:

```asm
         DO    FLAG
         LDA   #"A"             ; If FLAG <> 0
         ELSE
         LDA   #"B"             ; If FLAG = 0
         FIN
```

ON becomes OFF, OFF becomes ON.

#### FIN - Finish Conditional

Terminate DO or IF block:

```asm
         DO    FLAG
         LDA   #$00
         JSR   ROUTINE
         FIN                    ; End conditional
```

Continues with next highest level or turns assembly fully ON if this was outermost conditional.

**Nesting:** Every DO/IF needs matching FIN. Can nest up to 8 deep.

---

### Macros

#### MAC - Begin Macro Definition

Define reusable code sequence with parameters:

```asm
MOVE     MAC                    ; Define macro named MOVE
         LDA   ]1               ; Load from parameter ]1
         STA   ]2               ; Store to parameter ]2
         <<<                    ; End macro
```

**Label required:** Must label MAC with macro name.

**Parameters:** `]1` through `]8` are macro parameters.

**Merlin 16:** `]0` contains parameter count.

**Restrictions:**
- Macro name cannot be standard opcode or pseudo-opcode
- Cannot be same as DEND or POPD
- Cannot be referenced by DA, DFB, etc.
- PMC/>>> forms avoid these restrictions

#### EOM or <<< - End of Macro

Terminate macro definition:

```asm
SWAP     MAC
         LDA   ]1
         LDX   ]2
         STA   ]2
         STX   ]1
         EOM                    ; End of macro
```

Alternative form:

```asm
SWAP     MAC
         LDA   ]1
         LDX   ]2
         STA   ]2
         STX   ]1
         <<<                    ; Alternate end marker (three less-than)
```

**Can be labeled** for branching to end of macro:

```asm
ERROR    MAC
         LDA   #$07             ; Bell
         LDY   #$04
ERRLOOP  DEY
         BEQ   FINISH           ; Branch to EOM label
         JSR   COUT
         JMP   ERRLOOP
FINISH   <<<                    ; Labeled EOM
```

#### PMC or >>> - Put Macro Call

Call/expand a macro:

```asm
         MOVE  $06;$07          ; Direct call
         PMC   MOVE,$06;$07     ; PMC form
         >>>   MOVE,$06;$07     ; >>> form (three greater-than)
```

**Three calling forms:**

1. **Direct:** `MACRONAME param1;param2;...`
2. **PMC:** `PMC MACRONAME,param1;param2;...`
3. **>>>:** `>>> MACRONAME,param1;param2;...`

**Parameter separators:**
- Semicolon (`;`) between parameters
- Optional comma or other separator after macro name in PMC/>>> forms
- Allowed separators: `. , - ( [space]`
- No extra spaces allowed in parameter list

**Example with strings:**

```asm
PRINT    MAC                    ; Define PRINT macro
         JSR   SENDMSG
         ASC   ]1               ; String parameter
         BRK
         <<<

         PRINT "Hello World"    ; Must use quotes for strings with spaces
         PRINT 'Message'        ; Single quotes OK
         PRINT !"quote"!        ; Embedded quotes need different delimiter
```

**Parameter rules:**
- Strings with spaces or semicolons must be quoted
- Delimiter required at end: `PRINT "A"` not `PRINT "A`
- Number of parameters must match (fewer = BAD VARIABLE error)
- Extra parameters ignored (no error)

**Macro not found:** If macro undefined, first 3 letters treated as opcode (can cause confusion).

---

### Loops

#### LUP - Loop

Repeat code sequence:

```asm
         LUP   4                ; Loop 4 times
         ASL                    ; Repeated instruction
         --^                    ; End loop marker
```

**Maximum value:** $8000 (larger values ignored)

**Table building with variables:**

```asm
]A       =     0
         LUP   $FF              ; Loop 255 times
]A       =     ]A+1             ; Increment variable
         DFB   ]A               ; Define byte: 1,2,3,...$FF
         --^                    ; End loop
```

**@ label in loops** - creates sequential labels (max 26 iterations):

```asm
         LUP   26               ; Loop 26 times
@        ASL                    ; Label @ becomes A,B,C,...Z
         BNE   @                ; Branch to current label
         --^                    ; Counts backward: last = A
```

**Note:** Loop counter counts DOWN, so labels go in reverse (last iteration = A).

**Line numbers:** Repeated lines show same line number in listing.

**Variables in LUP:** Cannot use variable increment inside macro; must be in main source.

#### --^ - End Loop

Terminate LUP block - two hyphens followed by caret.

**Note:** This is the actual character sequence, not a symbol.

---

### CPU Control

#### XC - Extended Opcodes

Enable 65C02 or 65802/65816 opcodes:

**Enable 65C02:**
```asm
         XC                     ; Enable 65C02 opcodes
```

**Enable 65802/65816:**
```asm
         XC                     ; First XC
         XC                     ; Second XC enables 65802/65816
```

**Merlin 16:** 65816 opcodes enabled by default (PARMS configurable).

**Merlin 8:** Must use XC to enable. Safety feature prevents accidental 65C02 use on 6502.

**Rules:**
- XC must be first line (or first two lines) before any 65C02/65816 opcodes
- Serves as flag indicating extended opcodes intentional
- Prevents hard-to-debug errors from using wrong opcodes on 6502

**Rockwell 65C02 opcodes:** Not enabled by XC. Use ROCKWELL macro library instead.

**Apple IIe unenhanced ROMs:** Need `BRUN MON.65C02` before Sourceror for 65C02 code.

#### MX - Set M/X Register Modes (65802/65816)

Inform assembler of processor mode:

```asm
         MX    %00              ; M=16-bit, X=16-bit
         MX    %01              ; M=16-bit, X=8-bit
         MX    %10              ; M=8-bit, X=16-bit
         MX    %11              ; M=8-bit, X=8-bit (Emulation)
         MX    3                ; Same as %11 (decimal)
```

**Binary breakdown:**
- Bit 1 (from right): M flag (0=16-bit, 1=8-bit)
- Bit 0 (from right): X flag (0=16-bit, 1=8-bit)

**Purpose:** Tell assembler register sizes for correct immediate mode assembly.

**Examples:**

```asm
         MX    %00              ; M=16-bit
         LDA   #$1234           ; Assembles as 16-bit immediate

         MX    %11              ; M=8-bit
         LDA   #$12             ; Assembles as 8-bit immediate
```

**Merlin 16 startup default:** `MX %11` (Emulation mode) - configurable in PARMS.

**Merlin 8:** Requires two XC opcodes first.

**CRITICAL:** Must use MX to match actual processor state or immediate modes assemble incorrectly!

#### LONGA ON/OFF (Merlin 16)

Set accumulator size (alternative to MX):

```asm
         LONGA ON               ; Accumulator 16-bit (M=0)
         LONGA OFF              ; Accumulator 8-bit (M=1)
```

Affects only M bit; X bit unchanged.

#### LONGI ON/OFF (Merlin 16)

Set index register size (alternative to MX):

```asm
         LONGI ON               ; Index registers 16-bit (X=0)
         LONGI OFF              ; Index registers 8-bit (X=1)
```

Affects only X bit; M bit unchanged.

#### SW - Enable Sweet 16 (Merlin 8 only)

Enable Sweet 16 pseudo-opcodes:

```asm
         SW                     ; Enable Sweet 16 opcodes
```

**Sweet 16:** 16-bit pseudo-machine implemented in software.

**Register names:** R0-R15 (R prefix optional).

**SET opcode:** Space or comma between register and data:
```asm
         SET   R3,LABEL         ; Both forms valid
         SET   R3 LABEL
```

**NUL opcode:** Assembled as one-byte opcode (HEX 0D), not two-byte skip.

**Purpose:** If not using Sweet 16, can use opcodes (ADD, SUB, etc.) as macro names.

---

### File Operations

#### PUT - Include Source File

Insert source file during assembly:

```asm
         PUT   SOURCEFILE       ; DOS 3.3: looks for T.SOURCEFILE
         PUT   !SOURCE          ; DOS 3.3: SOURCE (no T. prefix, name starts with !)
         PUT   !SOURCE,D2       ; DOS 3.3: drive 2, slot parameter
         PUT   /PRE/SOURCE      ; ProDOS: pathname /PRE/SOURCE.S
```

**Purpose:**
- Break large source into manageable files
- Include common subroutine libraries
- Assemble source too large for memory

**DOS 3.3 filename conventions:**
- Automatic T. prefix unless name starts with character < '@' (e.g., `!`)
- Examples:
  - Disk catalog: `T.SOURCE CODE`
  - PUT command: `PUT SOURCE CODE`
  - Disk catalog: `SOURCE CODE` (no prefix)
  - PUT command: `PUT !SOURCE CODE`

**ProDOS:** All source files are text files (.S suffix automatic).

**Restrictions:**
- Cannot define macros in PUT file (must be in main source or USE file)
- Cannot PUT another PUT file (no nesting)
- All PUT opcodes must be in master source file
- Local variables ]1-]8 can be used in PUT files (set with VAR before PUT)

**Master source file structure:**

```asm
******************
* Master Source *
******************
* LABEL DEFINITIONS
LABEL1   EQU   $00
LABEL2   EQU   $02
COUT     EQU   $FDED

* MACRO DEFINITIONS
SWAP     MAC
         LDA   ]1
         STA   ]2
         <<<

* SAMPLE SOURCE CODE
         LDA   #LABEL1
         STA   LABEL2
         JSR   COUT
         RTS

* BEGIN PUT FILES
         PUT   FILE1           ; First source segment
         PUT   FILE2           ; Second source segment
         PUT   FILE3           ; Third source segment
```

**PUT for subroutines:** Create library of common routines (e.g., SENDMSG, PRDEC).

**PUT for equates:** Include ProDOS global page equates, etc.

**Speed improvement:** Put referenced files on RAM disk for faster assembly.

#### USE - Include Macro Library

Load macro library into memory:

```asm
         USE   MACROS           ; Load macro library
         USE   !MACROS          ; No T. prefix (DOS 3.3)
         USE   MACROS,S5,D1     ; Slot/drive (DOS 3.3)
         USE   /LIB/MACROS      ; ProDOS pathname
```

**Difference from PUT:**
- PUT: File inserted at assembly point, discarded after
- USE: File kept in memory for duration

**Purpose:** Load macro libraries that are used throughout source.

**Example - common equate file:**

```asm
**********************
* COMMON EQUATE FILE*
**********************
HOME     EQU   $FC58            ; Monitor clear screen
VTAB     EQU   $FC22            ; Monitor vertical tab
CH       EQU   $24              ; Horizontal cursor position
```

**Main program:**

```asm
**********************
* SAMPLE PROGRAM    *
**********************
PTR      EQU   $06              ; Pointer for my program
         USE   EQUATES          ; Load pre-defined equates
BEGIN    JSR   HOME             ; Use HOME label
         ; ...
```

**Suppress listing:** Use LST ON/RTN at start/end of library file.

**Speed improvement:** Put USE files on RAM disk.

#### SAV - Save Object Code

Save object code during assembly:

```asm
         SAV   FILENAME         ; Save current object code
         SAV   /OBJ/PROG        ; ProDOS pathname
```

**Purpose:**
- Save portions with different ORGs in one assembly
- Enable assembly of very large files
- Automatic object save during assembly

**Behavior:**
- First SAV: Saves all object code to that point
- Subsequent SAVs: Save only code since last SAV
- After SAV: Object address reset to last OBJ or $8000

**Files saved with BLOAD address:** Can be BLOADed and run.

**Example - Multiple ORG problem:**

```asm
         ORG   $800             ; First segment
         JSR   ROUTINE          ; 16K program spanning
         ; ... code ...         ; Hi-Res pages
         JMP   $6000            ; Skip Hi-Res

         ORG   $6000            ; Second segment (after Hi-Res)
         ; ... more code ...
```

**Problem:** All code assembled starting at $800; second ORG doesn't move code physically.

**Solution with SAV:**

```asm
         ORG   $800
         ; ... first segment ...
         SAV   FILE1            ; Save first segment

         ORG   $6000
         ; ... second segment ...
         SAV   FILE2            ; Save second segment
```

Now FILE1 loads at $800, FILE2 loads at $6000.

**Alternative:** Use Linker for large modular programs.

#### DSK - Assemble Directly to Disk

Write object code directly to disk during assembly:

```asm
         DSK   PROG             ; Write object to PROG
         DSK   /OBJ/PROG        ; ProDOS pathname
```

**Three purposes:**

1. **Large object files:** Larger than available memory
2. **Automatic save:** Don't have to remember object save
3. **Custom file type:** Use with TYP for non-BIN filetypes

**Required for REL files:** Relocatable code must use DSK.

**Behavior:**
- Opens file at start
- Writes sector every 256 bytes
- If second DSK: closes first file, opens second
- File closed at end of assembly

**Performance:** Slows assembly due to disk writes.

**Speed improvement:** Use RAM disk for DSK output.

**Example - Two files:**

```asm
* DSK SAMPLE
         DSK   FILEONE          ; Assemble to FILEONE
         ORG   $300             ; FILEONE loads at $300
HOME     EQU   $FC58
COUT     EQU   $FDED
         JSR   HOME
         LDX   #0
LOOP1    LDA   STRING1,X
         BEQ   DONE1
         JSR   COUT
         INX
         BNE   LOOP1
DONE1    RTS
STRING1  ASC   "THIS IS ONE"
         HEX   8D00

         DSK   FILETWO          ; Close FILEONE, open FILETWO
         ORG   $8000            ; FILETWO loads at $8000
         LDX   #0
LOOP2    LDA   STRING2,X
         BEQ   DONE2
         JSR   COUT
         INX
         BNE   LOOP2
DONE2    RTS
STRING2  ASC   "NOW IT'S TWO"
         HEX   8D00
```

**Conditional to speed development:**

```asm
DISK     =     1                ; 1=write to disk, 0=memory only
         DO    DISK
         DSK   PROGRAM
         FIN
         ORG   $8000
         ; ... source code ...
```

---

### Linking and Relocatable Code

#### REL - Relocatable Code Module

Generate relocatable code file for linker:

```asm
         REL                    ; Generate relocatable file
```

**MUST be first line before any labels.**

**Creates:** LNK file type (ProDOS filetype $F8)

**Purpose:** Create modules that can be:
- Linked together
- Run at any address (determined at link time)
- Reused without re-assembly

**Restrictions:**
- Cannot use ORG at start (ORG address specified at link time)
- Cannot use SAV or Main Menu save object
- Must use DSK to save object
- No multiplication/division/logical operations on relative labels
- ORG within REL file makes labels in that section absolute

**Relative vs. Absolute expressions:**

**Absolute (allowed in expressions):**
- EQUates to explicit addresses
- Difference between two relative labels
- Labels in DUMMY sections

**Relative (restrictions):**
- Ordinary labels
- Expressions using program counter: `LABEL = *`

**Merlin 8 Linker additional restrictions:**
- No more than one external per operand
- For `#>expression` or `>expression` with external, expression must be within 7 bytes of external's value

**Examples:**

```asm
         LDA   #>EXTERNAL+8     ; Illegal (> 7 bytes)
         DFB   >EXTERNAL-1      ; Legal (within 7 bytes)
```

**Initial reference address:** $8000 (fictional until linked)

**Merlin 16:** No restriction on externals per operand.

#### ENT - Entry Label

Make label visible to other modules:

```asm
PRINT    ENT                    ; Define entry point
         JSR   COUT
         RTS
```

**Merlin 16 multiple syntax:**

```asm
         ENT   LABEL1,LABEL2,LABEL3    ; No label in label column
```

**Purpose:**
- Allow other modules to call this routine
- Make label available to EXT declarations in other modules
- Entry labels can be used normally within their own module

**Symbol table:** Flagged with 'E'

**Example:**

```asm
START    LDA   POINTER          ; Normal use
         INC   POINTER
         BNE   SWAP             ; Branch to ENT label
         JMP   CONTINUE
SWAP     ENT                    ; Entry label for other modules
         LDA   POINTER          ; Can be used normally here
         STA   PTR
         ; ...
```

**Merlin 16 absolute symbols:** Can use ENT with equates:

```asm
HOME     EQU   $FC58
COUT     EQU   $FDED
BELL     EQU   $FBDD
         ENT   HOME,COUT,BELL   ; Make equates available to other modules
```

Avoids having to define in all modules. Uses more linker symbol space.

#### EXT - External Label

Reference label from another module:

```asm
PRINT    EXT                    ; Declare external label
         JSR   PRINT            ; Call external routine
```

**Merlin 16 multiple syntax:**

```asm
         EXT   LABEL1,LABEL2,LABEL3    ; No label in label column
```

**Purpose:**
- Reference routine/data in another REL module
- Label must be ENT in some other module
- Value resolved by linker

**Value during assembly:**
- Set to $8000 + reference number ($00-$FE)
- Final value determined at link time

**Symbol table:** Flagged with 'X'

**Example:**

```asm
* MODULE 1 - Main program
         REL
         DSK   MODULE1.L

* External references
PENUP    EXT                    ; These routines in another module
PENDOWN  EXT
NORTH    EXT
SOUTH    EXT
EAST     EXT
WEST     EXT

* Main code
BOX      LDY   #100
         JSR   PENDOWN          ; Call external
LOOP     JSR   NORTH            ; Call external
         INY
         CPY   #100
         BNE   LOOP             ; Local label OK
         ; ...
```

**Label conflicts:** Local labels in one module don't conflict with labels in other modules.

**Linking:** All EXT labels must match ENT labels in linked modules or linker error occurs.

---

### Output Control

#### TYP - Set ProDOS File Type

Set file type for DSK or SAV output:

```asm
         TYP   $00              ; No file type
         TYP   $06              ; BIN file type (default)
         TYP   $F0              ; User-defined
         TYP   $FF              ; SYS file type
```

**Valid types:**
- Merlin 8: 0, 6, $F0-$F7, $FF
- Merlin 16: No restrictions

**Default:** $06 (BIN)

**Purpose:** Create non-BIN object files (CMD files, SYS files, etc.)

**Example:**

```asm
         TYP   $F0              ; User-defined type
         DSK   MYFILE
```

#### AUX - Set Auxiliary Type (Merlin 16)

Set ProDOS auxiliary type field:

```asm
         AUX   $2000            ; Set aux type
```

**Purpose:** Set load address or other file-specific information in ProDOS catalog.

#### CHK - Checksum

Insert checksum byte in object code:

```asm
         CHK                    ; Insert checksum byte
```

**Purpose:** Verify program integrity at runtime.

**Calculation:** XOR of all bytes in object code.

**Position:** Usually at end of program.

**Runtime verification example:**

```asm
STARTCHK LDA   #<STARTCHK       ; Calculate checksum
         STA   PTR
         LDA   #>STARTCHK
         STA   PTR+1
         LDY   #$00
         LDA   #$00
         PHA                    ; Push zero on stack
LOOP     PLA                    ; Retrieve current checksum
         EOR   (PTR),Y          ; XOR with byte
         PHA                    ; Save temp
         INC   PTR
         BNE   CHK
         INC   PTR+1
CHK      LDA   PTR+1
         CMP   #>PROGEND        ; Done yet?
         BCC   LOOP
         LDA   PTR
         CMP   #<PROGEND
         BCC   LOOP
         BEQ   LOOP
CHKCS    PLA                    ; Retrieve calculated value
         CMP   CHKSUM           ; Compare to stored value
         BNE   ERROR            ; Error handler
         ; Fall through if OK
REALSTART ; ... program starts here ...

PROGEND  RTS                    ; End of program
CHKSUM   CHK                    ; Merlin checksum
```

**Use case:** Detecting errors in typed-in programs from magazines, or verifying program not corrupted.

**Note:** Not foolproof but adequate for most uses.

#### DAT - Date Stamp (ProDOS only)

Print date/time during assembly:

```asm
         DAT                    ; Print current date/time
```

**When:** Second pass of assembly

**Output:** Current ProDOS date and time in listing

**ProDOS only:** Not available in DOS 3.3 Merlin 8.

---

### Listing Control

#### LST ON/OFF/RTN - Listing Control

Control assembly listing output:

```asm
         LST   ON               ; Enable listing
         LST   OFF              ; Disable listing
         LST                    ; Same as LST ON
         LST   RTN              ; Return to previous state (Merlin 16 only)
```

**Purpose:**
- Print only portions of listing
- Speed assembly (OFF is faster)
- Suppress listing of macro libraries

**Symbol table:** If LST OFF at end, symbol table not printed.

**Check:** Assembler examines 3rd character of operand for space. `LST` alone = `LST ON`.

**Does not affect object code generation.**

**Control-D:** During pass 2, toggles LST flag from keyboard.

**LST RTN (Merlin 16):** Returns to state before last LST command.

**Use in macro libraries:**

```asm
         LST   OFF              ; Start of library
         DO    0
MACRO1   MAC
         ; ...
         <<<
         FIN
         LST   RTN              ; Return to previous state
```

This way library not listed, but main file LST state preserved.

#### LSTDO / LSTDO OFF - List DO Regions

Control listing of DO OFF code regions:

```asm
         LSTDO                  ; List DO OFF areas
         LSTDO OFF              ; Don't list DO OFF areas (default)
```

**Purpose:** See code in conditional assembly regions that are turned off.

**Default:** DO OFF areas not listed.

#### EXP ON/OFF/ONLY - Macro Expansion

Control macro expansion in listings:

```asm
         EXP   ON               ; Print entire macro (default)
         EXP   OFF              ; Print only macro call line
         EXP   ONLY             ; Print only generated code
```

**Modes:**

**EXP ON:**
```asm
100      MOVE  $06;$07          ; Macro call line
100      LDA   $06              ; Generated line 1
100      STA   $07              ; Generated line 2
```

**EXP OFF:**
```asm
100      MOVE  $06;$07          ; Only call line printed
```

**EXP ONLY:**
```asm
         LDA   $06              ; Only generated code
         STA   $07              ; (unless call line labeled)
```

**Default:** EXP ON

**No effect on object code.**

#### TR ON/OFF/ADR - Truncate Object Code

Limit object code display in listings:

```asm
         TR    ON               ; Limit to 3 bytes per line
         TR    OFF              ; Show all object bytes (default)
         TR    ADR              ; Suppress bank byte (Merlin 16 only)
```

**TR ON:** Shows only first 3 bytes per source line (even if line generates more).

**TR OFF:** Shows all generated bytes.

**TR ADR (Merlin 16):** Suppresses bank byte in address display at left of listing.

#### OBJ - Object Listing Control

Control object code display in listings:

```asm
         OBJ   ON               ; Show object code (default)
         OBJ   OFF              ; Hide object code
```

**Purpose:** Create clean listings without hex bytes.

#### CYC - Cycle Count

Display instruction cycle counts:

```asm
         CYC                    ; Show cycle counts
         CYC   OFF              ; Stop showing cycles
         CYC   AVE              ; Show average cycles
         CYC   FLAGS            ; Show MX flags (Merlin 16 only)
```

**Output format:** Printed to right of comment field

**Examples:**
```
5 ,0032       ; 5 cycles, 32 total
6 or 5',0326  ; 6 cycles (or 5' with possible +1), 326 total
5'',0326      ; 5 cycles, branch taken crossing page, 326 total
```

**Symbols:**
- `'` (single quote): Possible added cycle (page boundary or branch taken)
- `"` (double quote): Branch taken and crosses page boundary (extra cycle added to total)

**CYC AVE:** Averages uncertain cycles (branches, indexed, indirect).

**CYC FLAGS (Merlin 16):** Shows current MX status:

```
5 ,0326 11    ; 5 cycles, M=8-bit, X=8-bit
```

**Supported opcodes:**
- 6502, 65C02, 65802, 65816
- Not Rockwell-specific (RMB, SMB, BBR, BBS) except via macro library
- Rockwell BBS/BBR: 5 cycles + possible 1-2 for branch

**Zeroes total:** Second CYC zeroes accumulated count.

#### TR ON/OFF/ADR - Truncate Display

(Covered above - duplicate)

---

### Special Formatting

#### AST - Asterisk Line

Print line of asterisks in listing:

```asm
         AST   30               ; Print 30 asterisks
         AST   NUM              ; Print NUM asterisks (variable)
```

**Format:** Decimal (base 10)

**Range:** 0=256 asterisks (treated modulo 256)

**Purpose:** Create visual separators in listings.

#### SKP - Skip Lines

Insert blank lines in listing:

```asm
         SKP   5                ; Skip 5 lines
         SKP   LINES            ; Skip LINES lines (variable)
```

**Format:** Decimal (base 10)

**Range:** 0=256 lines (treated modulo 256)

**Purpose:** Improve listing readability.

**Note:** Blank lines in source treated as `SKP 1`.

#### PAG - New Page

Send formfeed to printer:

```asm
         PAG                    ; Send formfeed ($8C)
```

**Effect:**
- Printer: New page
- Screen: No effect (even with 80-column card)

**Use with TTL:** Often follow PAG with TTL to set new page title.

#### TTL - Title (Merlin 16 only)

Set page title for formatted output:

```asm
         TTL   "Segment Title" ; Set page header
```

**Syntax:** Same as ASC pseudo-op (delimited string)

**Purpose:** Change title during formatted listing (PRTR command).

**Use:** Usually followed by PAG for new page with new title.

**Example:**

```asm
         TTL   "Chapter 1: Introduction"
         PAG                    ; New page with new title
```

---

### Special Directives

#### ERR - Force Error

Generate error if condition true:

```asm
         ERR   *-1/$9600        ; Error if PC >= $9600
         ERR   ($300)-$80       ; Error if byte at $300 <> $80
         ERR   \$5000           ; REL: error if address >= $5000
```

**Purpose:**
- Enforce size constraints
- Verify assumptions during assembly
- Check for USR routine loaded

**Forms:**

**1. Check address limit:**
```asm
         ERR   *-1/$9600        ; Error if PC >= $9600
```

**2. Check memory contents (pass 1):**
```asm
         ERR   ($300)-$4C       ; Error if $300 doesn't contain $4C
```

Use to verify USR routine loaded before assembly.

**3. REL file constraint:**
```asm
         ERR   \$5000           ; Linker error if module >= $5000
```

**Behavior:**
- Standard form: Error on pass 2, doesn't abort
- Memory check form: Error on pass 1, aborts
- REL form: Linker checks constraint

**Error message:** "BREAK IN LINE ???"

**Position in REL files:** Put at end (position doesn't matter for action).

**Example checking label value:**

```asm
LABEL    ; ... code defining LABEL ...
DONE     ERR   LABEL-LABEL2    ; Error if LABEL <> LABEL2
```

#### KBD - Keyboard Input

Get value from keyboard during assembly:

```asm
OUTPUT   KBD                    ; Prompt: "Give value for OUTPUT:"
OUTPUT   KBD   "Printer port?" ; Custom prompt: "Printer port?:"
```

**Purpose:**
- Interactive conditional assembly
- Get configuration values during assembly
- Input port addresses, flags, etc.

**Input:** Any expression (including label references)

**Error:** BAD INPUT if expression can't be evaluated

**Colon appended** to custom prompt automatically.

**Example:**

```asm
N        EQU   0                ; Define N=0
Y        EQU   1                ; Define Y=1

FLAG     KBD   "Debug version? (Y/N)"

         DO    FLAG             ; Assemble if FLAG=Y (1)
         LDA   #"D"
         JSR   COUT
         FIN

         DO    FLAG-1           ; Assemble if FLAG=N (0)
         LDA   #"R"
         JSR   COUT
         FIN
```

#### VAR - Setup Variables

Set variables ]1-]8 before PUT file:

```asm
         VAR   1;$3;LABEL       ; Set ]1=1, ]2=$3, ]3=LABEL
```

**Purpose:** Pass parameters to PUT file that uses variables.

**Syntax:** Semicolon-separated expressions (same as macro parameters)

**Example:**

```asm
         VAR   3;$42;LABEL      ; Set ]1=3, ]2=$42, ]3=LABEL
         PUT   MYFILE           ; PUT file can use ]1, ]2, ]3
```

**Note:** PUT file using ]1-]8 (except in macro calls) must have prior VAR declaration.

#### END - End Assembly

Ignore rest of source:

```asm
         END                    ; Stop assembly here
```

**Purpose:** Rarely used. Terminate assembly early.

**Effect:** Labels after END not recognized.

**Use case:** Testing portions of large source file.

#### USR - User Defined Operation

Call custom routine during assembly:

```asm
         USR   expression       ; Call user routine
         USR0                   ; Merlin 16: USR0 (same as USR)
         USR1  expression       ; Merlin 16: USR1
         USR2  expression       ; Merlin 16: USR2
         ; ... USR3-USR9 ...
```

**Mechanism:** JSR to $B6DA (ampersand vector)

**Setup:** BRUN setup program from Main Menu that installs JMP at $B6DA

**Merlin 16:** Up to 10 USR opcodes (USR0-USR9)
- Number 0-9 doubled and placed in X register
- $B6DA: `JMP (VECTORTBL,X)` where VECTORTBL is address table

**Entry conditions:**
- A=0
- Y=0
- Carry set
- Native 8-bit mode (Merlin 16)

**Available routines for USR:**

```asm
USRADS   $B6DA               ; Must have JMP to routine
PUTBYTE  $E5F6               ; Put byte in object code
EVAL     $E5F9               ; Evaluate expression
PASSNUM  $2                  ; Assembly pass number
ERRCNT   $10                 ; Error count
VALUE    $55                 ; Value returned by EVAL
OPNDLEN  $BB                 ; Operand+comment length
NOTFOUND $FD                 ; Label not found flag
WORKSP   $280                ; Operand and comment buffer
```

**PUTBYTE:**
- JSR with byte in A
- Preserves Y
- Scrambles A, X
- Always returns with Z clear (BNE always branches)
- Must call same number of times each pass!

**EVAL:**
- X = index to first character to evaluate
- Returns: X = character after expression, Y = 0/1/2
- VALUE ($55-$56) = result (low, high)
- NOTFOUND ($FD) = non-zero if label not found
- Pass 2: UNKNOWN LABEL error aborts if label not found

**Safe locations:**
- Code: $300-$3CF, $8A0-$8FF (not $900)
- Zero page: $60-$6F (don't alter others)
- Don't change: $226-$27F, $2C4-$2FF

**Return:** RTS prints USR line on pass 2

**Verification:**
```asm
         ERR   ($310)-$60       ; Check for RTS at $310
LABEL    USR   OPERAND
         ERR   *-LABEL-2        ; Check USR generated 2 bytes
```

**Multiple routines:** Check first operand for routine selector:

```asm
         USR   1,whatever       ; Route to routine 1
         USR   2,stuff          ; Route to routine 2
```

**Example file:** `SOURCE/USR.EXAMPLE.S` (Merlin 16)

#### XC - Extended Opcodes

(Covered above in CPU Control section)

---

## 65816-Specific Features

### Overview

The 65816 extends the 6502 with:
- 24-bit addressing (16MB address space)
- 16-bit accumulator and index registers
- New addressing modes
- Additional instructions
- Native and Emulation modes

### Long Addressing Modes

Access full 24-bit address space:

```asm
         LDAL  $123456          ; Long absolute load
         STAL  $123456          ; Long absolute store
         JMPL  ($1234)          ; Long indirect jump
         JSRL  ROUTINE          ; Long subroutine call
```

**Forcing long mode:**

**1. Add 'L' as 4th character:**
```asm
LDAL     STAL     ADCL     ANDL
CMPL     EORL     ORAL     SBCL
```

**2. Use '!' operator in operand:**
```asm
         LDA   !$1234           ; Force long addressing
```

**Long vs. Short:**
- JML, JSL: Always long (24-bit)
- JMP, JSR: Always short (16-bit)
- LDA, STA, etc.: Use L suffix or ! operator for long

### Block Move Instructions

Copy memory blocks between banks:

```asm
         MVP   $01,$02          ; Move positive (decrementing)
         MVN   $01,$02          ; Move negative (incrementing)
```

**Parameters:**
- First operand: Source bank
- Second operand: Destination bank

**Registers (setup before MVP/MVN):**
- A: Number of bytes to move minus 1
- X: Source address (low 16 bits)
- Y: Destination address (low 16 bits)

**Direction:**
- MVP: Moves from high to low (X and Y decrement)
- MVN: Moves from low to high (X and Y increment)

### Stack Relative Addressing

Access data on stack:

```asm
         LDA   1,S              ; Stack relative
         LDA   3,S              ; Load from stack+3
         STA   5,S              ; Store to stack+5
         LDA   (1,S),Y          ; Stack relative indirect indexed
         LDA   (3,S),Y          ; Complex addressing
```

**Use:** Access local variables and parameters on stack.

### Direct Page Indirect Long

Indirect through direct page with 24-bit address:

```asm
         LDA   [$10]            ; DP indirect long
         LDA   [$10],Y          ; DP indirect long indexed
         STA   [$10]            ; Store via DP indirect long
```

**Pointer:** 24-bit pointer (3 bytes) at direct page address.

### Immediate Mode (16-bit)

When M or X bits indicate 16-bit mode:

```asm
         REP   #$20             ; Clear M bit (16-bit accumulator)
         MX    %10              ; Tell assembler
         LDA   #$1234           ; 16-bit immediate load

         REP   #$10             ; Clear X bit (16-bit indexes)
         MX    %01              ; Tell assembler
         LDX   #$5678           ; 16-bit immediate load
         LDY   #$9ABC           ; 16-bit immediate load
```

**Critical:** Must use MX directive to match actual processor state!

### Bank Byte Operator

Access bank byte (bits 16-23) of address:

```asm
         LDA   #^LABEL          ; Bank byte of LABEL
         PEA   ^DATA            ; Push bank byte
         PHB                    ; Push data bank register
         PLB                    ; Pull data bank register
         PHK                    ; Push program bank register
```

**Expression operators:**
- `<`: Low byte (bits 0-7)
- `>`: High byte (bits 8-15)
- `^`: Bank byte (bits 16-23)

### Native Mode Opcodes

Instructions new to 65816:

**Stack operations:**
```asm
PEA      value               ; Push effective address
PEI      (dp)                ; Push effective indirect address
PER      label               ; Push effective relative address
PHB                          ; Push data bank register
PHD                          ; Push direct page register
PHK                          ; Push program bank register
PLB                          ; Pull data bank register
PLD                          ; Pull direct page register
```

**Transfer operations:**
```asm
TCD                          ; Transfer C (16-bit A) to direct page
TCS                          ; Transfer C to stack pointer
TDC                          ; Transfer direct page to C
TSC                          ; Transfer stack pointer to C
TXY                          ; Transfer X to Y
TYX                          ; Transfer Y to X
XBA                          ; Exchange B and A accumulators
```

**Alternate mnemonics (same opcodes):**
```asm
TAS      ; Same as TCS
TSA      ; Same as TSC
TAD      ; Same as TCD
TDA      ; Same as TDC
SWA      ; Same as XBA
```

**Processor status:**
```asm
REP      #$20                ; Reset processor bits (M bit)
REP      #$10                ; Reset X bit
REP      #$30                ; Reset both M and X
SEP      #$20                ; Set processor bits (M bit)
SEP      #$10                ; Set X bit
SEP      #$30                ; Set both M and X
XCE                          ; Exchange carry and emulation bits
```

**Miscellaneous:**
```asm
BRL      label               ; Branch long (16-bit relative)
COP      #$xx                ; Co-processor instruction
WDM                          ; Reserved (William D. Mensch)
```

**Alternate branch mnemonics:**
```asm
BLT                          ; Branch if less than (same as BCC)
BGE                          ; Branch if greater/equal (same as BCS)
```

### Mode Switching

**Enter native mode:**
```asm
         CLC                    ; Clear carry
         XCE                    ; Exchange with emulation bit
```

**Enter emulation mode:**
```asm
         SEC                    ; Set carry
         XCE                    ; Exchange with emulation bit
```

**Set register sizes:**
```asm
* 16-bit accumulator
         REP   #$20             ; Clear M bit
         MX    %10              ; Tell assembler
         LONGA ON               ; Alternative

* 16-bit index registers
         REP   #$10             ; Clear X bit
         MX    %01              ; Tell assembler
         LONGI ON               ; Alternative

* 8-bit mode
         SEP   #$30             ; Set M and X bits
         MX    %11              ; Tell assembler
```

### Complete 65816 Example

```asm
         XC                     ; Enable 65816
         XC

         ORG   $8000

* Enter native mode
START    CLC
         XCE                    ; Native mode

* Set 16-bit accumulator
         REP   #$20             ; Clear M bit
         MX    %10              ; Tell assembler

* 16-bit operations
         LDA   #$1234           ; 16-bit immediate
         STA   DATA             ; 16-bit store
         LDA   DATA
         CLC
         ADC   #$5678
         STA   RESULT

* Set 8-bit accumulator
         SEP   #$20             ; Set M bit
         MX    %11              ; Tell assembler

* 8-bit operations
         LDA   #$12             ; 8-bit immediate
         STA   DATA

* Long addressing
         LDAL  $012345          ; 24-bit address
         STAL  $ABCDEF

* Return to emulation
         SEC
         XCE
         RTS

DATA     DW    $0000
RESULT   DW    $0000
```

---

## Macro System

### Overview

Macros provide:
- Code reuse and abstraction
- Parameterized code generation
- Simplified complex operations
- Custom language extensions

### Basic Macro Definition

```asm
         DO    0                ; Turn off assembly

MOVE     MAC                    ; Define MOVE macro
         LDA   ]1               ; Load from parameter 1
         STA   ]2               ; Store to parameter 2
         <<<                    ; End macro

         FIN                    ; Resume assembly
```

**Definition structure:**
1. Optional: `DO 0` to prevent assembly
2. Label with macro name
3. `MAC` opcode
4. Macro body (can reference ]1-]8)
5. `<<<` or `EOM` to end
6. Optional: `FIN` if using DO 0

### Macro Call Forms

```asm
         MOVE  $06;$07          ; Direct call
         PMC   MOVE,$06;$07     ; PMC form
         >>>   MOVE,$06;$07     ; >>> form
```

**Parameter rules:**
- Semicolon (`;`) separates parameters
- No spaces in parameter list
- PMC/>>> forms: comma (or other char) separates name from params

### Macro Parameters

**Available parameters:** `]1` through `]8`

**Merlin 16:** `]0` = parameter count

```asm
PRINT    MAC
         JSR   SENDMSG
         ASC   ]1               ; String parameter
         BRK
         <<<

         PRINT "Hello World"    ; Call with string
```

**Multiple parameters:**

```asm
MOVD     MAC
         LDA   ]1               ; First parameter
         STA   ]2               ; Second parameter
         LDA   ]3               ; Third parameter
         STA   ]4               ; Fourth parameter
         <<<

         MOVD  $10;$20;$30;$40  ; Four parameters
```

**Parameter literal substitution:**

Anything in parameter list substituted literally:

```asm
MOVE     MAC
         LDA   ]1
         STA   ]2
         <<<

         MOVE  #"A";DATA        ; ]1 = #"A", ]2 = DATA
         ; Generates: LDA #"A"
         ;            STA DATA

         MOVE  #$80;(PTR),Y     ; ]1 = #$80, ]2 = (PTR),Y
         ; Generates: LDA #$80
         ;            STA (PTR),Y
```

### Parameter Count (Merlin 16)

Use `]0` to create flexible macros:

```asm
OPTIONAL MAC
         LDA   ]1               ; Required parameter
         DO    ]0-1             ; If second parameter exists
         STA   ]2               ; Use it
         FIN
         <<<

         OPTIONAL $10           ; One parameter: just loads
         OPTIONAL $10;$20       ; Two parameters: loads and stores
```

### Nested Macros

Macros can call other macros:

```asm
         DO    0

POKE     MAC
         LDA   #]2              ; Load immediate
         STA   ]1               ; Store to address
         <<<

HTAB     MAC
         POKE  CH;]1            ; Call POKE from HTAB
         <<<

         FIN

         HTAB  20               ; Set horizontal tab to 20
         ; Expands to: POKE CH;20
         ; Which expands to: LDA #20
         ;                   STA CH
```

**Nesting depth:** Up to 15 levels

**Nested definition:**

```asm
TRDB     MAC
         TR    ]1+1;]2+1        ; Call TR with modified params
TR       MAC                    ; Nested definition
         LDA   ]1
         STA   ]2
         <<<                    ; Ends both macros
```

Usage:
```asm
         TR    LOC;DEST         ; Moves LOC to DEST
         TRDB  LOC;DEST         ; Moves LOC+1 to DEST+1, then LOC to DEST
```

### Conditionals in Macros

Use `DO`, `IF`, `ELSE`, `FIN` inside macros:

```asm
MOVD     MAC
         IF    #,]1             ; Test first char of ]1
         LDA   ]1/$100          ; Immediate mode - take high byte
         ELSE
         LDA   ]1               ; Address mode
         FIN
         STA   ]2
         <<<

         MOVD  #$80;DEST        ; Immediate: loads #$00 (high byte of #$80)
         MOVD  SOURCE;DEST      ; Address: loads from SOURCE
```

**Testing first character:**

```asm
PRINT    MAC
         IF    ",]1             ; If starts with double quote
         ASC   ]1               ; Negative ASCII
         ELSE
         ASC   ]1               ; Positive ASCII (or already processed)
         FIN
         <<<
```

**IF with MX flags (Merlin 16):**

```asm
PUSHWORD MAC
         IF    MX/2             ; If M is 8-bit
         LDA   ]1               ; Load word requires two ops
         PHA
         LDA   ]1+1
         PHA
         ELSE                   ; If M is 16-bit
         LDA   ]1               ; Single 16-bit load
         PHA
         FIN
         <<<
```

**Complex conditional example:**

```asm
MOVD     MAC                    ; Move with multiple addressing modes
         IF    (,]1             ; If ]1 starts with "("
         INY
         IF    (,]2             ; If ]2 also starts with "("
         ; Handle (indirect),Y to (indirect),Y
         ELSE
         ; Handle (indirect),Y to absolute
         FIN
         ELSE
         IF    (,]2             ; If ]2 starts with "("
         INY
         ; Handle absolute to (indirect),Y
         ELSE
         IF    #,]1             ; If ]1 starts with "#"
         ; Handle immediate to absolute
         ELSE
         ; Handle absolute to absolute
         FIN
         FIN
         FIN
         <<<
```

**All conditionals must be closed:** Count DOs and IFs, subtract FINs, must equal zero.

### Macro Labels

Labels inside macros updated each call:

```asm
REPEAT   MAC
         LDY   #]1              ; Loop count
LOOP     DEY                    ; Internal label
         BNE   LOOP             ; Branch to internal label
         <<<

         REPEAT 10              ; First call: LOOP at address1
         REPEAT 20              ; Second call: LOOP at address2
```

**Labeled EOM:**

```asm
COUNTR   MAC
         LDY   #]1
CNTLOOP  DEY
         BEQ   DONE             ; Branch to EOM label
         JSR   ]2
         JMP   CNTLOOP
DONE     <<<                    ; Labeled end of macro

         COUNTR 10;SUBROUTINE   ; Calls SUBROUTINE 10 times
```

**Error messages:** Usually show macro call line number, not internal line where error occurred. Check macro definition if error in macro call line.

### Macro Libraries

**Structure:**

```asm
         DO    0                ; Don't assemble definitions
         LST   OFF              ; Don't list (optional)

* Library documentation here

MACRO1   MAC
         ; ... macro 1 code ...
         <<<

MACRO2   MAC
         ; ... macro 2 code ...
         <<<

         LST   RTN              ; Return to previous list state
         FIN                    ; Resume assembly
```

**Using libraries:**

```asm
         USE   MACROS           ; Load macro library
         USE   /LIB/TOOLMACROS  ; ProDOS path

* Now can use macros from library
         MACRO1 param1;param2
         MACRO2 param3
```

**Included libraries (ProDOS):**
- `MACROS.S`: General purpose macros
- `MACROS.816.S`: 65816-specific macros
- `TOOL.MACROS`: IIgs Toolbox macros (use with MACROS.816.S)
- `FPMACROS.S`: Applesoft floating point routines
- `PRDEC.S`: Print A,X in decimal
- `ROCKWELL.S`: Rockwell 65C02 extended opcodes
- `TOOL.EQUATES`: IIgs Toolbox equates

**Library requirements:**
- Text file (.S suffix)
- Contains only MAC/EOM definitions and EQU statements
- DOS 3.3: Must have T. prefix (or use ! to bypass)
- Accessible at assembly time (on disk in drive)

### Advanced Macro Techniques

**Curly braces for expression evaluation (Merlin 16):**

```asm
ADDM     MAC
         LDA   {]1+]2}          ; Evaluate sum first
         <<<

]BASE    =     $10
         ADDM  ]BASE;5          ; Accesses $15, not "]BASE;5"
```

**String parameters:**

Must be quoted if containing spaces or semicolons:

```asm
PRINT    MAC
         JSR   SENDMSG
         ASC   ]1
         BRK
         <<<

         PRINT "Hello World"    ; Quotes required (space)
         PRINT 'Message'        ; Single quotes OK
         PRINT !"Hi"!           ; Different delimiter (embedded quotes)
```

**Final delimiter required:** `PRINT "A"` not `PRINT "A`

**Variable as string parameter:**

```asm
PRINT    MAC
         ASC   ]1
         <<<

]MSG     =     "Hello"
         PRINT ]MSG             ; Won't work - ]MSG not quoted

         PRINT "Hello"          ; Must use literal quoted string
```

**Multiple uses:**

```asm
         DO    0                ; Define once

CLEAR    MAC                    ; Clear memory region
         LDY   #]1-1
         LDA   #0
LOOP     STA   ]2,Y
         DEY
         BPL   LOOP
         <<<

         FIN                    ; Resume assembly

         CLEAR 100;$800         ; Clear 100 bytes at $800
         CLEAR 256;$2000        ; Clear 256 bytes at $2000
         CLEAR 50;BUFFER        ; Clear 50 bytes at BUFFER
```

---

## Examples

### Complete Program Example

```asm
********************************************************************************
* HELLO WORLD PROGRAM
* Demonstrates basic program structure with equates, data, and subroutines
********************************************************************************
         ORG   $8000            ; Origin at $8000

* ==================
* EQUATES
* ==================
HOME     EQU   $FC58            ; Monitor clear screen
COUT     EQU   $FDED            ; Monitor character output
BELL     EQU   $FBDD            ; Monitor bell

* ==================
* MAIN PROGRAM
* ==================
START    JSR   HOME             ; Clear screen
         LDX   #0               ; Initialize index
:LOOP    LDA   MESSAGE,X        ; Get character
         BEQ   :DONE            ; Zero = end of string
         JSR   COUT             ; Print character
         INX                    ; Next character
         BNE   :LOOP            ; Continue if not wrapped
:DONE    JSR   BELL             ; Ring bell
         RTS                    ; Return to caller

* ==================
* DATA
* ==================
MESSAGE  ASC   "Hello, World!"
         HEX   8D00             ; CR + null terminator
```

### Macro Example

```asm
********************************************************************************
* MACRO DEMONSTRATION
* Shows macro definition and usage
********************************************************************************
         ORG   $8000

* Equates
HOME     EQU   $FC58
COUT     EQU   $FDED

* ==================
* MACRO DEFINITIONS
* ==================
         DO    0                ; Don't assemble definitions

* Print string macro
PRINT    MAC
         JSR   SENDMSG
         ASC   ]1
         BRK
         <<<

* Move memory macro
MOVE     MAC
         LDA   ]1
         STA   ]2
         <<<

* Poke value into address macro
POKE     MAC
         LDA   #]2
         STA   ]1
         <<<

         FIN                    ; Resume assembly

* ==================
* MAIN PROGRAM
* ==================
START    JSR   HOME
         PRINT "Starting program..."
         MOVE  #$80;FLAG
         POKE  $C030;$FF
         PRINT "Complete!"
         RTS

FLAG     DFB   0

* Print string subroutine for PRINT macro
SENDMSG  LDX   #0
:LOOP    INX
         LDA   START,X
         BMI   :LOOP
         EOR   #$80
         JSR   COUT
         INX
         LDA   START,X
         BNE   :LOOP
         RTS
```

### Conditional Assembly Example

```asm
********************************************************************************
* CONDITIONAL ASSEMBLY DEMONSTRATION
* Shows multiple version control with flags
********************************************************************************

* ===================
* CONFIGURATION FLAGS
* ===================
DEBUG    =     1                ; 1=debug version, 0=release
TARGET   =     2                ; 0=IIe, 1=IIc, 2=IIgs
PRINTER  =     0                ; 1=printer, 0=screen

         ORG   $8000

* Equates
HOME     EQU   $FC58
COUT     EQU   $FDED

* ==================
* MAIN PROGRAM
* ==================
START    JSR   HOME

* Debug code
         DO    DEBUG
         LDA   #"D"             ; Show debug indicator
         JSR   COUT
         JSR   $FF69            ; Break to monitor (debug only)
         FIN

* Target-specific code
         DO    TARGET
         JSR   INITGS           ; IIgs initialization
         ELSE
         JSR   INITSTD          ; Standard init
         FIN

* Output device selection
         DO    PRINTER
OUTPUT   EQU   $C100            ; Printer port
         ELSE
OUTPUT   EQU   COUT             ; Screen output
         FIN

         LDA   #$C8             ; 'H'
         JSR   OUTPUT
         RTS

* ==================
* SUBROUTINES
* ==================
         DO    TARGET-2         ; If not IIgs
INITSTD  ; Standard initialization
         RTS
         FIN

         DO    TARGET/2         ; If IIgs
INITGS   ; IIgs-specific init
         RTS
         FIN
```

### 65816 Example

```asm
********************************************************************************
* 65816 EXAMPLE
* Demonstrates native mode, 16-bit operations, long addressing
********************************************************************************
         XC                     ; Enable 65816
         XC                     ; (two XCs required)

         ORG   $8000

* ==================
* ENTER NATIVE MODE
* ==================
START    CLC
         XCE                    ; Switch to native mode

* ==================
* 16-BIT ACCUMULATOR
* ==================
         REP   #$20             ; Clear M bit (16-bit accumulator)
         MX    %10              ; Inform assembler

         LDA   #$1234           ; 16-bit immediate load
         STA   DATA             ; 16-bit store

         CLC
         ADC   #$5678           ; 16-bit addition
         STA   RESULT

* ==================
* 16-BIT INDEXES
* ==================
         REP   #$10             ; Clear X bit (16-bit indexes)
         MX    %00              ; Inform assembler (both 16-bit)

         LDX   #$1000           ; 16-bit index load
         LDY   #$2000

* ==================
* LONG ADDRESSING
* ==================
         LDAL  $012345          ; Load from bank $01
         STAL  $ABCDEF          ; Store to bank $AB

* ==================
* BLOCK MOVE
* ==================
         LDA   #$00FF           ; 256 bytes minus 1
         LDX   #$1000           ; Source address
         LDY   #$2000           ; Destination address
         MVN   $00,$01          ; Move from bank 0 to bank 1

* ==================
* STACK RELATIVE
* ==================
         PEA   $1234            ; Push immediate word
         PEA   $5678
         LDA   1,S              ; Access stack relative
         STA   3,S              ; Swap words
         LDA   3,S
         STA   1,S

* ==================
* RETURN TO 8-BIT
* ==================
         SEP   #$30             ; Set M and X bits (8-bit mode)
         MX    %11              ; Inform assembler

         LDA   #$12             ; 8-bit immediate
         STA   DATA

* ==================
* RETURN TO EMULATION
* ==================
         SEC
         XCE                    ; Back to emulation mode
         RTS

* ==================
* DATA
* ==================
DATA     DW    $0000
RESULT   DW    $0000
```

### Relocatable Code Example

```asm
********************************************************************************
* MODULE 1 - Main Program (relocatable)
********************************************************************************
         REL                    ; Relocatable code
         DSK   MODULE1.L        ; Save to MODULE1.L

* ==================
* EXTERNAL REFERENCES
* ==================
DRAWLINE EXT                    ; Graphics routine
CLEARSCR EXT                    ; Screen clear
PRINT    EXT                    ; Print routine

* ==================
* MAIN PROGRAM
* ==================
START    JSR   CLEARSCR         ; Call external routine
         LDX   #10
         LDY   #20
         JSR   DRAWLINE         ; Another external
         JSR   PRINT            ; Third external
         RTS

********************************************************************************
* MODULE 2 - Graphics Library (relocatable)
********************************************************************************
         REL
         DSK   MODULE2.L

* ==================
* ENTRY POINTS
* ==================
DRAWLINE ENT                    ; Make visible to other modules
         ; ... draw line code ...
         RTS

CLEARSCR ENT                    ; Another entry point
         ; ... clear screen code ...
         RTS

********************************************************************************
* MODULE 3 - I/O Library (relocatable)
********************************************************************************
         REL
         DSK   MODULE3.L

* ==================
* ENTRY POINTS
* ==================
PRINT    ENT                    ; Print routine entry
         JSR   $FDED            ; Use monitor routine
         RTS

********************************************************************************
* LINK COMMAND FILE
* Links all modules together
********************************************************************************
         ORG   $8000            ; Final address
         LNK   MODULE1.L        ; Link main program
         LNK   MODULE2.L        ; Link graphics library
         LNK   MODULE3.L        ; Link I/O library
         SAV   FINAL            ; Save linked program
```

**To assemble and link:**
1. Assemble MODULE1 source (creates MODULE1.L)
2. Assemble MODULE2 source (creates MODULE2.L)
3. Assemble MODULE3 source (creates MODULE3.L)
4. Link with link command file (creates FINAL)
5. BLOAD FINAL and run at $8000

---

## Summary of Key Features

### Three-Tier Label System
- **Global labels**: Standard labels, visible everywhere
- **Local labels** (`:`): Scoped to last global label, can reuse names
- **Variables** (`]`): Redefinable, save symbol space, mainly for macros

### Number Formats
- **Decimal**: `100` (no prefix)
- **Hexadecimal**: `$64` (dollar sign prefix)
- **Binary**: `%01100100` (percent prefix)
- **ASCII**: `"A"` (high bit set, $C1) or `'A'` (clear, $41)

### String Directives
- `ASC`: ASCII string (delimiter controls high bit)
- `DCI`: Last character inverted
- `INV`: Inverse video
- `FLS`: Flashing
- `REV`: Reversed in memory
- `STR`: Length-prefixed (Pascal/ProDOS style)
- `HEX`: Raw hex data (no $ prefix)

### Data Definition
- `DFB`/`DB`: Define byte
- `DA`/`DW`: Define word (low, high)
- `DDB`: Define word (high, low)
- `ADR`: 24-bit address (Merlin 16)
- `ADRL`: 32-bit address (Merlin 16)
- `DS`: Define/reserve storage

### Expression Operators
- **Arithmetic**: `+ - * /` (left to right, no precedence)
- **Logical**: `& . !` (AND, OR, XOR)
- **Byte selection**: `< > ^` (low, high, bank)
- **Current address**: `*`

### Addressing Modes
- **Immediate**: `#$12` (value, not address)
- **Absolute**: `$1234` (16-bit address)
- **Zero page**: `$12` (8-bit address, automatic)
- **Indexed**: `$10,X` or `$1000,Y`
- **Indirect**: `($10),Y` or `($10,X)`
- **Long** (65816): `!$123456` or `LDAL $123456`
- **Stack relative** (65816): `1,S`
- **Force absolute**: Add char after opcode: `LDA: $10`

### Conditional Assembly
- `DO expression`: Assemble if non-zero
- `IF char,]var`: Test first character
- `ELSE`: Invert condition
- `FIN`: End conditional
- Up to 8 levels of nesting

### Macro System
- **Definition**: `MAC` ... `<<<` or `EOM`
- **Parameters**: `]1` through `]8`, `]0` (count in Merlin 16)
- **Calling**: Direct, `PMC`, or `>>>` forms
- **Conditionals**: `DO/IF/ELSE/FIN` inside macros
- **Nesting**: Up to 15 levels
- **Libraries**: Load with `USE` directive

### 65816 Extensions
- **Long addressing**: 24-bit addresses (`LDAL`, `!` operator)
- **16-bit mode**: `REP #$20` (accumulator), `REP #$10` (indexes)
- **MX directive**: Tell assembler register sizes
- **New instructions**: PEA, PHB, PHD, PHK, REP, SEP, XBA, XCE, etc.
- **Block move**: MVP, MVN
- **Stack relative**: `LDA 1,S`
- **Bank byte**: `^` operator

### File Operations
- `PUT`: Insert source file at assembly time
- `USE`: Load macro library (kept in memory)
- `DSK`: Assemble directly to disk
- `SAV`: Save object during assembly
- `TYP`: Set ProDOS file type

### Linking
- `REL`: Generate relocatable code
- `ENT`: Entry point (visible to other modules)
- `EXT`: External reference (from other modules)
- Enables: Code reuse, modular programming, faster development

### Listing Control
- `LST ON/OFF/RTN`: Control listing output
- `EXP ON/OFF/ONLY`: Control macro expansion
- `CYC`: Show instruction cycle counts
- `TR ON/OFF`: Truncate object code display
- `LST Control-D`: Toggle listing during assembly

---

## Quick Reference Card

```
COMMENT STYLES:              LABEL TYPES:
* Full line comment          GLOBAL    Normal label
; Comment (tabbed)           :LOCAL    Colon prefix (scoped)
  ; End of line              ]VAR      Bracket prefix (redefinable)

NUMBER FORMATS:              DATA DIRECTIVES:
100      Decimal             DFB/DB    Define byte
$64      Hexadecimal         DA/DW     Define word (low,high)
%0110    Binary              DDB       Define word (high,low)
"A"      ASCII (high set)    ADR       24-bit address (Merlin 16)
'A'      ASCII (high clear)  ADRL      32-bit address (Merlin 16)
                             DS        Define storage
OPERATORS:                   HEX       Raw hex data (no $)
+ - * /  Arithmetic (left to right)
& . !    AND OR XOR          STRING DIRECTIVES:
< > ^    Low/High/Bank       ASC       ASCII string
*        Current address     DCI       Last char inverted
                             INV       Inverse video
ADDRESSING:                  FLS       Flashing
#$12     Immediate           REV       Reversed
$1234    Absolute            STR       Length-prefixed
$12      Zero page
$12,X    Indexed             CONDITIONALS:
($12,X)  Indexed indirect    DO expr   Assemble if true
($12),Y  Indirect indexed    IF c,]v   Test variable char
!$1234   Force long (65816)  ELSE      Invert condition
1,S      Stack rel (65816)   FIN       End conditional

MACROS:                      CPU CONTROL:
name MAC                     XC        Enable 65C02/65816 (use twice)
  ]1-]8  Parameters (]0=count Merlin 16)
<<<      End macro           MX %00    M=16-bit, X=16-bit
PMC name Call form           MX %01    M=16-bit, X=8-bit
>>> name Call form           MX %10    M=8-bit, X=16-bit
                             MX %11    M=8-bit, X=8-bit (Emulation)
ORIGIN:                      LONGA     Set accumulator size
ORG addr Set origin          LONGI     Set index size
ORG      Re-ORG              SW        Enable Sweet 16 (Merlin 8)
DUM addr Dummy section
DEND     End dummy           LISTING:
OBJ addr Set object area     LST ON    Enable listing
                             LST OFF   Disable listing
LINKING:                     LST RTN   Return to previous (Merlin 16)
REL      Relocatable         EXP ON    Expand macros
ENT      Entry point         EXP OFF   Only macro call
EXT      External ref        EXP ONLY  Only generated code
                             CYC       Show cycles
LOOPS:                       TR ON     Truncate display
LUP n    Loop n times
--^      End loop            FILES:
@        Sequential labels   PUT file  Include source
                             USE file  Include macros
                             DSK file  Output to disk
                             SAV file  Save object during assembly

EQUATES:                     SPECIAL:
name EQU expr                ERR expr  Force error if true
name = expr                  KBD       Get keyboard input
                             VAR       Setup variables
                             CHK       Insert checksum
                             DAT       Date stamp (ProDOS)
                             PAG       New page
                             TTL       Title (Merlin 16)
                             END       End assembly
```

---

## Addressing Mode Summary

```
MODE                SYNTAX              EXAMPLE
================================================
Implied             [no operand]        RTS
Accumulator         [no operand]        LSR
Immediate           #expr               LDA #$12
Absolute            expr                LDA $1234
Zero Page           expr                LDA $12
Absolute,X          expr,X              LDA $1234,X
Absolute,Y          expr,Y              LDA $1234,Y
Zero Page,X         expr,X              LDA $12,X
Zero Page,Y         expr,Y              LDX $12,Y
Indirect            (expr)              JMP ($1234)
Indexed Indirect    (expr,X)            LDA ($12,X)
Indirect Indexed    (expr),Y            LDA ($12),Y
Relative            expr                BNE LOOP

65816 ADDITIONAL MODES:
================================================
Long Absolute       !expr or LDAL expr  LDA !$123456
Long Absolute,X     !expr,X             LDA !$123456,X
Long Indirect       [expr]              LDA [$12]
Long Indirect,Y     [expr],Y            LDA [$12],Y
Stack Relative      expr,S              LDA 1,S
Stack Rel Ind       (expr,S),Y          LDA (1,S),Y
Block Move          expr,expr           MVN $01,$02
```

---

## Pseudo-Op Quick Reference

```
CATEGORY            PSEUDO-OP           PURPOSE
================================================
Origin/Sections     ORG                 Set origin address
                    DUM/DEND            Dummy section (labels only)
                    OBJ                 Set object code area

Symbols             EQU or =            Define constant
                    ENT                 Entry point (linking)
                    EXT                 External reference (linking)

Data                DFB/DB              Define byte
                    DA/DW               Define word (low,high)
                    DDB                 Define word (high,low)
                    ADR                 24-bit address
                    ADRL                32-bit address
                    DS                  Define storage
                    HEX                 Raw hex data

Strings             ASC                 ASCII string
                    DCI                 Last char inverted
                    INV                 Inverse video
                    FLS                 Flashing
                    REV                 Reversed
                    STR                 Length-prefixed
                    STRL                Length word prefix

Conditionals        DO                  Assemble if true
                    IF                  Test character/MX
                    ELSE                Invert condition
                    FIN                 End conditional

Macros              MAC                 Begin macro
                    EOM or <<<          End macro
                    PMC or >>>          Put macro call

Loops               LUP                 Begin loop
                    --^                 End loop

CPU Control         XC                  Enable 65C02/65816
                    MX                  Set M/X status
                    LONGA               Set accumulator size
                    LONGI               Set index size
                    SW                  Enable Sweet 16

Files               PUT                 Include source file
                    USE                 Include macro library
                    DSK                 Assemble to disk
                    SAV                 Save object code
                    TYP                 Set file type
                    AUX                 Set aux type

Linking             REL                 Relocatable code
                    ENT                 Entry label
                    EXT                 External label

Listing             LST                 Listing control
                    LSTDO               List DO regions
                    EXP                 Macro expansion
                    TR                  Truncate display
                    OBJ                 Object listing
                    CYC                 Cycle counts
                    AST                 Asterisk line
                    SKP                 Skip lines
                    PAG                 New page
                    TTL                 Title (Merlin 16)

Special             ERR                 Force error
                    KBD                 Keyboard input
                    VAR                 Setup variables
                    CHK                 Checksum
                    DAT                 Date stamp
                    END                 End assembly
                    USR                 User defined op
```

---

*This comprehensive syntax guide documents Merlin 8/16 assembly language as used on Apple II computers. It is based on the Merlin 8/16 User's Manual and covers all documented features of the assembler. For specific implementation details or troubleshooting, consult the official Merlin 8/16 User's Manual by Glen Bredon, published by Roger Wagner Publishing, Inc.*

**Document Version:** Complete extraction from Merlin 8/16 User's Manual
**Assembler:** Merlin 8/16 (Merlin 8 ProDOS, Merlin 8 DOS 3.3, Merlin 16 ProDOS)
**Target Systems:** Apple IIgs, Apple IIe (128K), Apple IIc (128K)
**Processors:** 6502, 65C02, 65802, 65816
