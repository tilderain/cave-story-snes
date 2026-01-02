.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 15 ---
.section ".rodata_p15" superfree
BIN TSC_EggX	"./res/tsc/en/Stage/EggX.tsb"
BIN TSC_EggX2	"./res/tsc/en/Stage/EggX2.tsb"
BIN TSC_Fall	"./res/tsc/en/Stage/Fall.tsb"
BIN TSC_Frog	"./res/tsc/en/Stage/Frog.tsb"
BIN TSC_Gard	"./res/tsc/en/Stage/Gard.tsb"
BIN TSC_Hell1	"./res/tsc/en/Stage/Hell1.tsb"
BIN TSC_Hell2	"./res/tsc/en/Stage/Hell2.tsb"
BIN TSC_Hell3	"./res/tsc/en/Stage/Hell3.tsb"
BIN TSC_Hell4	"./res/tsc/en/Stage/Hell4.tsb"
BIN TSC_Hell42	"./res/tsc/en/Stage/Hell42.tsb"
BIN TSC_Island	"./res/tsc/en/Stage/Island.tsb"
BIN TSC_Itoh	"./res/tsc/en/Stage/Itoh.tsb"
BIN TSC_Jail1	"./res/tsc/en/Stage/Jail1.tsb"
BIN TSC_Jail2	"./res/tsc/en/Stage/Jail2.tsb"
BIN TSC_Jenka1	"./res/tsc/en/Stage/Jenka1.tsb"
BIN TSC_Jenka2	"./res/tsc/en/Stage/Jenka2.tsb"
BIN TSC_Kings	"./res/tsc/en/Stage/Kings.tsb"
BIN TSC_Little	"./res/tsc/en/Stage/Little.tsb"
BIN TSC_Lounge	"./res/tsc/en/Stage/Lounge.tsb"
BIN TSC_Malco	"./res/tsc/en/Stage/Malco.tsb"
BIN TSC_Mapi	"./res/tsc/en/Stage/Mapi.tsb"
BIN TSC_MazeA	"./res/tsc/en/Stage/MazeA.tsb"
BIN TSC_MazeB	"./res/tsc/en/Stage/MazeB.tsb"
BIN TSC_MazeD	"./res/tsc/en/Stage/MazeD.tsb"
BIN TSC_MazeH	"./res/tsc/en/Stage/MazeH.tsb"
BIN TSC_MazeI	"./res/tsc/en/Stage/MazeI.tsb"
BIN TSC_MazeM	"./res/tsc/en/Stage/MazeM.tsb"
BIN TSC_MazeO	"./res/tsc/en/Stage/MazeO.tsb"
BIN TSC_MazeS	"./res/tsc/en/Stage/MazeS.tsb"


; --- Auto-split: Switching to Bank 15 ---
BIN TSC_MazeW	"./res/tsc/en/Stage/MazeW.tsb"
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

.ends
