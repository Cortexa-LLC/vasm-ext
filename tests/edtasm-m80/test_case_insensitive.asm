; Test: Case-Insensitive Mode
; File: test_case_insensitive.asm
; Purpose: Verify case-insensitive mode (default for EDTASM-M80)
; Expected: 9 bytes output

  ORG 1000H

; Test case 1: Labels are case-insensitive
MyLabel EQU 42
  DEFB mylabel  ; Should resolve to MyLabel (42)

; Test case 2: Directives are case-insensitive
  defb 1  ; lowercase defb
  DEFB 2  ; uppercase DEFB
  DeFb 3  ; mixed case DeFb

; Test case 3: Instructions are case-insensitive (tested with symbols)
start:
START:  ; Same label as 'start'
  ; No code, just testing label equivalence

; Test case 4: EQU symbols are case-insensitive
VALUE equ 99
  DEFB value  ; Should resolve to VALUE (99)
  DEFB Value  ; Should also resolve to VALUE (99)

; Test case 5: Segment names are case-insensitive
  cseg  ; lowercase cseg
  CSEG  ; uppercase CSEG (same segment)

; Test case 6: DEFM preserves string case
  DEFM "Hello"  ; String contents preserve case

  END

; Expected binary output (9 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  2A                 mylabel = MyLabel = 42 (2AH)
; $0001:  01                 defb 1
; $0002:  02                 DEFB 2
; $0003:  03                 DeFb 3
; $0004:  63                 value = VALUE = 99 (63H)
; $0005:  63                 Value = VALUE = 99 (63H)
; $0006:  48 65 6C 6C 6F     DEFM "Hello" (preserves case)
;
; Total: 11 bytes (0x0B)
;
; NOTE: This test verifies that EDTASM-M80 operates in case-insensitive
; mode by default. All identifiers (labels, symbols, directives, instructions)
; should match regardless of case. However, string contents in DEFM
; should preserve their original case.
