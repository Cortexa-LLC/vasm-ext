; Test nested INCLUDE directives
; Expected: trs80.inc includes common.inc, all symbols available
; Run with: vasmz80_edtasm-m80 -Itests/edtasm-m80/phase2/inc ...

  ORG $8000

  INCLUDE "trs80.inc"     ; This includes common.inc

START:
  ; Use symbols from common.inc (via trs80.inc)
  LD A,'H'
  CALL PUTCHAR            ; From common.inc

  ; Use symbols from trs80.inc
  LD HL,SCREEN            ; From trs80.inc
  LD (HL),A

  LD HL,KBDBUF            ; From trs80.inc
  LD A,(HL)

  RET

  END START
