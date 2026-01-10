; Test basic LOCAL label replacement
; Each macro invocation should get unique label names

  ORG $8000

; Macro with a single LOCAL label
WAIT MACRO
  LOCAL LOOP
LOOP:
  DJNZ LOOP
  RET
ENDM

START:
  ; First invocation - should generate ??0000
  WAIT

  ; Second invocation - should generate ??0001
  WAIT

  ; Third invocation - should generate ??0002
  WAIT

  END START
