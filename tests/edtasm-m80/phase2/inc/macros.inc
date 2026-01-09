; Common macros for TRS-80

; Print a character
PUTCH MACRO
  LD A,#P1
  CALL PUTCHAR
ENDM

; Print a string (address in HL)
PRTSTR MACRO
LOOP#$YM:
  LD A,(HL)
  OR A
  RET Z
  CALL PUTCHAR
  INC HL
  JR LOOP#$YM
ENDM

; Clear screen
CLS MACRO
  CALL CLEAR
ENDM
