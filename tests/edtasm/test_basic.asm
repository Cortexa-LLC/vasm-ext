* Test EDTASM basic directives and features
* Asterisk in column 1 is a comment

        ORG     $1000

* Constants and labels (case-insensitive)
CONST1  EQU     42
const2  equ     $FF

* Section test
        SECTION CODE

* Data directives
START   FCB     1,2,3           ; Form Constant Byte
        FDB     $1234,$5678     ; Form Double Byte
MSG     FCC     /Hello, World!/ ; Form Constant Characters
BUF     RMB     16              ; Reserve Memory Bytes

* Instructions (6809)
LOOP    LDA     #CONST1
        LDB     #const2
        STA     BUF
        BRA     LOOP

* Conditional assembly
        COND    1=1
        FCB     $AA             ; This should assemble
        ENDC

        IFEQ    0
        FCB     $BB             ; This should NOT assemble
        ENDIF

* Macro test
DELAY   MACRO
        NOP
        NOP
        ENDM

        DELAY                   ; Invoke macro

* End of program
        END     START
