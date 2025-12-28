* Test .PH (phase) and .EP (end phase) directives
* Phase assembly allows code to assemble at one address
* but execute at another (useful for code relocation)
        .OR $8000

* Test 1: Simple phase assembly
START   LDA #$00
        STA $D020

* Begin phase - assemble as if at $C000 but actually at $8000+offset
        .PH $C000
PHASED  LDA #$FF        ; This will be at $C000 in phase
        STA $D021
        LDX #$00
:1      DEX
        BNE :1
        .EP             ; End phase

* Test 2: Code after phase returns to normal
AFTER   LDA #$42
        RTS

* Test 3: Nested phases not typically supported, but test single phase
        .PH $D000
CODE2   NOP
        NOP
        .EP

        RTS
