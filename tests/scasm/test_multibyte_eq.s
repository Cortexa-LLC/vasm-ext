*-----------------------------------
* Test: Multi-byte .EQ constants
* Feature #3: .EQ $36,37 syntax
*-----------------------------------
        .OR $2000

* Multi-byte .EQ for 16-bit zero-page pointers
HOOK.OUT   .EQ $36,37
HOOK.IN    .EQ $38,39
PTR1       .EQ $40,41
PTR2       .EQ $42,43

* Access both bytes
        LDA HOOK.OUT     ; Load low byte ($36)
        LDA HOOK.OUT+1   ; Load high byte ($37)
        STA HOOK.IN      ; Store to low byte ($38)
        STA HOOK.IN+1    ; Store to high byte ($39)

* Use in expressions
        LDA #<PTR1       ; Low byte
        LDA #>PTR1       ; High byte (should be 0)

* Store addresses in data
        .DA HOOK.OUT     ; Should produce $36
        .DA HOOK.IN      ; Should produce $38

        .END
