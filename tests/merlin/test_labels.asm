* Comprehensive Label System Test
* Tests: Global, :LOCAL, ]VARIABLE labels and scoping

        ORG   $8000

* Test 1: Global labels
GLOBAL1   NOP
          LDA   GLOBAL1
GLOBAL2   NOP
          LDA   GLOBAL2

* Test 2: Local labels (:LABEL) scoped to globals
FUNC1     NOP
:INIT     LDA   #$00
:LOOP     STA   $0400,X
          INX
          BNE   :LOOP
          RTS

FUNC2     NOP
:INIT     LDA   #$FF        ; Different :INIT (scoped to FUNC2)
:LOOP     STA   $0500,X     ; Different :LOOP (scoped to FUNC2)
          INX
          BNE   :LOOP
          RTS

* Test 3: Calling functions with local labels
MAIN      JSR   FUNC1
          JSR   FUNC2
          RTS

* Test 4: Variable labels (]VARIABLE) - mutable, backward-ref only
]COUNT    EQU   0           ; Initial value

]COUNT    EQU   ]COUNT+1    ; Increment: now 1
          LDA   #]COUNT     ; LDA #$01

]COUNT    EQU   ]COUNT+1    ; Increment: now 2
          LDA   #]COUNT     ; LDA #$02

]COUNT    EQU   ]COUNT+1    ; Increment: now 3
          LDA   #]COUNT     ; LDA #$03

* Test 5: Variable labels in loops
]INDEX    EQU   0
          LUP   5
]INDEX    EQU   ]INDEX+1
          LDA   #]INDEX
          --^

* Test 6: Mixing global, local, and variable labels
ROUTINE   NOP
]TEMP     EQU   $80
:START    LDA   ]TEMP
:PROCESS  STA   $0400
          INC   ]TEMP
          BNE   :START
          RTS

* Test 7: Local label forward references (within same global scope)
FORWARD   BNE   :SKIP       ; Forward ref to :SKIP
          LDA   #$00
:SKIP     RTS               ; Target of forward ref

* Test 8: Local label backward references
BACKWARD  NOP
:TARGET   LDA   #$FF
          DEX
          BNE   :TARGET     ; Backward ref to :TARGET
          RTS

* Test 9: Variable labels with conditional assembly
]DEBUG    EQU   1

          DO    ]DEBUG
          LDA   #$FF        ; Debug code
          FIN

]DEBUG    EQU   0           ; Disable debug

          DO    ]DEBUG
          LDA   #$00        ; This won't assemble
          FIN

* Test 10: Complex label scoping scenario
COMPLEX   NOP
]BASE     EQU   $1000
:OUTER    LDA   ]BASE
]OFFSET   EQU   0
:INNER    STA   ]BASE+]OFFSET
]OFFSET   EQU   ]OFFSET+1
          CPX   #$10
          BNE   :INNER
          RTS

* Test 11: Label name reuse across global scopes
BLOCK1    NOP
:COMMON   LDA   #$01
          RTS

BLOCK2    NOP
:COMMON   LDA   #$02        ; Different :COMMON (scoped to BLOCK2)
          RTS

* Test 12: Variable label as address pointer
]PTR      EQU   $40         ; Use zero-page address for indirect

          LDA   (]PTR),Y
]PTR      EQU   ]PTR+$10    ; Next ZP offset
          LDA   (]PTR),Y

        END
