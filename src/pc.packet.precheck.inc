.if     .defined(SSC) || .defined(Grappler)
WritePrep:
        jsr     Divide7
.endif
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
.if     .defined(KBOOHK)
        sta     tbodd
.endif
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
.if     .defined(Liron) || .defined(IIc) || .defined(SSC) || .defined(Grappler)
        lda     #0
        sta     tbodd
.endif
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
