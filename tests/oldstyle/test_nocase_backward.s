; Test backward reference
	ORG $0C00

PrintStr	LDY #0
		RTS

START	JSR PrintStr
	RTS
