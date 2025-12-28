

.include "data_config.asm"


; --- Part 5 ---
.section ".rodata_p5" superfree
BIN UFTC_Labo   "./res/Stage/PrtLabo_vert.pic"
BIN UFTC_Maze   "./res/Stage/Maze/PrtMaze_vert.pic"


; --- Auto-split: Switching to Bank 7 ---


; --- Auto-split: Switching to Bank 7 ---
BIN UFTC_Mimi   "./res/Stage/Mimi/PrtMimi_vert.pic"

.ends
