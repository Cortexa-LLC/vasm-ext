; Test basic INCLUDE directive
; Expected: Symbols from common.inc are defined and usable
; Run with: vasmz80_edtasm-m80 -Itests/edtasm-m80/phase2/inc ...

  ORG $8000

  INCLUDE "common.inc"

START:
  LD A,'X'
  CALL PUTCHAR      ; Should resolve to $0033
  CALL GETCHAR      ; Should resolve to $0049
  JP CLEAR          ; Should resolve to $01C9

  DEFB EOF          ; Should be $FF

  END START
