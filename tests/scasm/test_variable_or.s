*-----------------------------------
* Test: Variable .OR from symbol
* Feature #5: .OR <symbol>
*-----------------------------------
* Define memory regions
ZP.COMMON  .EQ $80
ZP.TEMP    .EQ $A0
CODE.START .EQ $2000
DATA.START .EQ $3000

* Main code section
        .OR CODE.START
MAIN:   LDA #$00
        JSR SUBROUTINE
        RTS

* Zero-page common variables
        .OR ZP.COMMON
COUNTER    .BS 1
FLAGS      .BS 1
TEMP       .BS 2

* Zero-page temp variables
        .OR ZP.TEMP
SCRATCH    .BS 4
BUFFER     .BS 16

* Data section
        .OR DATA.START
DATA:   .DA $1234
        .HS DEADBEEF

* Back to code section
        .OR CODE.START+$100
SUBROUTINE:
        INC COUNTER
        RTS

        .END
