* Test: Dash-equals pattern in trailing comment
* SCASM pattern:  - = comment
* Should not be parsed as subtraction expression

        .OR $2000

START   NOP
.40     LDA #0
        BMI .40      - = AUTO MODE, + = VERIFY MODE
        BEQ START    + = TEST
        RTS

        .END

* Expected: All instructions should assemble without "number or identifier expected" errors
* The " - = " and " + = " patterns are trailing comments, not expressions
