;*******************************************************************************
;* PC.BOOTSPACE                                                                *
;* Boot Space                                                                  *
;*******************************************************************************

;
; Auto Boot signature bytes
; This is also the boot (auto & PR#n) entry point.
;
        ldx     #$20
        ldx     #$00
        ldx     #$03
; 
        cmp     #$00       ;Flag that this is a boot
.if     .defined(IIc)
        bcs     BootC
.else   ;Liron
        bcs     bootcase
.endif

; 
; Here is the ProDOS normal entry point
; 
; Set up so that ProFLAG will have the top bit set
; 
ProDOSEntry:
        sec
        bcs     skip        ;Skip the clear
; 
; This is the MLIxface entry point
; 

MLIEntry:                   ;Only use this label in //c version
        clc
skip:
        ldx     #SlotNum
; Save the carry flag via ROR
        ror     ProFLAG,x   ;ProFLAG[7]=1 if ProDOS, =0 if MLI
        clc                 ;This is not a boot entry
; 
; Now save mslot and clear all $C800 ROMs
; 
bootcase:
        ldx     #MSlotValue ;Load value for MSLOT
        stx     MSlot
        ldx     #SlotNum
        lda     ClearIOROMs ;Clear all $C800 latches but ours


.if     .defined(IIc)
        jmp     SWPROTO     ;Have to bank switch
.else   ;Liron
        jmp     Entry
.endif

.if     .defined(IIc)
BootC:
        ldx     #SlotNum    ;Need Slot Number
; Will continue on to 'Bootcode' now
.endif
