* Comprehensive test for Phase 3 SCMASM directives
*
* Tests: .AS, .AZ, .AT, .EQ, .OP
*
        .OR $8000

* Test .EQ directive for constants
SCREEN  .EQ $D020
MAXVAL  .EQ 255
DEBUG   .EQ 1

* Test .DO conditional with .EQ constant
        .DO DEBUG
MSG1    .AS "Debug Mode"
        .FIN

START   NOP
        LDA #MAXVAL
        STA SCREEN

* Test .AS directive (ASCII string, no terminator)
TEXT1   .AS "Hello"

* Test .AZ directive (ASCII zero-terminated string)
TEXT2   .AZ "World"

* Test .AT directive (ASCII with high bit on last char)
TEXT3   .AT "Test"

* Test .OP directive - switch to 65C02
        .OP 65C02
        STZ $00         ; 65C02 STZ instruction
        STZ SCREEN

* Test .OP directive - switch to 65816
        .OP 816
        REP #$30        ; 65816 16-bit mode
        SEP #$20        ; Back to 8-bit accumulator

* Switch back to 6502
        .OP 6502
        RTS

* Data section with mixed string types
STRS    .AS /ASCII/
        .AZ "Zero-term"
        .AT 'HiBit'

END     .EQ *
