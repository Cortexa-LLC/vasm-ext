* Test USR directive for RW18 disk format
* Format: USR side,track,offset,length

         org   $8000

start    lda   #$00
         sta   $0400
         ldx   #$10
loop     dex
         bne   loop
         rts

* USR directive - write to side $a9, track 1, offset $0000
         usr   $a9,1,$0000,*-start
