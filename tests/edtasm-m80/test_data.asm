; Test: Data Definition Directives
; File: test_data.asm
; Purpose: Test DEFB, DEFW, DEFH, DEFS, DEFM
; Expected: 27 bytes output

  ORG 1000H

; Test case 1: DEFB single byte
  DEFB 42H

; Test case 2: DEFB multiple bytes
  DEFB 1,2,3,4,5

; Test case 3: DEFW single word (little-endian!)
  DEFW 1234H

; Test case 4: DEFW multiple words (little-endian!)
  DEFW 5678H,9ABCH

; Test case 5: DEFH is alias for DEFW
  DEFH 0ABCDH

; Test case 6: DEFS reserves space (filled with zeros)
  DEFS 5

; Test case 7: DEFM with double quotes
  DEFM "Hello"

; Test case 8: DEFM with single quotes
  DEFM 'Z80'

  END

; Expected binary output (27 bytes at $1000):
; Offset  Bytes                          Description
; ------  -----------------------------  -----------
; $0000:  42                             DEFB 42H
; $0001:  01 02 03 04 05                 DEFB 1,2,3,4,5
; $0006:  34 12                          DEFW 1234H (little-endian!)
; $0008:  78 56 BC 9A                    DEFW 5678H,9ABCH
; $000C:  CD AB                          DEFH 0ABCDH
; $000E:  00 00 00 00 00                 DEFS 5 (zeros)
; $0013:  48 65 6C 6C 6F                 DEFM "Hello" (ASCII)
; $0018:  5A 38 30                       DEFM 'Z80' (ASCII)
;
; Total: 27 bytes (0x1B)
