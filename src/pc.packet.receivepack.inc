;*******************************************************************************
;*                                                                             *
;* ReceivePack        Get a packet from bus resident                           *
;*              ____________________________                                   *
;*  REQ _______|2                          5|_________                         *
;*          _______________________________                                    *
;* /BSY ___|1               3             4|__________                         *
;*    1)  Drive signals ready to send packet                                   *
;*    2)  Host signals ready to receive data                                   *
;*    3)  Packet is transmitted (sync, mark, IDs, data, checksum [msb=1])      *
;*    4)  Drive signals packet dispatched                                      *
;*    5)  Host acknowledges receipt of packet                                  *
;*                                                                             *
;* The bytes are sent in slow mode (32 cycles/byte) and the timing is          *
;* critical. Branches which should not cross page boundaries are marked.       *
;*                                                                             *
;* Input:  buffer <- address where packet guts left                            *
;*                                                                             *
;* Output: carry set- handshake error                                          *
;*               clr- bytes received                                           *
;*         A <- error0 if carry set                                            *
;*                                                                             *
;*******************************************************************************
;* Init the checksum                                                           *
;*******************************************************************************
ReceivePack:
.if     .defined(Liron) || .defined(IIc)
        lda     #$00
.else   ;SoftSP
        lda     #$80
.endif
        sta     checksum
; 
; Copy over buffer -> buffer2
; 
        lda     buffer
        sta     buffer2
        lda     buffer+1
        sta     buffer2+1
; 
; Set up the indirect pointer for jump to 2nd part of code
; 
.if     .defined(Liron)
        lda     #<SlotDepRd
        sta     pointer
        lda     Slot
        clc
        adc     #$c0
        sta     pointer+1
.endif
; 
        jsr     enablechain ;Set X register to $N0
; 
        lda     l6set,x     ;Prep for sense mode
; 
; Now wait for BSY to go hi, signalling 'ready w/status'
; 
rdh1:   lda     l7clr,x     ;Read sense
        bpl     rdh1        ;Wait til a high
; 
; Signal Liron we're ready to receive
; 
        lda     reqset,x    ;Raise /REQ
; 
; Wait for a byte from Liron or timeout
; 
        ldy     #statmto    ;Max bytes 'til stat mark
rdh2:   lda     l6clr,x
        bpl     rdh2        ;*** No Page Cross ***
        dey
        bmi     markerr     ;Didn't find a packet in time
; 
; Is it the beginning of the packet?
; 
        cmp     #packetbeg  ;Find the packet begin mark
        bne     rdh2        ;Back again - no timeout for now
; 
; Okay load up the table with this stuff
; 
.if     .defined(Liron) || .defined(IIc)
        ldy     #6          ;Seven bytes of overhead
.else   ;SoftSP
        ldy     #0
        ldx     #5
.endif
rdh3:
.if     .defined(Liron) || .defined(IIc)
        lda     l6clr,x     ;If byte ready, grab it
.else   ;SoftSP
        lda     l6clr+TheOff ;If byte ready, grab it
.endif
        bpl     rdh3        ;*** No Page Cross ***
        and     #%01111111  ;Strip start bit
.if     .defined(Liron) || .defined(IIc)
        sta     rcvbuf,y
        eor     #$80        ;Pop MSB back on for checksum
.else   ;SoftSP
        sta     rcvbuf+1,x
.endif
        eor     checksum
        sta     checksum
.if     .defined(Liron) || .defined(IIc)
        dey
.else   ;SoftSP
        dex
.endif
        bpl     rdh3
; 
; Set groups of seven buffer pointer buffer2
; 
.if     .defined(Liron) || .defined(IIc)
        lda     oddbytes
        beq     start2      ;Skip alteration if no oddbytes
        clc
.else   ;SoftSP
        ldx     oddbytes
        clc
LC844:
        lda     l6clr+TheOff ;If byte ready, grab it
        bpl     LC844
        and     #%01111111  ;Strip start bit
        sta     grp7ctr
        txa
        beq     LC87F
.endif
        adc     buffer
        sta     buffer2
.if     .defined(Liron) || .defined(IIc)
        lda     buffer+1
        adc     #0
        sta     buffer2+1
; 
        ldy     #0
.else   ;SoftSP
; .if     .defined(KBOOHK)
; LCA09:  lda     l6clr+TheOff
;         bpl     LCA09
; .endif
        bcc     start0
        inc     buffer2+1
.endif

; 
; Now receive the odd bytes
; 
start0:
.if     .defined(Liron) || .defined(IIc)
        lda     l6clr,x     ;Read in the odd bytes topbits
.else   ;SoftSP
        lda     l6clr+TheOff ;Read in the odd bytes topbits
.endif
        bpl     start0
        asl     A
        sta     topbits
.ifdef  SoftSP
        lda     grp7ctr
        pha
        bne     LC886
.endif
start1:
.if     .defined(Liron) || .defined(IIc)
        lda     l6clr,x     ;Get an odd byte
.else   ;SoftSP
        lda     l6clr+TheOff ;Get an odd byte
.endif
        bpl     start1
        asl     topbits     ;Get an MSB
        bcs     gob1        ;If MSB set, leave start bit
        eor     #$80        ;MSB clear- flip start bit
gob1:   sta     (buffer),y  ;Squirrel it away
        iny                 ;Next spot
        cpy     oddbytes    ;Are we done?
        bcc     start1      ;If more, branch

.ifdef  SoftSP
LC877:
        lda     l6clr+TheOff
        bpl     LC877
        jmp     LC92E
LC87F:
        lda     grp7ctr
        pha
        beq     LC877
        bne     SlotDepRd
LC886:
        lda     l6clr+TheOff
        bpl     LC886
        asl     topbits
        bcs     LC891
        eor     #$80
LC891:
        sta     (buffer),y
        iny
        cpy     oddbytes
        bcc     LC886
        ldy     #$00
.endif
