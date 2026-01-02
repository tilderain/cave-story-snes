.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 10 ---
.section ".rodata_p10" superfree
BIN		PAT_Ill10	"./res/credits/ill10.pat"
BIN		MAP_Ill10	"./res/credits/ill10.map"
BIN		PAT_Ill11	"./res/credits/ill11.pat"
BIN		MAP_Ill11	"./res/credits/ill11.map"
BIN		PAT_Ill12	"./res/credits/ill12.pat"
BIN		MAP_Ill12	"./res/credits/ill12.map"

BIN		PAT_Ill14	"./res/credits/ill14.pat"
BIN		MAP_Ill14	"./res/credits/ill14.map"
BIN		PAT_Ill15	"./res/credits/ill15.pat"
BIN		MAP_Ill15	"./res/credits/ill15.map"
BIN		PAT_Ill16	"./res/credits/ill16.pat"
BIN		MAP_Ill16	"./res/credits/ill16.map"


; --- Auto-split: Switching to Bank 13 ---

.ends
