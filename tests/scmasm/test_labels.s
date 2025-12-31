* Test three-tier label system in SCMASM
* Global labels, :N private labels, .N local labels

        .OR $8000

* Test 1: Global labels
MAIN    LDA #$00
        STA $D020
        JSR SUBRTN
        RTS

* Test 2: Local labels scoped to global label
SUBRTN  LDX #$10        ; Global label establishes scope
.1      DEX             ; Local label .1 scoped to SUBRTN
        BNE .1          ; Branch to SUBRTN.1
        RTS

* Test 3: Private labels with different context
FUNC1   LDA #$00        ; New global - increments private context
:1      CMP #$05        ; Private label :1 in FUNC1 context
        BEQ :2          ; Branch to :2 in same context
        ADC #$01
        JMP :1          ; Loop back to :1
:2      RTS             ; Private label :2

FUNC2   LDA #$10        ; New global - increments private context again
:1      CMP #$15        ; Private label :1 in FUNC2 context (different from FUNC1)
        BEQ :2          ; Branch to :2 in FUNC2 context
        ADC #$01
        JMP :1          ; Loop back to FUNC2's :1
:2      RTS             ; Private label :2 in FUNC2 context

* Test 4: Mixing all three types
COMPLEX LDX #$00        ; Global label
.1      LDA TABLE,X     ; Local label .1
:1      CMP #$FF        ; Private label :1
        BEQ :2          ; Branch to private :2
        INX
        CPX #$10
        BNE .1          ; Branch back to local .1
:2      RTS             ; Private label :2

TABLE   .DA $01,$02,$03,$FF

* Test 5: Multiple local labels in same scope
COUNT   LDY #$00        ; Global label
.1      INY             ; Local .1
        CPY #$05
        BNE .1          ; Branch to .1
.2      DEY             ; Local .2 (different from .1)
        BNE .2          ; Branch to .2
        RTS
