; Test: Z80 Instruction Integration
; File: test_z80_instructions.asm
; Purpose: Test Z80 instructions with EDTASM-M80 directives
; Expected: 63 bytes output

  ORG 8000H

; Constants for testing
PORTIO EQU 0FEH
MEMADDR EQU 0C000H
CALLADDR EQU 9000H

START:
; Test case 1: Basic 8-bit loads
  LD A,42H
  LD B,10H
  LD C,20H
  LD D,30H
  LD E,40H
  LD H,50H
  LD L,60H

; Test case 2: 16-bit loads
  LD BC,1234H
  LD DE,5678H
  LD HL,MEMADDR
  LD SP,0FFFFH

; Test case 3: Memory operations
  LD A,(HL)
  LD (HL),A
  LD A,(MEMADDR)
  LD (MEMADDR),A

; Test case 4: Arithmetic operations
  ADD A,B
  ADC A,C
  SUB D
  SBC A,E
  AND H
  OR L
  XOR A
  CP B

; Test case 5: Increment/Decrement
  INC A
  DEC B
  INC HL
  DEC DE

; Test case 6: Jumps and calls
  JP START
  JR START
  CALL SUB1
  RET

SUB1:
  NOP
  RET

; Test case 7: Stack operations
  PUSH BC
  PUSH DE
  PUSH HL
  PUSH AF
  POP AF
  POP HL
  POP DE
  POP BC

; Test case 8: I/O operations
  IN A,(PORTIO)
  OUT (PORTIO),A

; Test case 9: Bit operations
  BIT 0,A
  SET 7,B
  RES 3,C

; Test case 10: Data mixed with instructions
  DEFB 0FFH
  LD A,1
  DEFW 1234H
  NOP
  DEFM "Z"
  RET

  END START

; Expected binary output (approximately 63 bytes at $8000):
; Offset  Bytes              Instruction
; ------  -----------------  -----------
; $0000:  3E 42              LD A,42H
; $0002:  06 10              LD B,10H
; $0004:  0E 20              LD C,20H
; $0006:  16 30              LD D,30H
; $0008:  1E 40              LD E,40H
; $000A:  26 50              LD H,50H
; $000C:  2E 60              LD L,60H
; $000E:  01 34 12           LD BC,1234H
; $0011:  11 78 56           LD DE,5678H
; $0014:  21 00 C0           LD HL,C000H
; $0017:  31 FF FF           LD SP,FFFFH
; $001A:  7E                 LD A,(HL)
; $001B:  77                 LD (HL),A
; $001C:  3A 00 C0           LD A,(C000H)
; $001F:  32 00 C0           LD (C000H),A
; $0022:  80                 ADD A,B
; $0023:  89                 ADC A,C
; $0024:  92                 SUB D
; $0025:  9B                 SBC A,E
; $0026:  A4                 AND H
; $0027:  B5                 OR L
; $0028:  AF                 XOR A
; $0029:  B8                 CP B
; $002A:  3C                 INC A
; $002B:  05                 DEC B
; $002C:  23                 INC HL
; $002D:  1B                 DEC DE
; $002E:  C3 00 80           JP 8000H (START)
; $0031:  18 CD              JR 8000H (START, relative)
; $0033:  CD 3A 80           CALL 803AH (SUB1)
; $0036:  C9                 RET
; $0037:  00                 NOP (SUB1)
; $0038:  C9                 RET
; $0039:  C5                 PUSH BC
; $003A:  D5                 PUSH DE
; $003B:  E5                 PUSH HL
; $003C:  F5                 PUSH AF
; $003D:  F1                 POP AF
; $003E:  E1                 POP HL
; $003F:  D1                 POP DE
; $0040:  C1                 POP BC
; $0041:  DB FE              IN A,(FEH)
; $0043:  D3 FE              OUT (FEH),A
; $0045:  CB 47              BIT 0,A
; $0047:  CB F8              SET 7,B
; $0049:  CB 99              RES 3,C
; $004B:  FF                 DEFB FFH
; $004C:  3E 01              LD A,1
; $004E:  34 12              DEFW 1234H (little-endian!)
; $0050:  00                 NOP
; $0051:  5A                 DEFM "Z" (ASCII)
; $0052:  C9                 RET
;
; Total: Approximately 83 bytes (0x53)
;
; NOTE: This test verifies that Z80 instructions work correctly
; with EDTASM-M80 directives and that labels, constants, and
; data directives integrate properly with instruction encoding.
