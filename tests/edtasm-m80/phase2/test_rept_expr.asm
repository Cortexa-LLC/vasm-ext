; Test REPT with expression-based count
; Expected: Expression evaluated, repetition works

  ORG $8000

COUNT EQU 5

  REPT COUNT
  NOP
  ENDR

  REPT 2+3
  NOP
  ENDR

  REPT COUNT*2
  NOP
  ENDR

  END
