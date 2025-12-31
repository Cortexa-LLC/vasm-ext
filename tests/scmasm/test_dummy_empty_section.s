* Test: Empty section before .OR should not generate padding
* This reproduces the IO.TWO.E pattern where X.DATA is included
* before the first .OR directive, creating an empty default section
* at address 0 that should NOT cause padding in the output.

* No .OR here - creates default section at $0000

* Include a dummy section (like X.DATA does)
        .DUMMY
        .OR $50
ZPVAR1  .BS 1           ; Zero-page variable
ZPVAR2  .BS 2           ; Another variable
        .ED             ; Returns to default section at $0000

* Now set the actual code origin
        .OR $6000

* Actual code at $6000
START   LDA #$42
        STA $D020
        RTS

* Expected output: Only 7 bytes (A9 42 8D 20 D0 60)
* NOT: $6000 bytes of padding followed by 7 bytes of code
