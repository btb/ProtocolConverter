; 
enablechain:
        jsr     SetXN0
        lda     ca1set,x
        lda     lstrbset,x
        rts
