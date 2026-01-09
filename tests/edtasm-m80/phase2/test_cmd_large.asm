; Test /CMD format with larger program
; Expected: Single large load block + transfer block

  ORG $8000

START:
  LD HL,DATA
  LD B,10
LOOP:
  LD A,(HL)
  INC HL
  DJNZ LOOP
  RET

DATA:
  DEFB "HELLO WORLD"
  DEFB 0

BUFFER:
  DEFS 64       ; 64 bytes of space

  END START
