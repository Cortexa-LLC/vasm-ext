* Comprehensive String Directives Test
* Tests: ASC, DCI, INV, FLS, REV, STR, STRL, HEX

        ORG   $8000

* Test 1: ASC - Normal ASCII string ('=high-bit, "=normal)
TEXT1     ASC   'HELLO'      ; High-bit set ($C8 $C5 $CC $CC $CF)
TEXT2     ASC   "WORLD"      ; Normal ASCII ($57 $4F $52 $4C $44)
TEXT3     ASC   'Mixed',"Text" ; Mixed quotes

* Test 2: DCI - Last char high-bit inverted
DCI1      DCI   'APPLE'      ; Last char has bit 7 flipped
DCI2      DCI   "PRODOS"     ; Last char has bit 7 flipped

* Test 3: INV - All chars high-bit inverted (flashing)
INV1      INV   'FLASH'      ; All chars bit 7 inverted
INV2      INV   "BLINK"      ; All chars bit 7 inverted

* Test 4: FLS - Alternating high-bit (flashing sequence)
FLS1      FLS   'ALTERNATE'  ; Alternating bit 7 pattern

* Test 5: REV - Reversed byte order
REV1      REV   'REVERSE'    ; String stored backwards
REV2      REV   "BACKWARD"   ; String stored backwards

* Test 6: STR - Pascal string (1-byte length prefix)
STR1      STR   'COUNT'      ; $05 'C' 'O' 'U' 'N' 'T'
STR2      STR   "LENGTH"     ; $06 'L' 'E' 'N' 'G' 'T' 'H'

* Test 7: STRL - Long Pascal string (2-byte length prefix, little-endian)
STRL1     STRL  'LONGER'     ; $06 $00 'L' 'O' 'N' 'G' 'E' 'R'
STRL2     STRL  "A MUCH LONGER STRING EXAMPLE" ; 2-byte count

* Test 8: HEX - Hexadecimal data (2 hex digits = 1 byte)
HEX1      HEX   0102030405
HEX2      HEX   DEADBEEF
HEX3      HEX   00,11,22,33,44  ; Commas optional
HEX4      HEX   A0A1A2A3A4A5A6A7A8A9AAABACADAEAF

* Test 9: Mixed string directives
MIXED1    ASC   'Part 1'
          DCI   " Part 2"
          HEX   00

* Test 10: Empty strings (edge case)
EMPTY1    ASC   ''
EMPTY2    ASC   ""

* Test 11: Special characters in strings
SPECIAL   ASC   'Tab',09,'LF',0A,'CR',0D
          ASC   'Quote',"'",'DoubleQuote',22

* Test 12: String with expressions
]VALUE    EQU   $42
EXPR1     ASC   'Value='
          DFB   ]VALUE
          ASC   0D

* Test 13: Multi-line strings
LONG      ASC   'Line 1',0D
          ASC   'Line 2',0D
          ASC   'Line 3',0D,00

* Test 14: High-bit behavior differences
NORM      ASC   "NORMAL"     ; Normal ASCII
HIGH      ASC   'HIGH'       ; High-bit set
          HEX   00           ; Terminator

        END
