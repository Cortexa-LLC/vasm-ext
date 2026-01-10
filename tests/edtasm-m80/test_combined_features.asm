; Test combined LOCAL labels and & concatenation
; Expected: Both features work together correctly

  ORG $8000

; Macro using both LOCAL and & concatenation
WAIT MACRO
  LOCAL LOOP
#P1&LOOP:
  DJNZ #P1&LOOP
  RET
ENDM

START:
  ; First invocation - creates TEST__L0001
  WAIT TEST_

  ; Second invocation - creates FOO__L0002
  WAIT FOO_

  ; Test & in labels outside macros
RESULT&_OK:
  NOP

  ; Test & inside strings (should be preserved)
  DEFB "A&B"

  END START
