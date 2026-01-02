.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 9 ---
.section ".rodata_p9" superfree
BIN		PAT_Ill06	"./res/credits/ill06.pat"
BIN		MAP_Ill06	"./res/credits/ill06.map"
BIN		PAT_Ill07	"./res/credits/ill07.pat"
BIN		MAP_Ill07	"./res/credits/ill07.map"
BIN		PAT_Ill08	"./res/credits/ill08.pat"
BIN		MAP_Ill08	"./res/credits/ill08.map"
BIN		PAT_Ill09	"./res/credits/ill09.pat"
BIN		MAP_Ill09	"./res/credits/ill09.map"


; --- Auto-split: Switching to Bank 12 ---

.ends
