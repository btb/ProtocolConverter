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
.if     .defined(IIc) || .defined(SSC)
        bcs     BootC
.else   ;Liron||KBOOHK
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
.if     .defined(KBOOHK) || .defined(Grappler)
        ror     MSlot       ;MSlot[7]=1 if ProDOS, =0 if MLI
.else
        ldx     #SlotNum
; Save the carry flag via ROR
        ror     ProFLAG,x   ;ProFLAG[7]=1 if ProDOS, =0 if MLI
.endif
        clc                 ;This is not a boot entry
; 
; Now save mslot and clear all $C800 ROMs
; 
bootcase:
.if     .defined(KBOOHK) || .defined(Grappler)
        php
.endif
.if     .defined(Liron) || .defined(IIc)
        ldx     #MSlotValue ;Load value for MSLOT
        stx     MSlot
        ldx     #SlotNum
.elseif .defined(SSC)
        lda     #MSlotValue ;Load value for MSLOT
        sta     MSlot
.endif
        lda     ClearIOROMs ;Clear all $C800 latches but ours
.if     .defined(KBOOHK) || .defined(Grappler)
        jsr     do_RTS      ;Get our address onto stack
        tsx
        lda     stack,x     ;pull hi byte of our address off stack
        ldx     MSlot       ;boot flag onto negative flag
        clc
        bpl     LC524       ;branch if MLI
        sec                 ;boot back onto carry flag
LC524:  sta     MSlot
        and     #$0f
        tax                 ;X <- slot number
        ror     ProFLAG,x   ;Finally have the boot on ProFLAG[7]
        plp
.endif

.if     .defined(IIc) || .defined(Grappler)
        jmp     SWPROTO     ;Have to bank switch
.else   ;Liron||SoftSP
        jmp     Entry
.endif

.if     .defined(IIc) || .defined(SSC)
BootC:
        ldx     #SlotNum    ;Need Slot Number
; Will continue on to 'Bootcode' now
.endif
