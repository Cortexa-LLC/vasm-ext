; Test macros and REPT in same file (not combined)
; Note: Macro parameters inside REPT blocks is a known limitation
; Expected: Both features work independently

  ORG $8000

; Macro without REPT
DOUBLE MACRO
  DEFB #P1
  DEFB #P1
ENDM

  DOUBLE $42    ; 2 bytes of $42

; REPT outside macro
  REPT 3
  NOP
  ENDR

; Macro that generates code, then REPT after
SETUP MACRO
  LD A,#P1
ENDM

  SETUP $55
  REPT 2
  INC A
  ENDR

  END
