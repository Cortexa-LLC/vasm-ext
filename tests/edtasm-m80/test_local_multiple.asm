; Test multiple LOCAL labels in same macro
; Expected: All LOCAL labels get unique names per invocation

  ORG $8000

; Macro with multiple LOCAL labels
COMPLEX MACRO
  LOCAL LOOP,DONE,SKIP
LOOP:
  DEC A
  JR Z,DONE
  CP #P1
  JR Z,SKIP
  JR LOOP
SKIP:
  INC B
DONE:
  RET
ENDM

START:
  ; First invocation - should generate _L0001,_L0002,_L0003
  LD A,10
  COMPLEX 5

  ; Second invocation - should generate _L0004,_L0005,_L0006
  LD A,20
  COMPLEX 15

  END START
