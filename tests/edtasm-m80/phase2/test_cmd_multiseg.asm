; Test TRS-DOS /CMD multi-segment output
; Three separate segments at different addresses

  ORG $8000
CODE1:
  LD A,$11
  LD B,$22

  ORG $9000
CODE2:
  LD C,$33
  LD D,$44

  ORG $A000
CODE3:
  LD E,$55
  RET

  END CODE1
