;*******************************************************************************
;* PC                                                                          *
;* Protocol Converter Code for SoftSP DIY Super Serial Card or GraphiCard      *
;*******************************************************************************

SSC     =       1           ;Which machine?
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
.include "pc.packet.inc"
.include "pc.main.inc"

        .org    $c800
        sec
.include "pc.packet.markerr.inc"
.include "pc.packet.receivepack.inc"
.include "pc.cread.inc"
.include "pc.packet.shifttables.inc"
.include "pc.packet.divide7.inc"
.include "pc.packet.sendonepack.inc"
.include "pc.packet.preamble.inc"
.include "pc.packet.auxptrinc.inc"
.include "pc.packet.enablechain.inc"
.include "pc.packet.setxn0.inc"
.include "pc.packet.start2.inc"
.include "pc.packet.senddata.inc"
.include "pc.packet.clrphases.inc"
.include "pc.packet.waitiwmoff.inc"
.include "pc.packet.rcvcount.inc"
.include "pc.packet.resetchain.inc"
.include "pc.main.assignid.inc"
.include "pc.main.entry.inc"

        .res    3,$00

        .org    $c500
;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the fifth.
;
C500org:
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       $60         ;Disk II in slot 6
.include "pc.bootspace.inc"
.include "pc.boot.inc"
.include "pc.boot.boottab.inc"
.include "pc.packet.precheck.inc"
.include "pc.main.paramctab.inc"
.include "pc.packet.divide7tables.inc"

        .res    6,$00

        .byte   PCID2
        .word   0
        .byte   PDIDByte
        .byte   <ProDOSEntry
