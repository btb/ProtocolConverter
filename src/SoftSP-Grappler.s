;*******************************************************************************
;* PC                                                                          *
;* Protocol Converter Code for SoftSP DIY Grappler Plus                        *
;*******************************************************************************

Grappler =      1           ;Which machine?
SoftSP  =       1           ;No IWM

.include "ca65_a2_strings.inc"
.include "ca65_assert_branch_page.inc"

        .setcpu "6502"

; PPPP  RRRR   OOO  TTTTT  OOO   CCC   OOO  L
; P   P R   R O   O   T   O   O C   C O   O L
; PPPP  RRRR  O   O   T   O   O C     O   O L
; P     R R   O   O   T   O   O C   C O   O L
; P     R  R   OOO    T    OOO   CCC   OOO  LLLLL
;
;  CCC   OOO  N   N V   V EEEEE RRRR  TTTTT EEEEE RRRR
; C   C O   O NN  N V   V E     R   R   T   E     R   R
; C     O   O N N N V   V EEEE  RRRR    T   EEEE  RRRR
; C   C O   O N  NN  V V  E     R R     T   E     R R
;  CCC   OOO  N   N   V   EEEEE R  R    T   EEEEE R  R

;
; Unidisk 3.5 Driver Firmware   Version 1.0.1
;
; Written by Michael Askins     May 15, 1985
; Revised by M. Askins and R. Chiang    April 10, 1986
;
; Copyright Apple Computer, Inc. 1985,1986
; All Rights Reserved
;
;
        msb on

.include "pc.equates.inc"

        .org    $c500
;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the fifth.
;
C500org:
MSlotValue =    >*
SlotNum =       MSlotValue & $07
.ifndef TheOff
TheOff  =       $60         ;Disk II in slot 6
.endif
.include "pc.bootspace.inc"

        .org    $c82e
.include "pc.boot.inc"

        .res    7,$00
        msb     off
        cstr    "V6"
        asc     "SoftSP"
        msb on

        jmp     Entry       ;Reset vector
        .byte   PCID2
        .word   0
        .byte   PDIDByte
        .byte   <ProDOSEntry

        .res    1186,$00

;
; Code for switching between banks
;

; X contains slot number of Grappler
SWPROTO:
        php
        txa
        asl     A
        asl     A
        asl     A
        asl     A
        plp
        tay                 ;Y <- Grappler soft switches offset
        sta     gbank2,y    ;select Grappler ROM bank 2
;
; At this point, execution should continue in the top half of the ROM,
; at 'Entry'
;

;
; Don't know yet how this is reached
; other than that's what would happen if you called 'Entry'
; when bank 1 is selected
;
LowEntry:
        lda     MSlot
        pha
        lda     #<Bootcode-1
        pha                     ;RTS will resume at 'Bootcode'
do_RTS:
        rts

.include "pc.boot.boottab.inc"

        .res    581,$00

        .org    $c800

.include "pc.packet.sendonepack.inc"
.include "pc.packet.waitiwmoff.inc"
.include "pc.packet.clrphases.inc"
.include "pc.packet.enablechain.inc"
.include "pc.packet.markerr.inc"
.include "pc.packet.receivepack.inc"
.include "pc.cread.inc"
.include "pc.packet.start2.inc"

LCB3B:
        ldx     #TheOff
        lsr     A
        bcc     LCB41
        inx
LCB41:
        lda     enable1,x
        rts

.include "pc.packet.divide7tables.inc"
.include "pc.packet.preamble.inc"
.include "pc.packet.shifttables.inc"
.include "pc.packet.auxptrinc.inc"
.include "pc.main.paramctab.inc"

LowEntry2:
        lda     MSlot
        pha
        lda     #<Bootcode-1
        pha                     ;RTS will resume at 'Bootcode'
        rts

.include "pc.packet.divide7.inc"
.include "pc.packet.precheck.inc"
.include "pc.packet.senddata.inc"
.include "pc.packet.rcvcount.inc"
.include "pc.packet.resetchain.inc"
.include "pc.main.assignid.inc"
.include "pc.main.entry.inc"

        cstr    "Sun"
        .res    45,$00
