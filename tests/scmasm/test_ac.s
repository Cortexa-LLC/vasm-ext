* Test .AC directive encoding
        .OR $2000
*--------------------------------
* Define encoding tables (same as SCASM)
        .AC 0
        .AC 1/ABCDEILMNORST %/
        .AC 2/FGPUXY$.*:?52!#/
        .AC 3/HJKQVWZ>1-...../
*--------------------------------
* First message: just end marker
MESSAGES
        .AC "%"
* Second message: S-C MACRO ASSEMBLER
        .AC /S-C MACRO ASSEMBLER %/
* Third message: TEST
        .AC /TEST%/
*--------------------------------
* Runtime tables for reference
FIRST.TABLE
        .HS 00
        .AS -/ABCDEILMNORST /
        .HS 7F
SECOND.TABLE
        .HS 00
        .AS -/FGPUXY$.*:?52/
        .HS 878D
THIRD.TABLE
        .AS -/.HJKQVWZ>1-...../
*--------------------------------
