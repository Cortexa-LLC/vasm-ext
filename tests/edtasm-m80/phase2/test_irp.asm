; Test M80 IRP directive (Iterate through list)
; Expected: Block repeats for each value in list

  ORG $8000

; Test 1: Simple IRP with constant
  IRP VAL,1,2,3
  DEFB VAL
  ENDR
; Should expand to: DEFB 1 / DEFB 2 / DEFB 3

; Test 2: IRP with hex values
  IRP ADDR,$10,$20,$30
  DEFB ADDR
  ENDR
; Should expand to: DEFB $10 / DEFB $20 / DEFB $30

  END
