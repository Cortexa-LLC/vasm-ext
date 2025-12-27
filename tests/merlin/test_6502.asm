* Test basic 6502 CPU features
* Standard 6502 instruction set with Merlin syntax

        ORG   $8000

* Variable labels - Merlin mutable values
]COUNT  EQU   10
]ADDR   EQU   $C000

* Global label
START   LDA   #]COUNT     ; Variable label in expression
        STA   ]ADDR       ; Variable label as address

* Redefine variable label
]COUNT  EQU   20
        LDX   #]COUNT     ; Now 20

* Local labels - scoped to global
LOOP1   LDA   #$00
:INNER  INX
        DEX
        BNE   :INNER      ; Local label reference

* New global = new local scope
LOOP2   LDY   #$00
:INNER  INY               ; Different :INNER
        DEY
        BNE   :INNER

* Macro with parameter expansion
DELAY   MAC
        LDX   #]1         ; ]1 = parameter
:WAIT   DEX
        BNE   :WAIT
        LDX   #]0         ; ]0 = count (should be 1)
        <<<

* Test macro invocation
        DELAY 255

* Data directives
MSG     ASC   "HELLO"     ; ASCII string
        DCI   "WORLD"     ; Last char inverted
        HEX   0D0A        ; Hex bytes
        DFB   $12,$34     ; Define bytes
        DA    START       ; Define address
        DS    16          ; Define space

* Conditional assembly
        DO    1
        NOP
        FIN

        RTS
        END
