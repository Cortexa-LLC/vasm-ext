; Test: Deep Nesting of Conditionals
; File: test_nested_cond.asm
; Purpose: Test deep nesting (10+ levels) of COND and IF directives
; Expected: 3 bytes output

  ORG 1000H

; Test case 1: 10-level nested COND
  COND 1
    COND 1
      COND 1
        COND 1
          COND 1
            COND 1
              COND 1
                COND 1
                  COND 1
                    COND 1
                      DEFB 10  ; 10 levels deep
                    ENDC
                  ENDC
                ENDC
              ENDC
            ENDC
          ENDC
        ENDC
      ENDC
    ENDC
  ENDC

; Test case 2: 10-level nested IF
  IF 1
    IF 1
      IF 1
        IF 1
          IF 1
            IF 1
              IF 1
                IF 1
                  IF 1
                    IF 1
                      DEFB 10  ; 10 levels deep
                    ENDIF
                  ENDIF
                ENDIF
              ENDIF
            ENDIF
          ENDIF
        ENDIF
      ENDIF
    ENDIF
  ENDIF

; Test case 3: Mixed nesting (COND inside IF inside COND)
  COND 1
    IF 1
      COND 1
        DEFB 3  ; Mixed nesting level 3
      ENDC
    ENDIF
  ENDC

; Test case 4: False condition at deep level
  COND 1
    COND 1
      COND 1
        COND 0
          DEFB 99H  ; Should NOT assemble
        ENDC
      ENDC
    ENDC
  ENDC

; Test case 5: Very deep nesting (15 levels) - stress test
  IF 1
    IF 1
      IF 1
        IF 1
          IF 1
            IF 1
              IF 1
                IF 1
                  IF 1
                    IF 1
                      IF 1
                        IF 1
                          IF 1
                            IF 1
                              IF 1
                                ; We're 15 levels deep!
                                ; (No output here, just testing nesting)
                              ENDIF
                            ENDIF
                          ENDIF
                        ENDIF
                      ENDIF
                    ENDIF
                  ENDIF
                ENDIF
              ENDIF
            ENDIF
          ENDIF
        ENDIF
      ENDIF
    ENDIF
  ENDIF

  END

; Expected binary output (3 bytes at $1000):
; Offset  Bytes              Description
; ------  -----------------  -----------
; $0000:  0A                 10 levels deep (COND)
; $0001:  0A                 10 levels deep (IF)
; $0002:  03                 Mixed nesting level 3
;
; Total: 3 bytes
;
; NOTE: This test verifies that the conditional stack can handle
; deep nesting without overflow or corruption. The typical stack
; size in vasm is 100 levels, so 15 levels should be well within
; safe limits.
