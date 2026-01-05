; res/soundbank_vasm.s
; Corrected for vasm6502_oldstyle syntax

     section _rodata.far.audio,"a"
    
    ; Use 'global' instead of 'public'
    ; Note: Ensure this line is indented with a space or tab
    global SOUNDBANK__

; Label starts at column 0
SOUNDBANK__:
    incbin "res/soundbank.bnk"
