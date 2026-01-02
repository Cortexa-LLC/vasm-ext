* Test 65816 16-bit immediate mode after REP $30
* This tests that vasm correctly tracks M/X register size after REP/SEP

org = $1000
 org org

 xc          ; 6502 -> 65C02
 xc          ; 65C02 -> 65816

* Switch to native mode
 clc
 xce

* Set 16-bit A and X/Y (M=0, X=0)
 rep $30

* These should all be 3-byte instructions (opcode + 16-bit immediate)
 lda #$1234   ; A9 34 12
 ldx #$0C03   ; A2 03 0C
 ldy #$5678   ; A0 78 56

* Switch back to 8-bit A (M=1), keep 16-bit X/Y (X=0)
 sep $20

* A is now 8-bit, X/Y still 16-bit
 lda #$12     ; A9 12 (2 bytes)
 ldx #$ABCD   ; A2 CD AB (3 bytes)
 ldy #$EF01   ; A0 01 EF (3 bytes)

* Switch back to 8-bit X/Y (X=1), keep 8-bit A (M=1)
 sep $10

* Now both A and X/Y are 8-bit
 lda #$99     ; A9 99 (2 bytes)
 ldx #$88     ; A2 88 (2 bytes)
 ldy #$77     ; A0 77 (2 bytes)

* Back to emulation mode
 sec
 xce
 rts

 xc off

 usr $a9,1,0,*-org
