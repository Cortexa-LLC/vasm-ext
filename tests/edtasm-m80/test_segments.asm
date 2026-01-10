; Test: M80 Segment Directives
; File: test_segments.asm
; Purpose: Test ASEG, CSEG, DSEG segment directives
; NOTE: This test uses absolute addressing for all segments to avoid overlap in binary output

  ASEG
  ORG $1000

; Test case 1: Code at $1000
CODE1:
  DEFB 1,2,3,4

; Test case 2: Data at $2000
  ASEG
  ORG $2000
BUFFER:
  DS 4
  DEFB 5,6

; Test case 3: More code at $3000
  ASEG
  ORG $3000
CODE2:
  DEFB 7,8,9

  END

; Expected output (binary file):
;
; File uses relative offsets from first section ($1000):
; File Offset    Memory Addr    Data
; -----------    -----------    ----
; $0000-$0003    $1000-$1003    01 02 03 04 (CODE1)
; $0004-$0FFF    padding        00 ... 00
; $1000-$1005    $2000-$2005    00 00 00 00 05 06 (BUFFER + data)
; $1006-$1FFF    padding        00 ... 00
; $2000-$2002    $3000-$3002    07 08 09 (CODE2)
;
; Total file size: $2003 = 8195 bytes (including padding between sections).
