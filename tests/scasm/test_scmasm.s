* Simple SCMASM syntax test
* This tests basic features

        .OR $8000

START   LDA #$00        ; Load zero
        STA $D020       ; Store to border color
        RTS             ; Return

DATA    .DA $1234,$5678
HEXDATA .HS 01020304
