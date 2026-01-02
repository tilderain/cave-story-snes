#include "common.h"
#include "snes_regs_xc.h"

#include "audio.h"
#include "camera.h"
//#include "dma.h"
#include "effect.h"
#include "entity.h"
//#include "error.h"
//#include "hud.h"
#include "joy.h"
//#include "pause.h"
#include "player.h"
//#include "resources.h"
#include "sheet.h"
#include "stage.h"
//#include "string.h"
#include "system.h"
#include "tables.h"
//#include "tools.h"
//#include "tsc.h"
#include "vdp.h"
#include "weapon.h"
//#include "window.h"
//#include "xgm.h"
//
#include "gamemode.h"

//#include "gba.h"

// z80_request and z80_release are defined in common.h
uint8_t gamemode = 0;
uint8_t paused = 0;
uint8_t gameFrozen = 0;

// On PAL the screen height is 16 pixels more, so these can't be constants
//uint8_t SCREEN_HALF_H = 0;
uint8_t FPS = 0;

// Initializes or re-initializes the game after "try again"
//void game_reset(uint8_t load);
// Tracks how many times your game loop has finished
u16 frames_drawn = 0; 

// The calculated FPS to display
u16 current_fps = 0; 

// Tracks the last time (in VBlanks) we updated the FPS
u32 next_fps_check = 0; 

// PVSnesLib global variable that ticks every VBlank
void game_reset(uint8_t load) {

	stage_load(13);


	/*
	vdp_map_clear(VDP_PLAN_B);
	camera_init();
	tsc_init();
	hud_create();
	// Default sprite sheets
	sheets_load_stage(255, TRUE, TRUE);

	gameFrozen = FALSE;
	if(load >= 4) {
		system_load_levelselect(load - 4);
	} else {
		system_load(sram_file);
	}
	const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
	if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
	
	SHEET_LOAD(&SPR_Bonk, 1, 1, 1, 1, 0,0);
	SHEET_LOAD(&SPR_QMark, 1, 1, TILE_QMARKINDEX, 1, 0,0);
	// Load up the main palettes
	//vdp_colors_next(0, PAL_Main.data, 16);
	//vdp_colors_next(16, PAL_Sym.data, 16);
	//vdp_colors(0, PAL_FadeOut, 64);*/
}
u8 tscState = 0;
u8 pal_mode = 0;
// joytype is defined in joy.c, declared in joy.h
   		u8 stage_no = 14;

void game_main(uint8_t load) {
	camera_init();

	player_init();

	gamemode = GM_GAME;
	
	// Enable NMI (VBlank) interrupts so vdp_vsync() works
	// Bit 7 (0x80) enables NMI interrupts
	REG_NMITIMEN = 0x80;

	//vdp_colors(0, PAL_FadeOut, 64);
	//vdp_color(15, 0x000);
	// This is the SGDK font with a blue background for the message window
	if(cfg_language != LANG_JA) {
        disable_ints;
        z80_request();
		//vdp_font_load(TS_MsgFont.tiles);
        z80_release();
        enable_ints;
	}
	//effects_init();
	game_reset(load);
	vdp_set_window(0, 0);
	// Load game doesn't run a script that fades in and shows the HUD, so do it manually
	if(load) {
		//hud_show();
		stage_setup_palettes();
		//vdp_fade(PAL_FadeOut, NULL, 4, TRUE);
	}
	paused = FALSE;
	player.x = (int32_t)((10LL*16LL) << CSF);
	player.y = (int32_t)((8LL*16LL) << CSF);
	player.x_next = player.x;
	player.y_next = player.y;
	while(TRUE) {
		//PF_BGCOLOR(0x000);

		if(paused) {
            //PF_BGCOLOR(0x0E0);
			//paused = update_pause();
		} else {
			// Pressing start opens the item menu (unless a script is running)
			if(joy_pressed(btn[cfg_btn_pause]) && !tscState) {
				// This unloads the stage's script and loads the "ArmsItem" script in its place
				//tsc_load_stage(255);
				//draw_itemmenu(TRUE);
					player.x = (int32_t)((10LL*16LL) << CSF);
					player.y = (int32_t)((8LL*16LL) << CSF);
					player.x_next = player.x;
					player.y_next = player.y;
				//paused = TRUE;
			} else if(joy_pressed(btn[cfg_btn_map]) && joytype == JOY_TYPE_PAD6 
					&& !tscState && (playerEquipment & EQUIP_MAPSYSTEM)) {
				// Shorthand to open map system
                disable_ints;
                z80_request();
				vdp_set_display(FALSE);
				if(stageBackgroundType == 4) {
					// Hide water
					static const uint32_t black[8] = {
						0x11111111,0x11111111,0x11111111,0x11111111,
						0x11111111,0x11111111,0x11111111,0x11111111
					};
					vdp_tiles_load_from_rom(black, TILE_FACEINDEX, 1);
					vdp_map_fill_rect(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,TILE_FACEINDEX), 0, 0, 40, 30, 0);
				} else {
					vdp_map_clear(VDP_PLAN_W);
				}
				vdp_set_window(0, pal_mode ? 30 : 28);
				vdp_set_display(TRUE);
                z80_release();
                enable_ints;

				paused = TRUE; // This will stop the counter in Hell
				//do_map();
				paused = FALSE;
				vdp_set_display(FALSE);
				//hud_force_redraw();

                disable_ints;
                z80_request();
				sheets_load_stage(stageID, TRUE, FALSE);
                z80_release();
                enable_ints;

				player_draw();
				//entities_draw();
				//hud_show();
				vdp_sprites_update();
				vdp_set_window(0, 0);
				vdp_set_display(TRUE);
			} else {
				// HUD on top
                //PF_BGCOLOR(0x00E);
				//hud_update();
				// Boss health, camera
                //PF_BGCOLOR(0x0EE);
				if(!gameFrozen) {
					//if(showingBossHealth) tsc_update_boss_health();
					camera_update();
					//iprintf("\x1b[1;1HX:%08llX Y:%08llX\nSX:%04d SY:%04d", 	(unsigned int32_t)camera.x, (unsigned int32_t)camera.y, (int)camera.x_shifted, (int)camera.y_shifted);
				}
				// Run the next set of commands in a script if it is running
                //PF_BGCOLOR(0x0E0);
				//uint8_t rtn = tsc_update();
				u8 rtn = 0;
				// Nonzero return values exit the game, or switch to the ending sequence
				if(rtn > 0) {
					if(rtn == 1) { // Return to title screen
						SYS_hardReset();
					} else if(rtn == 2) {
						//vdp_colors(0, PAL_FadeOut, 64);
						vdp_color(15, 0x000);
						stageBackground = 255; // Force background redraw
						game_reset(TRUE); // Reload save
						//hud_show();
						playerIFrames = 0;
						stage_setup_palettes();
						vdp_fade(NULL, NULL, 4, TRUE);
						continue;
					} else if(rtn == 3) {
						//vdp_colors(0, PAL_FadeOut, 64);
						vdp_color(15, 0x000);
						game_reset(FALSE); // Start from beginning
						continue;
					} else { // End credits
						break;
					}
				}
                //PF_BGCOLOR(0xEE0);
				//window_update();
				// Handle controller locking
				uint16_t lockstate = joystate, oldlockstate = oldstate;
				if(controlsLocked) joystate = oldstate = 0;
				// Don't update this stuff if a script is using <PRI
                //PF_BGCOLOR(0xE00);
				//effects_update();
                //PF_BGCOLOR(0xE0E);
				if(!gameFrozen) {
					//GBATODO
					player_update();
				//	entities_update(TRUE);
				} else {
					player_draw();
				//	entities_draw();
				}
				// Restore controller locking if it was locked
				joystate = lockstate;
				oldstate = oldlockstate;

				//stage_draw_background();
			}
		}
		//PF_BGCOLOR(0xEEE);
		//system_update();
		//ready = TRUE;
		//PF_BGCOLOR(0x000);
		spcProcess();
		vdp_vsync();

		// Map buffer is updated by stage_draw_screen() when needed, no need to copy every frame
		//dmaCopyVram(map_buffer_bg1, 0x6000, 4096);
        //dmaCopyVram(map_buffer_bg2, 0x7000, 2048);
		oamUpdate(); 
    	// 3. Increment our counter because we successfully finished one frame
    	frames_drawn++;

    	// 4. Check if 60 VBlanks (approx 1 second for NTSC) have passed
    	// Use 50 instead of 60 if you are compiling for PAL
    	uint16_t snes_vblank_count = vdp_get_vblank();
    	if (snes_vblank_count >= next_fps_check) {
    	    // Save the result
    	    current_fps = frames_drawn;
	
    	    // Reset the counter
    	    frames_drawn = 0;
	
    	    // Set the next check time to 1 second (60 ticks) in the future
    	    next_fps_check = snes_vblank_count + 60;
	
    	    // Optional: Update text on screen (requires consoleInitText to be set up)
    	    iprintf("FPS: %d \n", current_fps);
    	}

		bgSetEnable(0);
		stage_update();
		joy_update();
		//PF_BGCOLOR(0x00E);

        u8 pad0 = padsCurrent(0);
		//if(pad0 & KEY_L) {stage_no--; stage_load(stage_no);}
        //if(pad0 & KEY_R) {stage_no++; stage_load(stage_no);}
		//aftervsync();
		
	}
}
