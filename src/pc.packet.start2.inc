start2:
.if     .defined(IIc)
        jmp     SlotDepRd
.else   ;Liron
        jmp     (pointer)
.endif

Send80: lda     #$80
SendByte:
        ldy     l6clr,x
        bpl     SendByte
        sta     l6set,x
        eor     checksum
        sta     checksum
        rts
