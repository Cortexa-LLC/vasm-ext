; Test nested macros with LOCAL labels
; Expected: Each macro level gets unique LOCAL labels

  ORG $8000

; Inner macro with LOCAL label
INNER MACRO
  LOCAL ILOOP
ILOOP:
  DJNZ ILOOP
  RET
ENDM

; Outer macro with LOCAL label that calls inner
OUTER MACRO
  LOCAL OLOOP,ODONE
OLOOP:
  DEC A
  JR Z,ODONE
  INNER
  JR OLOOP
ODONE:
  RET
ENDM

START:
  ; First invocation - OUTER gets _L0001,INNER gets _L0002
  LD A,3
  LD B,5
  OUTER

  ; Second invocation - OUTER gets _L0003,INNER gets _L0004
  LD A,2
  LD B,3
  OUTER

  END START
