* Test SCASM 3.1 directives: .CS, .CZ, .HX, .PS, .TA
* Comprehensive test suite for A2osX assembler compatibility

         org $1000

*-------------------------------
* .CS - C-string with escape sequences
* Supports: \a \b \e \f \n \r \t \v \xNN \\ \" \'
*-------------------------------
test_cs_newline
         .cs "Line1\nLine2"
* Expected: 4C 69 6E 65 31 0A 4C 69 6E 65 32 (Line1<LF>Line2)

test_cs_tab
         .cs "Col1\tCol2"
* Expected: 43 6F 6C 31 09 43 6F 6C 32 (Col1<TAB>Col2)

test_cs_return
         .cs "A\rB"
* Expected: 41 0D 42 (A<CR>B)

test_cs_bell
         .cs "Ring\a"
* Expected: 52 69 6E 67 07 (Ring<BEL>)

test_cs_escape
         .cs "\eC"
* Expected: 1B 43 (ESC C) - clear screen on many terminals

test_cs_formfeed
         .cs "Page1\fPage2"
* Expected: 50 61 67 65 31 0C 50 61 67 65 32 (Page1<FF>Page2)

test_cs_vtab
         .cs "A\vB"
* Expected: 41 0B 42 (A<VT>B)

test_cs_backspace
         .cs "X\b_"
* Expected: 58 08 5F (X<BS>_)

test_cs_hex
         .cs "\x00\x41\xFF"
* Expected: 00 41 FF (NUL, 'A', 255)

test_cs_null
         .cs "null:\0end"
* Expected: 6E 75 6C 6C 3A 00 65 6E 64 (null:<NUL>end)

test_cs_backslash
         .cs "path\\file"
* Expected: 70 61 74 68 5C 66 69 6C 65 (path\file)

test_cs_quote
         .cs "say \"hi\""
* Expected: 73 61 79 20 22 68 69 22 (say "hi")

*-------------------------------
* .CZ - C-string zero terminated
*-------------------------------
test_cz_basic
         .cz "Hello"
* Expected: 48 65 6C 6C 6F 00 (Hello + NUL terminator)

test_cz_with_escapes
         .cz "Line\n"
* Expected: 4C 69 6E 65 0A 00 (Line<LF> + NUL terminator)

*-------------------------------
* .HX - Hex nibble storage
* Each hex digit stored as a byte (0-15)
*-------------------------------
test_hx_digits
         .hx 0123456789
* Expected: 00 01 02 03 04 05 06 07 08 09

test_hx_hex_letters
         .hx ABCDEF
* Expected: 0A 0B 0C 0D 0E 0F

test_hx_mixed
         .hx 1A2B3C
* Expected: 01 0A 02 0B 03 0C

test_hx_with_delims
         .hx 12.34,56
* Expected: 01 02 03 04 05 06 (delimiters ignored)

*-------------------------------
* .PS - Pascal string (length-prefixed)
* First byte is length, then string data
*-------------------------------
test_ps_short
         .ps "Hi"
* Expected: 02 48 69 (length=2, "Hi")

test_ps_longer
         .ps "Hello World"
* Expected: 0B 48 65 6C 6C 6F 20 57 6F 72 6C 64 (length=11, "Hello World")

test_ps_empty
         .ps ""
* Expected: (nothing - empty strings produce no output)

*-------------------------------
* .TA - Target Address (ignored in cross-assembly)
*-------------------------------
test_ta_ignored
         .ta $2000
* Should be silently ignored

verify_ta
         nop
* Expected: EA (NOP at current address, not $2000)

*-------------------------------
* End of tests
*-------------------------------
