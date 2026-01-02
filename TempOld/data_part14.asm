.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 14 ---
.section ".rodata_p14" superfree
BIN TSC_MiBox	"./res/tsc/en/Stage/MiBox.tsb"
BIN TSC_Mimi	"./res/tsc/en/Stage/Mimi.tsb"
BIN TSC_Momo	"./res/tsc/en/Stage/Momo.tsb"
BIN TSC_Oside	"./res/tsc/en/Stage/Oside.tsb"
BIN TSC_Ostep	"./res/tsc/en/Stage/Ostep.tsb"
BIN TSC_Pens1	"./res/tsc/en/Stage/Pens1.tsb"
BIN TSC_Pens2	"./res/tsc/en/Stage/Pens2.tsb"
BIN TSC_Pixel	"./res/tsc/en/Stage/Pixel.tsb"
BIN TSC_Plant	"./res/tsc/en/Stage/Plant.tsb"
BIN TSC_Pole	"./res/tsc/en/Stage/Pole.tsb"
BIN TSC_Pool	"./res/tsc/en/Stage/Pool.tsb"
BIN TSC_Prefa1	"./res/tsc/en/Stage/Prefa1.tsb"
BIN TSC_Prefa2	"./res/tsc/en/Stage/Prefa2.tsb"
BIN TSC_Priso1	"./res/tsc/en/Stage/Priso1.tsb"
BIN TSC_Priso2	"./res/tsc/en/Stage/Priso2.tsb"
BIN TSC_Ring1	"./res/tsc/en/Stage/Ring1.tsb"
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
