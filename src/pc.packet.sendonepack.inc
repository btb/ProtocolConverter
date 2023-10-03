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
        jsr     WritePrep   ;Does a bunch of stuff
; 
; Enable PC chain.
; 
        jsr     enablechain ;This sets X reg
        ldy     #iwmmode    ;This is the mode value
        jsr     SetIWMode   ;Don't mess unless we gotta
; 
; Turn on the IWM
; 
        lda     enable2,x   ;Don't disturb //c internal drive
        lda     monset,x
; 
; Loop until the chain becomes unbusy
; 
        ldy     #bsyto1     ;Each loop is 11 microseconds
ubsy1:  lda     l7clr,x     ;Test if /BSY is hi or lo
        bmi     chainunbsy  ;If hi, bus is not busy
        dey
        bne     ubsy1       ;Keep trying
        sec
        jmp     sd10

; 
; Tell the bus that data is coming and send the sync bytes
; Sync is groups of eight 2's separated by a 6 (micS cell)
; (111111110011111111001111111100 ...)
; 
chainunbsy:
        lda     reqset,x    ;Raise REQ
; 
        ldy     #5          ;Sync plus packet begin
; 
        lda     #$ff        ;Send out the 1st byte sync
        sta     l7set,x
; 

ssb:
ssc:    lda     preamble,y
; 
; 
ssd:    asl     l6clr,x     ;Wait 'til buffer empty
        bcc     ssd
        sta     l6set,x
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
        jsr     SendByte
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
        jsr     SendByte
        lda     grp7ctr
        ora     #$80
        jsr     SendByte
; 
; Now send the "oddbytes" part of the packet contents
; 
        lda     oddbytes    ;Get # of "odd" bytes
        beq     sob2        ;Skip if no odd bytes
; 
        ldy     #$ff
        lda     tbodd       ;Get the odd bytes msb's (A[7]=1)

sob1:
        asl     l6clr,x     ;Do a write handshake
        bcc     sob1
        sta     l6set,x
        iny
        lda     (buffer),y  ;Get the data byte
        ora     #$80        ;Flip on the hi bit
        cpy     oddbytes    ;Are we done?
        bcc     sob1
; 
; Now send over the groups of seven contents
; Currently assume there must be at least one group of 'em
; 
sob2:   lda     grp7ctr     ;Check if there are groups to send
        bne     sob3        ;=> At least one group
        jmp     datdone     ;Skip to send checksum

sob3:   nop                 ;Waste 2 cycles
        ldy     #0
start:
        lda     topbits
        sta     l6set,x
; 
; Send first byte
; 
        lda     next1
        ora     #$80
        sty     temp        ;Swap Y for short handshake
ache1:  ldy     l6clr,x     ;Wait 'til buffer ready
        bpl     ache1
        sta     l6set,x     ;Send the byte
        ldy     temp        ;Get back Y
; 
; Prep the next "1st" byte for next time
; 
        lda     (buffer2),y
        sta     next1
        asl     A
        rol     topbits     ;Store the top bit
        iny
; 
; It's possible that we're at a page boundary now. If so, bump the hi order part
; of the pointer.
; 
        bne     skip1
        inc     buffer2+1
        jmp     skip2

skip1:
        pha                 ;Equalize the cases
        pla
; 
; Push us ahead by an additional 8 cycles for margin reasons
; Plus I gotta get the topbits MSB set somehow...
; 
skip2:  lda     #%00000010  ;Flip what will be MSB
        ora     topbits
        sta     topbits
; 
; Send the second byte
; 
        lda     next2
        ora     #$80
        sta     l6set,x     ;Send the byte
        lda     (buffer2),y
        sta     next2
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
; 
; Send the third byte
; 
        lda     next3
        ora     #$80
        sta     l6set,x     ;Send the byte
        lda     (buffer2),y
        sta     next3
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte

; 
; Send the fourth byte
; 
        lda     next4
        ora     #$80
        sta     l6set,x     ;Send the byte
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
        jmp     skip4

skip3:
        pha
        pla
skip4:
; 
; Send the fifth byte
; 
        lda     next5
        ora     #$80
        sta     l6set,x     ;Send the byte
        lda     (buffer2),y
        sta     next5
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte

; 
; Send the sixth byte
; 
        lda     next6
        ora     #$80
        sta     l6set,x     ;Send the byte
        lda     (buffer2),y
        sta     next6
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
; 
; Send the last byte of the group
; 
        lda     next7
        ora     #$80
        sta     l6set,x     ;Send the byte
        lda     (buffer2),y
        sta     next7
        asl     A
        rol     topbits     ;Store the top bit
        iny                 ;Next byte
; 
; Now see if we have sent enough groups of seven
; 
        dec     grp7ctr
        beq     datdone
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
scm1:
        ldy     l6clr,x
        bpl     scm1        ;Handshake this byte
        sta     l6set,x     ;These are even bits
; 
        lda     checksum    ;c7 c6 c5 c4 c3 c2 c1 c0
        lsr     A           ; 0 c7 c6 c5 c4 c3 c2 c1
        ora     #$aa        ; 1 c7  1 c5  1 c3  1 c1
        jsr     SendByte
; 
; Send the end of packet mark
; 
        lda     #packetend
        jsr     SendByte
; 
; Wait until write underflow
; 
sd7:
        lda     l6clr,x
        and     #$40
        bne     sd7         ;Still writing data
; 
        sta     l6set,x     ;Back to sense mode (dummy write)
; 
; Now wait until the drive acknowledges receipt of the string or until timeout
; 
        ldy     #bsyto2     ;Load timeout to see bsy low
patch1: dey                 ;A little closer to an error
        bne     sd9         ;There's still time
; 
; Too much time has elapsed. Drive didn't get string.
; 
LCB71:
        lda     #ERR0_noanswer ;Report error in comm error byte
dberror:
        jsr     SetXN0      ;For dberror entry
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
