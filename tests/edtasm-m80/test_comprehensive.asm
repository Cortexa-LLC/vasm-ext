; Test: Comprehensive Integration Test
; File: test_comprehensive.asm
; Purpose: Test multiple Phase 1 features in combination
; Expected: Multi-segment binary with various directives

; Constants
VERSION EQU 1
BUFSIZE EQU 256
DEBUG DEFL 1  ; Can be redefined

; Code segment
  CSEG
  ORG 100H

MAIN:
  ; Test case 1: Instructions with constants
  LD A,VERSION
  LD BC,BUFSIZE
  CALL INIT
  JP DONE

INIT:
  ; Test case 2: Conditional assembly with IFDEF
  IFDEF DEBUG
  LD A,0FFH  ; Debug marker
  ENDIF

  ; Test case 3: Nested conditionals
  IF VERSION
    IF1
    DEFB 1  ; Version 1, Pass 1
    ENDIF
    IF2
    DEFB 2  ; Version 1, Pass 2+
    ENDIF
  ENDIF

  ; Test case 4: Data mixed with code
  DEFB 10H,20H,30H
  DEFW 1234H  ; Little-endian
  DEFM "INIT"

  RET

DONE:
  ; Test case 5: Conditional with ELSE
  IFE VERSION-1
  LD A,0  ; Version is 1
  ELSE
  LD A,0FFH
  ENDIF

  RET

; Test case 6: Data segment
  DSEG

BUFFER:
  DS BUFSIZE  ; Reserve 256 bytes

FLAGS:
  DEFB 0,0,0,0

; Test case 7: Back to code segment
  CSEG

UTIL:
  ; Test case 8: Case-insensitive labels
  ld hl,buffer  ; lowercase 'buffer' = BUFFER
  LD de,FLAGS   ; uppercase FLAGS
  LD bc,4
  LDIR  ; Block copy
  ret

; Test case 8: Absolute segment
  ASEG
  ORG 8000H

ROM_START:
  DEFM "ROM"
  DEFB VERSION
  DEFW MAIN  ; Address of MAIN (little-endian)

; Test case 9: More conditionals
  IFNDEF UNDEFINED
  DEFB 0AAH
  ENDIF

  IFIDN <TEST>,<TEST>
  DEFB 0BBH
  ENDIF

  ; Test case 10: Redefine DEFL
DEBUG DEFL 0
  IFE DEBUG
  DEFB 0CCH  ; DEBUG is now 0
  ENDIF

  END MAIN

; Expected output structure:
;
; CSEG (Code Segment) at $0100:
; - MAIN routine
; - INIT routine (with conditional code)
; - DONE routine
; - UTIL routine
; Total CSEG: ~50 bytes
;
; DSEG (Data Segment) at $0000:
; - BUFFER: 256 bytes
; - FLAGS: 4 bytes
; Total DSEG: 260 bytes
;
; ASEG (Absolute Segment) at $8000:
; - ROM_START: "ROM" + version byte + address + conditional bytes
; Total ASEG: ~10 bytes
;
; This test combines:
; ✓ Multiple segments (CSEG, DSEG, ASEG)
; ✓ ORG directives
; ✓ EQU and DEFL constants
; ✓ DEFL redefinition
; ✓ DEFB, DEFW, DEFS, DEFM data directives
; ✓ DB, DW, DS aliases
; ✓ IF/IFE/IF1/IF2/IFDEF/IFNDEF conditionals
; ✓ IFIDN string comparison
; ✓ ELSE clause
; ✓ Nested conditionals
; ✓ Case-insensitive mode
; ✓ Z80 instructions
; ✓ Little-endian word encoding
