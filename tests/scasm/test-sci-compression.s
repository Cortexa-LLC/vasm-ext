* Test SCI compression - compare with expected output
       .OR $1000

* Initialize
       .AC 0

* SCI compression tables (exact copy from SC.TABLES.s)
       .AC 1"ACDEFILNOPRST %"
       .AC 2"BGHKMUVWXY/():."
       .AC 3"QZ-4567890123@&"

* Compress first few SCI messages
* Message 0: " BLOCKS SHOWN ABOVE:6%"
MSG0   .AC / BLOCKS SHOWN ABOVE:6%/

* Message 1: "TYPE   NAME2BLOCKS  MODIFIED9CREATED9ENDFILE AUXTYPE%"
MSG1   .AC /TYPE   NAME2BLOCKS  MODIFIED9CREATED9ENDFILE AUXTYPE%/

* Message 2: "RANGE ERROR%"
MSG2   .AC /RANGE ERROR%/

* Final flush marker
       .AC "%"

       .HS 00
