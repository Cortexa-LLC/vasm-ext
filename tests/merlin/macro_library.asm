* Macro Library for USE directive testing

* Copy memory block
MEMCPY    MAC
]SRC      EQU   ]1
]DST      EQU   ]2
]LEN      EQU   ]3
          LDY   #]LEN-1
:LOOP     LDA   ]SRC,Y
          STA   ]DST,Y
          DEY
          BPL   :LOOP
          <<<

* Fill memory with value
MEMFILL   MAC
]ADDR     EQU   ]1
]VALUE    EQU   ]2
]COUNT    EQU   ]3
          LDA   #]VALUE
          LDX   #]COUNT
:LOOP     STA   ]ADDR,X
          DEX
          BPL   :LOOP
          <<<

* 16-bit add
ADD16     MAC
          CLC
          LDA   ]1
          ADC   ]2
          STA   ]1
          LDA   ]1+1
          ADC   ]2+1
          STA   ]1+1
          <<<
