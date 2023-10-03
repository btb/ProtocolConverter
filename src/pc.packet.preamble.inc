; 
; This table, when sent in reverse order, provides a sync pattern used to
; synchronize the drive IWM with the data stream. The first byte (last sent) is
; the packet begin mark.
; 
preamble: .byte packetbeg
synctab: .byte  $ff,$fc,$f3,$cf,$3f
