*-----------------------------------
* Test: XOR operator (^) in expressions
* Feature #1: XOR with # prefix in .DA
*-----------------------------------
        .OR $2000

* Basic XOR expressions
        LDA #$BE^$A5     ; 0xBE ^ 0xA5 = 0x1B
        LDA #$FF^$00     ; 0xFF ^ 0x00 = 0xFF
        LDA #$AA^$55     ; 0xAA ^ 0x55 = 0xFF

* XOR in .DA directive
        .DA $BE^$A5      ; Without # prefix
        .DA #$BE^$A5     ; With # prefix (SCASM extension)

* XOR with symbols
SCI.STARTUP .EQ $1234
        .DA SCI.STARTUP,#$BE^$A5

        .END
