; Test & concatenation in labels only
; Expected: Labels are concatenated correctly

  ORG $8000

; Simple concatenation
LAB1&EL:        ; Should become LAB1EL
  NOP

; Multiple concatenation
LA&BE&L2:       ; Should become LABEL2
  NOP

; Concatenation with whitespace
LABEL & 3:      ; Should become LABEL3
  NOP

START:
  ; Jump to concatenated labels to verify they exist
  JP LAB1EL
  JP LABEL2
  JP LABEL3

  END START
