; Test REPT with zero repetitions
; Expected: No code generated inside REPT block

  ORG $8000

START:
  DEFB $11
  REPT 0
  DEFB $FF    ; Should NOT appear in output
  ENDR
  DEFB $22

  END
