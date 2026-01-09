; Test: M80 Advanced Conditional Assembly
; File: test_m80_cond.asm
; Purpose: Test IF/IFE/IF1/IF2/IFDEF/IFNDEF/IFB/IFNB/IFIDN/IFDIF/ELSE/ENDIF
; Expected: Variable output depending on assembly pass

  ORG 1000H

; Define some symbols for testing
DEFINED EQU 42

; Test case 1: IF with non-zero expression (true)
  IF 1
  DEFB 1
  ENDIF

; Test case 2: IF with zero expression (false)
  IF 0
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 3: IFE with zero expression (true)
  IFE 0
  DEFB 2
  ENDIF

; Test case 4: IFE with non-zero expression (false)
  IFE 1
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 5: IF with expression evaluation
  IF 2+2
  DEFB 3
  ENDIF

; Test case 6: IF1 - assembles only in pass 1
  IF1
  DEFB 4
  ENDIF

; Test case 7: IF2 - assembles only in pass 2+
  IF2
  DEFB 5
  ENDIF

; Test case 8: IFDEF with defined symbol (true)
  IFDEF DEFINED
  DEFB 6
  ENDIF

; Test case 9: IFDEF with undefined symbol (false)
  IFDEF UNDEFINED
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 10: IFNDEF with undefined symbol (true)
  IFNDEF UNDEFINED
  DEFB 7
  ENDIF

; Test case 11: IFNDEF with defined symbol (false)
  IFNDEF DEFINED
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 12: IFB with blank argument (true)
; NOTE: IFB typically used in macros, testing basic functionality
  IFB
  DEFB 8
  ENDIF

; Test case 13: IFNB with non-blank argument (true)
; NOTE: IFNB typically used in macros, testing basic functionality
  IFNB <SOMETHING>
  DEFB 9
  ENDIF

; Test case 14: IFIDN with identical strings (true)
  IFIDN <ABC>,<ABC>
  DEFB 10
  ENDIF

; Test case 15: IFIDN with different strings (false)
  IFIDN <ABC>,<XYZ>
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 16: IFDIF with different strings (true)
  IFDIF <ABC>,<XYZ>
  DEFB 11
  ENDIF

; Test case 17: IFDIF with identical strings (false)
  IFDIF <ABC>,<ABC>
  DEFB 99H  ; Should NOT assemble
  ENDIF

; Test case 18: ELSE clause
  IF 0
  DEFB 99H  ; Should NOT assemble
  ELSE
  DEFB 12
  ENDIF

; Test case 19: Nested IF with ELSE
  IF 1
    IF 0
    DEFB 99H  ; Should NOT assemble
    ELSE
    DEFB 13
    ENDIF
  ENDIF

; Test case 20: Multiple ELSE conditions
  IF 0
  DEFB 99H
  ELSE
    IF 0
    DEFB 99H
    ELSE
    DEFB 14
    ENDIF
  ENDIF

  END

; Expected binary output (multi-pass assembly):
;
; Pass 1 output (with IF1):
; Offset  Bytes                          Description
; ------  -----------------------------  -----------
; $0000:  01                             IF 1
; $0001:  02                             IFE 0
; $0002:  03                             IF 2+2
; $0003:  04                             IF1 (pass 1 only)
; $0004:  06                             IFDEF DEFINED
; $0005:  07                             IFNDEF UNDEFINED
; $0006:  08                             IFB
; $0007:  09                             IFNB
; $0008:  0A                             IFIDN match
; $0009:  0B                             IFDIF different
; $000A:  0C                             ELSE clause
; $000B:  0D                             Nested ELSE
; $000C:  0E                             Multiple ELSE
; Total Pass 1: 13 bytes
;
; Pass 2+ output (with IF2):
; Offset  Bytes                          Description
; ------  -----------------------------  -----------
; $0000:  01                             IF 1
; $0001:  02                             IFE 0
; $0002:  03                             IF 2+2
; $0003:  05                             IF2 (pass 2+ only)
; $0004:  06                             IFDEF DEFINED
; $0005:  07                             IFNDEF UNDEFINED
; $0006:  08                             IFB
; $0007:  09                             IFNB
; $0008:  0A                             IFIDN match
; $0009:  0B                             IFDIF different
; $000A:  0C                             ELSE clause
; $000B:  0D                             Nested ELSE
; $000C:  0E                             Multiple ELSE
; Total Pass 2+: 13 bytes
;
; NOTE: Final output should be from pass 2+
