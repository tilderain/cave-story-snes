.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 3 ---
.section ".rodata_p3" superfree
BIN UFTC_Mimi   "./res/Stage/Mimi/PrtMimi_vert.pic"
BIN UFTC_Oside  "./res/Stage/PrtOside_vert.pic"
BIN UFTC_Pens   "./res/Stage/PrtPens_vert.pic"


; --- Auto-split: Switching to Bank 4 ---
BIN UFTC_River  "./res/Stage/PrtRiver_vert.pic"
BIN UFTC_Sand   "./res/Stage/PrtSand_vert.pic"

.ends
