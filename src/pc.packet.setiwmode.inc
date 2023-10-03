; 
; X is slot*16, Y is the desired mode
; 
; Set up the IWM mode register. Extreme care should be taken here. Setting the
; mode byte with indexed stores causes a false byte to be written a cycle before
; the real value is written. This false value, if it enables the timer, causes
; the IWM Rev A to pop the motor on, inhibiting the setting of the mode until
; the motor times out! We avoid this by setting the mode byte only when it is
; not what we want, and if it's not we stay here until the we see that it is
; what we want.
; 
SetIWMode:
        lda     monclr,x    ;Motor must be off
        lda     l6set,x     ;Set up to access mode register
        jmp     careful     ;Don't mess unless we gotta

biz:    tya
        sta     l7set,x     ;Try storing the mode value
careful:
        tya                 ;Get back the target value
        eor     l7clr,x     ;Compare with observed value
        and     #$1f        ;Can only read low 5 bits
        bne     biz         ;If not right, back to try again
        rts
