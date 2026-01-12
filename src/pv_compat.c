#include "pv_compat.h"
#include "snes_regs_xc.h"
#include "vdp.h"
/**
 * Sets the VRAM address for the character data (tiles) of a specific background.
 * 
 * @param bgNumber Background number (0-3)
 * @param address VRAM Word Address (e.g., 0x2000 for 8KB offset)
 */
void bgSetGfxPtr(uint8_t bgNumber, uint16_t address) {
    // Store in shadow array so we can combine neighbors later
    bg_gfx_addr[bgNumber & 3] = address;

    // SNES registers combine BG1/BG2 and BG3/BG4 into single bytes.
    // The register expects the address in steps of 4KB Words (0x1000).
    // So we shift right by 12 (divide by 4096).
    
    if (bgNumber < 2) {
        // Update BG1 and BG2 (REG_BG12NBA)
        // High nibble: BG2, Low nibble: BG1
        uint8_t val = ((bg_gfx_addr[1] >> 12) << 4) | (bg_gfx_addr[0] >> 12);
        REG_BG12NBA = val;
    } else {
        // Update BG3 and BG4 (REG_BG34NBA)
        // High nibble: BG4, Low nibble: BG3
        uint8_t val = ((bg_gfx_addr[3] >> 12) << 4) | (bg_gfx_addr[2] >> 12);
        REG_BG34NBA = val;
    }
}

/**
 * Sets the VRAM address and size for the tile map of a specific background.
 * 
 * @param bgNumber Background number (0-3)
 * @param address VRAM Word Address (e.g., 0x6000)
 * @param mapSize Map dimensions (SC_32x32, SC_64x32, etc.)
 */
void bgSetMapPtr(uint8_t bgNumber, uint16_t address, uint8_t mapSize) {
    // Determine which register to write to based on bgNumber (REG_BG1SC, REG_BG2SC, etc.)
    // Registers are sequential in memory: 2107, 2108, 2109, 210A
    volatile uint8_t* reg = &REG_BG1SC + (bgNumber & 3);
    
    // Format: aaaaaass
    // aaaaaa = Base Address >> 8 (which corresponds to 1KB Word steps)
    // ss = Map Size mode
    *reg = ((address >> 8) & 0xFC) | (mapSize & 0x03);
}

/**
 * Disables a specific background layer on the Main Screen (TM).
 * 
 * @param bgNumber Background number (0-3)
 */
void bgSetDisable(uint8_t bgNumber) {
    // Mask out the bit corresponding to the background
    bg_enabled_state &= ~(1 << bgNumber);
    REG_TM = bg_enabled_state;
}

/**
 * Enables a specific background layer on the Main Screen (TM).
 * (Added for completeness as bgSetDisable relies on the shared state)
 * 
 * @param bgNumber Background number (0-3)
 */
/*void bgSetEnable(uint8_t bgNumber) {
    bg_enabled_state |= (1 << bgNumber);
    REG_TM = bg_enabled_state;
}

/**
 * Initializes a tileset by copying graphics to VRAM and Palette to CGRAM,
 * then setting the Gfx pointer.
 * 
 * @param bgNumber Background number (0-3)
 * @param tileSource Pointer to tile data
 * @param tilePalette Pointer to palette data
 * @param paletteEntry Palette slot start index
 * @param tileSize Size of tile data in bytes
 * @param paletteSize Size of palette data in bytes
 * @param colorMode Color mode (e.g. BG_16COLORS)
 * @param address VRAM Word Address destination
 */
void bgInitTileSet(uint8_t bgNumber, const uint8_t *tileSource, const uint8_t *tilePalette, 
                   uint8_t paletteEntry, uint16_t tileSize, uint16_t paletteSize, 
                   uint16_t colorMode, uint16_t address) {
    
    // 1. Copy Tiles to VRAM
    // dmaCopyVram takes byte size, LoadVram uses word address internally in your compat layer
    dmaCopyVram(tileSource, address, tileSize);

    // 2. Copy Palette to CGRAM
    // Calculate the actual CGRAM word offset. 
    // Usually: BgNumber * 16 colors (for 4bpp) + Offset
    // Your dmaCopyCGram wrapper usually expects byte offset.
    
    uint16_t cgramOffset = 0;
    if (colorMode == BG_16COLORS) {
        // Standard 16 color mode logic from ASM
        // If paletteEntry is used, it maps to specific rows
        cgramOffset = (paletteEntry * 16); 
    } else {
        // 256 Color mode logic
        cgramOffset = paletteEntry * 2; 
    }

    LoadCGRam(tilePalette, cgramOffset, paletteSize);

    // 3. Set the Graphics Pointer register
    bgSetGfxPtr(bgNumber, address);
}
