.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 5 ---
.section ".rodata_p5" superfree
BIN UFTC_Eggs2  "./res/Stage/Eggs/PrtEggs2_vert.pic"
BIN UFTC_MazeM  "./res/Stage/Maze/PrtMazeM_vert.pic"
BIN UFTC_Kings  "./res/Stage/White/PrtKings_vert.pic"
BIN UFTC_Statue "./res/Stage/Hell/PrtStatue_vert.pic"
BIN UFTC_Ring2  "./res/Stage/White/PrtRing2_vert.pic"
BIN UFTC_Ring3  "./res/Stage/White/PrtRing3_vert.pic"

BIN PAL_Mimi "./res/Stage/Mimi/PrtMimi_vert.pal"
BIN PAL_Eggs "./res/Stage/Eggs/PrtEggs_vert.pal"
BIN PAL_Maze "./res/Stage/Maze/PrtMaze_vert.pal"
BIN PAL_Blcny "./res/Stage/White/PrtBlcny_vert.pal"


; --- Auto-split: Switching to Bank 6 ---
BIN PAL_Ring3 "./res/Stage/White/PrtRing3_vert.pal"
BIN PAL_Hell "./res/Stage/Hell/PrtHell_vert.pal"

BIN PAL_Almond	"./res/Stage/PrtAlmond_vert.pal"
BIN PAL_Barr	"./res/Stage/PrtBarr_vert.pal"
BIN PAL_Cave	"./res/Stage/PrtCave_vert.pal"
BIN PAL_Cent	"./res/Stage/PrtCent_vert.pal"
BIN PAL_EggIn	"./res/Stage/PrtEggIn_vert.pal"
BIN PAL_EggX	"./res/Stage/PrtEggX1_vert.pal"
BIN PAL_Fall	"./res/Stage/PrtFall_vert.pal"
BIN PAL_Gard	"./res/Stage/PrtGard_vert.pal"
BIN PAL_Jail	"./res/Stage/PrtJail_vert.pal"
BIN PAL_Labo	"./res/Stage/PrtLabo_vert.pal"
BIN PAL_Oside	"./res/Stage/PrtOside_vert.pal"
BIN PAL_Pens	"./res/Stage/PrtPens_vert.pal"
BIN PAL_River	"./res/Stage/PrtRiver_vert.pal"
BIN PAL_Sand	"./res/Stage/PrtSand_vert.pal"
BIN PAL_Store	"./res/Stage/PrtStore_vert.pal"
BIN PAL_Weed	"./res/Stage/PrtWeed_vert.pal"

BIN PAL_Sega		"./res/sprite/sega.pal.bin"
BIN PAL_Main		"./res/sprite/quote.pal.bin"
BIN PAL_Sym			"./res/sprite/door.pal.bin"
BIN PAL_Regu		"./res/sprite/kazuma.pal.bin"
BIN PAL_Gunsmith	"./res/sprite/gunsmith.pal.bin"
BIN PAL_Plant		"./res/sprite/flower.pal.bin"
BIN PAL_Frog		"./res/sprite/balfrog1.pal.bin"
BIN PAL_Chaco		"./res/sprite/chaco.pal.bin"
BIN PAL_Jenka		"./res/sprite/jenka.pal.bin"
BIN PAL_Dark		"./res/sprite/dark.pal.bin"
BIN PAL_X			"./res/sprite/xbody.pal.bin"
BIN PAL_XB			"./res/sprite/xbody.pal.bin"
BIN PAL_LabB		"./res/back/bkMimiRegu.pal.bin"
BIN PAL_Boulder		"./res/sprite/boulder.pal.bin"
BIN PAL_MazeM		"./res/back/bkRedPal.pal.bin"
BIN PAL_Eggs2		"./res/sprite/babydragon.pal.bin"
BIN PAL_Sisters		"./res/sprite/sishead.pal.bin"
BIN PAL_Red			"./res/sprite/dripred.pal.bin"
BIN PAL_Mapi		"./res/sprite/mapignon.pal.bin"
BIN PAL_Miza		"./res/sprite/misery2.pal.bin"
BIN PAL_Intro		"./res/sprite/bubbleintro.pal.bin"

BIN BG_Blue		"./res/back/bkBlue.pic" 
BIN BG_Eggs		"./res/back/bkEggs.pic" 
BIN BG_Gard		"./res/back/bkGard.pic" 
BIN BG_Gray		"./res/back/bkGray.pic" 
BIN BG_Cent		"./res/back/bkCent.pic" 
BIN BG_Maze		"./res/back/bkMaze.pic" 
BIN BG_Maze2	"./res/back/bkMaze.pic" 
BIN BG_Red		"./res/back/bkRed.pic" 
BIN BG_Water	"./res/back/bkWater.pic"
BIN BG_LabB		"./res/back/bkMimiRegu.pic" 

.ends
