; ca65 - macros for MPW IIgs-style String Handling

; The below macros implement string handling mainly in the style of MPW IIgs,
; plus some inspirations from Merlin. All of the macros except ds support up
; to 10 arguments.


.feature leading_dot_in_identifiers

; Mode flags for macros
__domsb .set 1  ; MSB on by default
__smode .set 0  ; "ASIS" by default

; control whether the various macros have msb set or not
.macro  msb   Arg
  .if .xmatch({Arg},on) .or .xmatch({Arg},ON)
    __domsb .set 1
  .else
    __domsb .set 0
  .endif
.endmacro

; control whether the various macros generate plain strings,
; Pascal strings, or C strings
.macro  string   Arg
  .if .xmatch({Arg},asis) .or .xmatch({Arg},ASIS)
    __smode .set 0
  .elseif .xmatch({Arg},pascal) .or .xmatch({Arg},PASCAL)
    __smode .set 1
  .elseif .xmatch({Arg},c) .or .xmatch({Arg},C)
    __smode .set 2
  .else
    .warning "unknown string mode, default asis"
    __smode .set 0
  .endif
.endmacro

; "high ascii"
.macro  hasc  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      .repeat .strlen(a0), I
        .byte .strat(a0, I) | $80
      .endrep
    .else
      .byte a0
    .endif
    hasc a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; "low ascii"
.macro  lasc  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      .repeat .strlen(a0), I
        .byte   .strat(a0, I) & $7f
      .endrep
    .else
      .byte a0
    .endif
    lasc  a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; "inverse ascii"
; on Apple II, force characters to inverse video mapping
; on 40-col screen
.macro  inv  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      .repeat .strlen(a0), I
        .byte .strat(a0, I) & $1f
      .endrep
    .else
      .byte a0
    .endif
    inv  a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; dextral character inverted
.macro  dci  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      repeat .strlen(a0)-1, I 
        .byte .strat(a0, I) | $80 
      .endrep
      .byte .strat(a0, .strlen(a0)-1) & $7F 
    .else
      .byte a0
    .endif
    dci  a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; ascii selected by msb directive
.macro  asc a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .if __domsb
    hasc  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .else
    lasc  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; pascal string
.macro    pstr  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .byte .strlen(a0)
    asc   a0
    pstr  a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; C string
.macro    cstr  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    asc   a0
    .byte $00
    cstr  a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; String by selected mode
.macro    str   a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .if __smode = 1 ; pascal
    pstr  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .elseif __smode = 2 ; C
    cstr  a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .else ; default to asis
    asc   a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

.macro  dc_b   a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      str a0
    .else
      .byte a0
    .endif
    dc_b a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

; These are used for dc.x and ds.x
.define .b byte,
.define .w word,
.define .l long,

.macro dc t0, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .if .xmatch({t0},byte)
    dc_b a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .elseif .xmatch({t0},word)
    .word a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .elseif .xmatch({t0},long)
    .dword a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .else
    ; default to treating as bytes
    dc_b t0
    dc_b a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro

.macro ds t0, l0, v0
  .local __here
  .if .xmatch({t0},/)
    ; Fill to next page boundary, sort of Merlin-style
    ; merlin uses a backslash, ca65 doesn't like it
    .if .const(*)
      .ifblank l0
        .res $100-(*&$ff)
      .else
        .res $100-(*&$ff),l0
      .endif
    .else
      .error "Can't use slash ds in relocatable mode"
    .endif
  .else
    .ifblank v0
      ds t0, l0, 0
    .else
      .ifnblank l0
        .if .xmatch({t0},byte)
          .res l0, v0
        .elseif .xmatch({t0},word)
          .res l0*2, v0
        .elseif .xmatch({t0},long)
          .res l0*4, v0
        .else
          .res t0, l0
        .endif
      .endif
    .endif
  .endif
.endmacro

; macro to mimic the HEX feature of some assemblers
; extended from and with thanks to https://pastebin.com/jiWdS68E
.macro hex a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
  .local strlen
  .local nib
  .local hiNib
  .local hiNibReady
  
  .ifnblank a0
    .if (.match (.left (1, {a0}), ""))
      ; a string was passed
      strlen = .strlen( a0 )
      hiNibReady .set 0
      
      ; check each character in the string and turn into bytes
      .repeat strlen, I
        nib .set .strat(a0, I)
        ; allow space or underscore or $, but ignore them.
        .if .not ( nib = ' ' .or nib = '_' .or nib = '$')
          ; convert nib if in range for 0..9 or A..F or a..f
          .if nib >= 48 .and nib <= 57
            nib .set nib - 48 
          .elseif nib >= 65 .and nib <= 70
            nib .set  nib - 55
          .elseif nib >= 97 .and nib <= 102
            nib .set nib - 87 
          .else
            .error "Invalid character in hex byte."
          .endif
          ; create a byte if ready
          .if hiNibReady
            .byte hiNib | nib
            hiNibReady .set 0
          .else
            ; if lone nybble at the end of the string, treat it as a byte
            .if I + 1 = strlen
              .warning "Incomplete hex byte at end of data."
              .byte nib
            .else
              hiNib .set nib << 4
              hiNibReady .set 1
            .endif
          .endif
        .endif
      .endrepeat
    .else
      ; not a string, try to use .byte
      .byte a0
    .endif
    hex a1, a2, a3, a4, a5, a6, a7, a8, a9
  .endif
.endmacro
