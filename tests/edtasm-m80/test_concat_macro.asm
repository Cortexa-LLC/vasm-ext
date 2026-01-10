; Test & concatenation with macro parameters
; Expected: Parameter values concatenated with suffixes/prefixes

  ORG $8000

; Macro that uses & concatenation with parameters
DEFLAB MACRO
  LOCAL LOOP
#P1&LOOP:
  DJNZ #P1&LOOP
  RET
ENDM

START:
  ; First invocation - creates TEST_L0001
  DEFLAB TEST_

  ; Second invocation - creates FOO_L0002
  DEFLAB FOO_

  END START
