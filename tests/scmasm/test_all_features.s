*-----------------------------------
* Test: All 5 new SCASM features together
* Comprehensive integration test
*-----------------------------------
* Feature #5: Variable .OR
ZP.BASE .EQ $80

        .OR ZP.BASE
* Feature #3: Multi-byte .EQ
HOOK.OUT .EQ $36,37
HOOK.IN  .EQ $38,39

        .OR $2000
START:
* Feature #1: XOR in expressions
        LDA #$BE^$A5           ; XOR in immediate
        .DA SCI.STARTUP,#$BE^$A5  ; XOR in .DA with # prefix

* Feature #2: .AS with delimiters
        .AS /Plain text message/
        .AS -/Explicit plain ASCII/
        .AS "High-bit message"

* Use multi-byte .EQ symbols
        LDA HOOK.OUT
        LDA HOOK.OUT+1
        STA HOOK.IN
        STA HOOK.IN+1

* Feature #4: Page alignment
        .PG

ALIGNED:
        NOP
        .DA ALIGNED      ; Should be on page boundary

* Combined features
TABLE:  .AS -/ABCDEFGH/
        .DA #$FF^$AA     ; XOR in .DA

SCI.STARTUP .EQ $1234

        .END
