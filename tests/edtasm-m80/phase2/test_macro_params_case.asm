; Test case-insensitive macro parameters
; Expected: #P1/#p1, #$YM/#$ym all work

  ORG $8000

; Test lowercase parameter references
LOWER MACRO
  DEFB #p1,#p2,#p3
ENDM

  LOWER $11,$22,$33

; Test mixed case
MIXED MACRO
  DEFB #P1,#p2,#P3
ENDM

  MIXED $44,$55,$66

; Test lowercase unique ID (use as label)
UNIQ MACRO
#$ym:
  DEFB $AA
ENDM

  UNIQ
  UNIQ    ; Second invocation should get different ID

  END
