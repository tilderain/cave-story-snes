.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


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
BIN UFTC_Cave   "./res/Stage/PrtCave_vert.pic"

.ends
