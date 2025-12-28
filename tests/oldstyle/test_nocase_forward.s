; Test forward reference with nocase
	ORG $0C00

START	JSR PrintStr
	RTS

PrintStr	LDY #0
		RTS
