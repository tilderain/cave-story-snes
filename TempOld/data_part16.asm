.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 16 ---
.section ".rodata_p16" superfree
BIN TSC_Ring2	"./res/tsc/en/Stage/Ring2.tsb"
BIN TSC_Ring3	"./res/tsc/en/Stage/Ring3.tsb"
BIN TSC_River	"./res/tsc/en/Stage/River.tsb"
BIN TSC_Sand	"./res/tsc/en/Stage/Sand.tsb"
BIN TSC_SandE	"./res/tsc/en/Stage/SandE.tsb"
BIN TSC_Santa	"./res/tsc/en/Stage/Santa.tsb"
BIN TSC_Shelt	"./res/tsc/en/Stage/Shelt.tsb"
BIN TSC_Start	"./res/tsc/en/Stage/Start.tsb"
BIN TSC_Statue	"./res/tsc/en/Stage/Statue.tsb"
BIN TSC_Stream	"./res/tsc/en/Stage/Stream.tsb"
BIN TSC_Weed	"./res/tsc/en/Stage/Weed.tsb"
BIN TSC_WeedB	"./res/tsc/en/Stage/WeedB.tsb"
BIN TSC_WeedD	"./res/tsc/en/Stage/WeedD.tsb"
BIN TSC_WeedS	"./res/tsc/en/Stage/WeedS.tsb"



.ends
