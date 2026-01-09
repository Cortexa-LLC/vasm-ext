; Test: M80 Segment Directives
; File: test_segments.asm
; Purpose: Test ASEG, CSEG, DSEG segment directives
; Expected: Multiple segments with proper addressing

; Test case 1: Default segment is CSEG (code segment)
CODE1:
  DEFB 1

; Test case 2: CSEG creates/continues code segment
  CSEG
  DEFB 2

; Test case 3: DSEG creates data segment
  DSEG
BUFFER:
  DS 4

; Test case 4: Multiple CSEG sections merge
  CSEG
CODE2:
  DEFB 3

; Test case 5: ASEG creates absolute segment
  ASEG
  ORG 2000H
ABS1:
  DEFB 10H,20H,30H

; Test case 6: Back to CSEG (continues from where it left off)
  CSEG
CODE3:
  DEFB 4

; Test case 7: Another DSEG (continues data segment)
  DSEG
  DS 2

  END

; Expected segments:
;
; CSEG (Code Segment) - Relocatable, default origin $0000:
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  01                 CODE1: DEFB 1
; $0001:  02                 CSEG: DEFB 2
; $0002:  03                 CODE2: DEFB 3
; $0003:  04                 CODE3: DEFB 4
; Total CSEG: 4 bytes
;
; DSEG (Data Segment) - Relocatable, default origin $0000:
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  00 00 00 00        BUFFER: DS 4
; $0004:  00 00              DS 2
; Total DSEG: 6 bytes
;
; ASEG (Absolute Segment) - Absolute, at ORG address:
; Offset  Bytes              Description
; ------  -----------------  -----------
; $2000:  10 20 30           ABS1: DEFB 10H,20H,30H
; Total ASEG: 3 bytes
;
; NOTE: Binary output depends on output format. For flat binary,
; ASEG at $2000 will create a file with padding. For relocatable
; output, segments are separate.
