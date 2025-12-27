* CPU Mode Directives Test
* Tests: MX, XC, LONGA, LONGI directives for 65816

        ORG   $8000

* Test 1: XC - Set CPU type
        XC                ; 65816 mode (default)
        XC    OFF         ; 6502 mode
        XC    ON          ; 65816 mode

* Test 2: MX directive - Set accumulator/index sizes
* MX %ab where a=accumulator, b=index (0=16-bit, 1=8-bit)

START   NOP

* 8-bit accumulator, 8-bit index (6502 emulation)
        MX    %11
        LDA   #$12        ; 8-bit immediate
        LDX   #$34        ; 8-bit immediate

* 16-bit accumulator, 8-bit index
        MX    %01
        LDA   #$1234      ; 16-bit immediate
        LDX   #$56        ; 8-bit immediate

* 8-bit accumulator, 16-bit index
        MX    %10
        LDA   #$78        ; 8-bit immediate
        LDX   #$9ABC      ; 16-bit immediate

* 16-bit accumulator, 16-bit index (native 65816)
        MX    %00
        LDA   #$DEAD      ; 16-bit immediate
        LDX   #$BEEF      ; 16-bit immediate

* Test 3: LONGA directive - Accumulator size
        LONGA ON          ; 16-bit accumulator
        LDA   #$CAFE
        LONGA OFF         ; 8-bit accumulator
        LDA   #$42

* Test 4: LONGI directive - Index register size
        LONGI ON          ; 16-bit index
        LDX   #$BABE
        LONGI OFF         ; 8-bit index
        LDX   #$99

* Test 5: Combined usage
        LONGA ON
        LONGI ON
        LDA   #$1111
        LDX   #$2222
        LDY   #$3333

        LONGA OFF
        LONGI OFF
        LDA   #$44
        LDX   #$55
        LDY   #$66

* Test 6: MX with different values
        MX    %11         ; Both 8-bit
        REP   #$30        ; Set M and X bits
        MX    %00         ; Tell assembler both 16-bit
        LDA   #$ABCD
        LDX   #$EF01

        SEP   #$30        ; Clear M and X bits
        MX    %11         ; Tell assembler both 8-bit
        LDA   #$23
        LDX   #$45

* Test 7: Context preservation across labels
FUNC1   LONGA ON
        LONGI ON
        LDA   #$9999
        RTS

FUNC2   MX    %11         ; Reset to 8-bit
        LDA   #$AA
        RTS

* Test 8: Conditional CPU modes
]MODE   EQU   1

        DO    ]MODE
        MX    %00         ; 16-bit mode
        LDA   #$CCCC
        FIN

        DO    0
        MX    %11         ; 8-bit mode (not assembled)
        LDA   #$DD
        FIN

        END
