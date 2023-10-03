;*******************************************************************************
;* PC.Packet                                                                   *
;* Send a CBus Packet                                                          *
;*******************************************************************************

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

; 
; This table, when sent in reverse order, provides a sync pattern used to
; synchronize the drive IWM with the data stream. The first byte (last sent) is
; the packet begin mark.
; 
preamble: .byte packetbeg
synctab: .byte  $ff,$fc,$f3,$cf,$3f

; 
; These routines are for wasting specific amounts of time
; This code segment should not cross page boundaries.
; 
waste32: jsr    waste14
waste18: nop
waste16: nop
waste14: nop
waste12: rts

markerr:
        jmp     dberror


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
        lda     #$00
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
        ldy     #6          ;Seven bytes of overhead
rdh3:
        lda     l6clr,x     ;If byte ready, grab it
        bpl     rdh3        ;*** No Page Cross ***
        and     #%01111111  ;Strip start bit
        sta     rcvbuf,y
        eor     #$80        ;Pop MSB back on for checksum
        eor     checksum
        sta     checksum
        dey
        bpl     rdh3
; 
; Set groups of seven buffer pointer buffer2
; 
        lda     oddbytes
        beq     start2      ;Skip alteration if no oddbytes
        clc
        adc     buffer
        sta     buffer2
        lda     buffer+1
        adc     #0
        sta     buffer2+1
; 
        ldy     #0
; 
; Now receive the odd bytes
; 
start0:
        lda     l6clr,x     ;Read in the odd bytes topbits
        bpl     start0
        asl     A
        sta     topbits
start1:
        lda     l6clr,x     ;Get an odd byte
        bpl     start1
        asl     topbits     ;Get an MSB
        bcs     gob1        ;If MSB set, leave start bit
        eor     #$80        ;MSB clear- flip start bit
gob1:   sta     (buffer),y  ;Squirrel it away
        iny                 ;Next spot
        cpy     oddbytes    ;Are we done?
        bcc     start1      ;If more, branch

start2:
.if     .defined(IIc)
        jmp     SlotDepRd
.elseif .defined(Liron)
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

; 
enablechain:
        jsr     SetXN0
        lda     ca1set,x
        lda     lstrbset,x
        rts

; 
ClrPhases:
        jsr     SetXN0
        lda     reqclr,x
        lda     ca1clr,x
        lda     ca2clr,x
LCB8F:
        lda     lstrbclr,x
        rts

; 
SetXN0:
.if     .defined(IIc)
        ldx     #TheOff
.else   ;Liron
        lda     Slot
        asl     A
        asl     A
        asl     A
        asl     A
        tax
.endif
; 
        rts

; 
; Shift tables for use when reading. Each table should not straddle pages.
; 
shift1: .byte   $80,$80,$80,$80,$80,$80,$80,$80
        .byte   $00,$00,$00,$00,$00,$00,$00,$00
shift2: .byte   $80,$80,$80,$80,$00,$00,$00,$00
        .byte   $80,$80,$80,$80,$00,$00,$00,$00
shift3: .byte   $80,$80,$00,$00,$80,$80,$00,$00
        .byte   $80,$80,$00,$00,$80,$80,$00,$00
shift4: .byte   $80,$00,$80,$00,$80,$00,$80,$00
        .byte   $80,$00,$80,$00,$80,$00,$80,$00

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
        bpl     spile1      ;If all fails, carry is set
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

;*******************************************************************************
;*                                                                             *
;* Divide7                                                                     *
;*  This routine divides the bytecount by seven. The quotient gives the number *
;* of groups of seven bytes to be sent, and the remainder gives the number of  *
;* "odd" bytes.                                                                *
;*                                                                             *
;* Input:  bytecountl,h <- # of bytes to write                                 *
;*         buffer       <- pointer to data                                     *
;* Output: auxptr       <- pointer to speed up csumming                        *
;*         oddbytes     <- bytecount MOD 7                                     *
;*         grp7ctr      <- bytecount DIV 7                                     *
;*                                                                             *
;*******************************************************************************
pdiv7tab:
        .byte   $00,$24,$49
pmod7tab:
        .byte   $00,$04,$01
div7tab: .byte  $00,$01,$02,$04,$09,$12
mod7tab: .byte  $00,$01,$02,$04,$01,$02
auxptrinc:
        .byte   $00,$7f,$ff

WritePrep:
Divide7:
; 
; Set up auxptr <- buffer+$80   if $0FF < bytecount < $200
;     or auxptr <- buffer+$100  if $1FF < bytecount
; 
        ldx     ByteCounth  ;0, 1, or 2
        beq     noauxptr    ;Auxptr used only for full pages
; 
        lda     buffer+1
        sta     auxptr+1    ;Copy over hi order part
; 
        lda     #$80        ;Anticipate smaller bytecount
        cpx     #1          ;Check bytecount
        beq     sap1        ;=> $0FF < bytecount < $200
; 
        inc     auxptr+1    ;Add $100 to bytecount instead
        lda     #0          ;Make sure lo order unaltered
sap1:   clc
        adc     buffer
        sta     auxptr
.if     .defined(IIc)       ;probably ok for Liron, but not in known rom dumps
        bcc     noauxptr
        inc     auxptr+1
.endif
; 
; Now look up the first order guess for DIV and MOD. X still has bytecount DIV
; 256.
noauxptr:
        lda     pdiv7tab,x
        sta     grp7ctr
        lda     pmod7tab,x
        sta     oddbytes
; 
; Now add in the mods and divs for each of the five hi order bits in the lo
; order bytecount, correcting each time MOD becomes bigger than 6.
; 
        ldx     #5          ;Do for five bits
        lda     ByteCountl
        sta     temp        ;Store lo order for shifting
        and     #%00000111  ;Save lo three for later
        tay
; 
divide3:
        asl     temp        ;C <- next from bytecountl
        bcc     divide2     ;If clear, no effect on DIV,MOD
        lda     mod7tab,x   ;Get MOD7 for 2^n
divide4:
        clc
        adc     oddbytes    ;Got new MOD value
        cmp     #7          ;Is it too big?
        bcc     divide1     ;=> NO leave MOD - 0->C
        sbc     #7          ;Bring MOD under 7 - C still set
divide1:
        sta     oddbytes
        lda     div7tab,x   ;Get DIV for this 2^n
        adc     grp7ctr     ;Add to DIV along with correction (C)
        sta     grp7ctr     ;Update the DIV
divide2:
        dex                 ;One less bit to deal with
        bmi     divide5     ;Escape after 6 times through loop
        bne     divide3     ;Take brnch 1st 5 loops
; 
        tya                 ;Get back the last three bits
        jmp     divide4     ;Sixth pass add in remains
divide5:

;*******************************************************************************
;*                                                                             *
;* PreCheck                  Does the checksumming prepass                     *
;*                                                                             *
;* Input:  bytecount   <- bytes in buffer                                      *
;*         buffer      <- pointer to data to send                              *
;*         auxptr      <- extra pointer to speed process                       *
;* Output: checksum    <- 8 bit XOR of data to be sent                         *
;*                                                                             *
;*******************************************************************************
PreCheck:
        lda     buffer+1
        pha                 ;Preserve buffer pointer
        lda     #0
        ldx     ByteCounth
        beq     lastpass    ;If no complete pages, skip this
; 
        ldy     auxptrinc,x ;Get number of bytes each ptr
xor1:   eor     (buffer),y
        eor     (auxptr),y
        dey                 ;One less
        bne     xor1
        eor     (buffer),y
        eor     (auxptr),y  ;Have to deal with 0 case
; 
; Now move the buffer up for next section
; 
        cpx     #1
        beq     xor5        ;If 256 and up bytes, bump x1
        inc     buffer+1    ;otherwise x2
xor5:   inc     buffer+1
; 
; Do the remaining less than a page with a single pointer
; 
lastpass:
        ldy     ByteCountl
        beq     xor4
        eor     (buffer),y  ;Compensate for nth byte
xor3:   eor     (buffer),y
        dey
        bne     xor3
        eor     (buffer),y  ;Last damn (0th) byte
; 
; Store result away. Retrieve old buffer value.
; 
xor4:   sta     checksum
        pla
        sta     buffer+1
;*******************************************************************************
;*                                                                             *
;* DetTopBits              Get topbits for odd bytes                           *
;*                                                                             *
;*  Also sets buffer2 pointer to pointer at groups of seven bytes.             *
;*                                                                             *
;* Input:  oddbytes <- # of "odd" bytes                                        *
;*         buffer   <- pointer to data                                         *
;* Output: tbodd    <- topbits for odd bytes                                   *
;*         buffer2  <- buffer+oddbytes                                         *
;*                                                                             *
;*******************************************************************************
DetTopBits:
        ldy     oddbytes
        dey
        lda     #0
        sta     tbodd
; 
gtbob:  lda     (buffer),y
        asl     A
        ror     tbodd
        dey
        bpl     gtbob
        sec
        ror     tbodd
; 
        lda     oddbytes
        clc
        adc     buffer
        sta     buffer2
        lda     buffer+1
        adc     #0
        sta     buffer2+1
;*******************************************************************************
;*                                                                             *
;* Sun                          Set up next buffer and topbits                 *
;*                                                                             *
;*  Primes the pipe for the group of seven bytes routine setting the topbits   *
;* byte and the "next" buffer. The routine also advances the buffer pointer by *
;* 7 to prepare for the groups of seven transfer.                              *
;*                                                                             *
;* Input:  buffer2   <- points to groups of 7 data                             *
;* Output: next1,7   <- first 7 bytes in buffer                                *
;*         topbits   <- MSBs of first 7 bytes                                  *
;*                                                                             *
;*******************************************************************************
;* Copy first seven bytes into the pipeline                                    *
;*******************************************************************************
Sun:    ldy     #6
sun2:   sec
        lda     (buffer2),y
        sta     next,y
        bmi     sun1
        clc
sun1:   ror     topbits
        dey
        bpl     sun2
        sec
        ror     topbits
; 
; Advance the pointer
; 
        lda     buffer2
        clc
        adc     #7
        sta     buffer2
        bcc     sun3
        inc     buffer2+1
sun3:   rts

; 
; X is slot*16, Y is the desired mode
; 
; Set up the IWM mode register. Extreme care should be taken here. Setting the
; mode byte with indexed stores causes a false byte to be written a cycle before
; the real value is written. This false value, if it enables the timer, causes
; the IWM Rev A to pop the motor on, inhibiting the setting of the mode until
; the motor times out! We avoid this by setting the mode byte only when it is
; not what we want, and if it's not we stay here until the we see that it is
; what we want.
; 
SetIWMode:
        lda     monclr,x    ;Motor must be off
        lda     l6set,x     ;Set up to access mode register
        jmp     careful     ;Don't mess unless we gotta

biz:    tya
        sta     l7set,x     ;Try storing the mode value
careful:
        tya                 ;Get back the target value
        eor     l7clr,x     ;Compare with observed value
        and     #$1f        ;Can only read low 5 bits
        bne     biz         ;If not right, back to try again
        rts

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

; 
; This takes grp7ctr and oddbytes and calculates 7*grp7ctr+oddbytes. The results
; are in Y(hi) and A(lo). This is the number of bytes that were received in the
; last ReceivePack.
; 
RcvCount:
        lda     grp7ctr
        tay
        ldx     #0
        stx     grp7ctr
        ldx     #3
times7: asl     A
        rol     grp7ctr
        dex
        bne     times7
        clc
        adc     oddbytes
        bcc     t71
        inc     grp7ctr
t71:    sty     oddbytes
        sec
        sbc     oddbytes
        bcs     t72
        dec     grp7ctr
t72:    ldy     grp7ctr
        rts
