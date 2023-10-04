Entry:  bcc     bentry      ;If non-boot, skip jump to boot
.if     .defined(SSC)
        jmp     BootC
.elseif .defined(Grappler)
        jmp     LowEntry2
.else   ;Liron||IIc
        jmp     Bootcode
.endif

;
; X is still set to slot number
;
bentry:
.if     .defined(IIc)
        lda     #%01000000
        trb     ProFLAG+SlotNum ;ProFLAG is fixed in //c

atentry:
.elseif .defined(SSC)
        lda     ProFLAG+SlotNum ;ProFLAG is fixed
        and     #%10111111
        sta     ProFLAG+SlotNum ;ProFLAG is fixed
.endif

        cld                 ;Don't want decimal mode!!
        txa
        tay                 ;Really want it in Y... no ROR ABS,Y
; 
; If this is a PC call, then get the address of the parm table
; 
        lda     ProFLAG,y
        bmi     noplay
; 
        pla                 ;Get lo order
        sta     SHTempX,y   ;Keep lo parm address-1
        clc
        adc     #3
        tax                 ;Lo order new return address
        pla                 ;Get hi order address
        sta     SHTempY,y   ;Keep hi parm addr-1
        adc     #0
        pha                 ;Push back new return address hi
        txa
        pha                 ;Push new return address lo

noplay:
;
; On the //c, it is important to have the Disk // enable lines
;  off for as long as possible before using the IWM (phases, /WRREQ
;  lines). Wait here 'til the Disk // motors are off.
;
.if     .defined(IIc) || .defined(SSC)
        jsr     WaitIWMOff  ;Must preserve Y!!
.endif
; 
; We can't really tolerate interrupts in most of the code, so disable
; 
        php                ;Save interrupt status
.if     .defined(KBOOHK) || .defined(Grappler)
        jsr     LCB93
.endif
        sei                ;No interrupts please
; 
; Preserve the zero page work area
; 
        ldx     #ZPSize-1
pzp:    lda     ZeroPage,x
        pha
        dex
        bpl     pzp
; 
; Okay, we're safe... now it's all right to store in zero page
; 
        sty     Slot
.if     .defined(Liron)
        lda     Power1,y
        cmp     #PBBValue
        bne     @LCC19
        eor     #PBCValue
        eor     Power2,y
        beq     allset
@LCC19:
.elseif .defined(KBOOHK) || .defined(Grappler)
        lda     ProFLAG,y
        cmp     #$a7
        beq     eqa7
        cmp     #$a6
        beq     eqa7
        and     #$7e
        eor     #$64
        beq     allset
; looks at line 17, character 1 and then line 9, characters 40..1
        ldx     #$08
LCDE9:
        lda     Line17
        bpl     allset
        dex
        bpl     LCDE9
        lda     #$e5
        bne     nee5
eqa7:   eor     #$42
nee5:
        sta     ProFLAG,y
        and     #$7f
        tax
        jsr     LCB8F
.endif
.if     .defined(Liron) || .defined(KBOOHK) || .defined(Grappler)
        lda     #$00
        jsr     AssignID
.endif
; 
; 
; Now map any ProDOS unit references to our sequential ones. The method is
; bizarre and magicians never reveal their secrets.
; 
allset: lda     CMDUnit     ;76543210 7&6 specify unit
        rol     A           ;6543210X C<-7
        php                 ;Save drive num
        rol     A           ;543210X7 C<-6
        rol     A           ;43210X76 (6 is grp of 2)
        plp                 ;C<-7
        rol     A           ;3210X767
        and     #%00000011  ;ProDOS only install up to 4
        eor     #%00000010  ;000000/67; 6 was /grpoftwo
        cpy     #4          ;If in slot 1, 2, or 3 reverse grps of two
        bcs     allset1
        eor     #%00000010
allset1: tax
        inx
        stx     CMDUnit     ;You got it
; 
; Now if this is through the MLI xface, gotta copy stuff into the send buffer
; from the parameter list.
; 
        lda     ProFLAG,y
        bpl     darnit
        jmp     skipcopy

; 
; Get the address of the in-line parameter table
; 
darnit: lda     SHTempX,y   ;Get back the low part buff addr
        sta     buffer
        lda     SHTempY,y   ; and the hi part
        sta     buffer+1
; 
; Now pull out the command code, and the address of the parameters.
; 
        ldy     #1          ;Stacked address is EA-1
        lda     (buffer),y
        sta     CMDCode     ;Nice
        iny
        lda     (buffer),y  ;Get lo part of parmlist address
        tax                 ;save it
        iny
        lda     (buffer),y  ;Get hi part
        sta     buffer+1
        stx     buffer
; 
; Now buffer points to parmlist
; Check command type, and pigeonhole the parmlist length
; 
        lda     #ERR_BadCmd
        ldx     CMDCode
        cpx     #$0a        ;Only valid codes are 0-9
        bcc     noeh        ;=> at least he got that right
ErrorHitch:
        jmp     Error       ;Gee, maybe we should promote this guy...

noeh:   ldy     #0          ;Set for indct compare
        lda     (buffer),y  ;Get # of parms?
        sta     Unit
; 
; Now copy the bytes
; 
        ldy     #CmdLength-1 ;Always copy the maximum
copyloop:
        lda     (buffer),y  ;Pull it out of their hat
        sta     CMDCode,y   ;Stuff it into mine
        dey
        bne     copyloop    ;Copy 'em all
; 
; Okay. The caller of the PC could be making one of three calls with a unit
; number of $00, Control, Init or Status. Check for these and do what is
; appropriate.
; 
        lda     CMDUnit
        bne     skipcopy    ;Never mind
; 
; Check the parameter count for this call to unit#0
; 
        ldx     CMDCode
        lda     ParamCTab,x ;Get the length of this command
        and     #$7f        ;Force 0 -> MSB
        tay                 ;Hang on
        lda     #ERR_BadPCnt ;Antic bad count
        cpy     Unit        ;User's pcount is currently here
        bne     ErrorHitch  ;What a baby!
; 
; Now service one of the three commands
; 
        cpx     #CMD_Init
        bne     notinit     ;Not an Init call
        lda     #PowerReset ;Just like power or reset key(//c)
        jsr     AssignID    ;Do a reset cycle
AOkay:  lda     #0          ;No error allowed
        jmp     sa2

notinit:
        txa                 ;Equiv to 'cmp #StatusCMD'
        bne     maybectrl
; 
        lda     #ERR_BadCtl ;Antic a non zero stat code
        ldx     CMDSCode    ;Stat unit#0 can only be code=0
        bne     ErrorHitch
        txa                 ;Equiv to 'lda #0'
        ldx     Slot
        ldy     #7
nin1:   sta     (CMDBuffer),y ;Clear some space
        dey
        bne     nin1
        lda     NumDevices,x
        sta     (CMDBuffer),y ;Stick it where they want it
        iny

.if     .defined(IIc) || .defined(SSC)
        lda     $4F9        ; //c Port 1 interrupt status
.else   ;Liron||KBOOHK
        lda     #$00
.endif
; 
        sta     (CMDBuffer),y ;Store PC interrupt status
; 
        lda     #8
        dey                 ;A,Y has 0008; # bytes status
        jsr     squirrel
; 
        jmp     AOkay       ;Skip down (up) with no error

maybectrl:
        cmp     #CMD_Control
        bne     BUnit       ;Unit #0 was a bad one
; 
        ldx     CMDSCode    ;We allow two control calls for Unit#0
        beq     enabint     ;0 means enable interrupts
        dex
        beq     disabint    ;1 means disable interrupts
        lda     #ERR_BadCtl
ErrorHitch2:
        bne     ErrorHitch  ;No other codes allowed
; 
BUnit:  lda     #ERR_BadUnit ;Only certain calls can have Unit#0
        bne     ErrorHitch  ;Branch always

.if     .defined(IIc) || .defined(SSC)
enabint:
        lda     #$c0
        sta     $5f9
 .ifdef IIc
        lda     #$0f
        tsb     $c09a
        bne     aokayhitch
 .endif

disabint:
 .ifdef IIc
        lda     #$01
        trb     $c09a
aokayhitch:
 .endif
        jmp     AOkay
.else   ;Liron||KBOOHK||Grappler
enabint:
disabint:
        lda     #ERR_NoInt ;Interrupts not supported
        bne     ErrorHitch
.endif

.if     .defined(Grappler) || .defined(KBOOHK)
        nop
        nop
.endif

; 
; Okay, everything's all groovy. ProDOS re-enters here. Check Unit number to be
; sure there is a corresponding device
; 
skipcopy:
        lda     #ERR_NoDrive ;Anticipate bad unit number
        ldy     Slot
        ldx     NumDevices,y
        cpx     CMDUnit
        bcc     ErrorHitch2 ;[//c: 'blt ErrorHitch2' Safe- If C clr then Z is clr]
; 
; Set buffer and bytecount in anticipation of the inevitable SendPack.
; 
        lda     #CmdLength
        sta     ByteCountl
        lda     #0
        sta     ByteCounth
        sta     buffer+1
        lda     #CMDCode
        sta     buffer
; 
; If it's a PC call, omit the next two steps
; 
        ldx     Slot
        lda     ProFLAG,x   ;Is it a call from ProDOS?
        bpl     notstat     ;=> Statcode already set...
; 
; Need to generate a parameter count for a ProDOS call
; 
        ldx     CMDCode
        lda     ParamCTab,x
        and     #$7f
        sta     Unit
; 
; ProDOS always needs the highest blockno byte zeroed
; 
        lda     #0
        sta     CMDBlockS
; 
; If this is a ProDOS status call, set stat code to zero
; 
        lda     CMDCode
        bne     notstat     ;=> Not status so forget it
; lda #SCDeviceStat ;A is already zero
        sta     CMDSCode    ;Store in command table
; 
; Okay, finally send over the damn command
; 
notstat:
        lda     Unit
        ldx     CMDPCount   ;Swap the Parmcount & unit#
        stx     Unit
        sta     CMDPCount   ;Now they're correct
; 
        lda     #cmdmark
        sta     WPacketType
; 
        jsr     ClrPhases   ;Bring all phases off for Quark
        jsr     SendPack
        bcs     behitch     ;If not okay, skip to bus error
; 
; Now copy over the buffer address for any data xfer
; 
        lda     CMDBufferl
        sta     buffer
        lda     CMDBufferh
        sta     buffer+1
; 
; Now for some commands, we have to send over a packet of data, too. See if this
; command is one of THOSE.
        ldx     CMDCode
        lda     ParamCTab,x
        bpl     noxtrasend  ;Encoded in top bit
; 
; The buffer address and bytecount depend on the call type.
; 
        cpx     #CMD_Control
        bne     NOControl
; 
; In the case of control, bytecount:=(buffer)
; and buffer := buffer+2
; 
        ldy     #1
        lda     (buffer),y  ;Get Hi order bytecount
        tax
        dey
        lda     (buffer),y
        pha                 ;Keep for later
        clc
        lda     #2
        adc     buffer
        sta     buffer
        pla                 ;Get back Lo order bytecount
        bcc     secondsend  ;Skip hi ord increment
        inc     buffer+1
        jmp     secondsend  ;Skip to store bytecount

NOControl:
        cpx     #CMD_WriteBlock ;Check for a writeblock
        bne     NOWBlock    ;Must be control or write
; 
; In the case of WriteBlock, the length is 512 and the buffer address is at
; buffer in the command table
; 
        lda     #0
        ldx     #2
        bne     secondsend

; 
; For FileWrite, the buffer address is at CMDBuffer and the length is at
; CMDBlock.
; 
NOWBlock:
        ldx     CMDBlockh
        lda     CMDBlockl
; 
secondsend:
        stx     ByteCounth
        sta     ByteCountl
; 
        lda     #datamark
        sta     WPacketType ;Identify this as a data packet
; 
        jsr     SendData
        bcc     noxtrasend
behitch:
        lda     #ERR_BusErr ;This is the bus error hitch
        bne     Error

; 
; On ProDOS status call, we've got to point the buffer pointer correctly to zero
; page... It's the only case special case (on Write, Format and Control no data
; comes back).
; 
noxtrasend:
        ldy     Slot
        lda     ProFLAG,y
        bpl     getresults
        lda     CMDCode
        bne     getresults
; 
        lda     #CMDBufferh ;Want status in these four
        ldx     #>CMDBufferh
        sta     buffer
        stx     buffer+1
; 
; Please to be calling ReceivePack
; 
getresults:
        jsr     RecPack     ;Get status byte (maybe read data too)
        bcs     behitch
; 
; Figure how many bytes were sent and put that in X,Y temps
; 
        jsr     RcvCount    ;Do the times 7...
        jsr     squirrel    ;Store away count in SHTEMPs
; 
; For the ProDOS status call, we've got to look at the status byte returned and
; return a DIP error if appropriate. Also overwrite the X,Y temps with # blocks
; if this is a ProDOS Stat call.
; 
        lda     CMDCode     ;Is it a ProDOS status call
        bne     noerror
        ldx     Slot
        lda     ProFLAG,x
        bpl     noerror
        lda     CMDBlockl   ;This'll get loaded into the XY regs later
        sta     SHTempX,x
        lda     CMDBlockh
        sta     SHTempY,x
        lda     CMDBufferh  ;Check status byte
.if     .defined(Liron)
        and     #$10
        bne     noerror
.elseif .defined(IIc) || .defined(SSC)
;probably ok for Liron, but not in known rom dumps
        lsr     a
        lsr     a
        lsr     a
        bcc     ChkOffLn    ;no error, go check off line
        lda     #ERR_WriteProt ;else set WPROT error
 .if    .defined(IIc)
        bra     Error
 .else
        bne     Error       ;bra is 65C02 only
 .endif

ChkOffLn:
        lsr     a
        lsr     a
.endif
.if     .defined(KBOOHK) || .defined(Grappler)
        and     #$10
        bne     noerror
.endif
        lda     #ERR_OffLine ;Assume error
.if    .defined(IIc) || .defined(SSC)
        bcc     Error
.else   ;Liron||KBOOHK||Grappler
        bne     Error
.endif

; 
; Now it's time to think about returning to the caller. Remember that ProDOS
; doesn't want to know about soft errors, only fatal ones. If this is a ProDOS
; call, and the soft error bit in the statbyte is set, there IS NO error
; (statbyte is cleared). Also, ProDOS wants only I/O, Write Protect, No Device,
; Offline. If any other hard error comes from the device on a ProdOS call, map
; it to an I/O Error. (Gross me out.)
; 
noerror: lda     statbyte
Error:  ldy     Slot        ;Need access to screenholes
        sta     Retry,y     ;Keep unadulterated error in shole
        tax                 ;Set the Z flag
        beq     sa2         ;Special case the zero
        ldx     ProFLAG,y   ;Set N to ProDOS call or not
        bpl     sa2         ;If PC call, no mapping occurs
        ldx     #0          ;Assume a soft error
        cmp     #%01000000  ;Soft error check
        bcs     storeaway   ;If $40 or bigger, map to zero
        ldx     #ERR_IOError ;Now anticipate ProDOS I/O error
        cmp     #ERR_WriteProt
        beq     sa2         ;OK to return Write Protect
        cmp     #ERR_NoDrive
        beq     sa2         ;OK to return Drive disconnected
        cmp     #ERR_OffLine
        beq     sa2
; 
storeaway:
        txa                 ;Use the default value
sa2:
        ldy     Slot
        sta     SHTemp1,y   ;Keep in screenhole
;
; If this is the //c version, we need to reset the IWM to its
;  Former disk // state. This is done by setting the mode register
;  to a little known (and less documented) mode which speeds up the
;  internal motor timeout. When the motor enable has timed out,
;  mode can be set back to zero. This method is necessary because
;  if the timer is enabled within the timeout period, the motor on
;  a Rev A IWM pops on for the full timeout period (since mode changes
;  are disabled when the motor is on. I know, it's bizarre. Blame Mac.
;
.if     .defined(IIc) || .defined(SoftSP)
        lda     monclr+TheOff ;Motor off
 .if    .defined(IIc)
        bit     l6set+TheOff ;Into mode reg access mode
        lda     #$2b        ;This is the magic "speed up" value
        sta     l7set+TheOff ;Throw into mode register
        nop                 ;You're supposed to wait a while
        nop
        nop
        nop
waitoff:
        lda     l7clr+TheOff ;Wait 'til motor off
        and     #$20
        bne     waitoff
        ldy     #0          ;Now set the reg back to $00
 .endif
 .if    .defined(IIc) || .defined(SSC)
        ldx     #TheOff    ;IWM's in slot 6
 .endif
 .if    .defined(IIc)
        jsr     SetIWMode
        lda     l6clr+TheOff
 .endif
        lda     ca1clr+TheOff
        lda     lstrbclr+TheOff
 .if    .defined(IIc) || .defined(SSC)
        ldy     Slot        ;Need Slot in Y
 .endif
.endif
; 
; Now, restore our zero page area.
; 
        ldx     #0
rzp:    pla
        sta     ZeroPage,x
        inx
        cpx     #ZPSize
        bcc     rzp
; 
; We're into the stretch! Restore interrupt mask, load X, Y, and A and set the
; carry if the error byte is non-zero.
        plp                 ;Restore interrupt flag
        lda     SHTempX,y   ;Get X value
        tax
        lda     SHTemp1,y   ;Grab the error result code
        pha
        lda     SHTempY,y   ;Pull out the Y value
        tay                 ;No more access to screenholes
        clc                 ;Anticipate zero result code
        pla                 ;Pull back result code
        beq     finalskip   ;Return with carry clear
        sec                 ;Some type of error
finalskip:
.if     .defined(IIc)
        php                 ;Save carry and Z flag
        bit     ProFLAG+SlotNum ;Ick - ProFLAG is fixed in //c
        bvs     ick1        ;If bit 6=1, then return to alt ROM
        plp                 ;Vclr so return across ROM bank bdy
        jmp     SWRTS2
ick1:
        plp
.endif
.if     !.defined(Grappler)
do_RTS:
.endif
        rts
