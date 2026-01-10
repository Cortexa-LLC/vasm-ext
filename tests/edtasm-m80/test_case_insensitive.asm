; Test: Case-Insensitive Mode
; File: test_case_insensitive.asm
; Purpose: Verify case-insensitive mode (default for EDTASM-M80)
; Expected: 11 bytes output

  ORG 1000H

; Test case 1: Labels are case-insensitive
MyLabel EQU 42
  DEFB mylabel  ; Should resolve to MyLabel (42)
  DEFB MYLABEL  ; Should also resolve to MyLabel (42)

; Test case 2: Directives are case-insensitive
  defb 1  ; lowercase defb
  DEFB 2  ; uppercase DEFB
  DeFb 3  ; mixed case DeFb

; Test case 3: Instructions are case-insensitive
  nop     ; lowercase
  NOP     ; uppercase
  NoP     ; mixed case

; Test case 4: DEFM preserves string case
  DEFM "X"  ; String contents preserve case

  END

; Expected binary output (11 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  2A                 mylabel = MyLabel = 42 (2AH)
; $0001:  2A                 MYLABEL = MyLabel = 42 (2AH)
; $0002:  01                 defb 1
; $0003:  02                 DEFB 2
; $0004:  03                 DeFb 3
; $0005:  00 00 00           nop, NOP, NoP
; $0008:  58                 DEFM "X" (ASCII 58H)
;
; Total: 9 bytes (0x09)
;
; NOTE: This test verifies that EDTASM-M80 operates in case-insensitive
; mode by default. All identifiers (labels, symbols, directives, instructions)
; should match regardless of case. However, string contents in DEFM
; should preserve their original case.
