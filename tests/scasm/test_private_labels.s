* Test private labels (:N) in SCMASM
        .OR $8000

* Test: Private labels in different contexts
FUNC1   LDA #$00
:1      CMP #$05
        BEQ :2
        ADC #$01
        JMP :1
:2      RTS

FUNC2   LDA #$10
:1      CMP #$15
        BEQ :2
        ADC #$01
        JMP :1
:2      RTS
