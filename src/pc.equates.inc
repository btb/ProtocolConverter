;*******************************************************************************
;* PC.EQUATES                                                                  *
;* Equates                                                                     *
;*******************************************************************************

PDIDByte =      $bf        ;ProDOS attributes byte
PCID2   =       $00        ;This means a Liron card

;*************************
;*  Zero Page  (temps)   *
;*************************

ZeroPage =      $40        ;Start of zero page work area
checksum =      $40
topbits  =      $41
CMDCode  =      $42        ;ProDOS parameter passing area
CMDPCount =     $43
CMDUnit =       $43
CMDBuffer =     $44
CMDBufferl =    $44
CMDBufferh =    $45
CMDBlockl =     $46
CMDSCode =      $46
CMDBlockh =     $47
CMDBlockS =     $48
grp7ctr =       $4b
rcvbuf  =       $4b
oddbytes =      $4c
next    =       $4d
ByteCountl =    $4d
next1   =       $4d
statbyte =      $4d
ByteCounth =    $4e
next2   =       $4e
next3   =       $4f
next4   =       $50
next5   =       $51
pointer =       $52
next6   =       $52
next7   =       $53
buffer  =       $54
auxptr  =       $56
buffer2 =       $56
Slot    =       $58
tbodd   =       $59
temp    =       $59
Unit    =       $5a         ;Current target unit
WPacketType =   $5b

ZPSize   =       $1c        ;Size of zero page work area


ClearIOROMs =   $cfff
stack   =       $100


Line17  =       $450        ;Line 17 of text screen

;*************************
;*                       *
;*   Screenhole Storage  *
;*                       *
;*************************
;*
;* The screenhole layout is as follows:
;*
;*               //e         //c
;*  ------------------------------
;*  ProFlag     $478+n      $478
;*  Retry       $4F8+n      $4F8
;*  SHTemp1     $578+n      $578
;*  SHTempX     $5F8+n      $5F8
;*  SHTempY     $678+n      $678
;*  Power1      $6F8+n      ----
;*  Power2      $778+n      ----
;*  NumDevices  $7F8+n      $6FE
;*  SvBcL       $6F8+n      $6F8
;*  SvBcH       $778+n      $778
;*

.if     .defined(IIc) || .defined(SSC) || .defined(Grappler)
scholes =       $473        ;Use the slot 0 sholes for temps
.else   ;Liron
scholes =       $478
.endif

ProFLAG =       scholes
Retry   =       scholes+$80
SHTemp1 =       scholes+$100
Retry2  =       SHTemp1
SHTempX =       scholes+$180
SHTempY =       scholes+$200
.if     .defined(IIc) || .defined(SSC) || .defined(Grappler)
NumDevices =    $6f9        ;Actually in slot 6
.else   ;Liron
Power1  =       $06f8
Power2  =       $0778
NumDevices =    $7f8
.endif

SvBcL   =       $06f8
SvBcH   =       $0778

MON_CV  =       $25         ;cursor vertical displacement
MON_CH  =       $24         ;cursor horizontal displacement
MON_VTAB =      $fc22       ;tab to row specified in CV
MON_COUT =      $fded       ;print Acc to output device via $36-37
bootscrn =      $7db
MSlot   =       $07f8
MON_SETVID =    $fe93       ;reset char output handler to ROM
MON_SETKBD =    $fe89       ;reset char input handler to ROM
AutoScan =      $faba
Basic   =       $e000
loc0    =       $0          ;Boot parms
loc1    =       $1          ;

BOOT1   =       $0800
.if     .defined(IIc)
SWPROTO =       $c797       ;//c bank switch to $C800
SWRTS2  =       $c784       ;RTS to bank 1
.endif


;*************************
;*                       *
;*    General Equates    *
;*                       *
;*************************

PBBValue =       $a5        ;Powerup Byte Base Value
PBCValue =       $ff        ;Powerup Byte Complement Value

PowerReset =     0
CommReset =      $80

bsyto1   =       50         ;(.55 ms) T/O on /BSY before send
.if     .defined(Liron) || .defined(IIc)
bsyto2   =       10         ;(.12 ms) T/O on /BSY after send
.else   ;SoftSP
bsyto2   =       26         ;(.xx ms) T/O on /BSY after send
.endif
statmto  =       30         ;30 bytes stat mark timeout
CmdLength =      9          ;Command packet length
packetbeg =      $c3        ;Mark at beginning of packet
packetend =      $c8        ;End of packet mark
cmdmark  =       $80        ;Command packet identifier
statmark =       $81        ;Status packet identifier
datamark =       $82        ;Data Packet identifier

iwmmode  =       $07        ;No timer, asynch, latch

SCDeviceStat    = $00       ;Get Device Specific Status
SCGetDCB        = $01       ;Get Dev Ctrl Block (modebits)
SCRetNLStat     = $02       ;Return Newline Status
SCGetDevInfo    = $03       ;Get Device Info Block

KBD     =       $c000       ;Last key pressed (with hi bit set)
KBDSTRB	=		$c010		;Keyboard Strobe (clear KBD hi bit)

iwm     =       $c080

reqclr  =       iwm+0       ;IWM phase 0 off
reqset  =       iwm+1       ;IWM phase 0 on
ca1clr  =       iwm+2       ;IWM phase 1 off
ca1set  =       iwm+3       ;IWM phase 1 on
ca2clr  =       iwm+4       ;IWM phase 2 off
ca2set  =       iwm+5       ;IWM phase 2 on
lstrbclr =      iwm+6       ;IWM phase 3 off
lstrbset =      iwm+7       ;IWM phase 3 on
monclr  =       iwm+8       ;IWM motor off
monset  =       iwm+9       ;IWM motor on
enable1 =       iwm+10      ;IWM select drive 1
enable2 =       iwm+11      ;IWM select drive 2
l6clr   =       iwm+12      ;IWM read
l6set   =       iwm+13      ;IWM WP-sense
l7clr   =       iwm+14      ;IWM WP-sense/read
l7set   =       iwm+15      ;IWM write

;*
;* Error0 codes
;*
ERR0_noanswer = $01
ERR0_nomark =   $02
ERR0_wasreset = $04
ERR0_bytecmp =  $08
ERR0_csumerr =  $10
ERR0_nopackend = $20
ERR0_bushog =   $40

;*
;* Command Codes
;*

CMD_Status =    $00         ;Returns status information about a particular device.
CMD_ReadBlock = $01         ;Reads one 512-byte block from a disk device, and writes it to memory
CMD_WriteBlock = $02        ;Writes one 512-byte block from memory to a disk device
CMD_Format =    $04         ;Prepares all blocks on a block device for reading and writing
CMD_Control =   $04         ;Controls some device functions
CMD_Init =      $05         ;Resets all resident devices


Soft    =       %01000000   ;The soft error bit in statbyte

ERR_BadCmd =    $01         ;A nonexistent command was issued
ERR_BadPCnt =   $04         ;Bad call parameter count
ERR_BusErr =    $06         ;Communications error between device controller and host
ERR_BadUnit =   $11         ;Unit number $00 was used in a call other than STATUS, CONTROL, or INIT
ERR_NoInt =     $1f         ;Not documented - Interrupts not available?
ERR_BadCtl =    $21         ;The control or status code is not supported by the device
ERR_BadCtlParm = $22        ;The control parameter list contains invalid information
ERR_IOError =   $27         ;The device encountered an I/O error when trying to read or write to the recording medium
ERR_NoDrive =   $28         ;The device is not connected
ERR_WriteProt = $2b         ;The medium in the device is write protected
ERR_BadBlock =  $2d         ;The block number is outside the range allowed for the medium in the device
ERR_OffLine =   $2f         ;Device off-line or no disk in drive
ERR_LastOne =   Soft+ERR_NoDrive
ERR_SoftError = Soft+ERR_IOError

SVMask1 =       $10

RC1     =       3000        ;Send a command pack 3000 times (3 sec)
RC2     =       5           ;Data Packs (sent/rcd) get tried only 5 times
