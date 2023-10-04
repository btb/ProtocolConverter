;*******************************************************************************
;* PC.Packet                                                                   *
;* Send a CBus Packet                                                          *
;*******************************************************************************

.if     .defined(Liron) || .defined(IIc)
.include "pc.packet.sendonepack.inc"
.include "pc.packet.preamble.inc"
.include "pc.packet.waste.inc"
.include "pc.packet.markerr.inc"
.include "pc.packet.receivepack.inc"
.include "pc.packet.start2.inc"
.include "pc.packet.resetchain.inc"
.include "pc.packet.enablechain.inc"
.include "pc.packet.clrphases.inc"
.include "pc.packet.setxn0.inc"
.include "pc.packet.shifttables.inc"
.include "pc.packet.senddata.inc"
.include "pc.packet.divide7tables.inc"
.include "pc.packet.auxptrinc.inc"
.include "pc.packet.divide7.inc"
.include "pc.packet.precheck.inc"
.include "pc.packet.setiwmode.inc"
.include "pc.packet.waitiwmoff.inc"
.include "pc.packet.rcvcount.inc"
.endif
