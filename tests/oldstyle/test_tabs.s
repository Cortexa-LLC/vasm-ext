	ORG $0C00

START	LDA #0
	JSR PrintChar
	RTS

PrintChar	PHA
		RTS
