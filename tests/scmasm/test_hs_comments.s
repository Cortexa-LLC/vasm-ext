* Test .HS directive with hex-like comments
* Bug: Comments like "0000-3FFF" were being parsed as hex data
        .OR $2000
MY.BITMAP
        .HS C3.00.00.00.00.00.00.00  0000-3FFF
        .HS 00.00.00.00.00.00.00.00  4000-7FFF
        .HS FF.FF.FF.FF.FF.FF.FF.F3  8000-BFFF
BITMAP.SIZE .EQ *-MY.BITMAP
        NOP
