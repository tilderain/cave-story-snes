.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 8 ---
.section ".rodata_p8" superfree
BIN		PAT_Ill01	"./res/credits/ill01.pat"
BIN		MAP_Ill01	"./res/credits/ill01.map"
BIN		PAT_Ill02	"./res/credits/ill02.pat"
BIN		MAP_Ill02	"./res/credits/ill02.map"
BIN		PAT_Ill03	"./res/credits/ill03.pat"
BIN		MAP_Ill03	"./res/credits/ill03.map"
BIN		PAT_Ill04	"./res/credits/ill04.pat"


; --- Auto-split: Switching to Bank 11 ---
BIN		MAP_Ill04	"./res/credits/ill04.map"
BIN		PAT_Ill05	"./res/credits/ill05.pat"
BIN		MAP_Ill05	"./res/credits/ill05.map"

.ends
