* Test 65C02 CPU features
* Demonstrates 65C02 extensions: BRA, PHX, PHY, STZ, etc.

        XC    65c02       ; Enable 65C02 mode
        ORG   $8000

* 65C02 stack instructions
START   PHX               ; Push X (65C02)
        PHY               ; Push Y (65C02)
        PLY               ; Pull Y (65C02)
        PLX               ; Pull X (65C02)

* Store zero (65C02 feature)
        STZ   $00         ; Zero page
        STZ   $1234       ; Absolute
        STZ   $00,X       ; Zero page,X
        STZ   $1234,X     ; Absolute,X

* Branch always (65C02)
        BRA   NEXT        ; Unconditional branch

* Indirect addressing without X
NEXT    LDA   ($12)       ; (ZP) indirect (65C02)
        STA   ($34)       ; (ZP) indirect (65C02)

* Bit immediate (65C02)
        BIT   #$80        ; Test bit immediate

* Test BRA with variable labels
]LOOP   EQU   5
        LDX   #]LOOP
:WAIT   DEX
        BNE   :WAIT
        BRA   DONE        ; Branch to end

DONE    RTS
        END
