* Test both > and _ macro invocation styles
* SCMASM 3.0 introduced _ prefix (older style used >)
        .OR $8000

* Define several macros for testing
        .MA MACRO1
        NOP
        .EM

        .MA MACRO2
        LDA #]1
        STA ]2
        .EM

        .MA MACRO3
        LDX #$00
        LDY #$00
        .EM

* Test 1: Invoke MACRO1 with > (traditional)
        >MACRO1

* Test 2: Invoke MACRO1 with _ (SCMASM 3.0 style)
        _MACRO1

* Test 3: Invoke MACRO2 with > and parameters
        >MACRO2 $42,$D020

* Test 4: Invoke MACRO2 with _ and parameters
        _MACRO2 $FF,$D021

* Test 5: Invoke MACRO3 with >
        >MACRO3

* Test 6: Invoke MACRO3 with _
        _MACRO3

* Test 7: Mixed invocations in sequence
        >MACRO1
        _MACRO1
        >MACRO3
        _MACRO3
        >MACRO2 $AA,$00
        _MACRO2 $55,$01

        RTS
