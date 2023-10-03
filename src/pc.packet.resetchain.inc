resetchain:
        jsr     ClrPhases
        lda     reqset,x
        lda     ca2set,x
        ldy     #80         ;Hard reset for 80 ms
        jsr     YMSWait
        jsr     ClrPhases
; 
        ldy     #10         ;About 10 ms reset time!
; 
YMSWait:
        jsr     OneMS
        dey
        bne     YMSWait
        rts

OneMS:  ldx     #200
onems1: dex
        bne     onems1
        rts
