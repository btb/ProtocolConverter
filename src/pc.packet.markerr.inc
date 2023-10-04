markerr:
.if     .defined(Liron) || .defined(IIc)
        jmp     dberror
.else   ;SoftSP
        sec
        jmp     LC963
.endif
