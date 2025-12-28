

.include "data_config.asm"


; --- Part 10 ---
.section ".rodata_p10" superfree
BIN UFTC_MazeM  "./res/Stage/Maze/PrtMazeM_vert.pic"


; --- Auto-split: Switching to Bank 12 ---


; --- Auto-split: Switching to Bank 12 ---
BIN UFTC_Kings  "./res/Stage/White/PrtKings_vert.pic"
BIN UFTC_Statue "./res/Stage/Hell/PrtStatue_vert.pic"
BIN UFTC_Ring2  "./res/Stage/White/PrtRing2_vert.pic"

.ends
