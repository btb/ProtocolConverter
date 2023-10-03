WritePrep:
Divide7:
; 
; Set up auxptr <- buffer+$80   if $0FF < bytecount < $200
;     or auxptr <- buffer+$100  if $1FF < bytecount
; 
        ldx     ByteCounth  ;0, 1, or 2
        beq     noauxptr    ;Auxptr used only for full pages
; 
        lda     buffer+1
        sta     auxptr+1    ;Copy over hi order part
; 
        lda     #$80        ;Anticipate smaller bytecount
        cpx     #1          ;Check bytecount
        beq     sap1        ;=> $0FF < bytecount < $200
; 
        inc     auxptr+1    ;Add $100 to bytecount instead
        lda     #0          ;Make sure lo order unaltered
sap1:   clc
        adc     buffer
        sta     auxptr
        bcc     noauxptr
        inc     auxptr+1
; 
; Now look up the first order guess for DIV and MOD. X still has bytecount DIV
; 256.
noauxptr:
        lda     pdiv7tab,x
        sta     grp7ctr
        lda     pmod7tab,x
        sta     oddbytes
; 
; Now add in the mods and divs for each of the five hi order bits in the lo
; order bytecount, correcting each time MOD becomes bigger than 6.
; 
        ldx     #5          ;Do for five bits
        lda     ByteCountl
        sta     temp        ;Store lo order for shifting
        and     #%00000111  ;Save lo three for later
        tay
; 
divide3:
        asl     temp        ;C <- next from bytecountl
        bcc     divide2     ;If clear, no effect on DIV,MOD
        lda     mod7tab,x   ;Get MOD7 for 2^n
divide4:
        clc
        adc     oddbytes    ;Got new MOD value
        cmp     #7          ;Is it too big?
        bcc     divide1     ;=> NO leave MOD - 0->C
        sbc     #7          ;Bring MOD under 7 - C still set
divide1:
        sta     oddbytes
        lda     div7tab,x   ;Get DIV for this 2^n
        adc     grp7ctr     ;Add to DIV along with correction (C)
        sta     grp7ctr     ;Update the DIV
divide2:
        dex                 ;One less bit to deal with
        bmi     divide5     ;Escape after 6 times through loop
        bne     divide3     ;Take brnch 1st 5 loops
; 
        tya                 ;Get back the last three bits
        jmp     divide4     ;Sixth pass add in remains
divide5:
