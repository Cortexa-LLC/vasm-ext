; Test IRP with single item
; Expected: Single iteration

  ORG $8000

  IRP VAL,42
  DEFB VAL
  ENDR

  END
