;*******************************************************************************
;* PC                                                                          *
;* Protocol Converter Code for Liron                                           *
;*******************************************************************************

Liron   =       1           ;Which machine?

.include "ca65_a2_strings.inc"

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

        cstr "Firmware written by Michael Askins"

        .res    221,$ff

; Slot ROMS are all identical except for hardware offsets

        .org    $c100

.macro  endBytes
        .res    1,$ff
        .byte   PCID2
        .word   0
        .byte   PDIDByte
        .byte   <ProDOSEntry
.endmacro

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the first.
;
C100org:
.scope Slot1
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the second.
;
C200org:
.scope Slot2
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the third.
;
C300org:
.scope Slot3
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the fourth.
;
C400org:
.scope Slot4
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the fifth.
;
C500org:
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the sixth.
;
C600org:
.scope Slot6
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the seventh.
;
C700org:
.scope Slot7
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       SlotNum << 4
.include "pc.bootspace.inc"
.include "pc.cread.inc"
        endBytes
.endscope

.include "pc.packet.inc"

.include "pc.main.inc"

.include "pc.boot.inc"

        .res    $d000-37-*,$ff
        cstr    "(C) 1985 Apple Computer, Inc. MSA"
        .byte   $10
        .byte   $ff
        .byte   $ff
