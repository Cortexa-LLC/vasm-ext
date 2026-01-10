; Test & inside quoted strings (should be preserved)
; Expected: & characters inside strings remain literal

  ORG $8000

START:
  ; & inside strings should NOT be concatenated
  DEFB "A&B&C"      ; Should output: "A&B&C" (5 bytes)
  DEFB 'X&Y'        ; Should output: "X&Y" (3 bytes)

  ; & outside strings SHOULD be concatenated
LABEL&1:
  NOP               ; Label should be "LABEL1"

  END START
