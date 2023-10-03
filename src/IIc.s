;*******************************************************************************
;* PC                                                                          *
;* Protocol Converter Code for IIc                                             *
;*******************************************************************************

IIc     =       1           ;Which machine?

.include "ca65_a2_strings.inc"

        .setcpu "65C02"

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
;        Unidisk 3.5 Driver Firmware   Version 1.0
;
;    Written by Michael Askins   x6243   May 15, 1985
;
;           Copyright Apple Computer, Inc. 1985
;                   All Rights Reserved
;
;
        msb on

.include "pc.equates.inc"

        .org    $c000
        .res    $c500-*,$00

        .org    $c500
;
; Here beginneth that code which resideth in the boot space at the time the
; card resteth in slot the fifth.
;
C500org:
MSlotValue =    >*
SlotNum =       MSlotValue & $07
TheOff  =       $60         ;On //c IWM in slot 6
.include "pc.bootspace.inc"
.include "pc.boot.inc"

        .res    $c5f5-*,$00

        .org    $c5f5
        jmp     bootfail    ;jump to the boot failure message
        jmp     Reset       ;Reset vector
        .byte   PCID2
        .word   0
        .byte   PDIDByte
        .byte   <ProDOSEntry

        .res    $c880-*,$00
        
        .org    $c880
        jmp     Entry       ;The //c bank switch jumps here
        jmp     AppleTalkEntry

.include "pc.packet.inc"

.include "pc.cread.inc"

.include "pc.main.inc"

        .res    4,0
