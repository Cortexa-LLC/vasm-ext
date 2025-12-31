* Simple .AC test
       .OR $1000

       .AC 0
       .AC 1"ACDEFILNOPRST %"
       .AC 2"BGHKMUVWXY/():."
       .AC 3"QZ-4567890123@&"

* "CAT%" should be: C=nib2, A=nib1, T=nib13($D), %=nib15($F)
* Nibbles: 2,1,D,F -> bytes: $21 $DF
TEST1  .AC /CAT%/

* End marker
       .HS FF
