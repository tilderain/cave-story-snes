

.include "data_config.asm"


; --- Part 15 ---
.section ".rodata_p15" superfree
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


; --- Auto-split: Switching to Bank 17 ---


; --- Auto-split: Switching to Bank 17 ---
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
