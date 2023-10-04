start2:
.if     .defined(IIc)
        jmp     SlotDepRd
.elseif .defined(Liron)
        jmp     (pointer)
.elseif .defined(SSC) || .defined(Grappler)
bitearts:
        .byte   $24
noprts: nop
        rts
.endif

Send80: lda     #$80
.ifdef  SoftSP
        bne     SendByte3
.endif
SendByte:
.if     .defined(Liron) || .defined(IIc)
        ldy     l6clr,x
        bpl     SendByte
.else   ;SoftSP
        bit     topbits
SendByte1:
        nop
SendByte0:
        .byte   $c5
SendByte2:
        nop
SendByte3:
.endif
        sta     l6set,x
.ifdef  SoftSP
        cmp     l6clr,x
.endif
        eor     checksum
        sta     checksum
        rts
