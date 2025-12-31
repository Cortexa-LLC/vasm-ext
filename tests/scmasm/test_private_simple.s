* Test private labels - simple version
        .OR $8000

FUNC1   LDX #$00
:1      INX
        CPX #$05
        BNE :1
        RTS
