.include "data_config.asm"

; \1 = Map Address, \2 = Palette Address, \3 = ID/Bank, \4 = Width, \5 = Height
.macro BG_INFO_ENTRY
    .dd \1      ; Map Pointer (4 bytes)
    .dd \2      ; Palette Pointer (4 bytes)
    .db \3      ; ID or custom byte
    .db \4      ; Width
    .db \5      ; Height
    .db 0       ; Padding to keep the struct aligned to 12 bytes total
.endm

.section ".rodata_table" superfree
.export background_info

background_info:
    BG_INFO_ENTRY BG_bk0,    PAL_bk0,    0, 8, 8
    BG_INFO_ENTRY BG_Black,  PAL_bkBlack, 0, 8, 8
    BG_INFO_ENTRY BG_Blue,   PAL_bkBlue,  0, 8, 8
    BG_INFO_ENTRY BG_Blue,   PAL_bkBlue,  3, 8, 8
    BG_INFO_ENTRY 0,         PAL_bkFog,   5, 0, 0  ; NULL is 0 in ASM
    BG_INFO_ENTRY BG_Gard,   PAL_bkGard,  0, 6, 8
    BG_INFO_ENTRY BG_Gray,   PAL_bkGray,  0, 8, 8
    BG_INFO_ENTRY 0,         PAL_bkMoon,  1, 0, 0
    BG_INFO_ENTRY BG_Maze,   PAL_bkMaze,  0, 8, 8
    BG_INFO_ENTRY BG_Maze2,  PAL_bkMaze,  0, 8, 8
    BG_INFO_ENTRY BG_Red,    PAL_bkRed,   0, 4, 4
    BG_INFO_ENTRY 0,         PAL_bkWater, 4, 0, 0
    BG_INFO_ENTRY BG_Red,    PAL_bkRed,   0, 4, 4
    BG_INFO_ENTRY BG_Green,  PAL_bkGreen, 0, 8, 8
    BG_INFO_ENTRY BG_Blue,   PAL_bkBlue,  0, 8, 8
    BG_INFO_ENTRY BG_Fall,   PAL_bkFall,  0, 8, 8
    BG_INFO_ENTRY BG_Green,  PAL_bkGreen, 0, 8, 8
.ends
