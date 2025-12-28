; Test mixed case references
	ORG $0C00

MyLabel	LDA #0
	JSR MYLABEL
	JSR mylabel  
	JSR MyLabel
	RTS
