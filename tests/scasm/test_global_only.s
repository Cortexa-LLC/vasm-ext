* Test global labels only
        .OR $8000
MAIN    LDA #$00
        JSR SUB
        RTS

SUB     LDA #$FF
        RTS
