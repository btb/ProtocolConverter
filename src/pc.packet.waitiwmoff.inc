WaitIWMOff:
;
; Make sure you're in read mode and wait 'til Disk // motor is off
;
.if     .defined(IIc)
        jsr     SetXN0      ;Set X
        lda     l7clr,x
        lda     l6set,x
wiwm1:
        lda     l7clr,x
        and     #%00100000
        bne     wiwm1
        lda     l6clr,x
;
; Wait an additional 700 microseconds to allow 12V on Disk // to decay
;
        phy
        ldy     #140
wiwm2:  dey
        bne     wiwm2
        ply

        rts
.endif
