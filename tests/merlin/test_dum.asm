* Test DUM and DEND directives for Merlin
* Dummy sections define variables without allocating space in output
* UNALLOCATED sections should NOT add to output file size

         org $1000

*-------------------------------
* Test 1: Code before dummy section
*-------------------------------
start    lda #$00        ; 2 bytes
         sta $10         ; 2 bytes

*-------------------------------
* Test 2: Define zero-page variables
*-------------------------------
         dum $00
ptr      ds 2            ; 2 bytes (NOT in output)
count    ds 1            ; 1 byte (NOT in output)
temp     ds 1            ; 1 byte (NOT in output)
         dend

*-------------------------------
* Test 3: Code after dummy - should be at $1004
*-------------------------------
cont     ldx ptr         ; 2 bytes (references $00)
         ldy count       ; 2 bytes (references $02)
         inx             ; 1 byte
         rts             ; 1 byte

*-------------------------------
* Test 4: Multiple DUM sections
*-------------------------------
         dum $80
buffer   ds 32           ; 32 bytes (NOT in output)
bufend   ds 0
         dend

more     lda buffer      ; 2 bytes (references $80)
         rts             ; 1 byte

*-------------------------------
* Test 5: DUM with explicit address
*-------------------------------
         dum $F8
screen   ds 2
cursor   ds 2
         dend

final    lda screen      ; 2 bytes (references $F8)
         sta cursor      ; 2 bytes (references $FA)
         rts             ; 1 byte

* Expected output size: 18 bytes total
* (4 + 6 + 3 + 5 = 18 bytes of actual code)
* DUM sections (4 + 32 + 4 = 40 bytes) should NOT be in output
