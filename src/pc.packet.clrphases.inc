; 
ClrPhases:
        jsr     SetXN0
        lda     reqclr,x
        lda     ca1clr,x
        lda     ca2clr,x
LCB8F:
        lda     lstrbclr,x
        rts
