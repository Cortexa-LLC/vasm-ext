; Test ASEG/CSEG/DSEG with TRS-DOS /CMD output
; ASEG = Absolute segment (fixed address)
; CSEG = Code segment (relocatable, but we use absolute for /CMD)
; DSEG = Data segment (relocatable, but we use absolute for /CMD)

; Code segment at $8000
  CSEG
  ORG $8000
START:
  LD HL,DATA
  LD A,(HL)
  RET

; Data segment at $9000
  DSEG
  ORG $9000
DATA:
  DEFB $42,$43

; Another code segment at $A000
  CSEG
  ORG $A000
FUNC2:
  XOR A
  RET

  END START
