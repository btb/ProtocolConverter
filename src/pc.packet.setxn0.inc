; 
SetXN0:
.if     .defined(IIc) || .defined(SoftSP)
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
