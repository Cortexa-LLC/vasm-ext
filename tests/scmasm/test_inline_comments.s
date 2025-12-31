* Test: Inline comments with various formats
* SCASM ignores trailing text after operands without requiring ;
* This includes text starting with "...", "-", "+", and other characters

        .OR $2000

* Test 1: Implied mode instructions with inline comments
FUNC1   NOP          This is a comment
        DEX          ...yes, see if two parameters
        INX          ...no, only one parameter
        DEY          -test with dash
        INY          +test with plus
        TAX          (test with paren
        TAY          [test with bracket

* Test 2: Instructions with operands and inline comments
        LDA #0       Load zero
        STA $00      Store to zero page
        LDX #$FF     ...maximum value
        STX $01      -store X register
        LDY #42      +decimal value

* Test 3: Branch instructions with inline comments
FUNC3   BEQ .1       ...branch if equal
        BNE .1       -branch if not equal
        BPL .1       +branch if plus
.1      RTS          Return from subroutine

* Test 4: Comma in inline comment (should not be treated as operand separator)
FUNC2   DEX          ...yes, see if two parameters
        BMI .2       ...no, use current values
        DEX          Were there any parameters?
.2      NOP

* Test 5: Mixed comment styles
        LDA #0       ; Standard semicolon comment
        STA $00      ...triple-dot comment
        LDX #1       Regular text comment
        STX $01      -dash comment

* Test 6: Comments with special characters
        NOP          Test: colons in comment
        NOP          Test @ symbols
        NOP          Test = equals
        NOP          Test ! exclamation

        .END

* Expected: All lines should assemble without "missing operand" errors
* The text after instructions should be treated as comments
