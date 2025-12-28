* Test complex label scoping interactions
* Tests global, private (:N), and local (.N) labels in various scenarios
*
* Expected output: 148 bytes
        .OR $8000

* Test 1: Basic three-tier scoping
FUNC1   LDA #$00        ; Global label
.1      INX             ; Local .1 scoped to FUNC1
        BNE .1          ; Reference local .1

:1      DEX             ; Private :1 in FUNC1 context
        BNE :1          ; Reference private :1

FUNC2   LDA #$01        ; New global label, starts new scopes
.1      INY             ; Different local .1, scoped to FUNC2
        BNE .1          ; Reference local .1 (different from FUNC1's .1)

:1      DEY             ; Different private :1, scoped to FUNC2 context
        BNE :1          ; Reference private :1 (different from FUNC1's :1)

* Test 2: Multiple local labels in same function
MAIN    LDX #$00        ; Global label
.1      INX             ; Local .1
        CPX #$05
        BNE .1

.2      DEX             ; Local .2 (same function)
        BNE .2

.3      INX             ; Local .3
        CPX #$10
        BNE .3

        RTS

* Test 3: Multiple private labels in same context
SORT    LDY #$00        ; Global label - increments private label context
:1      LDA ($00),Y     ; Private :1
        BEQ :3          ; Forward reference to :3

:2      INY             ; Private :2
        CPY #$10
        BNE :1          ; Back to :1

:3      RTS             ; Private :3

* Test 4: Mixing all three label types
PROCESS LDA #$FF        ; Global
.1      STA $00         ; Local .1
:1      DEC $00         ; Private :1
        BNE :1
        CPX #$00
        BEQ .2          ; Forward reference to local .2

.2      INX             ; Local .2
        CPX #$05
        BNE .1          ; Back to local .1
        RTS

* Test 5: Complex forward and backward references
LOOP    LDX #$00        ; Global
.1      LDA #$00        ; Local .1
        BNE .2          ; Forward to local .2 (not yet defined)

:1      INX             ; Private :1
        CPX #$03
        BNE :2          ; Forward to private :2

.2      LDA #$01        ; Local .2 (target of earlier forward ref)
        BNE .3          ; Forward to local .3

:2      DEX             ; Private :2 (target of earlier forward ref)
        BNE :1          ; Back to private :1

.3      RTS             ; Local .3

* Test 6: Labels with same numbers across contexts
* Global A has .1, :1
* Global B has .1, :1 (different symbols)
CONTEXTA LDX #$00       ; Global A
.1      INX             ; Local .1 in CONTEXTA
        CPX #$05
        BNE .1

:1      DEX             ; Private :1 in CONTEXTA
        BNE :1

        JSR CONTEXTB

CONTEXTB LDY #$00       ; Global B
.1      INY             ; Different local .1 in CONTEXTB
        CPY #$03
        BNE .1

:1      DEY             ; Different private :1 in CONTEXTB
        BNE :1

        RTS

* Test 7: Global label referencing
        JSR FUNC1       ; Can reference global from anywhere
        JSR FUNC2
        JSR MAIN
        JSR SORT

* Test 8: Deeply nested local labels
DEEP    LDX #$00        ; Global
.1      INX
        BNE .2
.2      INX
        BNE .3
.3      INX
        BNE .4
.4      INX
        BNE .5
.5      INX
        BNE .6
.6      INX
        BNE .7
.7      INX
        BNE .8
.8      INX
        BNE .9
.9      INX
        BNE .10
.10     RTS

END     RTS
