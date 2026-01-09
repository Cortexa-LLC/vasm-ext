; Test M80 REPT directive
; Expected: Blocks repeat N times

  ORG $8000

; Test 1: Simple REPT
  REPT 3
  NOP
  ENDR
; Should expand to: NOP / NOP / NOP

; Test 2: REPT with multiple instructions
  REPT 2
  LD A,$FF
  LD B,$00
  ENDR
; Should expand twice

; Test 3: REPT with count expression
COUNT EQU 4
  REPT COUNT
  INC A
  ENDR
; Should expand 4 times

; Test 4: Nested REPT
  REPT 2
    REPT 3
    DEC B
    ENDR
  ENDR
; Should expand: 2 outer * 3 inner = 6 DEC B instructions

; Test 5: REPT 0 (should not expand)
  REPT 0
  HALT
  ENDR
; Should not generate any code

  END
