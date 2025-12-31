* Test: .EQ with 3+ values
* SCASM allows defining multiple consecutive bytes with .EQ
* Only the first value is stored; additional values are for documentation
* Subsequent bytes accessed via symbol+1, symbol+2, etc.

        .OR $2000

* Test 1: Two-byte .EQ (already tested, but verify still works)
TWO.BYTES   .EQ $36,37
        LDA TWO.BYTES       ; Should load from $36
        LDA TWO.BYTES+1     ; Should load from $37

* Test 2: Three-byte .EQ
THREE.BYTES .EQ $BEF2,3,4
        LDA THREE.BYTES     ; Should load from $BEF2
        LDA THREE.BYTES+1   ; Should load from $BEF3 (value 3)
        LDA THREE.BYTES+2   ; Should load from $BEF4 (value 4)

* Test 3: Four-byte .EQ
FOUR.BYTES  .EQ $1000,$10,$20,$30
        LDA FOUR.BYTES      ; $1000
        LDA FOUR.BYTES+1    ; $1010
        LDA FOUR.BYTES+2    ; $1020
        LDA FOUR.BYTES+3    ; $1030

* Test 4: Many-byte .EQ
MANY.BYTES  .EQ $2000,1,2,3,4,5,6,7,8,9
        LDX MANY.BYTES+5    ; Access 6th byte

        .END

* Expected: All .EQ statements should parse without "identifier expected" errors
* The symbols should have the first value, subsequent bytes are implicit
