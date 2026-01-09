# EDTASM Syntax Module

## Overview

The EDTASM syntax module provides compatibility with Disk EDTASM+ assembler for TRS-80 Color Computer 6809/6309 development, including OS-9 operating system support.

**Target Platform:** TRS-80 Color Computer (CoCo 1/2/3), Tandy MC-10, Dragon 32/64
**Target OS:** OS-9 Level I/II, NitrOS-9
**CPU Support:** 6809, 6309, Turbo9, Konami-2

## Features

- ✓ **Comments**: Asterisk (`*`) in column 1, semicolon (`;`) anywhere
- ✓ **Case handling**: Case-sensitive by default, `-nocase` for case-insensitive
- ✓ **Dollar signs**: Full support for OS-9 system calls (`F$Link`, `I$Read`, etc.)
- ✓ **Data directives**: FCB, FDB, FCC (flexible delimiters), RMB
- ✓ **Conditional assembly**: COND/ENDC, IFEQ/IFNE/IFGT/IFGE/IFLT/IFLE, ELSE, ENDIF
- ✓ **Macro system**: MACRO/ENDM with `\\1-\\9` parameters, `\\@` unique ID, `\\.label` local labels
- ✓ **Sections**: SECTION, ORG
- ✓ **Listing control**: OPT, PAGE, TITLE, LIST, NOLIST
- ✓ **OS-9 support**: MOD directive (stub), system call definitions

## Building

```bash
make CPU=6809 SYNTAX=edtasm
```

This creates: `vasm6809_edtasm`

## Basic Usage

```bash
# Assemble to binary (case-sensitive, default)
./vasm6809_edtasm -Fbin -o program.bin source.asm

# Case-insensitive mode (per EDTASM+ specification)
./vasm6809_edtasm -nocase -Fbin -o program.bin source.asm

# With listing file
./vasm6809_edtasm -Fbin -L program.lst -o program.bin source.asm

# Include path for OS-9 definitions
./vasm6809_edtasm -I./include -Fbin -o program.bin source.asm
```

## Syntax Elements

### Comments

```asm
* Asterisk in column 1 is a comment line
        LDA     #10     ; Semicolon comment anywhere on line
```

### Case Sensitivity

**Default behavior (case-sensitive):**
```asm
MyLabel  EQU  $1000    ; Different from mylabel or MYLABEL
mylabel  EQU  $2000
MYLABEL  EQU  $3000
```

**Case-insensitive mode (`-nocase` flag):**
```asm
MyLabel  EQU  $1000    ; Same as mylabel, MYLABEL
        LDA  mylabel   ; References $1000
        STA  MYLABEL   ; References $1000
```

### Identifiers

Valid identifier characters:
- Start: `A-Z`, `a-z`, `_`, `@`
- Continue: `A-Z`, `a-z`, `0-9`, `_`, `@`, `$`

Examples:
```asm
start           ; Simple label
MyVariable      ; Mixed case
F$Link          ; OS-9 system call (dollar sign)
I$Read          ; OS-9 I/O call
_private        ; Underscore prefix
@global         ; At-sign prefix
```

## Data Directives

### FCB - Form Constant Byte

Define one or more 8-bit values:

```asm
        FCB     1,2,3,4,5           ; Multiple bytes
        FCB     $FF                 ; Hex value
        FCB     %11110000           ; Binary value
        FCB     'A'                 ; ASCII character
        FCB     255                 ; Decimal value
```

### FDB - Form Double Byte

Define one or more 16-bit words (big-endian):

```asm
        FDB     $1234               ; Single word
        FDB     $1000,$2000,$3000   ; Multiple words
        FDB     START               ; Address of label
        FDB     START+10            ; Expression
```

### FCC - Form Constant Characters

Define ASCII string with flexible delimiter:

```asm
        FCC     /Hello, World!/     ; Using slash delimiter
        FCC     "Another string"    ; Using quote delimiter
        FCC     'Single quotes'     ; Using apostrophe
        FCC     |Any character|     ; Any non-alphanumeric works
        FCC     #Delimiter is #     ; Even hash symbol
```

**Note:** First non-whitespace character after FCC is the delimiter.

### RMB - Reserve Memory Bytes

Reserve uninitialized space:

```asm
BUFFER  RMB     256                 ; 256 bytes of space
VARS    RMB     10                  ; 10 bytes
STACK   RMB     $100                ; 256 bytes (hex)
```

## Conditional Assembly

### COND/ENDC

Basic conditional assembly:

```asm
DEBUG   EQU     1                   ; Debug flag

        COND    DEBUG               ; Assemble if DEBUG != 0
        LDA     #$FF
        STA     DEBUGPORT
        ENDC
```

### IF/ELSE/ENDIF Family

```asm
TARGET  EQU     1                   ; 1=CoCo3, 0=CoCo2

        IFEQ    TARGET              ; If TARGET == 0
        LDA     #COCO2MODE
        ENDIF

        IFNE    TARGET              ; If TARGET != 0
        LDA     #COCO3MODE
        ENDIF

        IFGT    COUNT-10            ; If COUNT > 10
        ; ...
        ELSE                        ; Otherwise
        ; ...
        ENDIF

        IFGE    VALUE-5             ; If VALUE >= 5
        ; ...
        ENDIF

        IFLT    SIZE-100            ; If SIZE < 100
        ; ...
        ENDIF

        IFLE    LIMIT-50            ; If LIMIT <= 50
        ; ...
        ENDIF
```

## Macro System

### Macro Definition

```asm
* Define a macro with MACRO/ENDM
DELAY   MACRO
        LDB     \\1                 ; Parameter 1
\\.LOOP DECB                        ; Local label
        BNE     \\.LOOP
        ENDM
```

### Macro Parameters

- `\\1` through `\\9` - Macro parameters 1-9
- `\\@` - Unique ID for this macro invocation (generates `_nnnnnn`)
- `\\.label` - Local label unique to this macro invocation

### Macro Invocation

```asm
        DELAY   #10                 ; Invoke with parameter
        DELAY   #20                 ; Invoke again
```

### Macro Expansion Example

```asm
* Macro definition
PUSH16  MACRO
        PSHS    \\1,\\1+1           ; Push 16-bit value
        ENDM

ADDW    MACRO
        LDD     \\1                 ; Load first value
        ADDD    \\2                 ; Add second value
        STD     \\3                 ; Store result
        ENDM

* Usage
        PUSH16  MYWORD              ; Expands to: PSHS MYWORD,MYWORD+1
        ADDW    VALUE1,VALUE2,RESULT ; Complex macro
```

### Local Labels in Macros

```asm
WAITKEY MACRO
        LDA     #'?'
        JSR     PUTCHAR
\\.WAIT JSR     GETCHAR             ; Local label
        BEQ     \\.WAIT             ; Branch to local
        ENDM

* Each invocation gets unique labels:
        WAITKEY                     ; Creates WAITKEY_000001.WAIT
        WAITKEY                     ; Creates WAITKEY_000002.WAIT
```

## OS-9 Support

### OS-9 System Calls

Include OS-9 definitions:

```asm
        INCLUDE os9.d               ; OS-9 system definitions

* Use OS-9 system calls
        LDA     #READ               ; Access mode
        LDX     #FILENAME           ; Path name
        OS9     I$Open              ; Open file
        BCS     ERROR

        LDX     #BUFFER             ; Buffer address
        LDY     #256                ; Bytes to read
        OS9     I$Read              ; Read data
        BCS     ERROR
```

### MOD Directive (OS-9 Module Header)

```asm
TypeLang EQU    $11                 ; Module type/language
AttRev   EQU    $81                 ; Attributes/revision
Edition  EQU    $00                 ; Edition
Stack    EQU    $200                ; Stack size

        MOD     MODEND,MODNAME,TypeLang,AttRev,START,MEMSIZE

MODNAME FCC     /MyProgram/
        FCB     $00

START   ; Program code begins here
        ; ...

MEMSIZE EQU     .                   ; End of memory requirement
MODEND  EQU     *                   ; End of module
```

**Note:** MOD directive is currently recognized but does not generate full OS-9 module headers. Proper OS-9 module format support is planned for a future release.

### Typical OS-9 Program Structure

```asm
        INCLUDE os9.d               ; OS-9 definitions

TypeLang EQU    $11
AttRev   EQU    $81
Edition  EQU    $00
Stack    EQU    $200

        MOD     MODEND,MODNAME,TypeLang,AttRev,START,MEMSIZE

MODNAME FCC     /example/
        FCB     $00

START   PSHS    CC                  ; Save registers
        ; Program code
        PULS    CC,PC               ; Return

* Data area
BUFFER  RMB     256

MEMSIZE EQU     .
MODEND  EQU     *
```

## Sections

### SECTION Directive

Create named sections:

```asm
        SECTION CODE                ; Code section
        ; Code here

        SECTION DATA                ; Data section
        ; Data here

        SECTION BSS                 ; Uninitialized data
        ; Reserve space here
```

### ORG Directive

Set origin address:

```asm
        ORG     $0400               ; Start at $0400
START   LDA     #0

        ORG     $1000               ; Jump to $1000
MAIN    JSR     START
```

## Listing Control

### OPT Directive

Control listing options:

```asm
        OPT     MC                  ; List macro calls
        OPT     NOMC                ; Don't list macro calls

        OPT     MD                  ; List macro definitions
        OPT     NOMD                ; Don't list macro definitions

        OPT     MEX                 ; List macro expansions
        OPT     NOMEX               ; Don't list macro expansions

        OPT     L                   ; Enable listing
        OPT     NOL                 ; Disable listing

* Multiple options
        OPT     NOMC,MEX            ; Combine options
```

### PAGE and TITLE

```asm
        TITLE   My Program          ; Set listing title
        PAGE                        ; Start new page
```

### LIST/NOLIST

```asm
        LIST                        ; Enable listing
        ; Code listed here
        NOLIST                      ; Disable listing
        ; Code not listed
        LIST                        ; Re-enable
```

## Symbol Definition

### EQU - Equate (Constant)

Define constant symbol (cannot be redefined):

```asm
BUFSIZE EQU     256                 ; Define constant
MAXVAL  EQU     BUFSIZE*2           ; Expression
```

### SET - Set Symbol (Variable)

Define variable symbol (can be redefined):

```asm
COUNT   SET     0                   ; Initial value
COUNT   SET     COUNT+1             ; Increment
COUNT   SET     10                  ; Set new value
```

## Complete Example

```asm
* Example EDTASM program for TRS-80 Color Computer
* Demonstrates key syntax features

        INCLUDE os9.d               ; OS-9 definitions

* Constants
BUFSIZE EQU     256
DEBUG   EQU     0

* Module header
TypeLang EQU    $11
AttRev   EQU    $81
Stack    EQU    $200

        MOD     MODEND,MODNAME,TypeLang,AttRev,START,MEMSIZE

MODNAME FCC     /example/
        FCB     $00

* Macro definitions
PUTCH   MACRO
        LDA     \\1                 ; Character to output
        OS9     I$WritLn
        ENDM

DELAY   MACRO
        LDB     #\\1
\\.LOOP DECB
        BNE     \\.LOOP
        ENDM

* Main program
START   PSHS    CC                  ; Save registers

        COND    DEBUG
        PUTCH   #'D'                ; Debug output
        ENDC

        LDX     #MESSAGE            ; Print message
        LDY     #MSGLEN
        OS9     I$Write

        DELAY   #100                ; Wait

        PULS    CC,PC               ; Return

* Data
MESSAGE FCC     /Hello, OS-9!/
        FCB     $0D
MSGLEN  EQU     *-MESSAGE

BUFFER  RMB     BUFSIZE             ; Reserve buffer

MEMSIZE EQU     .
MODEND  EQU     *
```

## Command-Line Options

### Syntax-Specific Options

- `-nocase` - Enable case-insensitive mode (labels, symbols, directives)

### Common vasm Options

- `-Fbin` - Binary output
- `-Fhunk` - Amiga hunk format
- `-Felf` - ELF object format
- `-o <file>` - Output filename
- `-L <file>` - Listing file
- `-I<path>` - Include search path
- `-Dsymbol=value` - Define symbol
- `-w` - Show warnings
- `-quiet` - Suppress warnings

## Differences from Original EDTASM

1. **Case sensitivity**: Original EDTASM+ was case-insensitive by default. This implementation defaults to case-sensitive for compatibility with existing mixed-case code. Use `-nocase` for traditional behavior.

2. **MOD directive**: Currently recognized but does not generate full OS-9 module headers with CRC and header sync bytes. Binary output only.

3. **Expression evaluation**: Uses vasm's powerful expression evaluator with 128-bit arithmetic and complex relocations.

4. **Macro system**: Based on vasm's robust macro infrastructure with proper nesting and recursion support.

## Compatibility Notes

### OS-9 Include Files

You'll need OS-9 system definition files. A basic `os9.d` is provided with:
- System call codes (`F$Link`, `I$Read`, etc.)
- Error codes (`E$PthFul`, `E$EOF`, etc.)
- GetStat/SetStat codes (`SS_Opt`, `SS_Ready`, etc.)
- OS9 macro for system calls

### Real EDTASM Compatibility

Code written for original Disk EDTASM should assemble with minimal changes:
- Add `-nocase` flag if code relies on case-insensitivity
- Ensure `os9.d` is in include path
- MOD directive is recognized but doesn't generate complete headers yet

## Known Limitations

1. **OS-9 Module Generation**: The MOD directive is a stub. It recognizes the syntax but doesn't generate:
   - Module header sync bytes ($87CD)
   - Module size calculation
   - Header parity byte
   - Module CRC bytes

   Future enhancement will add proper OS-9 module format support.

2. **Listing Directives**: PAGE and TITLE are recognized but have limited effect on listing output.

## Testing

Test with included example:

```bash
# Create test file
cat > test.asm << 'EOF'
* Test EDTASM syntax
        ORG     $0400

START   LDA     #10
        STA     COUNT

LOOP    DEC     COUNT
        BNE     LOOP
        RTS

COUNT   RMB     1
        END     START
EOF

# Assemble
./vasm6809_edtasm -Fbin -o test.bin test.asm
```

## Example Projects

- **Invaders09**: Classic Space Invaders for OS-9 (https://github.com/barberd/Invaders09)
- Demonstrates real-world OS-9 program structure
- Uses MOD directive, OS-9 system calls, macros

## Resources

### Documentation

- **EDTASM Manual**: Original Disk EDTASM documentation
- **OS-9 Technical Manual**: System call reference
- **6809 Assembly Language Programming**: Hardware reference

### Online Resources

- **NitrOS-9 Project**: https://github.com/n6il/nitros9
- **CoCo Community**: https://www.cocopedia.com/
- **OS-9 Archive**: http://www.os9archive.com/

### Tools

- **toolshed**: OS-9 disk utilities for Linux/macOS/Windows
- **MAME**: TRS-80 CoCo emulation
- **VCC**: Virtual Color Computer emulator

## Future Enhancements

Planned additions:

1. **Complete OS-9 module generation**
   - Header sync bytes
   - Size calculation
   - CRC generation
   - Module validation

2. **Additional directives**
   - PSECT (Program Section)
   - VSECT (Variable Section)
   - NAM (Name)

3. **Extended listing control**
   - Full PAGE/TITLE support
   - Line numbering options
   - Cross-reference generation

4. **OS-9 output format**
   - Native OS-9 module format output
   - Direct integration with OS-9 filesystem tools

## Contributing

When contributing to EDTASM module:

1. Follow C90 coding standard
2. Maintain compatibility with original EDTASM syntax
3. Add tests for new features
4. Update this documentation
5. Test with real OS-9 code (Invaders09, NitrOS-9 sources)

## License

Part of vasm - Copyright © 2025 Bryan Woodruff, Cortexa LLC

Same license terms as vasm core. See main README.md for details.

## Contact

- **EDTASM module**: Bryan Woodruff - bryanw@cortexa.com
- **vasm core**: Volker Barthelmann - vb@compilers.de, Frank Wille - frank@phoenix.owl.de

## Version History

### 1.0 (January 2025)
- Initial implementation
- Complete directive set (FCB, FDB, FCC, RMB, COND, OPT, etc.)
- Macro system with `\\1-\\9`, `\\@`, `\\.label`
- Comment handling (`*` column 1, `;` anywhere)
- Dollar sign support in identifiers
- Case-sensitive/insensitive modes
- Basic OS-9 support (MOD directive stub)
- Successfully assembles Invaders09
