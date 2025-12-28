; Test with std syntax
	.org $0C00

START:	jsr Foo
	rts

Foo:	rts
