; Test all 9 macro parameters (#P1 through #P9)
; Expected: All 9 parameters substituted correctly

  ORG $8000

; Macro using all 9 parameters
NINE MACRO
  DEFB #P1,#P2,#P3,#P4,#P5,#P6,#P7,#P8,#P9
ENDM

; Invoke with all 9 parameters
  NINE $11,$22,$33,$44,$55,$66,$77,$88,$99

  END
