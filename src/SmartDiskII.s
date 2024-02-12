
        .org    $0000
        .byte   "SmartPort firmware written by Michael Askins.", $0a
        .byte   "SoftSP V6 conversion by MFA2 Labs. ", $0a
        .byte   "SmartDiskII integration by Bradley Bell. ", $0a
        .byte   "Inspiration from Adrian Black. ", $0a
        .byte   $00

        .res    $0100-*,$ff

        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"
        .incbin "DiskII-P5.bin"

        .res    $1000-*,$ff

        .incbin "SoftSP-KBOOHK_S1.bin"
        .incbin "SoftSP-KBOOHK_S2.bin"
        .incbin "SoftSP-KBOOHK_S3.bin"
        .incbin "SoftSP-KBOOHK_S4.bin"
        .incbin "SoftSP-KBOOHK_S5.bin"
        .incbin "SoftSP-KBOOHK_S6.bin"
        .incbin "SoftSP-KBOOHK_S7.bin"
