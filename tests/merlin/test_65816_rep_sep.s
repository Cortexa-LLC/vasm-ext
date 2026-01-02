* Test 65816 16-bit immediate
org = $1000
 org org

 xc
 xc
 clc
 xce          ; switch to native mode
 rep $30      ; 16-bit A and X/Y
 ldx #$0C03   ; should be A2 03 0C (3 bytes)
 hex 22,00,00,E1
 sec
 xce          ; back to emulation
 rts
 xc off

 usr $a9,1,0,*-org
