; 
SetXN0:
.if     .defined(IIc) || .defined(SSC)
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
