; Test IRPC with empty string
; Expected: No iterations, no output

  ORG $8000

START:
  DEFB $11
  IRPC C,
  DEFB 'C'
  ENDR
  DEFB $22

  END
