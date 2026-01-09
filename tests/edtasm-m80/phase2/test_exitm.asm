; Test M80 EXITM directive (Exit macro early)
; Expected: Macro exits before completing all instructions

  ORG $8000

; Test 1: Unconditional EXITM
PARTIAL MACRO
  NOP
  NOP
  EXITM         ; Exit here
  NOP           ; This should never be reached
  NOP
ENDM

  PARTIAL       ; Should only expand: NOP / NOP (stops at EXITM)

  END
