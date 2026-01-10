; Test multiple invocations of macro with LOCAL labels
; Expected: Each invocation gets unique label names (no conflicts)

  ORG $8000

; Simple macro with LOCAL label
DELAY MACRO
  LOCAL WAIT
WAIT:
  DEC HL
  LD A,H
  OR L
  JR NZ,WAIT
  RET
ENDM

START:
  ; Invocation 1 - should generate _L0001
  LD HL,1000
  DELAY

  ; Invocation 2 - should generate _L0002
  LD HL,2000
  DELAY

  ; Invocation 3 - should generate _L0003
  LD HL,3000
  DELAY

  ; Invocation 4 - should generate _L0004
  LD HL,4000
  DELAY

  ; Invocation 5 - should generate _L0005
  LD HL,5000
  DELAY

  END START
