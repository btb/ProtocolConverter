;*******************************************************************************
;* PC.CREAD                                                                    *
;*******************************************************************************

SlotDepRd:
        ldy     #0
        lda     grp7ctr
        pha                ;Save groups of seven counter
        bne     start3
        jmp     done       ;Go get the checksum
; 
; Okay, get the groups of seven
; start by getting the topbits for this group of seven
; 
start3: lda     l6clr+TheOff ;Get topbits
        bpl     start3
        sta     temp       ;Just a second

; 
; Split up the seven bits into two indices for topbit tables
; 
        lsr     A          ;0   1  d1  d2  d3  d4  d5  d6
        lsr     A          ;0   0   1  d1  d2  d3  d4  d5
        lsr     A          ;0   0   0   1  d1  d2  d3  d4
        and     #%00001111 ;0   0   0   0  d1  d2  d3  d4
        tax                ;First index into the tables
        lda     temp       ;1  d1  d2  d3  d4  d5  d6  d7
        and     #%00000111 ;0   0   0   0   0  d5  d6  d7
        sta     temp       ;Keep for last three bytes
; 
; Read the 1st byte, reunite its msb, store and checksum it
; 
first:  lda     l6clr+TheOff
        bpl     first      ;Back 1 instruction
        eor     shift1,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; Now, the second Y turn over occurs at this point in the loop. Update the
; buffer pointer if it occurred.
; 
        bne     second
        inc     buffer2+1
; 
; Now the second byte
; 
second: lda     l6clr+TheOff
        bpl     second     ;Back 1 instruction
        eor     shift2,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; Now the third byte
; 
third:  lda     l6clr+TheOff
        bpl     third      ;Back 1 instruction
        eor     shift3,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum
        sta     checksum
        iny
; 
; Now the fourth byte
; 
fourth: lda     l6clr+TheOff
        bpl     fourth     ;Back 1 instruction
        eor     shift4,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; The first Y turn over occurs at this point in the loop. Update the buffer
; pointer if it occurred.
; 
        bne     @LC582
        inc     buffer2+1
@LC582:
        ldx     temp       ;Now we need the other index
; 
; Now the fifth byte
; 
fifth:  lda     l6clr+TheOff
        bpl     fifth      ;Back 1 instruction
        eor     shift2,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; Now the sixth byte
; 
sixth:  lda     l6clr+TheOff
        bpl     sixth      ;Back 1 instruction
        eor     shift3,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; And, finally, the seventh byte
; 
seventh: lda    l6clr+TheOff
        bpl     seventh    ;Back 1 instruction
        eor     shift4,x   ;Recombine the MSB with data
        sta     (buffer2),y ;Store it away
        eor     checksum   ;Add it to the checksum
        sta     checksum
        iny
; 
; Now see if this is the last group of seven to receive
; 
        dec     grp7ctr
        beq     done       ;Go to get the checksum etc
        jmp     start3     ;Another topbits ...

; 
; Get and reconstruct the checksum
; 
done:   lda     l6clr+TheOff
        bpl     done
        sta     temp       ;1 c6 1 c4 1 c2 1 c
; 
        pla                ;Restore groups of 7 counter
        sta     grp7ctr
@LC5C2: lda     l6clr+TheOff ;1 c7 1 c5 1 c3 1 c1
        bpl     @LC5C2
        sec
        rol     A          ;c7 1 c5 1 c3 1 c1 1
        and     temp       ;c7 c6 c5 c4 c3 c2 c1 c0
        eor     checksum   ;When we're done, should be zero
; 
; Get the packet end mark. Is it correct?
; 
rdha:   ldy     l6clr+TheOff ;Preserve A
        bpl     rdha
; 
        cpy     #packetend
        bne     npenderr
; 
; Didn't have time before to checksum oddbytes. Do it now
; A still has the partial checksum
; 
        ldx     oddbytes
        beq     icbt1
        ldy     #0
icbt:   eor     (buffer),y
        iny
        dex
        bne     icbt
; 
; Okay, checksum oughta be zero. If not, checksum error.
; 
icbt1:  tax
        bne     cserror
; 
; Wait for /BSY to go low
; 
        lda     l6set+TheOff
rdh4:   lda     l7clr+TheOff
        bmi     rdh4

; 
; Got the bytes, now acknowledge their receipt
; 
LC963:
        lda     reqclr+TheOff ;Lower REQ
; 
        clc
        rts

npenderr:
        lda     #ERR0_nopackend
        bne     gserror

cserror:
        lda     #ERR0_csumerr
gserror:
        sec
        rts
