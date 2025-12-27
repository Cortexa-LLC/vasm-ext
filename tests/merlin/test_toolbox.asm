* Test Apple IIgs Toolbox calling convention
* Demonstrates Tool macro and typical GS/OS patterns

        MX    %00          ; 65816 mode
        ORG   $8000

* Tool macro - generates JSL $E10000 followed by tool number
Tool    MAC
        JSL   $E10000     ; GS Toolbox dispatcher
        DW    ]1          ; Tool call number
        <<<

* Toolbox wrapper macros
_TLStartUp MAC
        Tool  $201        ; Tool Locator StartUp
        <<<

_MMStartUp MAC
        Tool  $202        ; Memory Manager StartUp
        <<<

_SomeToolCall MAC
        Tool  $1234
        <<<

* Sample GS/OS application initialization
START   PHK               ; Set data bank = program bank
        PLB

* Initialize Tool Locator
        _TLStartUp
        PHA               ; Save user ID

* Initialize Memory Manager
        _MMStartUp
        PLA               ; Retrieve user ID
        STA   userID      ; Store for later

* Typical parameter block usage
        PEA   0           ; Push parameters
        PEA   ^dataBlock  ; Bank byte
        PEA   dataBlock   ; Address
        _SomeToolCall     ; Make tool call

        RTL

* Data
userID  DW    0
dataBlock DS  16

        END
