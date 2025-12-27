* Test Apple IIgs 65816 CPU features
* Demonstrates MX directive, 65816 instructions, and bank byte operator

        MX    %00          ; 16-bit accumulator and index registers
        ORG   $8000

* 65816-specific stack instructions
START   PHK              ; Push program bank
        PLB              ; Pull data bank
        PHD              ; Push direct page
        PLD              ; Pull direct page

* Push effective address (16-bit immediate)
        PEA   $1234      ; Push word
        PEA   ^START     ; Push bank byte of START

* 16-bit immediate mode (enabled by MX %00)
        LDA   #$1234     ; 16-bit load
        LDX   #$5678     ; 16-bit index load

* Switch to 8-bit modes
        MX    %11        ; 8-bit accumulator and index
        LDA   #$12       ; 8-bit load
        LDX   #$34       ; 8-bit index load

        RTL              ; Return long (65816)

        END
