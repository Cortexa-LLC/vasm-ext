* Comprehensive test for 65816 16-bit mode handling
* This tests MX directive, REP/SEP tracking, and XC mode transitions
org = $1000
 org org

*-------------------------------
* Test 1: MX directive
*-------------------------------
 xc
 xc
 mx %00        ; 16-bit A and X/Y
test1_16bit
 lda #$1234    ; should be 3 bytes: A9 34 12
 ldx #$5678    ; should be 3 bytes: A2 78 56
 ldy #$9ABC    ; should be 3 bytes: A0 BC 9A

 mx %11        ; 8-bit A and X/Y
test1_8bit
 lda #$12      ; should be 2 bytes: A9 12
 ldx #$34      ; should be 2 bytes: A2 34
 ldy #$56      ; should be 2 bytes: A0 56
 xc off

*-------------------------------
* Test 2: REP/SEP tracking
*-------------------------------
 xc
 xc
test2_rep
 clc
 xce           ; enter native mode
 rep $30       ; 16-bit A and X/Y (tracked)
 lda #$ABCD    ; should be 3 bytes
 ldx #$EF01    ; should be 3 bytes

 sep $20       ; 8-bit A, 16-bit X/Y (tracked)
 lda #$12      ; should be 2 bytes
 ldx #$3456    ; should be 3 bytes (X is still 16-bit)

 sep $10       ; 8-bit A and X/Y (tracked)
 ldy #$78      ; should be 2 bytes

 sec
 xce           ; back to emulation
 xc off

*-------------------------------
* Test 3: XC OFF resets to 8-bit
*-------------------------------
 xc
 xc
 rep $30       ; set 16-bit mode
 ldx #$BEEF    ; 3 bytes in 16-bit mode
 xc off        ; should reset to 8-bit

 xc            ; 65C02 mode
 lda #$42      ; should be 2 bytes (8-bit mode)
 xc off

 rts

 usr $a9,1,0,*-org
