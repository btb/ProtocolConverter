;*******************************************************************************
;* PC.MAIN                                                                     *
;* Protocol Converter / CBus Driver                                            *
;*******************************************************************************


.if     .defined(Liron) || .defined(IIc)
.include "pc.main.entry.inc"
.include "pc.main.paramctab.inc"
.include "pc.main.assignid.inc"
.endif
