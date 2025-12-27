* Comprehensive Macro System Test
* Tests: MAC/<<<, parameters ]0-]8, invocation methods

        ORG   $8000

* Test 1: Simple macro with no parameters
NoParams MAC
        NOP
        LDA   #$00
        <<<

        NoParams          ; Direct invocation

* Test 2: Macro with single parameter
OneParm MAC
        LDA   #]1
        <<<

        OneParm $42       ; Direct with param

* Test 3: Macro with multiple parameters (]0 = count)
MultiParm MAC
        LDX   #]1
        LDY   #]2
        LDA   #]3
        <<<

        MultiParm $10,$20,$30

* Test 4: Merlin parameter separator (comma supported)
Separators MAC
        LDA   ]1
        STA   ]2
        <<<

* Comma is a supported parameter separator
        Separators $1000,$2000

* Test 5: >>> invocation syntax
TripleMac MAC
        LDX   #]1
        <<<

        >>> TripleMac $99

* Test 6: PMC invocation syntax
PmcMac MAC
        LDY   #]1
        <<<

* PMC: macro name followed by space then params
PMCTEST PMC   PmcMac $55

* Test 7: ]0 parameter (count)
CountParm MAC
        LDX   #]0        ; Number of parameters
        <<<

        CountParm
        CountParm $11
        CountParm $22,$33
        CountParm $44,$55,$66

* Test 8: Maximum parameters (]1 through ]8)
MaxParms MAC
        LDA   #]1
        LDX   #]2
        LDY   #]3
        STA   ]4
        STX   ]5
        STY   ]6
        JMP   ]7
        JSR   ]8
        <<<

        MaxParms $01,$02,$03,$04,$05,$06,$0700,$0800

* Test 9: Macro using parameter in expression
ExprMac MAC
        LDA   #]1+$10    ; Parameter in expression
        LDX   #]1-$10
        <<<

        ExprMac $50

* Test 10: Macros with local labels
LabelMac MAC
:LOCAL  LDA   #]1
        BNE   :LOCAL
        <<<

* Need global label context for local labels in macro
LABTEST LabelMac $88

* Test 11: EOM (alternate end macro)
EomTest MAC
        LDX   #]1
        EOM

        EomTest $99

* Test 12: Macro invoking another macro
FirstMac MAC
        LDA   #]1
        <<<

SecondMac MAC
        >>> FirstMac ]1
        STA   ]2
        <<<

        SecondMac $AA,$BB

        END
