; Test: Core EDTASM Directives
; File: test_directives.asm
; Purpose: Test ORG, EQU, DEFL, END directives
; Expected: 6 bytes output

; Test case 1: ORG sets origin
  ORG 1000H

; Test case 2: EQU defines constant
MYCONST EQU 42

; Test case 3: DEFL creates redefinable symbol
MYVAR DEFL 100

; Test case 4: Use EQU constant
START:
  DEFB 1,2,3
  DEFB MYCONST

; Test case 5: Use and redefine DEFL
  DEFB MYVAR
MYVAR DEFL 200  ; Redefine MYVAR
  DEFB MYVAR

; Test case 6: END directive
  END START

; Expected binary output (6 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  01 02 03           DEFB 1,2,3
; $0003:  2A                 DEFB MYCONST (42 = 2AH)
; $0004:  64                 DEFB MYVAR (100 = 64H)
; $0005:  C8                 DEFB MYVAR (200 = C8H)
;
; Total: 6 bytes
