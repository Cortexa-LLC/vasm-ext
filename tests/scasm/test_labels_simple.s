* Test three-tier label system - Simple test
        .OR $8000

* Test 1: Global labels only
MAIN    LDA #$00
        JSR SUB
        RTS

SUB     LDA #$FF
        RTS

* Test 2: Local labels
LOOP    LDX #$10
.1      DEX
        CPX #$00
        BNE .1
        RTS
