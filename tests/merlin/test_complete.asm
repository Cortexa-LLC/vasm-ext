* Complete Directive Coverage Test
* Tests all remaining directives for 100% coverage

        ORG   $8000

* Test TYP - ProDOS file type
        TYP   $F1            ; Merlin source type

* Test AUX - ProDOS auxiliary type
        AUX   $2000          ; Load address

* Test TR - Truncate addressing mode
        TR    ON             ; Enable truncation

START   NOP

* Test TR modes
        TR    OFF            ; Disable truncation
        TR    ADR            ; Address mode truncation only
        TR    ON             ; Back to ON

* Test EXP - Expand macros in listing
        EXP   ON             ; Expand macros

* Define test macro
TestExp MAC
        LDA   #]1
        STA   ]2
        <<<

* Test OBJ - Object code listing control
        OBJ   ON             ; Show object code

* Test macro expansion
        >>> TestExp $42,$10

* Test CYC - Cycle counting
        CYC   ON             ; Enable cycle counting

        LDA   $10            ; 3 cycles
        STA   $20            ; 3 cycles
        INX                  ; 2 cycles

        CYC   OFF            ; Disable cycle counting

* Test KND - OMF segment kind
        KND   $0000          ; Code segment

* Test ALI - OMF alignment
        ALI   PAGE           ; Page alignment
        NOP
        ALI   NONE           ; No alignment
        NOP

* Test LNK - Linker script generation (rare)
        LNK   MYLINK

* Test conditional ERR
]DEBUG  EQU   0

        DO    ]DEBUG
        ERR   "Debug mode error test"
        FIN

* Test EXP modes
        EXP   OFF            ; Don't expand

* Test OBJ modes
        OBJ   OFF            ; Hide object code
        OBJ   ON             ; Show object code

* Test data reference
MYDATA  DA    START
        DW    $1234

        END
