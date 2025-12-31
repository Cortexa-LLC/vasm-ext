* Test: Private labels in macros should be unique per invocation
* SCASM allows :N private labels in macros
* Each macro invocation should get a unique scope for private labels

        .OR $2000

* Define a macro with private labels
        .MA INCD
        INC ]1
        BNE :1
        INC ]1+1
:1
        .EM

* Test 1: Call macro multiple times with different arguments
* Each call should get unique :1 labels
START   INCD $40        ; First call - :1 should be unique
        INCD $42        ; Second call - :1 should be unique (different from first)
        INCD $44        ; Third call - :1 should be unique

* Test 2: Macro with multiple private labels
        .MA COPY
        LDX #0
:1      LDA ]1,X
        STA ]2,X
        INX
        CPX #]3
        BNE :1
:2      RTS
        .EM

* Multiple calls to COPY
        COPY $1000,$2000,#100   ; First call
        COPY $3000,$4000,#50    ; Second call - should have unique :1 and :2

* Test 3: Nested private labels
        .MA NESTED
:1      LDA ]1
        BNE :2
        JMP :3
:2      STA ]2
:3      RTS
        .EM

        NESTED $50,$51
        NESTED $60,$61

        .END

* Expected: All macro calls should assemble without label redefinition errors
* Each :1, :2, :3 label should be unique within each macro invocation
