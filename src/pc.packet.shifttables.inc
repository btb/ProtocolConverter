; 
; Shift tables for use when reading. Each table should not straddle pages.
; 
shift1: .byte   $80,$80,$80,$80,$80,$80,$80,$80
        .byte   $00,$00,$00,$00,$00,$00,$00,$00
shift2: .byte   $80,$80,$80,$80,$00,$00,$00,$00
        .byte   $80,$80,$80,$80,$00,$00,$00,$00
shift3: .byte   $80,$80,$00,$00,$80,$80,$00,$00
        .byte   $80,$80,$00,$00,$80,$80,$00,$00
shift4: .byte   $80,$00,$80,$00,$80,$00,$80,$00
        .byte   $80,$00,$80,$00,$80,$00,$80
.if     !.defined(Grappler)
;uses overlapping of first byte of next constant
        .byte   $00
.endif
