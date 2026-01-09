; Test: CRITICAL - Little-Endian Word Encoding
; File: test_endianness.asm
; Purpose: Verify DEFW/DW/DEFH produce little-endian byte order
; Expected: 20 bytes output
;
; CRITICAL: Z80 uses LITTLE-ENDIAN byte order!
; DEFW 1234H must produce: 34 12 (NOT 12 34!)
;
; If this test fails, the implementation is BROKEN and must be fixed!

  ORG 1000H

START:

; Test case 1: Single DEFW word
  DEFW 1234H  ; MUST produce: 34 12

; Test case 2: Multiple DEFW words
  DEFW 5678H,9ABCH  ; MUST produce: 78 56 BC 9A

; Test case 3: DW alias (must match DEFW behavior)
  DW 0ABCDH  ; MUST produce: CD AB

; Test case 4: DEFH alias (must match DEFW behavior)
  DEFH 0EF01H  ; MUST produce: 01 EF

; Test case 5: Maximum word value
  DEFW 0FFFFH  ; MUST produce: FF FF

; Test case 6: Minimum word value
  DEFW 0  ; MUST produce: 00 00

; Test case 7: Expression in DEFW
  DEFW 20+22  ; 42 decimal = 2AH, MUST produce: 2A 00

; Test case 8: Label address in DEFW
  DEFW START  ; START = 1000H, MUST produce: 00 10

; Test case 9: Mixed bytes and words
  DEFB 0AAH  ; Single byte: AA
  DEFW 0BBCCH  ; Word: CC BB

  END

; Expected binary output (20 bytes at $1000):
; Offset  Bytes              Description                 CRITICAL CHECK
; ------  -----------------  --------------------------  --------------
; $0000:  34 12              DEFW 1234H                  ✓ Little-endian!
; $0002:  78 56              DEFW 5678H                  ✓ Little-endian!
; $0004:  BC 9A              DEFW 9ABCH                  ✓ Little-endian!
; $0006:  CD AB              DW 0ABCDH                   ✓ Little-endian!
; $0008:  01 EF              DEFH 0EF01H                 ✓ Little-endian!
; $000A:  FF FF              DEFW 0FFFFH                 ✓ Little-endian!
; $000C:  00 00              DEFW 0                      ✓ Little-endian!
; $000E:  2A 00              DEFW 42                     ✓ Little-endian!
; $0010:  00 10              DEFW START (1000H)          ✓ Little-endian!
; $0012:  AA                 DEFB 0AAH                   (single byte)
; $0013:  CC BB              DEFW 0BBCCH                 ✓ Little-endian!
;
; Total: 20 bytes (0x14)
;
; ┌─────────────────────────────────────────────────────────────────┐
; │ CRITICAL VERIFICATION:                                          │
; │                                                                 │
; │ Run: hexdump -C test_endianness.bin                            │
; │                                                                 │
; │ Expected first 6 bytes: 34 12 78 56 BC 9A                      │
; │                                                                 │
; │ If you see: 12 34 56 78 9A BC ← WRONG! BIG-ENDIAN BUG!        │
; │                                                                 │
; │ This test MUST show little-endian or the CPU module is broken! │
; └─────────────────────────────────────────────────────────────────┘
