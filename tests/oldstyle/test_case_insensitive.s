; Test that -nocase actually works for case-insensitive matching
	ORG $0C00

START	JSR printstr
	RTS

PrintStr	LDY #0
		RTS
