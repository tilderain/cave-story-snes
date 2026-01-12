#include "common.h"
#include "snes_regs_xc.h"
#include "vdp.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// -------------------------------------------------------------------------
// Constants & Defines
// -------------------------------------------------------------------------

#define BANK_SRAM           0x70 // LoROM mapping
#define OFFSET_SRAM         0x0000

#define PPU_50HZ            (1 << 4)
#define REG_STAT78_ADDR     0x213F
#define REG_DEBUG_ADDR      0x21FC

// SNES PPU Register Addresses for BG3
#define REG_BG3SC_ADDR      0x2109
#define REG_BG34NBA_ADDR    0x210C

// VRAM Allocation for Console (Adjusted for BG3 usage)
// Font Tiles at 0x3000 (12KB offset)
#define TXT_VRAMADR         0x3000 
// Tile Map at 0x6800 (26KB offset)
#define TXT_VRAMBGADR       0x6800 
// Tile Index Offset (0 because we load tiles exactly at the address pointer)
#define TXT_VRAMOFFSET      0x0000

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------

uint8_t snes_50hz = 0;          
uint8_t snes_fps = 60;          

volatile uint8_t scr_txt_dirty = 0; 
uint8_t txt_pal_adr = 0;        
uint16_t txt_vram_bg = TXT_VRAMBGADR;
uint16_t txt_vram_adr = TXT_VRAMADR;
uint16_t txt_vram_offset = TXT_VRAMOFFSET;

char text_buffer[128];          
uint16_t cons_val1 = 0;         

uint16_t scr_txt_font_map[32 * 32]; 

static uint16_t snes_rand_seed1 = 0;
static uint16_t snes_rand_seed2 = 0;

extern volatile uint16_t vblank_count; 

// -------------------------------------------------------------------------
// Helper Functions
// -------------------------------------------------------------------------

uint16_t rand(void) {
    uint16_t r0 = (snes_rand_seed2 >> 1) + snes_rand_seed1;
    snes_rand_seed1 = r0;
    r0 ^= 0x00FF;
    snes_rand_seed2 = snes_rand_seed2 - r0;
    return snes_rand_seed2;
}

void consoleMesenBreakpoint(void) {
   // __asm__("wdm 0x00"); 
}



// -------------------------------------------------------------------------
// SRAM Functions
// -------------------------------------------------------------------------

void consoleCopySram(uint8_t *source, uint16_t size) {
    uint8_t *sram = (uint8_t *)(0x700000 + OFFSET_SRAM);
    uint16_t i;
    for (i = 0; i < size; i++) sram[i] = source[i];
}

void consoleLoadSram(uint8_t *dest, uint16_t size) {
    uint8_t *sram = (uint8_t *)(0x700000 + OFFSET_SRAM);
    uint16_t i;
    for (i = 0; i < size; i++) dest[i] = sram[i];
}

void consoleCopySramWithOffset(uint8_t *source, uint16_t size, uint16_t offset) {
    uint8_t *sram = (uint8_t *)(0x700000 + OFFSET_SRAM + offset);
    uint16_t i;
    for (i = 0; i < size; i++) sram[i] = source[i];
}

void consoleLoadSramWithOffset(uint8_t *dest, uint16_t size, uint16_t offset) {
    uint8_t *sram = (uint8_t *)(0x700000 + OFFSET_SRAM + offset);
    uint16_t i;
    for (i = 0; i < size; i++) dest[i] = sram[i];
}

// -------------------------------------------------------------------------
// VBlank Handler
// -------------------------------------------------------------------------

void consoleVblank(void) {
    //if (scr_txt_dirty == 1) {
        // DMA RAM buffer to VRAM
        dmaCopyVram(scr_txt_font_map, txt_vram_bg, 0x800);
        scr_txt_dirty = 0;
    //}
}

// -------------------------------------------------------------------------
// Initialization
// -------------------------------------------------------------------------

void consoleInit(void) {
    REG_NMITIMEN = 0; 

    scr_txt_dirty = 0;
    snes_rand_seed1 = 1;
    snes_rand_seed2 = 5;

    if (*(volatile uint8_t*)REG_STAT78_ADDR & PPU_50HZ) {
        snes_50hz = 1;
        snes_fps = 50;
    } else {
        snes_50hz = 0;
        snes_fps = 60;
    }

    txt_vram_bg = TXT_VRAMBGADR;
    txt_vram_adr = TXT_VRAMADR;
    txt_vram_offset = TXT_VRAMOFFSET;

    // --- BG3 CONFIGURATION START ---

    // 1. Configure BG3 Map Address (REG_BG3SC - $2109)
    // Bits 7-2: Base Address >> 8. (0x6800 >> 8 = 0x68)
    // Bits 1-0: SC Size (00 = 32x32)
    *(volatile uint8_t*)REG_BG3SC_ADDR = (TXT_VRAMBGADR >> 8) & 0xFC;

    // 2. Configure BG3 Tile Data Address (REG_BG34NBA - $210C)
    // Bits 0-3: BG3 Base Address >> 12. (0x3000 >> 12 = 0x03)
    // Bits 4-7: BG4 Base Address (Preserve 0 or assumed 0)
    *(volatile uint8_t*)REG_BG34NBA_ADDR = (TXT_VRAMADR >> 12) & 0x0F;

    // 3. Enable BG3 on Main Screen (REG_TM - $212C)
    // Bit 0: BG1, Bit 1: BG2, Bit 2: BG3, Bit 3: BG4, Bit 4: OBJ
    // We read current state and enable bit 2.
    REG_TM |= 0x04; 

    // --- BG3 CONFIGURATION END ---
}

void consoleInitText(uint8_t palnum, uint8_t palsize, uint8_t *tilfont, uint8_t *palfont) {
    int i;
    for (i = 0; i < 0x400; i++) {
        scr_txt_font_map[i] = 0;
    }

    // Copy Font Tiles to VRAM (at 0x3000)
    dmaCopyVram(tilfont, txt_vram_adr, 3072);

    // Copy Palette to CGRAM
    extern void LoadCGRam(const unsigned char * source, uint16_t address, uint16_t size);
    LoadCGRam(palfont, palnum * 16 * 2, palsize); // Adjusted for standard 4bpp spacing

    // Calculate Palette Attribute High Byte
    // Format: vhopppcc
    // We set bit 5 (Priority) to ensure Text appears above standard BG layers
    // palnum << 2 places the palette index into bits 10-12 of the map entry
    txt_pal_adr = (palnum << 2) | (1 << 5); 
}

void consoleSetTextGfxPtr(uint16_t vramfont) {
    txt_vram_adr = vramfont;
    // Update hardware register to reflect new tile location
    *(volatile uint8_t*)REG_BG34NBA_ADDR = (txt_vram_adr >> 12) & 0x0F;
}

void consoleSetTextMapPtr(uint16_t vrambgfont) {
    txt_vram_bg = vrambgfont;
    // Update hardware register to reflect new map location
    *(volatile uint8_t*)REG_BG3SC_ADDR = (txt_vram_bg >> 8) & 0xFC;
}

void consoleSetTextOffset(uint16_t offsetfont) {
    txt_vram_offset = offsetfont;
}

void consoleSetTextPal(uint8_t palnum, uint8_t *palfont, uint8_t palsize) {
    extern void LoadCGRam(const unsigned char * source, uint16_t address, uint16_t size);
    LoadCGRam(palfont, palnum * 16 * 2, palsize);
}

// -------------------------------------------------------------------------
// Drawing Functions
// -------------------------------------------------------------------------

void print_screen_map(uint16_t x, uint16_t y, uint16_t *map, uint8_t attributes, char *buffer) {
    uint16_t offset = y * 32 + x;
    
    while (*buffer) {
        if (*buffer == 13 || *buffer == '\n') {
            offset = (offset / 32 + 1) * 32;
        } else {
            // In 2bpp mode, tiles are half the size, so we need to double the tile index
            uint16_t tile_index = (*buffer - 32) * 2; // Multiply by 2 for 2bpp
            uint8_t tile_low = tile_index + (uint8_t)txt_vram_offset;
            uint8_t tile_high = attributes + (uint8_t)(txt_vram_offset >> 8) + (tile_index >> 8);
            map[offset] = tile_low | (tile_high << 8);
            offset++;
        }
        buffer++;
        if (offset >= 0x400) break; 
    }
}
void consoleClear(void) {
    int i;
    
    // Calculate the entry for an empty space (character 0 is space after subtracting 32)
    // Low byte: tile index (0 + txt_vram_offset low byte)
    // High byte: attributes (txt_pal_adr + txt_vram_offset high byte)
    uint8_t tile_low = (uint8_t)txt_vram_offset;
    uint8_t tile_high = txt_pal_adr + (uint8_t)(txt_vram_offset >> 8);
    uint16_t empty_entry = tile_low | (tile_high << 8);
    
    // Fill the shadow buffer (32x32 tiles)
    for (i = 0; i < 32 * 32; i++) {
        scr_txt_font_map[i] = empty_entry;
    }
    
    // Mark the buffer as "Dirty" so consoleVblank uploads it to VRAM
    scr_txt_dirty = 1;
}
void consoleDrawText(uint16_t x, uint16_t y, const char *fmt, ...) {
    scr_txt_dirty = 2; 

    va_list ap;
    va_start(ap, fmt);
    minimal_vsprintf(text_buffer, fmt, ap);
    va_end(ap);

    print_screen_map(x, y, scr_txt_font_map, txt_pal_adr, text_buffer);

    scr_txt_dirty = 1; 
}

void consoleDrawTextMap(uint16_t x, uint16_t y, uint8_t *map, uint8_t attributes, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text_buffer, fmt, ap);
    va_end(ap);

    print_screen_map(x, y, (uint16_t*)map, attributes, text_buffer);
}

void consoleDrawTextMapCenter(uint16_t y, uint16_t *map, uint8_t attributes, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text_buffer, fmt, ap);
    va_end(ap);

    uint16_t len = strlen(text_buffer);
    uint16_t x = 16 - (len >> 1); 

    print_screen_map(x, y, map, attributes, text_buffer);
}

void consoleUpdate(void) {
    if (scr_txt_dirty == 1) {
        REG_INIDISP = 0x80;
        dmaCopyVram(scr_txt_font_map, txt_vram_bg, 0x800);
        scr_txt_dirty = 0;
        REG_INIDISP = 0x0F; 
    }
}

uint8_t consoleRegionIsOK(void) {
    uint8_t country = *(volatile uint8_t*)0xFFD9;
    uint8_t is_pal_cart = 0;

    if (country != 0x00 && country != 0x01 && country != 0x0D && country != 0x0F && country != 0x10) {
        is_pal_cart = 1;
    }

    if (snes_50hz == is_pal_cart) {
        return 1;
    }
    return 0;
}


