* Test comprehensive macro features
* Tests: ]1-]9 parameters, ]# count, ]] escape, conditionals, private labels
*
* Expected output: 48 bytes
        .OR $8000

DEBUG   .EQ 1

* Test 1: Macro with ]# parameter count
        .MA PARAMCOUNT
        LDA #]#
        STA $00
        .EM

* Test 2: Macro with all 9 parameters
        .MA NINEPARAM
        LDA #]1
        LDX #]2
        LDY #]3
        STA ]4
        STX ]5
        STY ]6
        LDA ]7
        LDX ]8
        LDY ]9
        .EM

* Test 3: Macro with conditional assembly
        .MA DEBUGLOAD
        .DO DEBUG
        NOP             ; Debug marker
        .FIN
        LDA #]1
        STA ]2
        .EM

* Test 4: Macro with ]] escape in comment
        .MA BRACKET
        LDA #]1
* Comment with ]] bracket escape
        STA ]2
        .EM

* Test 5: Macro with private label (single invocation)
        .MA DELAY
        LDX #]1
:1      DEX
        BNE :1
        .EM

* Test 6: Empty macro
        .MA NOOP
        .EM

START   NOP

* Invoke PARAMCOUNT with different parameter counts
        >PARAMCOUNT             ; 0 params
        >PARAMCOUNT $01         ; 1 param
        _PARAMCOUNT $01,$02     ; 2 params (test _ prefix)

* Invoke NINEPARAM with all 9 parameters
        >NINEPARAM $11,$22,$33,$40,$41,$42,#$50,#$51,#$52

* Invoke DEBUGLOAD (conditional inside)
        >DEBUGLOAD $FF,$D020

* Invoke BRACKET (]] escape)
        >BRACKET $AA,$D021

* Invoke DELAY (private label)
        >DELAY $08

* Invoke empty macro
        >NOOP

        RTS
