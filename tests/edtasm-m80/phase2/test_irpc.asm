; Test M80 IRPC directive (Iterate through characters)
; Expected: Block repeats for each character in string

  ORG $8000

; Test 1: IRPC with numbers
  IRPC DIGIT,123
  DEFB DIGIT
  ENDR
; Should expand to: DEFB 1 / DEFB 2 / DEFB 3

; Test 2: IRPC with letters
  IRPC CHAR,XYZ
  NOP
  ENDR
; Should expand 3 times: NOP / NOP / NOP

  END
