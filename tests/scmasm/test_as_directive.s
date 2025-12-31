*-----------------------------------
* Test: .AS directive with alternate delimiters
* Feature #2: .AS with - flag
*-----------------------------------
        .OR $2000

* Standard .AS with various delimiters
        .AS /Hello World/
        .AS |Test String|
        .AS @Another test@

* SCASM: - flag suppresses high-bit setting
        .AS -/Plain ASCII text/
        .AS -|No high bits|

* Delimiters < 0x27 set high-bit automatically
        .AS "This has high bits"
        .AS !Also high bits!

* Combined with data directives
FIRST.TABLE  .HS 00
             .AS -/ABCDEILMNORST /
             .HS 7F

        .END
