* Test SCMASM .OP directive - CPU type selection
        .OR $8000

* Start with default 6502
START   NOP
        LDA #$00
        STA $D020

* Switch to 65C02 for enhanced instructions
        .OP 65C02
        STZ $00         ; 65C02 instruction

* Switch to 65816 for 16-bit operations
        .OP 816
        REP #$30        ; 65816 instruction

        RTS
