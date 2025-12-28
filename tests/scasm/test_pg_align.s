*-----------------------------------
* Test: .PG page alignment directive
* Feature #4: Align to 256-byte boundary
*-----------------------------------
        .OR $2050

START:  NOP
        NOP
        LDA #$00

* Align to next page boundary
        .PG              ; Should pad to $2100

* Verify alignment
PAGE1:  .DA PAGE1        ; Should be $2100
        NOP

* Another page alignment
        .OR $30A0
        LDA #$FF
        .PG              ; Should pad to $3100

PAGE2:  .DA PAGE2        ; Should be $3100
        NOP

* Page align from exact page boundary
        .OR $4000
        .PG              ; Already aligned, should not pad

PAGE3:  .DA PAGE3        ; Should be $4000

        .END
