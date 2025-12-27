* File Import Test
* Tests: PUT (source include)

        ORG   $8000

START   NOP

* Test 1: PUT directive - include source file
* Defines: ZP_TEMP, ZP_PTR, ZP_COUNT, SCREEN, COLOR, DELAY macro
        PUT   include_defs.asm

* Use symbols from included file
        LDA   #$00
        STA   ZP_TEMP
        LDA   #<SCREEN
        STA   ZP_PTR
        LDA   #>SCREEN
        STA   ZP_PTR+1

* Use macro from included file (need global label for local labels in macro)
USEDELAY
        LDX   #$FF
        >>> DELAY

* Alternative: Use HEX directive for binary data
BINDATA   HEX   EAEAEAEAA9008D0004

* Verify we can reference all imported symbols
DATA1     DA    ZP_TEMP
DATA2     DA    SCREEN
DATA3     DA    COLOR

        END
