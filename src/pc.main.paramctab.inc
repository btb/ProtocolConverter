; 
ParamCTab:
        .byte   %00000011   ;Status:  3 parms/no data send
        .byte   %00000011   ;Read:    3 parms/no data send
        .byte   %10000011   ;Write:   3 parms/data send
        .byte   %00000001   ;Format:  1 parm /no data send
        .byte   %10000011   ;Control: 3 parms/data send
        .byte   %00000001   ;Init:    1 parm/no data send
        .byte   %00000001   ;Open:    1 parm /no data send
        .byte   %00000001   ;Close:   1 parm /no data send
        .byte   %00000011   ;CharRead: 3 parms/data send
        .byte   %10000011   ;CharWrite: 3 parms/data send
