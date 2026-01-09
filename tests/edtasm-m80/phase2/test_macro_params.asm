; Test EDTASM-M80 macro parameter substitution (#P1-#P9)
; Expected: All macros expand correctly with parameters

  ORG $8000

; Test 1: Single parameter
PUTCH MACRO
  LD A,#P1
  CALL $0033
ENDM

  PUTCH 'H'           ; Should expand: LD A,'H' / CALL $0033
  PUTCH $FF           ; Should expand: LD A,$FF / CALL $0033

; Test 2: Multiple parameters (2)
LDXY MACRO
  LD A,#P1
  LD (#P2),A
ENDM

  LDXY $42,$4000      ; Should expand: LD A,$42 / LD ($4000),A

; Test 3: Multiple parameters (3)
LDXYZ MACRO
  LD A,#P1
  LD B,#P2
  LD C,#P3
ENDM

  LDXYZ 1,2,3         ; Should expand: LD A,1 / LD B,2 / LD C,3

; Test 4: Nine parameters (maximum)
MAX9 MACRO
  LD A,#P1
  LD B,#P2
  LD C,#P3
  LD D,#P4
  LD E,#P5
  LD H,#P6
  LD L,#P7
  ; #P8 and #P9 in expressions
  DEFB #P8,#P9
ENDM

  MAX9 1,2,3,4,5,6,7,8,9

; Test 5: Case-insensitive parameters
TESTCASE MACRO
  LD A,#P1
  LD B,#p2          ; Lowercase 'p'
  LD C,#P3
ENDM

  TESTCASE $AA,$BB,$CC

  END
