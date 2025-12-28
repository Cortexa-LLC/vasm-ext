* Test .DUMMY and .ED directives
* Dummy sections are for defining data structures without allocating space
* Useful for defining memory layouts, zero-page variables, etc.
        .OR $8000

* Test 1: Define some code first
START   LDA #$00
        STA $D020

* Test 2: Define a dummy section (no bytes allocated)
        .DUMMY
ZPVAR1  .BS 1           ; Zero-page variable 1
ZPVAR2  .BS 2           ; Zero-page variable 2 (16-bit)
ZPBUF   .BS 16          ; Zero-page buffer
        .ED             ; End dummy

* Test 3: Code continues normally after dummy section
        LDA ZPVAR1
        STA ZPVAR2
        RTS

* Test 4: Another dummy section
        .DUMMY
TEMP1   .BS 1
TEMP2   .BS 1
TEMP3   .BS 2
        .ED

* Test 5: More code
        LDX #$FF
        TXS
        RTS
