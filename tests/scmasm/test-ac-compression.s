* Test .AC compression directive
* Tests nibble-based text compression as used by SCASM
*
* Bug: .AC directive was not compressing text, outputting raw characters
* Fix: Implemented full nibble compression in vasm syntax.c handle_ac()
*
* Also tests: Macro argument quote stripping
* Bug: Quoted macro arguments included quotes in substitution
* Fix: parse_macro_arg() now strips quotes from string arguments

       .OR $1000

* Initialize compression state
       .AC 0

* Define compression tables (SCI style)
       .AC 1"ACDEFILNOPRST %"
       .AC 2"BGHKMUVWXY/():."
       .AC 3"QZ-4567890123@&"

* Test 1: Simple message "RANGE ERROR%"
* R=t1[10]=nib11, A=t1[0]=nib1, N=t1[7]=nib8
* G=t2[1]=nib0,2, E=t1[3]=nib4
* (space)=t1[13]=nib14, E=nib4, R=nib11, R=nib11
* O=t1[8]=nib9, R=nib11, %=t1[14]=nib15
* Nibbles: B,1,8,0,2,4,E,4,B,B,9,B,F (13 nibbles = 7 bytes)
* Expected: $B1 $80 $24 $E4 $BB $9B $F0
TEST.RANGE.ERROR
       .AC /RANGE ERROR%/

* Test 2: Simple "CAT%" (4 nibbles = 2 bytes)
* C=t1[1]=nib2, A=t1[0]=nib1, T=t1[12]=nib13, %=nib15
* Expected: $21 $DF
TEST.CAT
       .AC /CAT%/

* Test 3: "BIG%" using table 2 (6 nibbles = 3 bytes)
* B=t2[0]=nib0,1, I=t1[5]=nib6, G=t2[1]=nib0,2, %=nib15
* Expected: $01 $60 $2F
TEST.BIG
       .AC /BIG%/

* ========================================
* Test macro argument quote stripping
* ========================================

* Re-initialize with ASM1 style tables
       .AC 0
       .AC 1/ABCDEILMNORST %/
       .AC 2/FGPUXY$.*:?52!#/
       .AC 3/HJKQVWZ>1-...../

* Define QT macro (like X.NEW.QUOTES.s)
       .MA QT
       .AC /]2/
       .EM

* Test 4: Direct .AC without macro
TEST.DIRECT
       .AC /SIMPLE%/

* Test 5: Via macro with quoted argument
* The quotes should be stripped, so ]2 = TEST MESSAGE%
TEST.MACRO
       >QT QTEST,"TEST MESSAGE%"

* Test 6: Macro with special chars (# * !)
TEST.SPECIAL
       >QT QSPEC,"#TEST!%"

       .HS 00
