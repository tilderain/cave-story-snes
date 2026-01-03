#include "common.h"

//#include "bank_data.h"
#include "camera.h"
//#include "dma.h"
//#include "effect.h"
#include "entity.h"
//#include "error.h"
//#include "hud.h"
//#include "joy.h"
//#include "memory.h"
#include "npc.h"
//#include "player.h"
//#include "resources.h"
//#include "sheet.h"
//#include "string.h"
#include "system.h"
#include "tables.h"
//#include "tools.h"
//#include "tsc.h"
#include "vdp.h"
//#include "xgm.h"

#include "stage.h"

//#include "gba.h"
//#include "gba_video.h"

#include "bank_data.h"

// Could fit under the Oside map (192 tile gap)
#define TILE_MOONINDEX (TILE_TSINDEX + 32*8)
// Another tile gap, fits under both Almond and Cave
#define TILE_WATERINDEX (TILE_TSINDEX + 384)

// Index of background in db/back.c and the effect type
uint8_t stageBackground = 0xFF;

int16_t backScrollTable[32];
//uint8_t *stageBlocks = NULL;
uint16_t stageTable[240];
const uint8_t *stagePXA = NULL;

typedef struct {
	uint8_t index;
	uint8_t dir;
} Current;
Current currents[4];
uint8_t currentsCount = 0;
uint8_t currentsTimer = 0;

uint16_t stageWidth, stageHeight = 0;
// A multiplication lookup table for each row of stageBlocks
// Removes all mulu.w and __mulsi3 instructions in entity stage collision
// Copy of level layout data loaded into RAM
// This takes up extra space, but there are times where scripts make modifications to the
// level layout (allowing player to reach some areas) so it is necessary to do this
 uint8_t stagePXM[8];
 uint8_t stageBlocks[17924];
// Which tileset (db/tileset.c) is used by the current stage
 uint8_t stageTileset = 0;
// Prepares to draw off-screen tiles when stage_update() is later called
// Camera calls this each time it scrolls past 1 block length (16 pixels)
 int8_t morphingRow, morphingColumn = 0;

 uint16_t backScrollTimer = 0;
 uint8_t stageBackgroundType = 0;

// Function declarations moved to stage.h - removed static to match header
void stage_draw_background();
static void stage_draw_moonback();

//#include "gba.h"

uint16_t stageID = 0;


// Define your map size (e.g., 64x32 tiles)
#define MAP_WIDTH 32
#define MAP_HEIGHT 32

// Shadow buffers in WRAM (RAM)
// 64 * 32 tiles = 2048 entries * 2 bytes = 4096 bytes per layer
uint16_t map_buffer_bg1[64 * MAP_HEIGHT];
uint16_t map_buffer_bg2[MAP_WIDTH * MAP_HEIGHT];

// Helper to construct SNES tile entry
// Format: vhopppcc cccccccc
#define SNES_TILE_ENTRY(tile, palette, prio, hflip, vflip) \
    ((tile & 0x3FF) | ((palette & 7) << 10) | ((prio & 1) << 13) | ((hflip & 1) << 14) | ((vflip & 1) << 15))
#include "snes_regs_xc.h"
void test_draw_sequential() {

    // 1. Pre-calculate constant attributes to avoid shifting in the loop
    // Palette 0, High Priority (1 << 13) = 0x2000
    // We pre-calculate this once.
    const uint16_t tile_attributes = (1 << 10) | (1 << 13);
    
    // 2. Cache dimensions in local variables (faster access)
    uint16_t asset_w = background_info[stageBackground].width;
    uint16_t asset_h = background_info[stageBackground].height;
    
    // 3. Use a pointer for the destination (sequential writes are faster)
    uint16_t *dest = map_buffer_bg2;

    // 4. Track asset Y position manually to avoid modulo
    uint16_t local_y = 0;
    uint16_t row_start_index = 0; // This will track (local_y * asset_w)
	uint16_t screen_y;
    for ( screen_y = 0; screen_y < 32; screen_y++) {
        
        // Track asset X position manually to avoid modulo
        uint16_t local_x = 0;
        uint16_t screen_x;
        for ( screen_x = 0; screen_x < 32; screen_x++) {
            
            // 5. Calculate Tile Index using addition instead of multiplication
            uint16_t tile_index = row_start_index + local_x;

            // 6. Write to buffer using pointer increment (auto-increments dest)
            *dest++ = (tile_index & 0x3FF) | tile_attributes;

            // 7. Manual Modulo for X: Increment and reset if it hits asset width
            local_x++;
            if (local_x == asset_w) local_x = 0;
        }

        // 8. Manual Modulo for Y: Increment and reset if it hits asset height
        local_y++;
        if (local_y == asset_h) {
            local_y = 0;
            row_start_index = 0;
        } else {
            // Instead of multiplying, just add the width for the next row
            row_start_index += asset_w;
        }
    }
}

void loadStageBackground(int index) {
    // Note: I'm using 0x2000 (8KB) for tileset size as (64*8) is usually too small 
    // for a full background. Adjust 0x2000 to match your actual .pic file size.
    uint16_t tileSize = 0x2000; 
    uint16_t palSize = 32; // 16 colors * 2 bytes
    uint16_t vramAddr = 0x8000;
	
    switch (index) {
        case 0: // BG_bk0
            bgInitTileSet(1, BG_bk0, PAL_bk0, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 1: // BG_Black
            bgInitTileSet(1, BG_Black, PAL_bkBlack, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 2: // BG_Blue (Village/Grasstown)
        case 3: // BG_Blue (Main Artery)
        case 14: // BG_Blue (Arthur's House)
            bgInitTileSet(1, BG_Blue, PAL_bkBlue, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 4: // Balcony (Fog) - No Tileset
            //dmaCopyPalette(PAL_bkFog, 1 * 16, palSize); // Load palette only
            break;

        case 5: // Sand Zone Storehouse
            bgInitTileSet(1, BG_Gard, PAL_bkGard, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 6: // Boulder Chamber
            bgInitTileSet(1, BG_Gray, PAL_bkGray, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 7: // Outer Wall (Moon) - No Tileset
            //dmaCopyPalette(PAL_bkMoon, 1 * 16, palSize); // Load palette only
            break;

        case 8: // Labyrinth W
            bgInitTileSet(1, BG_Maze, PAL_bkMaze, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 9: // Labyrinth I/O
            bgInitTileSet(1, BG_Maze2, PAL_bkMaze, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 10: // Labyrinth M
        case 12: // Hell
            bgInitTileSet(1, BG_Red, PAL_bkRed, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 11: // Almond (Water) - No Tileset
            //dmaCopyPalette(PAL_bkWater, 1 * 16, palSize); // Load palette only
            break;

        case 13: // Egg Corridor
        case 16: // Sand Zone
            bgInitTileSet(1, BG_Green, PAL_bkGreen, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        case 15: // Fall
            bgInitTileSet(1, BG_Fall, PAL_bkFall, 1, tileSize, palSize, BG_16COLORS, vramAddr);
            break;

        default:
            break;
    }
	bgSetEnable(0);
	bgSetEnable(1);
//setMode(BG_MODE1, 0);   // Set Mode 1
bgSetEnable(0);         // Explicitly enable BG0
setScreenOn();          // Turn on display
}
void stage_load(uint16_t id) {
	iprintf("Loading stage %d\n", id);
	vdp_set_display(FALSE);
	//oldstate = 65535;
	// Prevents an issue where a column of the previous map would get drawn over the new one
	//DMA_clearQueue();
	stageID = id;
	// Clear out or deactivate stuff from the old stage
	//effects_clear();
	//entities_clear();
	if(stageTable) {
		//free(stageTable);
		//stageTable = NULL;
	}
	vdp_sprites_clear();
	//water_entity = NULL;
	//bossEntity = NULL;

	// Load the tileset
	if(stageTileset != stage_info[id].tileset) {
		stageTileset = stage_info[id].tileset;
        disable_ints;
        //z80_request();
		stage_load_tileset();
        //z80_release();
        enable_ints;
	}
	// Load sprite sheets
    disable_ints;
    //z80_request();
	//sheets_load_stage(id, FALSE, TRUE);
	// Load backgrounds
	if(background_info[stage_info[id].background].type == 4 || stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		stageBackgroundType = background_info[stageBackground].type;
		vdp_set_backcolor(0); // Color index 0 for everything except fog
		if(stageBackgroundType == 0 || stageBackgroundType == 3) { // Tiled image
			vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
			if(background_info[stageBackground].tileset != 0)
			{

				vdp_tiles_load_from_rom((volatile const uint32_t *)(uintptr_t)background_info[stageBackground].tileset, TILE_BACKINDEX, 
						1024);
				loadStageBackground(stageBackground);
			}

			stage_draw_background();
		} else if(stageBackgroundType == 1) { // Moon
			vdp_set_scrollmode(HSCROLL_TILE, VSCROLL_PLANE);
			stage_draw_moonback();
		} else if(stageBackgroundType == 2) { // Solid Color
			vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
			vdp_map_clear(VDP_PLAN_B);
		} else if(stageBackgroundType == 4) { // Almond Water
			vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
			vdp_map_clear(VDP_PLAN_B);
            backScrollTable[0] = (SCREEN_HEIGHT >> 3) + 1;
			//	gbatodo
			vdp_tiles_load_from_rom((volatile const uint32_t *)BG_Water, TILE_WATERINDEX, 64);
		} else if(stageBackgroundType == 5) { // Fog
			vdp_set_scrollmode(HSCROLL_TILE, VSCROLL_PLANE);
			// Use background color from tileset
			vdp_set_backcolor(32);
			stage_draw_moonback();
		}
	}
    //z80_release();
    enable_ints;

	// Load stage PXM into RAM
	stage_load_blocks();
	// Move camera to player's new position
	//camera_set_position(player.x, player.y - (stageBackgroundType == 3 ? 8<<CSF : 0));
	//camera.target = &player;
	//camera.x_offset = 0;
	//camera.y_offset = 0;

    //disable_ints;
    //z80_request();
	//test_draw_sequential(); // Draw 64x32 foreground PXM area at camera's position
	stage_draw_screen();
    //z80_release();
    //enable_ints;

	/*stage_load_entities(); // Create entities defined in the stage's PXE
	// For rooms where the boss is always loaded
	if(stageID == STAGE_WATERWAY_BOSS) {
		bossEntity = entity_create(0, 0, 360 + BOSS_IRONHEAD, 0);
	}
	if(stageID == STAGE_BLACK_SPACE) {
		bossEntity = entity_create(0, 0, 360 + BOSS_UNDEADCORE, 0);
	}
	if(stageID == STAGE_HELL_B3 || stageID == STAGE_HELL_PASSAGEWAY_2) {
		bossEntity = entity_create(0, 0, 360 + BOSS_HEAVYPRESS, 0);
	}

    disable_ints;
    z80_request();
	DMA_flushQueue();
    z80_release();
    enable_ints;

	if((playerEquipment & EQUIP_CLOCK) || stageID == STAGE_HELL_B1) system_draw_counter();
	tsc_load_stage(id);
	vdp_set_display(TRUE);*/
	setPaletteColor(0, 0);


}

void stage_load_credits(uint8_t id) {
	stageID = id;
	
	entities_clear();
	vdp_sprites_clear();
	if(stageTable) {
		//free(stageTable);
		//stageTable = NULL;
	}

    disable_ints;
    z80_request();
	vdp_set_display(FALSE);

	stageTileset = stage_info[id].tileset;
	stage_load_tileset();
	sheets_load_stage(id, FALSE, TRUE);
	stage_load_blocks();
	stage_draw_screen_credits();
	stage_load_entities();
	DMA_flushQueue();
	tsc_load_stage(id);

	vdp_set_display(TRUE);
    z80_release();
    enable_ints;
}

#include "snes_regs_xc.h"

void stage_load_tileset() {
    uint32_t *buf = (uint32_t*) 0xFF0100;
    uint16_t numtile = tileset_info[stageTileset].size << 2;
    //for(uint16_t i = 0; i < numtile; i += 128) {
     //   uint16_t num = min(numtile - i, 128);
		//GBATODO
        //decompress_uftc(buf, tileset_info[stageTileset].pat, i, num);
        //vdp_tiles_load(tileset_info[stageTileset].pat, TILE_TSINDEX, tileset_info[stageTileset].size*32);
		iprintf("loading tileset %d\n", stageTileset);
		setScreenOff();
		bgInitTileSet(0, tileset_info[stageTileset].pat, tileset_info[stageTileset].palette, 0, (tileset_info[stageTileset].size*128), 16 * 2, BG_16COLORS, 0x2000);
		setScreenOn();

    //}
	// Inject the breakable block sprite into the tileset
	stagePXA = tileset_info[stageTileset].PXA;
	//iprintf("stage pxa: %04X\n", stagePXA);
		uint16_t i;
	for( i = 0; i < numtile >> 2; i++) {
	//	iprintf("i%d %04X ", i, stagePXA[i]);
	//	if(stagePXA[i] == 0x43) vdp_tiles_load_from_rom(TS_Break.tiles, TILE_TSINDEX + (i << 2), 4);
	}
	// Search for any "wind" tiles and note their index to animate later
	currentsCount = 0;

	for( i = 0; i < numtile >> 2; i++) {
		if(!(stagePXA[i] & 0x80)) continue;
		currents[currentsCount] = (Current) { .index = i, .dir = stagePXA[i] & 0x3 };
		if(++currentsCount == 4) break;
	}
}



void decompress_slz(const uint8_t *in, uint8_t *out) {
   // Retrieve uncompressed size from SLZ header
   uint16_t size = ((uint16_t)in[0] << 8) | in[1];
   in += 2;
   
   // Temporary buffer to hold the PXM header (8 bytes)
   // 0-2: Sig, 3: Ver, 4-5: W, 6-7: H
   uint8_t header[8];
   
   uint8_t num_tokens = 1;
   uint8_t tokens = 0;
   uint16_t out_idx = 0; // Total bytes decompressed so far

   while (size > 0) {
      // Need more tokens?
      num_tokens--;
      if (num_tokens == 0) {
         tokens = *in++;
         num_tokens = 8;
      }
      
      // Determine next byte(s) to output
      if (tokens & 0x80) {
         // Compressed string
         uint16_t dist = ((uint16_t)in[0] << 8) | in[1];
         uint8_t len = (dist & 0x0F) + 3;
         dist = dist >> 4;
         in += 2;
         size -= len;

         // Copy loop
         while(len--) {
            uint8_t byte;
            // Handle back-reference logic
            // If the reference goes back into the header, read from header[]
            // Otherwise read from the output buffer
            int16_t ref_pos = out_idx - dist - 3;
            
            if (ref_pos < 0) {
                // Should not happen in valid PXM usually, but safety first
                byte = 0; 
            } else if (ref_pos < 8) {
                // Reference falls inside the PXM header
                byte = header[ref_pos];
            } else {
                // Reference is inside the tile body
                byte = out[ref_pos - 8];
            }

            // Write logic
            if (out_idx < 8) {
                header[out_idx] = byte;
            } else {
                *out++ = byte;
            }
            out_idx++;
         }
      }
      else {
         // Uncompressed byte
         uint8_t byte = *in++;
         size--;

         // Write logic
         if (out_idx < 8) {
             header[out_idx] = byte;
         } else {
             *out++ = byte;
         }
         out_idx++;
      }
      
      // Check if we just finished reading the header (byte index 7)
      if (out_idx == 8) {
          stageWidth  = header[4] | (header[5] << 8);
          stageHeight = header[6] | (header[7] << 8);
          iprintf("Map Size: %d x %d\n", stageWidth, stageHeight);
      }

      tokens <<= 1;
   }
}


void stage_load_blocks() {
    iprintf("decompress");
    
    // Decompress directly into stageBlocks
    // The function now handles stripping the header and setting W/H
    decompress_slz(stage_info[stageID].PXM, stageBlocks);

    // Rebuild the Y-lookup table (stageTable)
    // stageBlocks is now pure tile data, so offset 0 is row 0
    uint16_t blockTotal = 0;
    uint16_t y;
    for( y = 0; y < stageHeight; y++) {
        // Safety bounds check
        if (y < 240) {
            stageTable[y] = blockTotal;
        }
        blockTotal += stageWidth;
    }
	iprintf("Map Size: %d x %d\n", stageWidth, stageHeight);
}

void stage_load_entities() {
	const uint8_t *PXE = stage_info[stageID].PXE;
	// PXE[4] is the number of entities to load. It's word length but never more than 255
	uint16_t i;
	for( i = 0; i < PXE[4]; i++) {
		uint16_t x, y, id, event, type, flags;
		// Like all of cave story's data files PXEs are little endian
		x     = PXE[8  + i * 12] + (PXE[9  + i * 12]<<8);
		y     = PXE[10 + i * 12] + (PXE[11 + i * 12]<<8);
		id    = PXE[12 + i * 12] + (PXE[13 + i * 12]<<8);
		event = PXE[14 + i * 12] + (PXE[15 + i * 12]<<8);
		type  = PXE[16 + i * 12] + (PXE[17 + i * 12]<<8);
		flags = PXE[18 + i * 12] + (PXE[19 + i * 12]<<8);
		// There are some unused entities that have all these values as 0, as well as
		// entities that should only exist when specific flags are on/off
		// Loading these would be a waste of memory, just skip them
		if(!id && !event && !type && !flags) continue;
		if((flags&NPC_DISABLEONFLAG) && system_get_flag(id)) continue;
		if((flags&NPC_ENABLEONFLAG) && !system_get_flag(id)) continue;
		// Special case to not load save points if SRAM is not found
		if(type == OBJ_SAVE_POINT && system_get_flag(FLAG_DISABLESAVE)) continue;
		// I'll probably need this code block again in the future.
		// When an NPC is assigned the improper number of sprites for their metasprite
		// loading it will crash BlastEm and possibly hardware too. This steps through
		// each entity as it is loaded so the problematic NPC can be found
	#if DEBUG
		if(joy_down(BUTTON_A)) {
			vdp_set_display(TRUE);
			vdp_color(0, 0x444);
			vdp_color(15, 0xEEE);
			char str[40];
			sprintf(str, "Debug Entity # %03hu", i);
			vdp_puts(VDP_PLAN_A, str, 2, 2);
			sprintf(str, "X:%04hu Y:%04hu I:%04hu", x, y, id);
			vdp_puts(VDP_PLAN_A, str, 2, 5);
			sprintf(str, "E:%04hu T:%04hu F:%04hX", event, type, flags);
			vdp_puts(VDP_PLAN_A, str, 2, 7);
			
			while(!joy_pressed(BUTTON_C)) {
				vdp_vsync();
				//xgm_vblank();
				joy_update();
				vdp_hscroll(VDP_PLAN_A, 0);
				vdp_vscroll(VDP_PLAN_A, 0);
			}
			vdp_color(0, 0);
			vdp_set_display(FALSE);
		}
	#endif
	//	entity_create_ext(block_to_sub(x) + 0x1000, block_to_sub(y) + 0x1000, type, flags, id, event);
	}
}

// Replaces a block with another (for <CMP, <SMP, and breakable blocks)
void stage_replace_block(int16_t bx, int16_t by, uint8_t index) {
	/*stageBlocks[stageTable[by] + bx] = index;
	int16_t cx = sub_to_block(camera.x), cy = sub_to_block(camera.y);
	if(cx - 16 > bx || cx + 16 < bx || cy - 8 > by || cy + 8 < by) return;
	// Only redraw if change was made onscreen
	stage_draw_block(bx, by);*/
}
extern u8 pal_mode; // Defined in game.c
// Stage vblank drawing routine
void stage_update() {
    //z80_request();
	// Background Scrolling
	// Type 2 is not included here, that's blank backgrounds which are not scrolled
	
	// Safety check: if stageBackgroundType is invalid, default to type 0
	if(stageBackgroundType > 5) {
		stageBackgroundType = 0;
	}
	
	if(stageBackgroundType == 0) {
		vdp_hscroll(VDP_PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		vdp_vscroll(VDP_PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		vdp_hscroll(VDP_PLAN_B, -sub_to_pixel(camera.x) / 4 + SCREEN_HALF_W);
		vdp_vscroll(VDP_PLAN_B, sub_to_pixel(camera.y) / 4 - SCREEN_HALF_H);
	} else if(stageBackgroundType == 1 || stageBackgroundType == 5) {
		// PLAN_A Tile scroll
		int16_t off[32];
		off[0] = -sub_to_pixel(camera.x) + SCREEN_HALF_W;
		uint8_t i;
		for( i = 1; i < 32; i++) {
			off[i] = off[0];
		}
		vdp_hscroll_tile(VDP_PLAN_A, off);
		vdp_vscroll(VDP_PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		// Moon background has different spots scrolling horizontally at different speeds
		backScrollTimer--;
		
		if(pal_mode) {
			uint8_t y = 28;
			for(;y >= 22; --y) backScrollTable[y] = backScrollTimer << 1;
			for(;y >= 18; --y) backScrollTable[y] = backScrollTimer;
			for(;y >= 15; --y) backScrollTable[y] = backScrollTimer >> 1;
			for(;y >= 11; --y) backScrollTable[y] = backScrollTimer >> 2;
			vdp_hscroll_tile(VDP_PLAN_B, backScrollTable);
			//VDP_setVerticalScroll(VDP_PLAN_B, -8);
		} else {
			uint8_t y = 27;
			for(;y >= 21; --y) backScrollTable[y] = backScrollTimer << 1;
			for(;y >= 17; --y) backScrollTable[y] = backScrollTimer;
			for(;y >= 14; --y) backScrollTable[y] = backScrollTimer >> 1;
			for(;y >= 10; --y) backScrollTable[y] = backScrollTimer >> 2;
			vdp_hscroll_tile(VDP_PLAN_B, backScrollTable);
			//VDP_setVerticalScroll(VDP_PLAN_B, 0);
		}
	} /*else if(stageBackgroundType == 3) {
		// Lock camera at specific spot
		camera.target = NULL;
		// Ironhead boss background auto scrolls leftward
		backScrollTable[0] -= 2;
		vdp_hscroll(VDP_PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		vdp_vscroll(VDP_PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		vdp_hscroll(VDP_PLAN_B, backScrollTable[0]);
	} else if(stageBackgroundType == 4) {
		int16_t rowc = SCREEN_HEIGHT >> 3;
		int16_t rowgap = 31 - rowc;
		// Water surface relative to top of screen
		int16_t scroll = (water_entity->y >> CSF) - ((camera.y >> CSF) - SCREEN_HALF_H);
		int16_t row = scroll >> 3;
		int16_t oldrow = backScrollTable[0];
		uint16_t x;
		while(row < oldrow) { // Water is rising (Y decreasing)
			oldrow--;
			uint8_t rowup = 31 - ((oldrow + rowgap) & 31);// Row that will be updated
			if(oldrow > rowc) { // Below Screen
				uint16_t mapBuffer[64];

				for( x = 0; x < 64; x++) mapBuffer[x] = 0;
				//DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or above
				uint16_t mapBuffer[64];

				for( x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == rowc ? x&3 : 4 + (random()&15)));
				}
				//DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		while(row > oldrow) { // Water is lowering (Y increasing)
			oldrow++;
			uint8_t rowup = 31 - (oldrow & 31); // Row that will be updated
			if(oldrow <= 0) { // Above screen
				uint16_t mapBuffer[64];
				for( x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == 0 ? x&3 : 4 + (random()&15)));
				}
				//DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or below
				uint16_t mapBuffer[64];
				for( x = 0; x < 64; x++) mapBuffer[x] = 0;
				//DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		
		vdp_hscroll(VDP_PLAN_B, -sub_to_pixel(camera.x) + SCREEN_HALF_W - backScrollTimer);
		vdp_vscroll(VDP_PLAN_B, -scroll);
		vdp_hscroll(VDP_PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		vdp_vscroll(VDP_PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		backScrollTable[0] = row;
	} else {
		// Only scroll foreground
		vdp_hscroll(VDP_PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		vdp_vscroll(VDP_PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
	}
	if(currentsCount) { // Waterway currents
		currentsTimer = (currentsTimer + 1) & 0x1F;
		uint8_t t = currentsTimer & 3;
		if(t < currentsCount) {
			uint16_t from_index = 0;
			uint8_t *from_ts = NULL;
			uint16_t to_index = TILE_TSINDEX + (currents[t].index << 2);
			switch(currents[t].dir) {
				case 0: // Left
					//from_ts = (uint8_t*) TS_WindH.tiles;
					from_index = (currentsTimer >> 1) & ~1;
				break;
				case 1: // Up
					//from_ts = (uint8_t*) TS_WindV.tiles;
					from_index = (currentsTimer >> 1) & ~1;
				break;
				case 2: // Right
					//from_ts = (uint8_t*) TS_WindH.tiles;
					from_index = 14 - ((currentsTimer >> 1) & ~1);
				break;
				case 3: // Down
					//from_ts = (uint8_t*) TS_WindV.tiles;
					from_index = 14 - ((currentsTimer >> 1) & ~1);
				break;
				default: return;
			}
			// Replace the tile in the tileset
			//DMA_doDma(DMA_VRAM, (uint32_t) (from_ts + (from_index << 5)), to_index << 5, 32, 2);
			from_index += 16;
			to_index += 2;
			//DMA_doDma(DMA_VRAM, (uint32_t) (from_ts + (from_index << 5)), to_index << 5, 32, 2);
		}
	}*/
    //z80_release();
}

void stage_setup_palettes() {
	// Stage palette and shared NPC palette
	/*vdp_colors_next(0, PAL_Main, 16);
	if(stageID == STAGE_INTRO) {
		vdp_colors_next(16, PAL_Intro, 16);
	} else {
		vdp_colors_next(16, PAL_Sym, 16);
	}
	if(stageID == STAGE_WATERWAY) {
		vdp_colors_next(32, PAL_RiverAlt, 16); // For Waterway green background
	} else {
		vdp_colors_next(32, tileset_info[stage_info[stageID].tileset].palette, 16);
	}
	vdp_colors_next(48, stage_info[stageID].npcPalette->data, 16);*/
}

#include "snes_regs_xc.h"

// Globals to track previous position
// Initialize to a value that forces a full redraw on the first frame
uint16_t last_x = 0xFFFF;
uint16_t last_y = 0xFFFF;

// Your existing shadow buffers


void stage_draw_tile(uint16_t x, uint16_t y, const uint8_t* pxa) {
    // 1. Get Meta-Block Info (from collision map or stage data)
    // Assuming stage_get_block returns a meta-tile index (0-255)
    uint16_t b = stage_get_block(x >> 1, y >> 1);
    
    // 2. Get Tile Attributes from PXA array
    // pxa[b] holds flags for palette, priority, etc.
    uint16_t ta = pxa[b];
    
    // 3. Calculate Base Tile Index
    // 'b << 2' implies each meta-block consists of 4 hardware tiles (2x2)
    uint16_t base_tile = b << 2; 
    
    // Calculate the specific quadrant (0, 1, 2, or 3)
    // (x&1) adds 1 for right half, ((y&1)<<1) adds 2 for bottom half
    uint16_t final_tile_index = TILE_TSINDEX + base_tile + (x & 1) + ((y & 1) << 1);

    // 4. Decode Attributes
    // Example interpretation of 'ta' byte:
    // Bit 7: Palette (0 or 1)
    // Bit 6: Priority
    // Adjust masks to match your actual data format
    uint16_t palette = (ta & 0x80) ? 0 : 0; 
    uint16_t priority = (ta & 0x40) ? 1 : 1; 

    // 5. Construct SNES Map Entry
    // Format: vhopppcc cccccccc (v=vflip, h=hflip, o=prio, p=pal, c=char)
    uint16_t tile_entry = SNES_TILE_ENTRY(final_tile_index, palette, priority, 0, 0);

    // 6. Calculate Offset for 64x32 Map Layout
    // SNES 64x32 mode stores two 32x32 screens linearly.
    // Screen A: offsets 0-1023
    // Screen B: offsets 1024-2047
    
    // Mask X to 63 (map width) and Y to 31 (map height)
    uint16_t map_x = x & 63;
    uint16_t map_y = y & 31;
    
    // Base offset within a 32x32 block
    uint16_t offset = (map_y << 5) + (map_x & 31);
    
    // If we are in the right half (x >= 32), add 1024 (0x400)
    if (map_x >= 32) {
        offset += 1024;
    }

    // 7. Write to Shadow Buffer
    // Note: This does not update VRAM instantly. 
    // You must DMA 'map_buffer_bg1' to VRAM during VBlank or use setMapEntry if doing one by one.
    map_buffer_bg1[offset] = tile_entry;
}



void stage_update_screen(u16 view_x, u16 view_y) {
    const uint8_t *pxa = tileset_info[stageTileset].PXA;

    // Define the view dimensions (assuming 32x32 view area)
    const u16 VIEW_W = 32;
    const u16 VIEW_H = 32;
		u16 y;
		u16 x;
		u16 i;
    // --- CASE 1: First Run / Force Full Redraw ---
    if (last_x == 0xFFFF) {

        for ( y = 0; y < VIEW_H; y++) {
            for ( x = 0; x < VIEW_W; x++) {
                stage_draw_tile(view_x + x, view_y + y, pxa);
            }
        }
    } 
    else {
        // --- CASE 2: Horizontal Movement ---
        
        // Moved RIGHT -> Draw new column on the right edge
        if (view_x > last_x) {
            u16 draw_x = view_x + VIEW_W - 1; // The right-most column
            for ( i = 0; i < VIEW_H; i++) {
                stage_draw_tile(draw_x, view_y + i, pxa);
            }
        }
        // Moved LEFT -> Draw new column on the left edge
        else if (view_x < last_x) {
            u16 draw_x = view_x; // The left-most column
            for ( i = 0; i < VIEW_H; i++) {
                stage_draw_tile(draw_x, view_y + i, pxa);
            }
        }

        // --- CASE 3: Vertical Movement ---
        
        // Moved DOWN -> Draw new row at the bottom
        if (view_y > last_y) {
            u16 draw_y = view_y + VIEW_H - 1; // The bottom-most row
            for ( i = 0; i < VIEW_W; i++) {
                stage_draw_tile(view_x + i, draw_y, pxa);
            }
        }
        // Moved UP -> Draw new row at the top
        else if (view_y < last_y) {
            u16 draw_y = view_y; // The top-most row
            for ( i = 0; i < VIEW_W; i++) {
                stage_draw_tile(view_x + i, draw_y, pxa);
            }
        }
        
        // Handle "Corner" cases for diagonal movement
        // If we moved Right AND Down, the loops above cover the edges, 
        // but the very corner tile might get drawn twice (which is fine) 
        // or missed depending on loop order. The logic above covers it 
        // because the X-loop uses the *new* Y, and Y-loop uses the *new* X.
    }

    // Update history
    last_x = view_x;
    last_y = view_y;

    // --- VRAM Update ---
    // Wait for VBlank before DMA to prevent tearing/glitches
    WaitForVBlank();
    
    // Copy the updated shadow buffers to VRAM
    // 0x6000 is your BG1 address, 0x6800 is your BG2 address (check your init!)
    //dmaCopyVram(map_buffer_bg1, 0x6000, 1024 * 2); // 64x32 map size
    //dmaCopyVram(map_buffer_bg2, 0x6800, 1024 * 2); 
    
    // Update Hardware Scroll
    // Multiply by 8 to convert tiles to pixels
    bgSetScroll(0, view_x * 8, view_y * 8);
    bgSetScroll(1, view_x * 8, view_y * 8);
}
void stage_clear_map_buffers() {
    // Define what an "empty" tile looks like for your tileset
    // Usually TILE_TSINDEX is the first tile (often blank/transparent)
    uint16_t empty_entry = SNES_TILE_ENTRY(TILE_TSINDEX, 0, 0, 0, 0);

    uint16_t i;
    for (i = 0; i < 1024; i++) {
        map_buffer_bg1[i] = empty_entry;
        map_buffer_bg2[i] = empty_entry;
    }
}

void stage_draw_screen() {
    const uint8_t *pxa = tileset_info[stageTileset].PXA;
	//uint16_t maprow[64];
	uint16_t y = sub_to_tile(camera.y) - 16;
	uint16_t i;
	for( i = 32; i--; ) {
		//if(vblank) aftervsync(); // So we don't lag the music
		//vblank = 0;
		
		if(y < (stageHeight + 32) << 1) {
			uint16_t x = sub_to_tile(camera.x) - 16;
			uint16_t j;
			for( j = 32; j--; ) {
				//if(x >= stageWidth << 1) break;
				//if(x >= 0) {
					stage_draw_tile(x, y, pxa);
				//}
				x++;
			}
            //dma_now(DmaVRAM, (uint32_t)maprow, VDP_PLANE_A + ((y & 31) << 7), 64, 2);
		}
		y++;
	}
	dmaCopyVram(map_buffer_bg1, 0x6000, 2048);
}
void stage_draw_screen_credits() {
	uint16_t maprow[20];
	uint16_t y;
	for( y = 0; y < 30; y++) {
		uint16_t x;
		for( x = 20; x < 40; x++) {
			uint16_t b = stage_get_block(x>>1, y>>1);
			uint16_t t = b << 2; //((b&15) << 1) + ((b>>4) << 6);
			maprow[x-20] = TILE_ATTR(PAL2,0,0,0, TILE_TSINDEX + t + (x&1) + ((y&1)<<1));
		}
		//DMA_doDma(DMA_VRAM, (uint32_t)maprow, VDP_PLAN_A + y*0x80 + 40, 20, 2);
	}
}

// Draws just one block
void stage_draw_block(uint16_t x, uint16_t y) {
	if(x >= stageWidth || y >= stageHeight) return;
	uint16_t b, xx, yy; uint8_t p;
	p = (stage_get_block_type(x, y) & 0x40) > 0;
	b = TILE_TSINDEX + (stage_get_block(x, y) << 2);
	xx = block_to_tile(x) % 64;
	yy = block_to_tile(y) % 32;

	vdp_map_xy(VDP_PLAN_A, TILE_ATTR(2, p, 0, 0, b), xx, yy);
	vdp_map_xy(VDP_PLAN_A, TILE_ATTR(2, p, 0, 0, b+1), xx+1, yy);
	vdp_map_xy(VDP_PLAN_A, TILE_ATTR(2, p, 0, 0, b+2), xx, yy+1);
	vdp_map_xy(VDP_PLAN_A, TILE_ATTR(2, p, 0, 0, b+3), xx+1, yy+1);
	
}

// Fills VDP_PLAN_B with a tiled background
int ind = 0;
u8 ind2 = 0;
void stage_draw_background() {
	uint16_t w = background_info[stageBackground].width;
	uint16_t h = background_info[stageBackground].height;
	uint16_t pal = background_info[stageBackground].palette;
	uint16_t y, x;
	test_draw_sequential();
	vdp_vsync();
	dmaCopyVram(map_buffer_bg2, 0x7000, 2048);
	vdp_vsync();
	return;
	for( y = 0; y < 32; y += h) {
		for( x = 0; x < 64; x += w) {
			//vdp_map_fill_rect(BASE_BACK, TILE_ATTR(pal,0,0,0,TILE_BACKINDEX), x, y, w, h, 1);
			vdp_map_fill_rect(BASE_BACK, TILE_BACKINDEX + ind2, x, y, w, h, 1);
			//uint16_t tile = TILE_ATTR(pal,0,0,0,TILE_BACKINDEX);
			//for(uint16_t yy = 0; yy < h; yy++) {
			//	for(uint16_t xx = 0; xx < w; xx++) {
			//		vdp_map_xy(VDP_PLAN_B, tile++, x+xx, y+yy);
			//	}
			//}
		}
	}
}

static void stage_draw_moonback() {
	/*const uint32_t *topTiles, *btmTiles;
	const uint16_t *topMap, *btmMap;
	if(stageBackgroundType == 1) {
		// Moon
		topTiles = (uint32_t*) PAT_MoonTop;
		btmTiles = (uint32_t*) PAT_MoonBtm;
		topMap = (uint16_t*) MAP_MoonTop;
		btmMap = (uint16_t*) MAP_MoonBtm;
	} else {
		// Fog
		topTiles = (uint32_t*) PAT_FogTop;
		btmTiles = (uint32_t*) PAT_FogBtm;
		topMap = (uint16_t*) MAP_FogTop;
		btmMap = (uint16_t*) MAP_FogBtm;
	}
	// Load the top section in the designated background area
	vdp_tiles_load_from_rom(topTiles, TILE_BACKINDEX, 12);
	// Load the clouds under the map, it just fits
	vdp_tiles_load_from_rom(btmTiles, TILE_MOONINDEX, 188);
	uint8_t y;
	for( y = 0; y < 32; y++) backScrollTable[y] = 0;
	vdp_vscroll(VDP_PLAN_B, 0);
	// Top part
	uint16_t index = pal_mode ? 0 : 40;
	uint16_t y;
	for( y = 0; y < (pal_mode ? 11 : 10); y++) {
		DMA_doDma(DMA_VRAM, (uint32_t) &topMap[index], VDP_PLAN_B + (y << 7), 40, 2);
		index += 40;
	}
	
	if(vblank) aftervsync(); // So we don't lag the music
	vblank = 0;
	
	// Bottom part
	index = 0;
	uint16_t y;
	for( y = (pal_mode ? 11 : 10); y < (pal_mode ? 32 : 28); y++) {
		DMA_doDma(DMA_VRAM, (uint32_t) &btmMap[index], VDP_PLAN_B + (y << 7),             32, 2);
		DMA_doDma(DMA_VRAM, (uint32_t) &btmMap[index], VDP_PLAN_B + (y << 7) + (32 << 1), 32, 2);
		index += 32;
	}
	backScrollTimer = 0;*/
}
