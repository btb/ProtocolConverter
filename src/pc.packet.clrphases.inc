; 
ClrPhases:
        jsr     SetXN0
        lda     reqclr,x
.if     .defined(Grappler) || .defined(KBOOHK)
        lda     ca2clr,x
        lda     ca1clr,x
.else   ;Liron||IIc||SSC
        lda     ca1clr,x
        lda     ca2clr,x
.endif
LCB8F:
        lda     lstrbclr,x
        rts
