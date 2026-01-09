; Test: M80 Data Directive Aliases
; File: test_aliases.asm
; Purpose: Test DB, DW, DS, DC aliases
; Expected: 10 bytes output

  ORG 1000H

; Test case 1: DB is alias for DEFB
  DB 0AAH,0BBH,0CCH

; Test case 2: DW is alias for DEFW (little-endian!)
  DW 1234H

; Test case 3: DS is alias for DEFS
  DS 3

; Test case 4: DC defines constants
  DC 42

; Test case 5: Mixed aliases in same program
  DW 5678H

  END

; Expected binary output (10 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  AA BB CC           DB 0AAH,0BBH,0CCH
; $0003:  34 12              DW 1234H (little-endian!)
; $0005:  00 00 00           DS 3 (zeros)
; $0008:  2A                 DC 42 (2AH)
; $0009:  78 56              DW 5678H (little-endian!)
;
; Total: 10 bytes (0x0A)
