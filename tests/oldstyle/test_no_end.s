		ORG $0C00

START:	JSR NewLine
		JSR PrintStr
		RTS

HelloWorld	DB	"Hello", 255

PrintStr	LDY #0
		RTS
