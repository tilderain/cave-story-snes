

.include "data_config.asm"


; --- Part 0 ---
.section ".rodata_p0" superfree




snesfont:
.incbin "pvsneslibfont.pic"

snespal:
.incbin "pvsneslibfont.pal"

mariogfx: .incbin "mario_sprite.pic"
mariogfx_end:

mariopal: .incbin "mario_sprite.pal"

jumpsnd: .incbin "mariojump.brr"
jumpsndend:

walksnd: .incbin "mariowalk.brr"
walksndend:


; Compressed tileset patterns
BIN UFTC_Almond "./res/Stage/PrtAlmond_vert.pic"
BIN UFTC_Barr   "./res/Stage/PrtBarr_vert.pic"


; --- Auto-split: Switching to Bank 2 ---


; --- Auto-split: Switching to Bank 2 ---

.ends
