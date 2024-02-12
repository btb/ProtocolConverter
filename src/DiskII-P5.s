; Target assembler: cc65 v2.19.0 [--target none -C C600ROM_cc65.cfg]
;*******************************************************************************
;* Disk ][ controller card "BOOT0" code, found in the slot ROM.  Reads the     *
;* BOOT1 code from track 0, sector 0, and jumps to it.                         *
;*                                                                             *
;* Copyright Apple Computer Inc.                                               *
;*                                                                             *
;* Written by [a genius...Woz?]                                                *
;*******************************************************************************
;* Extracted from AppleWin at $C600.                                           *
;*                                                                             *
;* Project created by Andy McFadden, using 6502bench SourceGen v1.5            *
;* Last updated 2020/01/15                                                     *
;*******************************************************************************
         .setcpu "6502"
STACK   =       $0100
TWOS_BUFFER =   $0300       ;holds the 2-bit chunks
CONV_TAB =      $0356       ;6+2 conversion table
BOOT1   =       $0800       ;buffer for next stage of loader
IWM_PH0_OFF =   $c080       ;stepper motor control
IWM_PH0_ON =    $c081       ;stepper motor control
IWM_MOTOR_ON =  $c089       ;starts drive spinning
IWM_SEL_DRIVE_1 = $c08a     ;selects drive 1
IWM_Q6_OFF =    $c08c       ;read
IWM_Q7_OFF =    $c08e       ;WP sense/read
MON_WAIT =      $fca8       ;delay for (26 + 27*Acc + 5*(Acc*Acc))/2 cycles
MON_IORTS =     $ff58       ;JSR here to find out where one is

data_ptr .set   $26         ;pointer to BOOT1 data buffer
slot_index .set $2b         ;slot number << 4
bits    .set    $3c         ;temp storage for bit manipulation
sector  .set    $3d         ;sector to read
found_track .set $40        ;track found
track   .set    $41         ;track to read
ENTRY:  ldx     #$20        ;20/00/03 is the controller signature
; 
; Generate a decoder table for 6+2 encoded data.
; 
; This stores the values $00-$3f in a table on page 3.  The byte values that
; will be decoded are non-consecutive, so the decoder entries occupy various
; locations from $36c to $3d5.  Nearby bytes are left unchanged.
; 
; We want 64 values that have the high bit set and don't have two consecutive 0
; bits.  This is required by the disk hardware.  There are 70 possible values,
; so we also mandate that there are two adjacent 1 bits, excluding bit 7.  (Note
; that $D5 and $AA, used to identify sector headers, do not meet these criteria,
; which means they never appear in the encoded data.)
; 
; In the code below, ASL+BIT+BCS test checks for adjacent 1s: if no two are
; adjacent, the BIT will be zero.  If the high bit is set, ASL will set the
; carry.
; 
; When we ORA the original and shifted values together, if there were three
; adjacent 0s, there will still be at least two adjacent 0s.  We EOR to invert
; the bits, and then look for two adjacent 1s.  We do this by just shifting
; right until a 1 shifts into the carry, and if the A-reg is nonzero we know
; there were at least two 1 bits.  We need to ignore the bits on the ends:
; nonzero high bit was handled earlier, and the low bit can false-positive
; because ASL always shifts a 0 in (making it look like a 0 in the low bit is
; adjacent to another 0), so we just mask those off with the AND.
; 
; For example, we want to decode $A6 to $07.  Y=$07 when X=$26...
;   TXA --> 0010 0110
;   ASL --> 0100 1100 C=0   (high bit is clear)
;   BIT --> Z=0             (only possible with adjacent bits)
;   ORA --> 0110 1110       (adjacent 0s become visible)
;   EOR --> 1001 0001       (turn them into 1s)
;   AND --> 0001 0000       (ignore the hi/lo)
;   LSR --> 0000 1000, repeat until A=0 C=1
; 
        ldy     #$00
        ldx     #$03
CreateDecTabLoop:
        stx     bits
        txa
        asl     A           ;shift left, putting high bit in carry
        bit     bits        ;does shifted version overlap?
        beq     @reject     ;no, doesn't have two adjacent 1s
        ora     bits        ;merge
        eor     #$ff        ;invert
        and     #$7e        ;clear hi and lo bits
@check_dub0:
        bcs     @reject     ;initial hi bit set *or* adjacent 0 bits set
        lsr     A           ;shift right, low bit into carry
        bne     @check_dub0 ;if more bits in byte, loop
        tya                 ;we have a winner... store Y-reg to memory
        sta     CONV_TAB,x  ;actual lookup will be on bytes with hi bit set
        iny                 ; so they'll read from CONV_TAB-128
@reject:
        inx                 ;try next candidate
        bpl     CreateDecTabLoop
; 
; Prep the hardware.
; 
        jsr     MON_IORTS   ;known RTS
        tsx
        lda     STACK,x     ;pull hi byte of our address off stack
        asl     A           ;(we assume no interrupts have hit)
        asl     A           ;multiply by 16
        asl     A
        asl     A
        sta     slot_index  ;keep this around
        tax
        lda     IWM_Q7_OFF,x ;set to read mode
        lda     IWM_Q6_OFF,x
        lda     IWM_SEL_DRIVE_1,x ;select drive 1
        lda     IWM_MOTOR_ON,x ;spin it up
; 
; Blind-seek to track 0.
; 
        ldy     #80         ;80 phases (40 tracks)
@seek_loop:
        lda     IWM_PH0_OFF,x ;turn phase N off
        tya
        and     #$03        ;mod the phase number to get 0-3
        asl     A           ;double it to 0/2/4/6
        ora     slot_index  ;add in the slot index
        tax
        lda     IWM_PH0_ON,x ;turn on phase 0, 1, 2, or 3
        lda     #86
        jsr     MON_WAIT    ;wait 19664 cycles
        dey                 ;next phase
        bpl     @seek_loop
        sta     data_ptr    ;A-reg is 0 when MON_WAIT returns
        sta     sector      ;so we're looking for T=0 S=0
        sta     track
        lda     #>BOOT1     ;write the output here
        sta     data_ptr+1
; 
; Sector read routine.
; 
; Read bytes until we find an address header (D5 AA 96) or data header (D5 AA
; AD), depending on which mode we're in.
; 
; This will also be called by the BOOT1 code read from the floppy disk.
; 
; On entry:
;   X: slot * 16
;   $26-27: data pointer
;   $3d: desired sector
;   $41: desired track
; 
ReadSector:
        clc                 ;C=0 to look for addr (C=1 for data)
@ReadSector_C:
        php
;* * * * * * * * * * * * * * * * * * * * *
;* The following code is sacred in it's  *
;* present form.  To change it would     *
;* cause volcanoes to erupt, the ground  *
;* to shake, and ProDOS not to boot!     *
;* * * * * * * * * * * * * * * * * * * * *
@rdbyte1:
        lda     IWM_Q6_OFF,x ;wait for byte
        bpl     @rdbyte1    ;not yet, loop
@check_d5:
        eor     #$d5        ;is it $d5?
        bne     @rdbyte1    ;no, keep looking
@rdbyte2:
        lda     IWM_Q6_OFF,x ;grab another byte
        bpl     @rdbyte2
        cmp     #$aa        ;is it $aa?
        bne     @check_d5   ;no, check if it's another $d5
        nop                 ;(?)
@rdbyte3:
        lda     IWM_Q6_OFF,x ;grab a third byte
        bpl     @rdbyte3
        cmp     #$96        ;is it $96?
        beq     @FoundAddress ;winner
        plp                 ;did we want data?
        bcc     ReadSector  ;nope, keep looking
        eor     #$ad        ;yes, see if it's data prologue
        beq     @FoundData  ;got it, read the data (note A-reg = 0)
        bne     ReadSector  ;keep looking

; 
; Read the sector address data.  Four fields, in 4+4 encoding: volume, track,
; sector, checksum.
; 
@FoundAddress:
        ldy     #$03        ;sector # is the 3rd item in header
@hdr_loop:
        sta     found_track ;store $96, then volume, then track
@rdbyte11:
        lda     IWM_Q6_OFF,x ;read first part
        bpl     @rdbyte11
        rol     A           ;first byte has bits 7/5/3/1
        sta     bits
@rdbyte21:
        lda     IWM_Q6_OFF,x ;read second part
        bpl     @rdbyte21
        and     bits        ;merge them
        dey                 ;is this the 3rd item?
        bne     @hdr_loop   ;nope, keep going
        plp                 ;pull this off to keep stack in balance
        cmp     sector      ;is this the sector we want?
        bne     ReadSector  ;no, go back to looking for addresses
        lda     found_track
        cmp     track       ;correct track?
        bne     ReadSector  ;no, try again
        bcs     @ReadSector_C ;correct T/S, go find data (branch-always)

; 
; Read the 6+2 encoded sector data.
; 
; Values range from $96 - $ff.  They must have the high bit set, and must not
; have three consecutive zeroes.
; 
; The data bytes are written to disk with a rolling XOR to compute a checksum,
; so we read them back the same way.  We keep this in the A-reg for the
; duration.  The actual value is always in the range [$00,$3f] (6 bits).
; 
; On entry:
;   A: $00
; 
@FoundData:
        ldy     #86         ;read 86 bytes of data into $300-355
@read_twos_loop:
        sty     bits        ;each byte has 3 sets of 2 bits, encoded
@rdbyte12:
        ldy     IWM_Q6_OFF,x
        bpl     @rdbyte12
        eor     $02d6,y     ;$02d6 + $96 = $36c, our first table entry
        ldy     bits
        dey
        sta     TWOS_BUFFER,y ;store these in our page 3 buffer
        bne     @read_twos_loop
; 
@read_sixes_loop:
        sty     bits        ;read 256 bytes of data into $800
@rdbyte22:
        ldy     IWM_Q6_OFF,x ;each byte has the high 6 bits, encoded
        bpl     @rdbyte22
        eor     CONV_TAB-128,y
        ldy     bits
        sta     (data_ptr),y ;store these in the eventual data buffer
        iny
        bne     @read_sixes_loop
; 
@rdbyte31:
        ldy     IWM_Q6_OFF,x ;read checksum byte
        bpl     @rdbyte31
        eor     CONV_TAB-128,y ;does it match?
@another:
        bne     ReadSector  ;no, try to find one that's undamaged
; 
; Decode the 6+2 encoding.  The high 6 bits of each byte are in place, now we
; just need to shift the low 2 bits of each in.
; 
        ldy     #$00        ;update 256 bytes
@init_x:
        ldx     #86         ;run through the 2-bit pieces 3x (86*3=258)
@decode_loop:
        dex
        bmi     @init_x     ;if we hit $2ff, go back to $355
        lda     (data_ptr),y ;foreach byte in the data buffer...
        lsr     TWOS_BUFFER,x ; grab the low two bits from the stuff at $300-$355
        rol     A           ; and roll them into the low two bits of the byte
        lsr     TWOS_BUFFER,x
        rol     A
        sta     (data_ptr),y
        iny
        bne     @decode_loop
;* * * * * * * * * * * * * * * * *
;* Code beyond this point is not *
;* sacred... It may be perverted *
;* in any manner by any pervert. *
;* * * * * * * * * * * * * * * * *
; 
; Advance the data pointer and sector number, and check to see if the sector
; number matches the first byte of BOOT1.  If it does, we're done.  If not, go
; read the next sector.
; 
        inc     data_ptr+1
        inc     sector
        lda     sector      ;sector we'd read next
        cmp     BOOT1       ;is next sector < BOOT1?
        ldx     slot_index
        bcc     @another    ;yes, go get another sector (note branch x2)
; All done, jump to BOOT1 ($0801).
        jmp     BOOT1+1

        .res    5,$00       ;spare bytes
