;*******************************************************************************
;*                                                                             *
;* SendOnePack                   Send a CBus Packet                            *
;*                                                                             *
;*   This routine sends a packet of data across the bus. The protocol is as    *
;* follows:                                                                    *
;*              ____________________________                                   *
;*  REQ _______|2                          5|_________                         *
;*          _______________________________                                    *
;* /BSY ___|1               3             4|__________                         *
;*                                                                             *
;*    1)  Device signals ready for data                                        *
;*    2)  Host signals data imminent                                           *
;*    3)  Packet is transmitted (sync, command mark, ids, contents, checksum   *
;* [msb=1])                                                                    *
;*    4)  Device signals packet received                                       *
;*    5)  Host finishes send data cycle                                        *
;*                                                                             *
;*   The bytes are sent in slow mode (32 cycles/byte) and the timing is        *
;* critical. Branches which should not cross page boundaries are marked.       *
;*                                                                             *
;*  Input:  buffer (2 bytes) <- ptr to data to send                            *
;*          bytecount(2)     <- length (bytes) of data                         *
;*          packettype (1)   <- command or data packet                         *
;*          CMDUnit (1)      <- # of device to receive                         *
;*                                                                             *
;*  Output: carry set- handshake error                                         *
;*                clr- bytes sent                                              *
;*******************************************************************************
;* Prep for the transmission                                                   *
;*******************************************************************************

SendOnePack:
.if     .defined(KBOOHK)
        jsr     WritePrep2  ;Does a bunch of stuff
.else   ;KBOOHK
        jsr     WritePrep   ;Does a bunch of stuff
.endif
; 
; Enable PC chain.
; 
        jsr     enablechain ;This sets X reg
.if     .defined(Liron) || .defined(IIc)
        ldy     #iwmmode    ;This is the mode value
        jsr     SetIWMode   ;Don't mess unless we gotta
; 
; Turn on the IWM
; 
        lda     enable2,x   ;Don't disturb //c internal drive
        lda     monset,x
.else   ;SoftSP
        lda     auxptr
        beq     LCA0C
 .if    .defined(SSC)
        nop
 .endif
        .byte   $2c         ;combines with next line as 'bit $57c6'
LCA0C:  dec   auxptr+1
.endif

; 
; Loop until the chain becomes unbusy
; 
        ldy     #bsyto1     ;Each loop is 11 microseconds
.ifdef  SoftSP
        lda     l6set,x
.endif
ubsy1:  lda     l7clr,x     ;Test if /BSY is hi or lo
        bmi     chainunbsy  ;If hi, bus is not busy
        dey
        bne     ubsy1       ;Keep trying
        sec
        jmp     sd10

.ifdef  SoftSP
jmpdatdone:
        jmp     datdone
.endif

; 
; Tell the bus that data is coming and send the sync bytes
; Sync is groups of eight 2's separated by a 6 (micS cell)
; (111111110011111111001111111100 ...)
; 
chainunbsy:
.if     .defined(Liron) || .defined(IIc) || .defined(SoftSP)
        lda     reqset,x    ;Raise REQ
.endif
; 
.if     .defined(Liron) || .defined(IIc) || .defined(SSC)
        ldy     #5          ;Sync plus packet begin
.else   ;Grappler||KBOOHK
        nop
.endif
; 
        lda     #$ff        ;Send out the 1st byte sync
        sta     l7set,x
; 
.if     .defined(SoftSP)
        ora     l6clr,x
 .if    .defined(SSC)
        and     ($00),y
 .else  ;Grappler||KBOOHK
        ldy     #5          ;Sync plus packet begin
        bit     topbits
 .endif
.endif

ssb:
.ifdef  SoftSP
        jsr     noprts
.endif
ssc:    lda     preamble,y
.if     .defined(Liron) || .defined(IIc)
; 
; 
ssd:    asl     l6clr,x     ;Wait 'til buffer empty
        bcc     ssd
.endif
        sta     l6set,x
.ifdef  SoftSP
        cmp     l6clr,x
.endif
        dey
        bpl     ssb         ;Back for more bytes
; 
; Send over the destination ID
; 
        lda     Unit
        ora     #$80        ;Make the device ID
        jsr     SendByte
; 
; Send the source ID (that's us... we're an $80)
; 
        jsr     Send80
; 
; Send over the packet type (command or data)
; 
        lda     WPacketType
.if     .defined(Liron) || .defined(IIc)
        jsr     SendByte
.else   ;SoftSP
        jsr     SendByte2
.endif
; 
; Send the Auxiliary Type byte (an $80 from this rev PC)
; 
        jsr     Send80
; 
; Send the status byte (null for us), and length bytes
; 
        jsr     Send80
        lda     oddbytes
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        jsr     SendByte
.else   ;SoftSP
        jsr     SendByte3
.endif
        lda     grp7ctr
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        jsr     SendByte
.else   ;SoftSP
        jsr     SendByte3
.endif
; 
; Now send the "oddbytes" part of the packet contents
; 
        lda     oddbytes    ;Get # of "odd" bytes
        beq     sob2        ;Skip if no odd bytes
; 
.if     .defined(Liron) || .defined(IIc)
        ldy     #$ff
.endif
        lda     tbodd       ;Get the odd bytes msb's (A[7]=1)
.ifdef  SoftSP
        ldy     #$ff
        bne     LCA74
.endif

sob1:
.if     .defined(Liron) || .defined(IIc)
        asl     l6clr,x     ;Do a write handshake
        bcc     sob1
        sta     l6set,x
        iny
.else   ;SoftSP
        nop
        nop
        nop
.endif
        lda     (buffer),y  ;Get the data byte
        ora     #$80        ;Flip on the hi bit
.ifdef  SoftSP
LCA74:
        nop
        sta     l6set,x
        cmp     l6clr,x
        iny
.endif
        cpy     oddbytes    ;Are we done?
        bcc     sob1
.ifdef  SoftSP
        bit     topbits
.endif

; 
; Now send over the groups of seven contents
; Currently assume there must be at least one group of 'em
; 
sob2:   lda     grp7ctr     ;Check if there are groups to send
.if     .defined(Liron) || .defined(IIc)
        bne     sob3        ;=> At least one group
        jmp     datdone     ;Skip to send checksum

sob3:   nop                 ;Waste 2 cycles
        ldy     #0
start:
        lda     topbits
        sta     l6set,x
.else   ;SoftSP
        beq     jmpdatdone
        ldy     buffer2
        dey
        lda     #0
        sta     buffer2
        lda     topbits
start:
        sta     l6set+TheOff
        cmp     l6clr+TheOff
.endif

; 
; Send first byte
; 
        lda     next1
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sty     temp        ;Swap Y for short handshake
ache1:  ldy     l6clr,x     ;Wait 'til buffer ready
        bpl     ache1
        sta     l6set,x     ;Send the byte
        ldy     temp        ;Get back Y
.else   ;SoftSP
        iny
        beq     LCA9E
        nop
        .byte   $2c         ;combines with next line as 'bit $57e6'
LCA9E:
        inc     buffer2+1
        nop
        nop
        nop
        ldx     next3
        sta     l6set+TheOff
        cmp     l6clr+TheOff
.endif

; 
; Prep the next "1st" byte for next time
; 
        lda     (buffer2),y
        sta     next1
        asl     A
.if     .defined(Liron) || .defined(IIc)        
        rol     topbits     ;Store the top bit
.else   ;SoftSP
        rol     A
        ora     #%00000010
        sta     topbits
.endif
        iny
; 
; It's possible that we're at a page boundary now. If so, bump the hi order part
; of the pointer.
; 
        bne     skip1
        inc     buffer2+1
.if     .defined(Liron) || .defined(IIc)        
        jmp     skip2
.endif

skip1:
.if     .defined(Liron) || .defined(IIc)        
        pha                 ;Equalize the cases
        pla
; 
; Push us ahead by an additional 8 cycles for margin reasons
; Plus I gotta get the topbits MSB set somehow...
; 
skip2:  lda     #%00000010  ;Flip what will be MSB
        ora     topbits
        sta     topbits
.endif

; 
; Send the second byte
; 
.if     .defined(Liron) || .defined(IIc)        
        lda     next2
.else   ;SoftSP
        lda     a:next2
.endif
        ora     #$80
.if     .defined(Liron) || .defined(IIc)        
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next2
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
.ifdef  SoftSP
        bne     LCAD1
        inc     buffer2+1
LCAD1:
.endif

; 
; Send the third byte
; 
.if     .defined(Liron) || .defined(IIc)
        lda     next3
.else   ;SoftSP
        txa
.endif
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next3
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
.ifdef  SoftSP
        bne     LCAE6
        inc     buffer2+1
LCAE6:
.endif

; 
; Send the fourth byte
; 
        lda     next4
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        ldx     next5
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next4
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
; 
; After the first 256 bytes, we will cross pages here. If we did cross, bump the
; buffer pointer. If no, equalize the cases with seven cycles of time wasting.
; 
        bne     skip3
        inc     buffer2+1
.if     .defined(Liron) || .defined(IIc)
        jmp     skip4
.endif

skip3:
.if     .defined(Liron) || .defined(IIc)
        pha
        pla
.endif
skip4:
; 
; Send the fifth byte
; 
.if     .defined(Liron) || .defined(IIc)
        lda     next5
.else   ;SoftSP
        txa
.endif
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next5
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
.ifdef  SoftSP
        bne     LCB13
        inc     buffer2+1
LCB13:
.endif

; 
; Send the sixth byte
; 
        lda     next6
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        ldx     next7
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next6
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
.ifdef  SoftSP
        bne     LCB2B
        inc     buffer2+1
LCB2B:
.endif
; 
; Send the last byte of the group
; 
.if     .defined(Liron) || .defined(IIc)
        lda     next7
.else   ;SoftSP
        txa
.endif
        ora     #$80
.if     .defined(Liron) || .defined(IIc)
        sta     l6set,x     ;Send the byte
.else   ;SoftSP
        sta     l6set+TheOff ;Send the byte
        cmp     l6clr+TheOff
.endif
        lda     (buffer2),y
        sta     next7
        asl     A
.if     .defined(Liron) || .defined(IIc)
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
.else   ;SoftSP
        lda     topbits
        rol     A
.endif
; 
; Now see if we have sent enough groups of seven
; 
        dec     grp7ctr
        beq     datdone
.ifdef  SoftSP
        bit     topbits
.endif

; 
; Otherwise, back to do more. Note it's too far for a branch.
; 
        jmp     start

; 
; Whew! Now send the damn checksum as two FM bytes
; 
datdone:
        lda     checksum    ;c7 c6 c5 c4 c3 c2 c1 c0
        ora     #$aa        ; 1 c6  1 c4  1 c2  1 c0
.if     .defined(Liron) || .defined(IIc)
scm1:
        ldy     l6clr,x
        bpl     scm1        ;Handshake this byte
        sta     l6set,x     ;These are even bits
.else   ;SoftSP
        sta     l6set+TheOff ;These are even bits
        cmp     l6clr+TheOff
        ldx     #TheOff
.endif
; 
        lda     checksum    ;c7 c6 c5 c4 c3 c2 c1 c0
        lsr     A           ; 0 c7 c6 c5 c4 c3 c2 c1
        ora     #$aa        ; 1 c7  1 c5  1 c3  1 c1
        jsr     SendByte
; 
; Send the end of packet mark
; 
        lda     #packetend
.if     .defined(Liron) || .defined(IIc)
        jsr     SendByte
.else   ;SoftSP
        jsr     SendByte0
 .if    .defined(SSC)
        jsr     bitearts
 .else  ;KBOOHK||Grappler
        jsr     SendByte1
 .endif
.endif
; 
; Wait until write underflow
; 
sd7:
.ifdef  SoftSP
        lda     l7clr,x
.endif
        lda     l6clr,x
.if     .defined(Liron) || .defined(IIc)
        and     #$40
        bne     sd7         ;Still writing data
; 
        sta     l6set,x     ;Back to sense mode (dummy write)
.endif

; 
; Now wait until the drive acknowledges receipt of the string or until timeout
; 
        ldy     #bsyto2     ;Load timeout to see bsy low
.ifdef  SoftSP
        lda     l6set,x
.endif
patch1: dey                 ;A little closer to an error
        bne     sd9         ;There's still time
; 
; Too much time has elapsed. Drive didn't get string.
; 
LCB71:
        lda     #ERR0_noanswer ;Report error in comm error byte
dberror:
.if     .defined(Liron) || .defined(IIc)
        jsr     SetXN0      ;For dberror entry
.else
        ldx     #TheOff
.endif
        sec                 ;Signal a problem
        bcs     sd10

; 
; See if drive has acknowledged the bytes yet
; 
sd9:    lda     l7clr,x     ;Wait 'til /BSY lo
        bmi     patch1

; 
; Finish the sequence
; 
        clc                 ;This is a normal exit
sd10:   lda     reqclr,x    ;Set REQ lo
        lda     l6clr,x     ;Back into read mode
; 
; Pull back the bytecount in all cases
; 
        rts
