* Hello World Sample Program

NewLine	EQU $FC62   ; CR - Carriage Return to Screen

		ORG $0C00

START:	JSR NewLine

		;Load the address of the message into the zero page
		LDA #>HelloWorld
		STA $41         ; H byte
		LDA #<HelloWorld
		STA $40         ; L byte

		JSR	PrintStr
		RTS

HelloWorld	DB	"Hello, World!", 255

PrintChar	PHA
				CLC
				ADC #128    ; correction for character map
				JSR $FDF0   ; COUT1 - output char to Screen
			PLA
			RTS

PrintStr	LDY #0

PS_loop		LDA ($40),Y

			CMP $FF
			BEQ PS_done

			JSR PrintChar
			INY
			JMP PS_loop

PS_done		RTS

			END