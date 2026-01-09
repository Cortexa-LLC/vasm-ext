; Test EDTASM-M80 unique macro ID (#$YM)
; Expected: Each invocation gets unique label

  ORG $8000

; Test 1: Basic unique ID
DELAY MACRO
LOOP#$YM:
  DEC A
  JR NZ,LOOP#$YM
ENDM

  LD A,10
  DELAY               ; LOOP_000001
  LD A,20
  DELAY               ; LOOP_000002 (different!)

; Test 2: Multiple unique labels per macro
WAITKEY MACRO
WAIT#$YM:
  CALL $0049
  OR A
  JR Z,WAIT#$YM
DONE#$YM:
  RET
ENDM

  WAITKEY             ; WAIT_000003, DONE_000003
  WAITKEY             ; WAIT_000004, DONE_000004 (all different!)

; Test 3: Case-insensitive #$YM
TESTYM MACRO
LAB#$YM:
  NOP
LAB#$ym:            ; Lowercase should work same as uppercase
  NOP
ENDM

; This should cause error: both expand to same label
; TESTYM            ; Would get: "label redefined" error

; Test 4: Nested macro invocations
OUTER MACRO
OUTER#$YM:
  DELAY             ; Inner macro gets its own unique ID
  JP OUTER#$YM
ENDM

  LD A,5
  OUTER               ; OUTER_000005, plus DELAY's LOOP_000006

  END
