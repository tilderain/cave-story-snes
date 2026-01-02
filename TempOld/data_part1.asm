.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 1 ---
.section ".rodata_p1" superfree
BIN UFTC_Cent   "./res/Stage/PrtCent_vert.pic"


; --- Auto-split: Switching to Bank 2 ---
BIN UFTC_EggIn  "./res/Stage/PrtEggIn_vert.pic"
BIN UFTC_Eggs   "./res/Stage/Eggs/PrtEggs_vert.pic"
BIN UFTC_EggX   "./res/Stage/PrtEggX1_vert.pic"
BIN UFTC_Fall   "./res/Stage/PrtFall_vert.pic"

.ends
