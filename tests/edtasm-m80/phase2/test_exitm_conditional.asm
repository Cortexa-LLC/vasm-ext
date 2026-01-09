; Test EXITM with conditional assembly
; Expected: Macro exits early based on condition

  ORG $8000

; Macro with conditional EXITM
CHECK MACRO
  DEFB #P1
  IF #P1>5
  EXITM
  ENDIF
  DEFB $FF    ; Only generated if P1 <= 5
ENDM

  CHECK 3     ; Should generate: 03 FF
  CHECK 7     ; Should generate: 07 (no FF)

  END
