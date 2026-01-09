; Test: Basic Conditional Assembly
; File: test_conditionals.asm
; Purpose: Test COND/ENDC directives
; Expected: 3 bytes output

  ORG 1000H

; Test case 1: COND with true expression includes code
  COND 1
  DEFB 1
  ENDC

; Test case 2: COND with false expression excludes code
  COND 0
  DEFB 99H  ; This should NOT be assembled
  ENDC

; Test case 3: COND with expression evaluation
  COND 2+2
  DEFB 2
  ENDC

; Test case 4: Nested COND (2 levels)
  COND 1
    COND 1
      DEFB 3
    ENDC
  ENDC

; Test case 5: Nested COND with false inner
  COND 1
    COND 0
      DEFB 99H  ; This should NOT be assembled
    ENDC
  ENDC

; Test case 6: Empty COND block
  COND 0
  ; Nothing here
  ENDC

  END

; Expected binary output (3 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  01                 Inside COND 1
; $0001:  02                 Inside COND 2+2
; $0002:  03                 Inside nested COND
;
; Total: 3 bytes
