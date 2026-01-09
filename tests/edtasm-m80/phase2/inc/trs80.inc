; TRS-80 Model III definitions
; Includes common.inc and adds screen/keyboard definitions

  INCLUDE "common.inc"

; Video memory
SCREEN  EQU $3C00
VDGCTRL EQU $EC00

; Keyboard buffer
KBDBUF  EQU $4000
KBDEND  EQU $4200
