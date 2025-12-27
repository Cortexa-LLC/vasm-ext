* Include file for import tests
* Defines constants and macros

* Zero page addresses
ZP_TEMP   EQU   $80
ZP_PTR    EQU   $82
ZP_COUNT  EQU   $84

* System constants
SCREEN    EQU   $0400
COLOR     EQU   $D800

* Utility macro
DELAY     MAC
:LOOP     DEX
          BNE   :LOOP
          <<<
