.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 2 ---
.section ".rodata_p2" superfree
BIN UFTC_Gard   "./res/Stage/PrtGard_vert.pic"
BIN UFTC_Hell   "./res/Stage/Hell/PrtHell_vert.pic"


; --- Auto-split: Switching to Bank 3 ---
BIN UFTC_Jail   "./res/Stage/PrtJail_vert.pic"
BIN UFTC_Labo   "./res/Stage/PrtLabo_vert.pic"
BIN UFTC_Maze   "./res/Stage/Maze/PrtMaze_vert.pic"

.ends
