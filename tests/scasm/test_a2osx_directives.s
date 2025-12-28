* Test A2osX editor directives
* Tests NEW, AUTO, .TF, and TEXT directives
*
* Expected output: 22 bytes
* Metadata extracted: BIN/TEST and SRC/TEST.S
NEW
AUTO 3,1
				.TF BIN/TEST
*--------------------------------------
* Simple Test Program
*--------------------------------------
				.OR $8000
*--------------------------------------
START			LDA #$00
				STA $D020
				LDX #$00
.1				LDA MSG,X
				BEQ .2
				INX
				BNE .1
.2				RTS
*--------------------------------------
MSG				.AZ "A2osX"
*--------------------------------------
				TEXT SRC/TEST.S
