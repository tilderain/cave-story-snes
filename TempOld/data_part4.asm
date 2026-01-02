.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 4 ---
.section ".rodata_p4" superfree
BIN UFTC_Store  "./res/Stage/PrtStore_vert.pic"
BIN UFTC_Weed   "./res/Stage/PrtWeed_vert.pic"
BIN UFTC_Blcny  "./res/Stage/White/PrtBlcny_vert.pic"





; --- Auto-split: Switching to Bank 5 ---
BIN UFTC_EggX2  "./res/Stage/PrtEggX2_vert.pic"

.ends
