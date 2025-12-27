* Test Advanced Merlin Features
* Tests: LUP/--^, >>> macro, DDB, ADRL, ENT/EXT, SAV

        ORG   $8000

* Test ENT (entry point export)
        ENT   START,MYDATA

* Test EXT (external symbol import)
* Note: EXT causes linking errors in binary output mode, so commented out
*       EXT   EXTERNAL_FUNC

START   NOP

* Test LUP/--^ loop
]COUNT  EQU   3
        LUP   ]COUNT
        LDA   #$00
        --^

* Test >>> macro invocation
TestMac MAC
        LDX   #]1
        <<<

        >>> TestMac $42

* Test DDB (big-endian word)
BIGWORD DDB   $1234,$5678

* Test ADRL (32-bit address)
LONGADDR ADRL  $12345678,$AABBCCDD

* Test variable label in loop
]LOOP   EQU   2
        LUP   ]LOOP
        INX
        --^

* Test data reference
MYDATA  DA    START
        DA    BIGWORD

* Optional: SAV directive (output filename - may not affect binary output)
*       SAV   TESTADV

        END
