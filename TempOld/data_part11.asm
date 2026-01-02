.include "data_config.asm"

.ROMBANKSIZE $10000             ; Every ROM bank is 32 KBytes in size
.ROMBANKS 24                     ; 2 Mbits - Tell WLA we want to use 4 ROM Banks


; --- Part 11 ---
.section ".rodata_p11" superfree
BIN		PAT_Ill17	"./res/credits/ill17.pat"
BIN		MAP_Ill17	"./res/credits/ill17.map"
BIN		PAT_Ill18	"./res/credits/ill18.pat"
BIN		MAP_Ill18	"./res/credits/ill18.map"

/* Level Select data */
BIN LS_00		"./res/save/00_firstcave.sram-trim"
BIN LS_01		"./res/save/01_mimigavillage.sram-trim"
BIN LS_02		"./res/save/02_eggcorridor.sram-trim"
BIN LS_03		"./res/save/03_grasstown.sram-trim"
BIN LS_04		"./res/save/04_malco.sram-trim"
BIN LS_05		"./res/save/05_balfrog.sram-trim"
BIN LS_06		"./res/save/06_sandzone.sram-trim"
BIN LS_07		"./res/save/07_omega.sram-trim"
BIN LS_08		"./res/save/08_storehouse.sram-trim"
BIN LS_09		"./res/save/09_labyrinth.sram-trim"
BIN LS_10		"./res/save/10_monsterx.sram-trim"
BIN LS_11		"./res/save/11_labyrinthm.sram-trim"
BIN LS_12		"./res/save/12_core.sram-trim"
BIN LS_13		"./res/save/13_waterway.sram-trim"
BIN LS_14		"./res/save/14_eggcorridor2.sram-trim"
BIN LS_15		"./res/save/15_outerwall.sram-trim"
BIN LS_16		"./res/save/16_plantation.sram-trim"
BIN LS_17		"./res/save/17_lastcave.sram-trim"
BIN LS_18		"./res/save/18_lastcave2.sram-trim"
BIN LS_19		"./res/save/19_balcony.sram-trim"
BIN LS_20		"./res/save/20_sacredground.sram"
BIN LS_21		"./res/save/21_sealchamber.sram"


/* Start of localizable data */
.db "LANGDAT", 0


LANGUAGE:
.db "EN\0\0"


BMP_ASCII:
.db 0

BMP_KANJI:
.db 0

STAGE_NAMES:
.db 0

CREDITS_STR:
.db 0

CONFIG_STR:
.db 0

/* Pointer Tables */

TSC_GLOB:
.dl TSC_ArmsItem
.dl TSC_Head
.dl TSC_StageSelect
.dl TSC_Credits

TSC_STAGE:
.dl 0
.dl TSC_Pens1
.dl TSC_Eggs
.dl TSC_EggX
.dl TSC_Egg6
.dl TSC_EggR
.dl TSC_Weed
.dl TSC_Santa
.dl TSC_Chako
.dl TSC_MazeI
.dl TSC_Sand
.dl TSC_Mimi
.dl TSC_Cave
.dl TSC_Start
.dl TSC_Barr
.dl TSC_Pool
.dl TSC_Cemet
.dl TSC_Plant
.dl TSC_Shelt
.dl TSC_Comu
.dl TSC_MiBox
.dl TSC_EgEnd1
.dl TSC_Cthu
.dl TSC_Egg1
.dl TSC_Pens2
.dl TSC_Malco
.dl TSC_WeedS
.dl TSC_WeedD
.dl TSC_Frog
.dl TSC_Curly
.dl TSC_WeedB
.dl TSC_Stream
.dl TSC_CurlyS
.dl TSC_Jenka1
.dl TSC_Dark
.dl TSC_Gard
.dl TSC_Jenka2
.dl TSC_SandE
.dl TSC_MazeH
.dl TSC_MazeW
.dl TSC_MazeO
.dl TSC_MazeD
.dl TSC_MazeA
.dl TSC_MazeB
.dl TSC_MazeS
.dl TSC_MazeM
.dl TSC_Drain
.dl TSC_Almond
.dl TSC_River
.dl TSC_Eggs2
.dl TSC_Cthu2
.dl TSC_EggR2
.dl TSC_EggX2
.dl TSC_Oside
.dl TSC_EgEnd2
.dl TSC_Itoh
.dl TSC_Cent
.dl TSC_Jail1
.dl TSC_Momo
.dl TSC_Lounge
.dl TSC_CentW
.dl TSC_Jail2
.dl TSC_Blcny1
.dl TSC_Priso1
.dl TSC_Ring1
.dl TSC_Ring2
.dl TSC_Prefa1
.dl TSC_Priso2
.dl TSC_Ring3
.dl TSC_Little
.dl TSC_Blcny2
.dl TSC_Fall
.dl TSC_Kings
.dl TSC_Pixel
.dl TSC_e_Maze
.dl TSC_e_Jenk
.dl TSC_e_Malc
.dl TSC_e_Ceme
.dl TSC_e_Sky
.dl TSC_Prefa2
.dl TSC_Hell1
.dl TSC_Hell2
.dl TSC_Hell3
.dl TSC_Mapi
.dl TSC_Hell4
.dl TSC_Hell42
.dl TSC_Statue
.dl TSC_Ballo1
.dl TSC_Ostep
.dl TSC_e_Labo
.dl TSC_Pole
.dl TSC_Island
.dl TSC_Ballo2
.dl TSC_e_Blcn
.dl TSC_Clock

; Scripts (TSC) - English
; Global
BIN TSC_ArmsItem	"./res/tsc/en/ArmsItem.tsb"
BIN TSC_Head		"./res/tsc/en/Head.tsb"
BIN TSC_StageSelect	"./res/tsc/en/StageSelect.tsb"
BIN TSC_Credits		"./res/tsc/en/Stage/0.tsb"
; Stage Specific


; --- Auto-split: Switching to Bank 14 ---

.ends
