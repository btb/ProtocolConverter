; 
; This takes grp7ctr and oddbytes and calculates 7*grp7ctr+oddbytes. The results
; are in Y(hi) and A(lo). This is the number of bytes that were received in the
; last ReceivePack.
; 
RcvCount:
        lda     grp7ctr
        tay
        ldx     #0
        stx     grp7ctr
        ldx     #3
times7: asl     A
        rol     grp7ctr
        dex
        bne     times7
        clc
        adc     oddbytes
        bcc     t71
        inc     grp7ctr
t71:    sty     oddbytes
        sec
        sbc     oddbytes
        bcs     t72
        dec     grp7ctr
t72:    ldy     grp7ctr
        rts
