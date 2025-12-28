* Test macro parameters in SCMASM
        .OR $8000

* Test 1: Macro with single parameter (]1)
        .MA LOADVAL
        LDA #]1
        .EM

        >LOADVAL $42
        STA $00

* Test 2: Macro with two parameters (]1, ]2)
        .MA STORE16
        LDA #<]1
        STA ]2
        LDA #>]1
        STA ]2+1
        .EM

        >STORE16 $1234,$20

* Test 3: Macro with three parameters
        .MA ADD3
        CLC
        LDA ]1
        ADC ]2
        ADC ]3
        .EM

        >ADD3 $10,$20,$30
        STA $40

* Test 4: Parameter count ]#
        .MA SHOWCOUNT
        LDA #]#
        .EM

        >SHOWCOUNT $01,$02,$03

* Test 5: Escape sequence ]] for single ]
        .MA BRACKETS
        LDA #']]'
        .EM

        >BRACKETS
        STA $50

* Test 6: Multiple invocations with different parameters
        >LOADVAL $FF
        >LOADVAL $00
        >LOADVAL $55

        RTS
