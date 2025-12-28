* Test SCMASM conditionals: .DO, .ELSE, .FIN

        .OR $8000

* Test with .DO TRUE
DEBUG   .EQ 1

        .DO DEBUG
START   LDA #$FF        ; This should be included
        .ELSE
START   LDA #$00        ; This should be excluded
        .FIN

        STA $D020
        RTS

* Test with .DO FALSE
FAST    .EQ 0

        .DO FAST
        NOP             ; Should be excluded
        NOP
        .ELSE
        LDA #$01        ; Should be included
        .FIN

        RTS
