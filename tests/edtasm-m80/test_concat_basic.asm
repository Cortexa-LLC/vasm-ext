; Test basic & concatenation operator
; Expected: Tokens merged with & removed

  ORG $8000

; Simple concatenation
LAB1&EL:
  NOP

; Multiple concatenation
LA&BE&L2:
  NOP

; Concatenation with whitespace
LABEL & 3:
  NOP

; Concatenation in constants (avoid expression contexts)
VAL&UE1 EQU $12
VAL&UE2 EQU $34

START:
  LD A,VALUE1
  LD B,VALUE2

  END START
