* Test: .OR directive with symbol defined as .EQ *
* SCASM allows symbols to be defined as current PC and used in .OR
* This tests the real-world pattern:
*   1. Save address with .EQ *
*   2. Switch to different section
*   3. Return to saved address with .OR

        .OR $2000
ZP.COMMON  .EQ *        ; Save $2000 as ZP.COMMON

* Immediately switch to different section (don't emit code at $2000 yet)
        .OR $8000
        NOP
        NOP
        LDX #$42

* Now return to zero page at saved address
        .OR ZP.COMMON
        LDA #0
        STA $00

* Test forward reference too
        .OR $9000
CODE.START .EQ *        ; Save $9000
        RTS

        .OR $A000
        JSR CODE.START  ; Use the saved address

        .END
