* Test: .0 local label support
* SCASM allows .0 through .255 as local labels
* Each global label creates a new scope for local labels

        .OR $2000

* Test 1: .0 label in first function
FUNC1   NOP
.0      LDA #0          ; .0 in FUNC1 scope
        STA $00
        BEQ .0          ; Branch to .0 (same scope)
.1      LDX #1
        BEQ .1

* Test 2: .0 label in second function (different scope)
FUNC2   NOP
.0      LDA #$FF        ; .0 in FUNC2 scope (different from FUNC1's .0)
        STA $01
        BNE .0
.1      LDY #2
        BNE .1
        RTS

* Test 3: Multiple references to .0
FUNC3   JSR FUNC1
        JSR FUNC2
.0      LDA #$42        ; .0 in FUNC3 scope
        BEQ .0
        JSR .0          ; Forward reference to FUNC4's .0

* Test 4: .0 can coexist with other local labels
FUNC4   NOP
.0      LDA #0          ; .0
.1      LDX #1          ; .1
.2      LDY #2          ; .2
.255    NOP             ; .255 (maximum)
        BEQ .0          ; Branch to .0
        BEQ .1          ; Branch to .1
        BEQ .2          ; Branch to .2
        BEQ .255        ; Branch to .255
        RTS

        .END

* Expected behavior:
* - .0 is valid in all scopes
* - Each global label creates new scope for .0
* - FUNC1's .0 is different from FUNC2's .0
* - Local labels .0-.255 all work correctly
