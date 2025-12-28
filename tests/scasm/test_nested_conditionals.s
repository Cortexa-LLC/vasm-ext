* Test deeply nested .DO/.ELSE/.FIN conditionals
* SCMASM supports up to 63 levels of nesting
* This test verifies nested conditionals work correctly
*
* Expected output: 26 bytes
* Tests various nesting depths with different conditions
        .OR $8000

* Test constants
LEVEL1  .EQ 1
LEVEL2  .EQ 1
LEVEL3  .EQ 0
LEVEL4  .EQ 1
LEVEL5  .EQ 0

* Test 1: Simple nesting (3 levels)
        .DO LEVEL1
        LDA #$01        ; Included
        .DO LEVEL2
        LDA #$02        ; Included
        .DO LEVEL3
        LDA #$03        ; Excluded (LEVEL3=0)
        .ELSE
        LDA #$04        ; Included
        .FIN
        .FIN
        .FIN

* Test 2: Deep nesting with alternating conditions (10 levels)
        .DO LEVEL1              ; Level 1 - TRUE
        LDX #$10
        .DO LEVEL2              ; Level 2 - TRUE
        LDY #$20
        .DO LEVEL3              ; Level 3 - FALSE
        NOP
        .ELSE
        INX                     ; Included
        .DO LEVEL4              ; Level 4 - TRUE (inside ELSE)
        INY
        .DO LEVEL5              ; Level 5 - FALSE
        NOP
        .ELSE
        DEX                     ; Included
        .DO LEVEL1              ; Level 6 - TRUE
        DEY
        .DO LEVEL2              ; Level 7 - TRUE
        TXA
        .DO LEVEL3              ; Level 8 - FALSE
        NOP
        .ELSE
        TYA                     ; Included
        .DO LEVEL4              ; Level 9 - TRUE
        STA $00
        .DO LEVEL5              ; Level 10 - FALSE
        NOP
        .ELSE
        STX $01                 ; Included
        .FIN                    ; End level 10
        .FIN                    ; End level 9
        .FIN                    ; End level 8
        .FIN                    ; End level 7
        .FIN                    ; End level 6
        .FIN                    ; End level 5
        .FIN                    ; End level 4
        .FIN                    ; End level 3
        .FIN                    ; End level 2
        .FIN                    ; End level 1

* Test 3: All false path (verify ELSE works at each level)
        .DO LEVEL3              ; FALSE
        NOP
        .ELSE
        LDA #$FF                ; Included
        .DO LEVEL5              ; FALSE (inside ELSE)
        NOP
        .ELSE
        STA $D020               ; Included
        .FIN
        .FIN

        RTS

* Expected hex dump:
* a9 01 a9 02 a9 04 a2 10 a0 20 e8 c8 ca c0 98 8a
* 98 85 00 86 01 a9 ff 8d 20 d0 60
