

.include "data_config.asm"


; --- Part 12 ---
.section ".rodata_p12" superfree
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


; --- Auto-split: Switching to Bank 14 ---


; --- Auto-split: Switching to Bank 14 ---
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

.ends
