#include <stdint.h>
#include "snes_regs_xc.h"
#include "data_converted_part1.h"
#include "data_converted_part2.h"
#include "vdp.h"

#include "initsnes.h"
#include "pv_compat.h"

#include "common.h"

#include "snesmod.h"
//---------------------------------------------------------------------------------
int main(void)
{
    initSNES(FASTROM);

    // Initialize sound engine (take some time)
    spcBoot();

    // Initialize text console with our font
    //consoleSetTextMapPtr(0x6000);
    //consoleSetTextGfxPtr(0x3000);
    //consoleInitText(1, 16 * 2, &snesfont, &snespal);
    // Set give soundbank
    //spcSetBank(&SOUNDBANK__);
    // allocate around 10K of sound ram (39 256-byte blocks)
    //spcAllocateSoundRegion(39);
    // Load music
    //spcLoad(MOD_GESTATION);
    // Load sample
    //spcSetSoundEntry(15, 8, 6, &walksndend - &walksnd, &walksnd, &walksnd);
    //spcSetSoundEntry(15, 8, 6, &jumpsndend - &jumpsnd, &jumpsnd, &Jump);




    //bgSetDisable(2);

    // Now Put in 16 color mode and disable Bgs except current
    setMode(BG_MODE1, 0);

        // Init background
    bgSetGfxPtr(1, 0x2000);
    bgSetMapPtr(0, 0x6000, SC_64x32);
    bgSetGfxPtr(0, 0x4000);
    //bgInitTileSet(0, &UFTC_Cave, tileset_info[3].palette, 0, (tileset_info[stageTileset].size*32), 16 * 2, BG_16COLORS, 0x2000);
    bgSetMapPtr(1, 0x7000, SC_32x32);
    
    /*
    // Draw a wonderful text :P
    // Put some text
    //consoleDrawText(6, 16, "MARIOx00  WORLD TIME");
    //consoleDrawText(6, 17, " 00000 ox00 1-1  000");

    // Wait for nothing :P
    setScreenOn();
    //spcPlay(0);
    //spcSetModuleVolume(100);

   // consoleMesenBreakpoint();

    //stage_load(13);
    // Init sprite engine (0x0000 for large, 0x1000 for small)
   // oamInitDynamicSprite(0x0000, 0x1000, 0, 0, OBJ_SIZE8_L16);

    // Object engine activate
    //objInitEngine();

    // Init function for state machine
   // objInitFunctions(0, &marioinit, &marioupdate, NULL);

    uint16_t frame = 0;

    // Initialize camera
    short x = 0; 
    short y = 0;

    // Force initialization of the graphics on the first frame
    stage_update_screen(x, y); 

    setScreenOn();

    char stage_no = 13;

    joy_init();*/


    dmaCopyVram(mariogfx, 0x0000, 32*8);

    while(1) {
        game_main(0);
    }
    return 0;
}

// Cross-compiler interrupt handlers, must be present
void snesXC_cop(void) {
}

void snesXC_brk(void) {
}

void snesXC_abort(void) {
}

void snesXC_nmi(void) {
    // Set vblank flag so WaitForVBlank() can proceed
    vdp_set_vblank_flag();
}
