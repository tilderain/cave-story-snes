.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 7 ---
.section ".rodata_p7" superfree
BIN PXM_Priso2	"./res/Stage/Priso2.cpxm"
BIN PXM_River	"./res/Stage/River.cpxm"
BIN PXM_Sand	"./res/Stage/Sand.cpxm"
BIN PXM_SandE	"./res/Stage/SandE.cpxm"
BIN PXM_Santa	"./res/Stage/Santa.cpxm"
BIN PXM_Shelt	"./res/Stage/Shelt.cpxm"
BIN PXM_Start	"./res/Stage/Start.cpxm"
BIN PXM_Stream	"./res/Stage/Stream.cpxm"
BIN PXM_Weed	"./res/Stage/Weed.cpxm"
BIN PXM_WeedB	"./res/Stage/WeedB.cpxm"
BIN PXM_WeedD	"./res/Stage/WeedD.cpxm"
BIN PXM_WeedS	"./res/Stage/WeedS.cpxm"

; Entities (PXE)
BIN PXE_0       "./res/Stage/0.pxe"
BIN PXE_Almond	"./res/Stage/Almond.pxe"
BIN PXE_Ballo1	"./res/Stage/Ballo1.pxe"
BIN PXE_Ballo2	"./res/Stage/Ballo2.pxe"
BIN PXE_Barr	"./res/Stage/Barr.pxe"
BIN PXE_Blcny1	"./res/Stage/Blcny1.pxe"
BIN PXE_Blcny2	"./res/Stage/Blcny2.pxe"
BIN PXE_Cave	"./res/Stage/Cave.pxe"
BIN PXE_Cemet	"./res/Stage/Cemet.pxe"
BIN PXE_Cent	"./res/Stage/Cent.pxe"
BIN PXE_CentW	"./res/Stage/CentW.pxe"
BIN PXE_Chako	"./res/Stage/Chako.pxe"
BIN PXE_Clock	"./res/Stage/Clock.pxe"
BIN PXE_Comu	"./res/Stage/Comu.pxe"
BIN PXE_Cthu	"./res/Stage/Cthu.pxe"
BIN PXE_Cthu2	"./res/Stage/Cthu2.pxe"
BIN PXE_Curly	"./res/Stage/Curly.pxe"
BIN PXE_CurlyS	"./res/Stage/CurlyS.pxe"
BIN PXE_Dark	"./res/Stage/Dark.pxe"
BIN PXE_Drain	"./res/Stage/Drain.pxe"
BIN PXE_e_Blcn	"./res/Stage/e_Blcn.pxe"
BIN PXE_e_Ceme	"./res/Stage/e_Ceme.pxe"
BIN PXE_e_Jenk	"./res/Stage/e_Jenk.pxe"
BIN PXE_e_Labo	"./res/Stage/e_Labo.pxe"
BIN PXE_e_Malc	"./res/Stage/e_Malc.pxe"
BIN PXE_e_Maze	"./res/Stage/e_Maze.pxe"
BIN PXE_e_Sky	"./res/Stage/e_Sky.pxe"
BIN PXE_EgEnd1	"./res/Stage/EgEnd1.pxe"
BIN PXE_EgEnd2	"./res/Stage/EgEnd2.pxe"
BIN PXE_Egg1	"./res/Stage/Egg1.pxe"
BIN PXE_Egg6	"./res/Stage/Egg6.pxe"
BIN PXE_EggR	"./res/Stage/EggR.pxe"
BIN PXE_EggR2	"./res/Stage/EggR2.pxe"
BIN PXE_Eggs	"./res/Stage/Eggs.pxe"
BIN PXE_Eggs2	"./res/Stage/Eggs2.pxe"
BIN PXE_EggX	"./res/Stage/EggX.pxe"
BIN PXE_EggX2	"./res/Stage/EggX2.pxe"
BIN PXE_Fall	"./res/Stage/Fall.pxe"
BIN PXE_Frog	"./res/Stage/Frog.pxe"
BIN PXE_Gard	"./res/Stage/Gard.pxe"
BIN PXE_Hell1	"./res/Stage/Hell1.pxe"
BIN PXE_Hell2	"./res/Stage/Hell2.pxe"
BIN PXE_Hell3	"./res/Stage/Hell3.pxe"
BIN PXE_Hell4	"./res/Stage/Hell4.pxe"
BIN PXE_Hell42	"./res/Stage/Hell42.pxe"
BIN PXE_Island	"./res/Stage/Island.pxe"
BIN PXE_Itoh	"./res/Stage/Itoh.pxe"
BIN PXE_Jail1	"./res/Stage/Jail1.pxe"
BIN PXE_Jail2	"./res/Stage/Jail2.pxe"
BIN PXE_Jenka1	"./res/Stage/Jenka1.pxe"
BIN PXE_Jenka2	"./res/Stage/Jenka2.pxe"
BIN PXE_Kings	"./res/Stage/Kings.pxe"
BIN PXE_Little	"./res/Stage/Little.pxe"
BIN PXE_Lounge	"./res/Stage/Lounge.pxe"
BIN PXE_Malco	"./res/Stage/Malco.pxe"
BIN PXE_Mapi	"./res/Stage/Mapi.pxe"
BIN PXE_MazeA	"./res/Stage/MazeA.pxe"
BIN PXE_MazeB	"./res/Stage/MazeB.pxe"
BIN PXE_MazeD	"./res/Stage/MazeD.pxe"
BIN PXE_MazeH	"./res/Stage/MazeH.pxe"
BIN PXE_MazeI	"./res/Stage/MazeI.pxe"
BIN PXE_MazeM	"./res/Stage/MazeM.pxe"
BIN PXE_MazeO	"./res/Stage/MazeO.pxe"
BIN PXE_MazeS	"./res/Stage/MazeS.pxe"
BIN PXE_MazeW	"./res/Stage/MazeW.pxe"
BIN PXE_MiBox	"./res/Stage/MiBox.pxe"
BIN PXE_Mimi	"./res/Stage/Mimi.pxe"
BIN PXE_Momo	"./res/Stage/Momo.pxe"
BIN PXE_Oside	"./res/Stage/Oside.pxe"
BIN PXE_Ostep	"./res/Stage/Ostep.pxe"
BIN PXE_Pens1	"./res/Stage/Pens1.pxe"
BIN PXE_Pens2	"./res/Stage/Pens2.pxe"
BIN PXE_Pixel	"./res/Stage/Pixel.pxe"
BIN PXE_Plant	"./res/Stage/Plant.pxe"
BIN PXE_Pole	"./res/Stage/Pole.pxe"
BIN PXE_Pool	"./res/Stage/Pool.pxe"
BIN PXE_Prefa1	"./res/Stage/Prefa1.pxe"
BIN PXE_Prefa2	"./res/Stage/Prefa2.pxe"
BIN PXE_Priso1	"./res/Stage/Priso1.pxe"
BIN PXE_Priso2	"./res/Stage/Priso2.pxe"
BIN PXE_Ring1	"./res/Stage/Ring1.pxe"
BIN PXE_Ring2	"./res/Stage/Ring2.pxe"
BIN PXE_Ring3	"./res/Stage/Ring3.pxe"
BIN PXE_River	"./res/Stage/River.pxe"
BIN PXE_Sand	"./res/Stage/Sand.pxe"
BIN PXE_SandE	"./res/Stage/SandE.pxe"
BIN PXE_Santa	"./res/Stage/Santa.pxe"
BIN PXE_Shelt	"./res/Stage/Shelt.pxe"
BIN PXE_Start	"./res/Stage/Start.pxe"
BIN PXE_Statue	"./res/Stage/Statue.pxe"
BIN PXE_Stream	"./res/Stage/Stream.pxe"
BIN PXE_Weed	"./res/Stage/Weed.pxe"
BIN PXE_WeedB	"./res/Stage/WeedB.pxe"
BIN PXE_WeedD	"./res/Stage/WeedD.pxe"
BIN PXE_WeedS	"./res/Stage/WeedS.pxe"

; NPC Table


; --- Auto-split: Switching to Bank 10 ---
BIN     NPC_TABLE		"./res/npc.tbl"

/* Background tilemaps */
; Moon
;BIN		PAT_MoonTop		"./res/back/bkMoonTop.pat"
;BIN		MAP_MoonTop		"./res/back/bkMoonTop.map"
BIN		PAT_MoonBtm		"./res/back/bkMoonBottom.pat"
BIN		MAP_MoonBtm		"./res/back/bkMoonBottom.map"
; Fog
;BIN		PAT_FogTop		"./res/back/bkFogTop.pat"
;BIN		MAP_FogTop		"./res/back/bkFogTop.map"
;BIN		PAT_FogBtm		"./res/back/bkFogBottom.pat"
;BIN		MAP_FogBtm		"./res/back/bkFogBottom.map"
; Sound Test
;BIN		PAT_SndTest	    "./res/back/soundtest.pat"
;BIN		MAP_SndTest	    "./res/back/soundtest.map"






/* 0x380000 */
;    .align 0x80000

; Japanese Font - 1bpp bitmap data
;BIN     BMP_Ascii		"./res/ja_ascii.dat"
;BIN     BMP_Kanji		"./res/ja_kanji.dat"

; Japanese stage names and credits text
;BIN     JStageName		"./res/ja_stagename.dat"
;BIN     JCreditStr		"./res/ja_credits.dat"
;BIN     JConfigText		"./res/ja_config.dat"

/* Credits Illustrations */

.ends
