; 
; 
; 
SendData:
        lda     #RC2
        ldy     #>RC2
        jsr     SendPile
        bcc     sdoubt
        lda     #CommReset
.if     .defined(Grappler) || .defined(KBOOHK)
        jmp     AssignID
.else
        jsr     AssignID
sdoubt: rts
.endif

; 
SendPack:
        jsr     SendPile    ;Try to send a pack
        bcc     sdoubt
        lda     #CommReset  ;This is a communications failure
        jsr     AssignID    ;Reset to try again
; 
        lda     SvBcL       ;Get back the packetlength
        sta     ByteCountl
        lda     SvBcH
        sta     ByteCounth
; 
SendPile:
        lda     #<RC1       ;Retry count (big!)
        ldy     #>RC1
; 
AltSendPile:
        ldx     Slot
        sta     Retry,x
        tya
        sta     Retry2,x
; 
; SendPack destroys the bytecount
; 
spile1: lda     ByteCountl
        sta     SvBcL
        lda     ByteCounth
        sta     SvBcH
; 
        jsr     SendOnePack ;Send the packet
; 
        lda     SvBcL
        sta     ByteCountl
        lda     SvBcH
        sta     ByteCounth
; 
        bcc     spilout
        ldx     Slot
        dec     Retry,x
        bne     spile1
        dec     Retry2,x
.if     .defined(Grappler) || .defined(KBOOHK)
        lda     Retry2,x
        bmi     spilout
        cmp     #$08
        bcs     spile1
        lsr     a
        lsr     a
        eor     #$01
        jsr     LCB3B
        ldx     Slot
        bne     spile1
.else
        bpl     spile1      ;If all fails, carry is set
.endif
.if     .defined(Grappler) || .defined(KBOOHK)
sdoubt:
.endif
spilout: rts

RecPack:
        ldy     Slot
        lda     #RC2
        sta     Retry,y
rpk1:   jsr     ReceivePack
        bcc     rpout
        ldy     #1
        jsr     YMSWait
        jsr     dberror     ;Recycle handshake and set carry
        ldx     Slot
        dec     Retry,x
        bne     rpk1        ;Carry set still
rpout:  rts
