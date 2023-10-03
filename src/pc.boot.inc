;*******************************************************************************
;* PC.BOOT                                                                     *
;* Service Boot Request                                                        *
;*******************************************************************************

Bootcode:
        stx     Slot

.if     .defined(Liron)
        lda     #$aa
        sta     ProFLAG,x
        sta     Power1,x
.elseif .defined(IIc)
        lda     #MSlotValue
        sta     MSlot
        jsr     Reset
.endif
; 
        ldy     #5         ;Copy a command table
bc1:    lda     boottab,y
        sta     CMDCode,y
        dey
        bpl     bc1

.if     .defined(Liron)
; 
; Now on //e, patch the Unit number (slot*16)
; 
        lda     Slot
        asl     A
        asl     A
        asl     A
        asl     A
        sta     CMDUnit
.endif

; 
; Now do the read from block zero
; 
.if     .defined(IIc)
        jsr     ProDOSEntry
.else   ;Liron
        jsr     bentry
.endif
        bcs     bootfail   ;If fail, check loc
; 
        ldx     BOOT1      ;If ($800)<>1 this is no A// boot disk
        dex
        bne     bootfail
; 
        ldx     BOOT1+1    ;If $801 is zero, no boot
        beq     bootfail
; 
; It all looks okay. Jump to the code with N0 in X.
; 
        lda     Slot
        asl     A
        asl     A
        asl     A
        asl     A
        tax
        jmp     BOOT1+1    ;Jump to it
;
; Do this code if the boot can't be done.
;  If this was an autoboot (loc=$CN00), continue the slot scan.
;  If not, drop into basic after issuing appropriate message
;

bootfail:
.if     .defined(IIc)
        ldx     #<bmsglen-1
morchrs:
        lda     bootmsg,x
        sta     bootscrn,x
        dex
        bpl     morchrs
coma:   bra     coma        ;He's dead Jim.

bootmsg: asc    "Check Disk Drive."
bmsglen = *-bootmsg
.else   ;Liron
        jsr     MON_SETVID
        jsr     MON_SETKBD
        ldx     $00
        bne     @LCEA4
        ldx     $01
        cpx     MSlot
        bne     @LCEA4
        jmp     AutoScan

@LCEA4: ldx     #23
        stx     MON_CV
        jsr     MON_VTAB
        lda     #0
        sta     MON_CH
        ldx     #IOErr-BootMsgs
        ldy     Slot
        lda     Retry,y
        bne     notUnBootable
        ldx     #NotBoot-BootMsgs
notUnBootable:
        cmp     #ERR_NoDrive
        bne     notNoDrive
        ldx     #NoDev-BootMsgs
notNoDrive:
        cmp     #ERR_OffLine
        bne     morchrs2
        ldx     #NoDisk-BootMsgs
morchrs2:
        lda     BootMsgs,x
        beq     jmp2basic
        jsr     MON_COUT
        inx
        bne     morchrs2
jmp2basic:
        jmp     Basic

BootMsgs:
IOErr:  cstr    "I/O ERROR"
NotBoot:
        cstr    "NOT A BOOTABLE DISK"
NoDev:  cstr    "NO DEVICE CONNECTED"
NoDisk:
        cstr    "NO DISK TO BOOT"
.endif

boottab: .byte  CMD_ReadBlock,$50,$00,$08,$00,$00 ;Read from 1st; blk8->$801

;
; This routine is called from the //c reset code. It forces a
;  reset of the PC Bus.
;
.if     .defined(IIc)
Reset:
        ldx     #8
rst1:
        lda     rcode,x
        sta     loc0,x
        dex
        bpl     rst1
        jmp     loc0

rcode:
        jsr MLIEntry
        .byte   CMD_Init
        .word   $0007
        rts

cmdlist: .byte  1,0
.endif
