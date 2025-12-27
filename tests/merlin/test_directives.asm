* Test Merlin directives
* Comprehensive coverage of data, string, and control directives

        ORG   $8000

* Data directives
START   DFB   $12,$34,$56          ; Define bytes
        DA    $1234                ; Define address (16-bit)
        DW    $5678                ; Define word (alias)
        DS    8                    ; Define space

* String directives
STR1    ASC   "NORMAL"             ; Normal ASCII
STR2    ASC   'HIGHBIT'            ; High bit set
STR3    DCI   "INVERTED"           ; Last char inverted
STR4    INV   "ALLHIGH"            ; All high bit set
STR5    FLS   "FLASH"              ; Flashing
STR6    REV   "REVERSE"            ; Reversed
STR7    STR   "LENPREFIX"          ; 1-byte length prefix
STR8    STRL  "LONGLEN"            ; 2-byte length prefix
STR9    HEX   0102030405           ; Hex bytes

* Symbol definition
VALUE1  EQU   100
VALUE2  =     200                  ; Alternate syntax

* Conditional assembly
        DO    1
        NOP                        ; Assembled
        FIN

        DO    0
        BRK                        ; Not assembled
        FIN

        DO    1
        LDA   #$01
        ELSE
        LDA   #$02                 ; Not assembled
        FIN

* Loop directive (REPT)
        REPT  3
        NOP                        ; Repeated 3 times
        ENDR

* Output control directives
        REL                        ; Relocatable mode
        LST   OFF                  ; Listing off

        RTS
        END
