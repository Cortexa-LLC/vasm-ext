* Test single-character strings in Merlin string directives
* This tests the fix for parse_string returning NULL for single-char strings

         org $1000

* ASC - ASCII string with high bit from delimiter
asc_single  asc "A"      ; single char with high bit
asc_multi   asc "AB"     ; multi char for comparison
asc_nohigh  asc 'C'      ; single char without high bit

* DCI - high bit inverted on last character
dci_single  dci "X"      ; single char - high bit toggled
dci_multi   dci "XY"     ; multi char for comparison

* INV - all chars with high bit set (inverse video)
inv_single  inv "D"      ; single char inverse
inv_multi   inv "DE"     ; multi char for comparison

* FLS - flashing (alternating high bit)
fls_single  fls "E"      ; single char - no alternation possible
fls_multi   fls "EF"     ; multi char - alternates on odd indices

* REV - reversed byte order
rev_single  rev "R"      ; single char - same reversed
rev_multi   rev "RS"     ; multi char for comparison

* STR - 1-byte length prefix
str_single  str "F"      ; single char with length prefix
str_multi   str "FG"     ; multi char for comparison

* STRL - 2-byte length prefix
strl_single strl "G"     ; single char with 2-byte length
strl_multi  strl "GH"    ; multi char for comparison

* Test with different delimiters
delim_dquote asc "H"     ; double quote sets high bit (< 0x27)
delim_squote asc 'I'     ; single quote no high bit (= 0x27)

         lst off
