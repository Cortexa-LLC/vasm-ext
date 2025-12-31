* Comprehensive test: All three label types together
        .OR $8000

* Global label FUNC1 with private labels
FUNC1   LDX #$00
:1      INX             ; Private :1 in FUNC1 context
        CPX #$05
        BNE :1
:2      RTS             ; Private :2 in FUNC1 context

* Global label FUNC2 with same private label numbers (different context)
FUNC2   LDX #$10
:1      DEX             ; Private :1 in FUNC2 context (different from FUNC1's :1)
        CPX #$0B
        BNE :1
:2      RTS             ; Private :2 in FUNC2 context (different from FUNC1's :2)

* Global label with local labels
MAIN    JSR FUNC1
        JSR FUNC2
        LDY #$00
.1      INY             ; Local .1 scoped to MAIN
        CPY #$03
        BNE .1
        RTS

* Another global with local labels
LOOP    LDX #$08
.1      DEX             ; Local .1 scoped to LOOP (different from MAIN's .1)
        BNE .1
        RTS
