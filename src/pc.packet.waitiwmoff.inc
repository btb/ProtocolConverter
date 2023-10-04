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
.elseif .defined(SoftSP)
 .if    .defined(SSC)
        ldx     #TheOff
        lda     enable1,x
LCB93:
        lda     monset,x
        lda     l6clr,x
 .else  ;Grappler
        lda     enable1+TheOff
        lda     monset+TheOff
LCB9B:  lda     l6clr+TheOff
 .endif
        pha
LCBF0:
        lda     #$8c
LCBF2:
        sbc     #$01
        bne     LCBF2
        pla
LCBAB:  rts
.endif
