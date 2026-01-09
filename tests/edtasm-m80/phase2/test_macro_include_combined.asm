; Test macros + includes combined
; Expected: Macros from include file work with #P1 and #$YM
; Run with: vasmz80_edtasm-m80 -Itests/edtasm-m80/phase2/inc ...

  ORG $8000

  INCLUDE "trs80.inc"     ; Defines PUTCHAR, CLEAR, etc.
  INCLUDE "macros.inc"    ; Defines PUTCH, PRTSTR, CLS macros

START:
  CLS                     ; Macro using CLEAR from trs80.inc

  PUTCH 'H'               ; Macro with #P1 parameter
  PUTCH 'i'
  PUTCH '!'

  LD HL,MSG
  PRTSTR                  ; Macro with #$YM unique labels

  RET

MSG:
  DEFM "Hello"
  DEFB 0

  END START
