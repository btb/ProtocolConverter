;*******************************************************************************
;* ID Assignment Cycle                                                         *
;*******************************************************************************
AssignID:
        pha                 ;Save the init code
        jsr     resetchain  ;Reset all of those things
        pla
        tax                 ;Save InitCode
; 
; Save the command code, unit, and init code 'cause we'll trample 'em.
; 
        lda     CMDCode
        pha
        lda     CMDPCount
        pha
        lda     CMDSCode
        pha
        stx     CMDSCode    ;Store away the type of INIT
; 
; Set up to send DefID command packets
; 
        lda     #CMD_Init
        sta     CMDCode
        lda     #0
        sta     Unit
        lda     #2          ;# parms in Init call
        sta     CMDPCount
; 
; Point the buffer pointer
; 
        lda     #CMDCode
        sta     buffer
        lda     #>CMDCode
        sta     buffer+1
        lda     #cmdmark
        sta     WPacketType
        jsr     ClrPhases   ;Make sure phases are off for Quark
; 
; Send an ID for the next device in the chain
; 
mordevices:
        inc     Unit
        lda     #CmdLength
        sta     ByteCountl ;ReceivePack scrambles count
        lda     #>CmdLength
        sta     ByteCounth
; 
        jsr     SendOnePack ;Send the command
        bcc     mdev2      ;If okay, skip to get response

.if     .defined(KBOOHK) || .defined(Grappler)
        lda     Unit
        cmp     #$01
        bne     LCCD5
        ldx     Slot
        lda     ProFLAG,x
        eor     #$01
        sta     ProFLAG,x
        jsr     LCB3B
        jsr     SendOnePack
        bcc     mdev2
LCCD5:
.endif

; 
        dec     Unit
        jmp     mdev1

mdev2:  jsr     ReceivePack ;Get the response
        lda     statbyte
        beq     mordevices
; 
; Okay, we done last device. Squirrel away the number of devices
; 
mdev1:  lda     Unit
        ldy     Slot
        sta     NumDevices,y ;Devices out there
; 
; Recover the scrambled ProDOS parms
; 
        pla
        sta     CMDSCode
        pla
        sta     CMDPCount
        pla
        sta     CMDCode

.if     .defined(Liron)
        lda     #PBBValue
        sta     Power1,y
        eor     #PBCValue
        sta     Power2,y
.endif
        rts

; 
squirrel:
        ldx     Slot
        sta     SHTempX,x
        tya
        sta     SHTempY,x
        rts
