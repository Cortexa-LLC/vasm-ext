; Test forward and backward references to LOCAL labels
; Expected: All references correctly resolve to unique labels

  ORG $8000

; Macro with forward reference (JR to label defined later)
FWDREF MACRO
  LOCAL START,END
  JR START        ; Forward reference
  DEFB $FF
START:
  NOP
  JR END          ; Forward reference
  DEFB $EE
END:
  RET
ENDM

; Macro with backward reference (loop)
BCKREF MACRO
  LOCAL LOOP
LOOP:
  DEC A           ; Backward reference target
  JR NZ,LOOP     ; Backward reference
  RET
ENDM

ENTRY:
  ; Test forward references
  FWDREF
  FWDREF

  ; Test backward references
  LD A,5
  BCKREF
  LD A,3
  BCKREF

  END ENTRY
