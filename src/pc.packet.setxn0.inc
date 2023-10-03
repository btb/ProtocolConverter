; 
SetXN0:
.if     .defined(IIc)
        ldx     #TheOff
.else   ;Liron
        lda     Slot
        asl     A
        asl     A
        asl     A
        asl     A
        tax
.endif
; 
        rts
