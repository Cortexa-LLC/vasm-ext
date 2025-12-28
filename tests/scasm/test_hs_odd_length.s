* Test: .HS directive with odd-length hex strings
* SCASM pads odd-length hex strings with leading zero

        .OR $2000

* Even length - should work as normal
        .HS 00              ; Single byte: 0x00
        .HS 1234            ; Two bytes: 0x12 0x34
        .HS ABCDEF          ; Three bytes: 0xAB 0xCD 0xEF

* Odd length - single digit (padded to 0xN)
        .HS 5               ; Becomes 0x05
        .HS A               ; Becomes 0x0A
        .HS F               ; Becomes 0x0F

* Odd length - three digits (padded to 0xNNN)
        .HS 123             ; Becomes 0x01 0x23
        .HS ABC             ; Becomes 0x0A 0xBC
        .HS FED             ; Becomes 0x0F 0xED

* Odd length - five digits (padded to 0xNNNNN)
        .HS 12345           ; Becomes 0x01 0x23 0x45
        .HS ABCDE           ; Becomes 0x0A 0xBC 0xDE

* Mixed with spaces and periods (SCASM allows these as separators)
        .HS 1.2.3           ; Three digits: 0x01 0x23
        .HS A B C D E       ; Five digits: 0x0A 0xBC 0xDE

* With inline comments (hex parsing stops at non-hex)
        .HS 1234 Two bytes  ; Comment after hex
        .HS F Single digit  ; Comment after odd hex

        .END

* Expected output (29 bytes):
* 00 12 34 AB CD EF - even length examples
* 05 0A 0F           - single digit padded
* 01 23 0A BC 0F ED  - three digits padded
* 01 23 45 0A BC DE  - five digits padded
* 01 23 0A BC DE     - with separators
* 12 34 0F           - with comments
