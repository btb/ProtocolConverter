;*******************************************************************************
;*                                                                             *
;* Divide7                                                                     *
;*  This routine divides the bytecount by seven. The quotient gives the number *
;* of groups of seven bytes to be sent, and the remainder gives the number of  *
;* "odd" bytes.                                                                *
;*                                                                             *
;* Input:  bytecountl,h <- # of bytes to write                                 *
;*         buffer       <- pointer to data                                     *
;* Output: auxptr       <- pointer to speed up csumming                        *
;*         oddbytes     <- bytecount MOD 7                                     *
;*         grp7ctr      <- bytecount DIV 7                                     *
;*                                                                             *
;*******************************************************************************
pdiv7tab:
        .byte   $00,$24,$49
pmod7tab:
        .byte   $00,$04,$01
div7tab: .byte  $00,$01,$02,$04,$09,$12
mod7tab: .byte  $00,$01,$02,$04,$01,$02
