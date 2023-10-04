; 
; 
; 
SendData:
        lda     #RC2
        ldy     #>RC2
        jsr     SendPile
        bcc     sdoubt
        lda     #CommReset
        jsr     AssignID
sdoubt: rts

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
.if     .defined(KBOOHK)
        lda     Retry2,x
        bmi     spilout
        cmp     #$02
        bcs     spile1
        eor     #$01
        sta     checksum
        tax
        lda     enable1+TheOff,x
        ldx     Slot
        lda     ProFLAG,x
        and     #$80
        ora     #$02
        ora     checksum
        sta     ProFLAG,x
        bne     spile1
.else
        bpl     spile1      ;If all fails, carry is set
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
