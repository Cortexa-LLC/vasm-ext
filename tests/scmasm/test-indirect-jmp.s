* Test indirect JMP addressing mode
*
* Bug: JMP (address) was assembling as JMP address ($4C opcode)
*      instead of JMP (address) ($6C opcode)
* Fix: Added indir check to ABS addressing mode case in cpu.c
*
* This bug affected ALL vasm syntaxes (SCASM, Merlin, etc.)

       .OR $1000

* Direct JMP - should use opcode $4C
DIRECT.JMP
       JMP $2000          ; Expected: 4C 00 20

* Indirect JMP - should use opcode $6C
INDIRECT.JMP
       JMP ($2000)        ; Expected: 6C 00 20

* Indirect JMP with label
VECTOR .EQ $BD0A
INDIRECT.LABEL
       JMP (VECTOR)       ; Expected: 6C 0A BD

* Indirect JMP to zero page
INDIRECT.ZP
       JMP ($00FF)        ; Expected: 6C FF 00

* Verify both forms can coexist
TEST.BOTH
       JMP DIRECT.JMP     ; Direct: 4C 00 10
       JMP (VECTOR)       ; Indirect: 6C 0A BD
