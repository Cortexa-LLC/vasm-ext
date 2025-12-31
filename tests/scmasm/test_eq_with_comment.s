* Test: .EQ with trailing inline comment
* Comments after .EQ value should not be parsed as instructions

        .OR $2000

* Test 1: .EQ with comment starting with instruction-like word
VERSION    .EQ 3       VERSION NUMBER
SYMBOL     .EQ $0100   SYMBOL PACK AREA
HASH       .EQ $0130   HASH POINTER TABLE
WORKING    .EQ $0200   WORKING BUFFER
PRINT      .EQ $F94A   PRINT ROUTINE

* Test 2: Multi-value .EQ with comment
TWO.BYTES  .EQ $36,37  TWO BYTE VALUE

* Test 3: Use the symbols
START      LDA VERSION
           LDA SYMBOL
           LDA TWO.BYTES

           .END

* Expected: All .EQ directives should parse without "unknown mnemonic" errors
* Trailing comments should be ignored
