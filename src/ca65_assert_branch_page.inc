; From https://forums.nesdev.org/viewtopic.php?t=14622

; test:
;         beq :+ ; .assert should happen if this would cross a page
;         assert_branch_page :+
;         nop
;         :
;         rts

.macro assert_branch_page label_
 .assert >(label_) = >*, error, "Page crossing detected!"
.endmacro
