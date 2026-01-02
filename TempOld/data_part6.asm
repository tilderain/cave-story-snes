.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 6 ---
.section ".rodata_p6" superfree
BIN BG_Stream	"./res/back/bkStream.pic" 
BIN BG_Sand		"./res/back/bkSand.pic" 
BIN BG_Hell		"./res/back/bkHell.pic" 
BIN BG_Fall		"./res/back/bkFall.pic" 

BIN BG_bk0		"./res/back/bk0.pic" 
BIN BG_Black		"./res/back/bkBlack.pic" 

BIN BG_Green		"./res/back/bkGreen.pic" 



; --- Auto-split: Switching to Bank 7 ---
BIN BG_Moon		"./res/back/bkMoon.pic" 


; --- Auto-split: Switching to Bank 8 ---
BIN BG_Fog		"./res/back/bkFog.pic" 

BIN PAL_bkBlue		"./res/back/bkBlue.pal"
BIN PAL_bkGard		"./res/back/bkGard.pal"
BIN PAL_bkMaze		"./res/back/bkMaze.pal"
BIN PAL_bkGray		"./res/back/bkGray.pal"
BIN PAL_bkGreen		"./res/back/bkGreen.pal"
BIN PAL_bkFall		"./res/back/bkFall.pal"
BIN PAL_bkWater		"./res/back/bkWater.pal"
BIN PAL_bkMoon		"./res/back/bkMoon.pal"
BIN PAL_bkFog		"./res/back/bkFog.pal"
BIN PAL_bkRed		"./res/back/bkRed.pal"

BIN PAL_bk0		"./res/back/bk0.pal"
BIN PAL_bkBlack		"./res/back/bkBlack.pal"


; Alternate for waterway (Green background)

BIN PAL_RiverAlt "./res/Stage/PrtRiver_alt_vert.pal"
; Tile Attributes
BIN PXA_Almond	"./res/Stage/Almond.pxa"
BIN PXA_Barr	"./res/Stage/Barr.pxa"
BIN PXA_Cave	"./res/Stage/Cave.pxa"
BIN PXA_Cent	"./res/Stage/Cent.pxa"
BIN PXA_EggIn	"./res/Stage/EggIn.pxa"
BIN PXA_EggX	"./res/Stage/EggX.pxa"
BIN PXA_Fall	"./res/Stage/Fall.pxa"
BIN PXA_Gard	"./res/Stage/Gard.pxa"
BIN PXA_Jail	"./res/Stage/Jail.pxa"
BIN PXA_Labo	"./res/Stage/Labo.pxa"
BIN PXA_Oside	"./res/Stage/Oside.pxa"
BIN PXA_Pens	"./res/Stage/Pens.pxa"
BIN PXA_River	"./res/Stage/River.pxa"
BIN PXA_Sand	"./res/Stage/Sand.pxa"
BIN PXA_Store	"./res/Stage/Store.pxa"
BIN PXA_Weed	"./res/Stage/Weed.pxa"

; Optimized Tileset Stages
BIN PXA_Mimi		"./res/Stage/Mimi/Mimi.pxa"
BIN PXM_Barr		"./res/Stage/Mimi/Barr.cpxm"
BIN PXM_Cemet		"./res/Stage/Mimi/Cemet.cpxm"
BIN PXM_e_Ceme		"./res/Stage/Mimi/e_Ceme.cpxm"
BIN PXM_MiBox		"./res/Stage/Mimi/MiBox.cpxm"
BIN PXM_Mimi		"./res/Stage/Mimi/Mimi.cpxm"
BIN PXM_Plant		"./res/Stage/Mimi/Plant.cpxm"
BIN PXM_Pool		"./res/Stage/Mimi/Pool.cpxm"

BIN PXA_Eggs		"./res/Stage/Eggs/Eggs.pxa"
BIN PXM_Eggs		"./res/Stage/Eggs/Eggs.cpxm"

BIN PXA_Eggs2		"./res/Stage/Eggs/Eggs2.pxa"
BIN PXM_Eggs2		"./res/Stage/Eggs/Eggs2.cpxm"

BIN PXA_Maze		"./res/Stage/Maze/Maze.pxa"
BIN PXM_e_Maze		"./res/Stage/Maze/e_Maze.cpxm"
BIN PXM_MazeB		"./res/Stage/Maze/MazeB.cpxm"
BIN PXM_MazeD		"./res/Stage/Maze/MazeD.cpxm"
BIN PXM_MazeH		"./res/Stage/Maze/MazeH.cpxm"
BIN PXM_MazeI		"./res/Stage/Maze/MazeI.cpxm"
BIN PXM_MazeO		"./res/Stage/Maze/MazeO.cpxm"
BIN PXM_MazeS		"./res/Stage/Maze/MazeS.cpxm"
BIN PXM_MazeW		"./res/Stage/Maze/MazeW.cpxm"

BIN PXA_MazeM		"./res/Stage/Maze/MazeM.pxa"
BIN PXM_MazeM		"./res/Stage/Maze/MazeM.cpxm"

BIN PXA_Blcny		"./res/Stage/White/Blcny.pxa"
BIN PXM_e_Blcn		"./res/Stage/White/e_Blcn.cpxm"
BIN PXM_Blcny1		"./res/Stage/White/Blcny1.cpxm"
BIN PXM_Blcny2		"./res/Stage/White/Blcny2.cpxm"

BIN PXA_Kings		"./res/Stage/White/Kings.pxa"
BIN PXM_Kings		"./res/Stage/White/Kings.cpxm"
BIN PXM_Ring1		"./res/Stage/White/Ring1.cpxm"
BIN PXM_Ostep		"./res/Stage/White/Ostep.cpxm"

BIN PXA_Ring2		"./res/Stage/White/Ring2.pxa"
BIN PXM_Ring2		"./res/Stage/White/Ring2.cpxm"

BIN PXA_Ring3		"./res/Stage/White/Ring3.pxa"
BIN PXM_Ring3		"./res/Stage/White/Ring3.cpxm"

BIN PXA_Hell		"./res/Stage/Hell/Hell.pxa"
BIN PXM_Hell1		"./res/Stage/Hell/Hell1.cpxm"
BIN PXM_Hell2		"./res/Stage/Hell/Hell2.cpxm"
BIN PXM_Hell3		"./res/Stage/Hell/Hell3.cpxm"
BIN PXM_Hell4		"./res/Stage/Hell/Hell4.cpxm"
BIN PXM_Hell42		"./res/Stage/Hell/Hell42.cpxm"
BIN PXM_Ballo1		"./res/Stage/Hell/Ballo1.cpxm"
BIN PXM_Ballo2		"./res/Stage/Hell/Ballo2.cpxm"

BIN PXA_Statue		"./res/Stage/Hell/Statue.pxa"
BIN PXM_Statue		"./res/Stage/Hell/Statue.cpxm"

; Stages (PXM)
BIN PXM_Almond	"./res/Stage/Almond.cpxm"
BIN PXM_Cave	"./res/Stage/Cave.cpxm"


; --- Auto-split: Switching to Bank 9 ---
BIN PXM_Cent	"./res/Stage/Cent.cpxm"
BIN PXM_CentW	"./res/Stage/CentW.cpxm"
BIN PXM_Chako	"./res/Stage/Chako.cpxm"
BIN PXM_Clock	"./res/Stage/Clock.cpxm"
BIN PXM_Comu	"./res/Stage/Comu.cpxm"
BIN PXM_Cthu	"./res/Stage/Cthu.cpxm"
BIN PXM_Cthu2	"./res/Stage/Cthu2.cpxm"
BIN PXM_Curly	"./res/Stage/Curly.cpxm"
BIN PXM_CurlyS	"./res/Stage/CurlyS.cpxm"
BIN PXM_Dark	"./res/Stage/Dark.cpxm"
BIN PXM_Drain	"./res/Stage/Drain.cpxm"
BIN PXM_e_Jenk	"./res/Stage/e_Jenk.cpxm"
BIN PXM_e_Labo	"./res/Stage/e_Labo.cpxm"
BIN PXM_e_Malc	"./res/Stage/e_Malc.cpxm"
BIN PXM_e_Sky	"./res/Stage/e_Sky.cpxm"
BIN PXM_EgEnd1	"./res/Stage/EgEnd1.cpxm"
BIN PXM_EgEnd2	"./res/Stage/EgEnd2.cpxm"
BIN PXM_Egg1	"./res/Stage/Egg1.cpxm"
BIN PXM_Egg6	"./res/Stage/Egg6.cpxm"
BIN PXM_EggR	"./res/Stage/EggR.cpxm"
BIN PXM_EggR2	"./res/Stage/EggR2.cpxm"
BIN PXM_EggX	"./res/Stage/EggX.cpxm"
BIN PXM_EggX2	"./res/Stage/EggX2.cpxm"
BIN PXM_Fall	"./res/Stage/Fall.cpxm"
BIN PXM_Frog	"./res/Stage/Frog.cpxm"
BIN PXM_Gard	"./res/Stage/Gard.cpxm"
BIN PXM_Itoh	"./res/Stage/Itoh.cpxm"
BIN PXM_Island	"./res/Stage/Island.cpxm"
BIN PXM_Jail1	"./res/Stage/Jail1.cpxm"
BIN PXM_Jail2	"./res/Stage/Jail2.cpxm"
BIN PXM_Jenka1	"./res/Stage/Jenka1.cpxm"
BIN PXM_Jenka2	"./res/Stage/Jenka2.cpxm"
BIN PXM_Little	"./res/Stage/Little.cpxm"
BIN PXM_Lounge	"./res/Stage/Lounge.cpxm"
BIN PXM_Malco	"./res/Stage/Malco.cpxm"
BIN PXM_Mapi	"./res/Stage/Mapi.cpxm"
BIN PXM_MazeA	"./res/Stage/MazeA.cpxm"
BIN PXM_Momo	"./res/Stage/Momo.cpxm"
BIN PXM_Oside	"./res/Stage/Oside.cpxm"
BIN PXM_Pens1	"./res/Stage/Pens1.cpxm"
BIN PXM_Pens2	"./res/Stage/Pens2.cpxm"
BIN PXM_Pixel	"./res/Stage/Pixel.cpxm"
BIN PXM_Pole	"./res/Stage/Pole.cpxm"
BIN PXM_Prefa1	"./res/Stage/Prefa1.cpxm"
BIN PXM_Prefa2	"./res/Stage/Prefa2.cpxm"
BIN PXM_Priso1	"./res/Stage/Priso1.cpxm"

.ends
