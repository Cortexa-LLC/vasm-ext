; Test REPT and IRP in same file (not nested)
; Expected: Both work independently

  ORG $8000

; REPT block
  REPT 3
  NOP
  ENDR

; IRP block
  IRP REG,A,B,C
  LD REG,0
  ENDR

; REPT with expression
  REPT 2+1
  DEFB $FF
  ENDR

  END
