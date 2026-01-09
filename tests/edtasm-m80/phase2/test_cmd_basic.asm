; Test TRS-DOS /CMD output format
; Single segment test

  ORG $8000

START:
  LD A,$55        ; Load test value
  LD HL,$1234     ; Load test address
  RET             ; Return

  END START
