* Test simple macros in SCMASM
        .OR $8000

* Test 1: Simple macro without parameters
        .MA CLEARACC
        LDA #$00
        .EM

* Test 2: Invoke with > prefix
        >CLEARACC
        STA $D020

* Test 3: Invoke with _ prefix
        _CLEARACC
        STA $D021

* Test 4: Macro with single instruction
        .MA STOREZERO
        STA $00
        .EM

        >STOREZERO

* Test 5: Macro with multiple instructions
        .MA INIT
        LDA #$00
        TAX
        TAY
        .EM

        _INIT

        RTS
