#include "common.h"
#include "snes_regs_xc.h"
#include "vdp.h"
#include "initsnes.h"
#include <stdarg.h>

// PVSneslib compatibility functions
static volatile uint8_t vblank_flag = 0;
static volatile uint16_t vblank_count = 0;

// VRAM logging counter
static uint8_t vram_log_count = 0;

// Random number generator (simple LFSR implementation)
static uint16_t random_seed = 1;

int random(void) {
    // Simple linear feedback shift register (LFSR) for pseudo-random numbers
    random_seed ^= random_seed << 7;
    random_seed ^= random_seed >> 9;
    random_seed ^= random_seed << 8;
    return (int)random_seed;
}

// Function to set vblank flag (called from NMI interrupt)
void vdp_set_vblank_flag(void) {
    vblank_flag = 1;
    vblank_count++;
}

void WaitForVBlank(void) {
    // Wait for vblank flag to be set
    while(!vblank_flag) {
        // Busy wait
    }
    vblank_flag = 0;
}

void setScreenOn(void) {
    // Turn on screen (clear force blank bit)
    REG_INIDISP = 0x0F; // Full brightness, screen on
}

void setScreenOff(void) {
    // Turn off screen (set force blank bit)
    REG_INIDISP = 0x80; // Force blank
}

void consoleInit(void) {
    // Stub - console functionality not needed for game
}

void consoleDrawText(uint8_t x, uint8_t y, const char *text) {
    // Stub - console text not needed for game
    (void)x; (void)y; (void)text;
}

// --- Initialization & Control ---

void vdp_init() {
    // Initialize SNES
    // Screen will be turned on elsewhere
}

void vdp_vsync() {
    WaitForVBlank();
}

void vdp_set_display(uint8_t enabled) {
    if(enabled) setScreenOn();
    else setScreenOff();
}

void vdp_set_autoinc(uint8_t val) {}
void vdp_set_scrollmode(uint8_t hoz, uint8_t vert) {}
void vdp_set_highlight(uint8_t enabled) {}
void vdp_set_backcolor(uint8_t index) {}
void vdp_set_window(uint8_t x, uint8_t y) {}

// --- DMA / Memory Transfer ---

void vdp_dma_vram(uint32_t from, uint16_t to, uint16_t len) {}
void vdp_dma_cram(uint32_t from, uint16_t to, uint16_t len) {}
void vdp_dma_vsram(uint32_t from, uint16_t to, uint16_t len) {}

// --- Debug / Console Helpers ---

// Minimal number to string conversion helpers
static void uint_to_str(char *buf, uint32_t val, uint8_t base) {
    char *p = buf;
    if (val == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    char temp[16];
    uint8_t i = 0;
    while (val > 0) {
        uint8_t digit = val % base;
        temp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        val /= base;
    }
    // Reverse
    while (i > 0) {
        *p++ = temp[--i];
    }
    *p = '\0';
}

static void int_to_str(char *buf, int32_t val) {
    if (val < 0) {
        *buf++ = '-';
        val = -val;
    }
    uint_to_str(buf, (uint32_t)val, 10);
}

// Helper to pad hex string with leading zeros
static void pad_hex_str(char *out, const char *in, uint8_t width) {
    uint8_t len = 0;
    while (in[len]) len++;
    uint8_t pad = (width > len) ? (width - len) : 0;
    uint8_t i = 0;
    for (; i < pad; i++) out[i] = '0';
    for (uint8_t j = 0; in[j]; j++) out[i++] = in[j];
    out[i] = '\0';
}

// DMA copy to VRAM - SNES native implementation
void dmaCopyVram(const void *src, uint16_t dest, uint16_t size) {
    // Log only the first 20 VRAM writes
    if (vram_log_count < 20) {
        // Log the LoadVram call (24-bit pointer: bank + offset)
        uint32_t ptr = (uint32_t)(uintptr_t)src;
        uint8_t bank = (ptr >> 16) & 0xFF;
        uint16_t offset = ptr & 0xFFFF;
        // Format with leading zeros manually since minimal printf doesn't support width specifiers
        char bank_str[3], offset_str[5], dest_str[5];
        uint_to_str(bank_str, bank, 16);
        uint_to_str(offset_str, offset, 16);
        uint_to_str(dest_str, dest, 16);
        char bank_padded[3], offset_padded[5], dest_padded[5];
        pad_hex_str(bank_padded, bank_str, 2);
        pad_hex_str(offset_padded, offset_str, 4);
        pad_hex_str(dest_padded, dest_str, 4);
        consoleNocashMessage("LoadVram[%u]: src=0x%s:%s dest=0x%s size=%u\n", 
                            vram_log_count, bank_padded, offset_padded, dest_padded, size);
        vram_log_count++;
    }
    // Use snesXC LoadVram function for direct DMA transfer
    LoadVram((const unsigned char *)src, dest, size);
}

// Flush DMA queue - SNES native implementation (stub)
// DMA operations in snesXC are immediate, so this is a no-op
void DMA_flushQueue(void) {
    // Stub - DMA operations in snesXC are immediate
}

// --- Tile Management ---

void vdp_tiles_load(volatile const uint32_t *data, uint16_t index, uint16_t num) {}
void vdp_tiles_load_from_rom(volatile const uint32_t *data, uint16_t index, uint16_t num) {}

// --- Map / Plane Management ---

void vdp_map_xy(uint16_t plan, uint16_t tile, uint16_t x, uint16_t y) {}
void vdp_map_hline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len) {}
void vdp_map_vline(uint16_t plan, const uint16_t *tiles, uint16_t x, uint16_t y, uint16_t len) {}
void vdp_map_fill_rect(uint16_t plan, uint16_t index, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t inc) {}
void vdp_map_clear(uint16_t plan) {
    // Optional: clear SNES console?
    // consoleClear();
}

// --- Palette ---

void vdp_colors(uint16_t index, const uint16_t *values, uint16_t count) {}
void vdp_color(uint16_t index, uint16_t color) {
    // Set a single palette color in CGRAM
    // index is the color index (0-255), color is 15-bit BGR format
    LoadCGRam((const unsigned char *)&color, index * 2, 2);
}
void vdp_colors_next(uint16_t index, const uint16_t *values, uint16_t count) {}
void vdp_color_next(uint16_t index, uint16_t color) {}

// Set palette color - SNES native implementation
void setPaletteColor(uint8_t index, uint16_t color) {
    // Use vdp_color which uses LoadCGRam for SNES hardware
    vdp_color(index, color);
}

uint16_t vdp_fade_step() {
    return 0; // Return 0 to indicate fade is "done" immediately
}

void vdp_fade(const uint16_t *src, const uint16_t *dst, uint16_t speed, uint8_t async) {}

// --- Scroll ---
uint16_t prev_h = 0;
uint16_t prev_v = 0;

// Background scroll registers mapping
// BG1 = plan 0, BG2 = plan 1, etc.
void bgSetScroll(uint8_t bg, int16_t h, int16_t v) {
    // SNES BG scroll registers are write-twice registers
    // Write low byte first, then high byte
    switch(bg) {
        case 0: // BG1
            REG_BG1HOFS = (uint8_t)h;
            REG_BG1HOFS = (uint8_t)(h >> 8);
            REG_BG1VOFS = (uint8_t)v;
            REG_BG1VOFS = (uint8_t)(v >> 8);
            break;
        case 1: // BG2
            REG_BG2HOFS = (uint8_t)h;
            REG_BG2HOFS = (uint8_t)(h >> 8);
            REG_BG2VOFS = (uint8_t)v;
            REG_BG2VOFS = (uint8_t)(v >> 8);
            break;
        case 2: // BG3
            REG_BG3HOFS = (uint8_t)h;
            REG_BG3HOFS = (uint8_t)(h >> 8);
            REG_BG3VOFS = (uint8_t)v;
            REG_BG3VOFS = (uint8_t)(v >> 8);
            break;
        case 3: // BG4
            REG_BG4HOFS = (uint8_t)h;
            REG_BG4HOFS = (uint8_t)(h >> 8);
            REG_BG4VOFS = (uint8_t)v;
            REG_BG4VOFS = (uint8_t)(v >> 8);
            break;
    }
}

void vdp_hscroll(uint16_t plan, int16_t hscroll) {
	bgSetScroll(plan, -hscroll, prev_v);
	if(plan == VDP_PLAN_A)
	{
		//tileScrollX = -hscroll;
		//scroll stage back too
		//bgSetScroll(2, -hscroll, prev_v);
	}
    prev_h = -hscroll;
}

void vdp_hscroll_tile(uint16_t plan, int16_t *hscroll) {
	// For tile-based scrolling, we'd need HDMA, but for now just use first value
	bgSetScroll(plan, hscroll[0], prev_v);
    prev_h = hscroll[0];
}

void vdp_vscroll(uint16_t plan, int16_t vscroll) {
	bgSetScroll(plan, prev_h, vscroll);
	if(plan == VDP_PLAN_A)
	{
		//tileScrollX = vscroll;
		//scroll stage back too
		//bgSetScroll(2, prev_h, vscroll);
	}
    prev_v = vscroll;
}


// --- Sprites / OAM ---

void vdp_sprite_add(const VDPSprite *spr) {}
void vdp_sprites_add(const VDPSprite *spr, uint16_t num) {}
void vdp_sprites_clear() {}
void vdp_sprites_update() {}

// OAM Update - SNES native implementation (stub for now)
// OAM updates should be done via DMA using LoadOAMCopy during vblank
void oamUpdate(void) {
    // OAM updates are typically done via DMA during vblank
    // This is a stub - actual OAM updates should use LoadOAMCopy
}

// OAM Set sprite - SNES native implementation (stub for now)
// oamSet(id, x, y, priority, hFlip, vFlip, gfxOffset, paletteOffset)
// In snesXC, OAM should be updated via LoadOAMCopy during vblank
void oamSet(uint8_t id, int16_t x, int16_t y, uint8_t priority, uint8_t hFlip, uint8_t vFlip, uint16_t gfxOffset, uint8_t paletteOffset) {
    // Stub for PVSneslib oamSet function
    // In snesXC, OAM should be updated via LoadOAMCopy during vblank
    (void)id;
    (void)x;
    (void)y;
    (void)priority;
    (void)hFlip;
    (void)vFlip;
    (void)gfxOffset;
    (void)paletteOffset;
}

// --- Text / Fonts ---

void vdp_font_load(const uint32_t *tiles) {}
void vdp_font_pal(uint16_t pal) {}

void vdp_puts(uint16_t plan, const char *str, uint16_t x, uint16_t y) {
    // Stub - text rendering not implemented yet
    (void)plan; (void)str; (void)x; (void)y;
}

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len) {
    // Stub - text clearing not implemented yet
    (void)plan; (void)x; (void)y; (void)len;
}

uint16_t vdp_get_vblank(void) {
    // Return the vblank count
    return vblank_count;
}

// --- Debug / Console ---

// Minimal printf implementation - only supports %d, %u, %x, %X, %s, %c, %%
// This avoids pulling in the entire standard library
static void minimal_vsprintf(char *buf, const char *format, va_list ap) {
    char *p = buf;
    const char *f = format;
    char num_buf[32];
    
    while (*f != '\0') {
        if (*f == '%') {
            f++;
            switch (*f) {
                case 'd':
                case 'i': {
                    // On 65816, int is 16-bit, so use int not int32_t
                    int val = va_arg(ap, int);
                    int_to_str(num_buf, (int32_t)val);
                    const char *n = num_buf;
                    while (*n) *p++ = *n++;
                    break;
                }
                case 'u': {
                    // On 65816, unsigned int is 16-bit, so use unsigned int not uint32_t
                    unsigned int val = va_arg(ap, unsigned int);
                    uint_to_str(num_buf, (uint32_t)val, 10);
                    const char *n = num_buf;
                    while (*n) *p++ = *n++;
                    break;
                }
                case 'x':
                case 'X': {
                    // On 65816, unsigned int is 16-bit, so use unsigned int not uint32_t
                    unsigned int val = va_arg(ap, unsigned int);
                    uint8_t uppercase = (*f == 'X');
                    uint_to_str(num_buf, (uint32_t)val, 16);
                    const char *n = num_buf;
                    while (*n) {
                        if (uppercase && *n >= 'a' && *n <= 'z') {
                            *p++ = *n - 32;  // Convert to uppercase
                        } else {
                            *p++ = *n;
                        }
                        n++;
                    }
                    break;
                }
                case 's': {
                    const char *str = va_arg(ap, const char*);
                    if (str) {
                        while (*str) *p++ = *str++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(ap, int);
                    *p++ = c;
                    break;
                }
                case '%':
                    *p++ = '%';
                    break;
                default:
                    // Unknown format, output as-is
                    *p++ = '%';
                    if (*f) *p++ = *f;
                    break;
            }
            f++;
        } else {
            *p++ = *f++;
        }
    }
    *p = '\0';
}

// Console message function - writes formatted message to no$sns debug register
void consoleNocashMessage(const char *format, ...) {
    char text_buffer[64];
    va_list ap;
    
    // Format the string using minimal printf implementation
    va_start(ap, format);
    minimal_vsprintf(text_buffer, format, ap);
    va_end(ap);
    
    // Write each character to REG_DEBUG until null terminator
    const char *p = text_buffer;
    while (*p != '\0') {
        REG_DEBUG = *p;
        p++;
    }
}

// --- Audio (stubs for now) ---

// Process SPC700 audio (stub for now)
void spcProcess(void) {
    // Audio processing would go here
    // For now, just a stub
}

// Play sound via SPC700 (stub for now)
void spcPlaySound(uint8_t sound) {
    // Stub for PVSneslib spcPlaySound function
    // Use sound_play() from audio.h instead
    (void)sound;
}

// --- Background Mode & Layer Control ---

// Set background mode - SNES native implementation
// mode: 0-7 (BG mode)
// bgSize: tile size flags (bit 0-3 for BG1-4, 0=8x8, 1=16x16)
void setMode(uint8_t mode, uint8_t bgSize) {
    // REG_BGMODE format: DCBA emmm
    // D/C/B/A = tile size for BG4/BG3/BG2/BG1 (1=16x16, 0=8x8)
    // e = mode 1 BG3 priority bit
    // mmm = BG mode (0-7)
    uint8_t bgmode_value = (mode & 0x07) | ((bgSize & 0x0F) << 4);
    REG_BGMODE = bgmode_value;
    
    // Configure screen base addresses for tilemaps
    // BG1SC format: aaaa aayx
    // aaaaaa = screen base address >> 10 (in word addresses, 1KB boundaries)
    // y = vertical flip, x = horizontal flip
    // 0x6000 byte = 0x3000 word = 12 * 0x400, so bits 7-2 = 12 = 0x0C
    // For 32x32 screen, bits 1-0 = 00
    REG_BG1SC = 0x30;  // BG1 tilemap at 0x6000 (byte) = 0x3000 (word), 32x32 screen
    
    // 0x7000 byte = 0x3800 word = 14 * 0x400, so bits 7-2 = 14 = 0x0E
    REG_BG2SC = 0x38;  // BG2 tilemap at 0x7000 (byte) = 0x3800 (word), 32x32 screen
    
    // Configure tile data base addresses
    // BG12NBA format: bbbb aaaa
    // aaaa = BG1 tile address >> 12 (4KB boundaries in word addresses)
    // bbbb = BG2 tile address >> 12
    // 0x4000 byte = 0x2000 word = 2 * 0x1000, so both BG1 and BG2 = 2 = 0x02
    // Value = (BG2 << 4) | BG1 = (2 << 4) | 2 = 0x22
    REG_BG12NBA = 0x22;  // BG1 and BG2 tiles at 0x4000 (byte) = 0x2000 (word)
}

// Enable/disable background layer - SNES native implementation
// bg: layer number (0=BG1, 1=BG2, 2=BG3, 3=BG4)
// Note: The original implementation had inconsistent behavior.
// This version enables the specified layer by setting its bit in REG_TM.
void bgSetEnable(uint8_t bg) {
    // TM register controls which layers are enabled on main screen
    // TS register controls which layers are enabled on sub screen
    // Bit 0 = BG1, Bit 1 = BG2, Bit 2 = BG3, Bit 3 = BG4, Bit 4 = OBJ
    
    uint8_t current_tm = REG_TM;
    
    if(bg < 4) {
        // Enable specific BG layer (bits are active high, 1=enabled)
        uint8_t mask = 1 << bg;
        REG_TM = current_tm | mask;
    } else {
        // For values >= 4, enable all layers (BG1-4 + OBJ)
        REG_TM = 0x1F;
    }
}

// Initialize background tile set - SNES native implementation
// bg: background layer (0-3)
// tiles: pointer to tile graphics data
// palette: pointer to palette data
// tileoffset: offset for tile numbering
// tilesize: size of tile data in bytes
// palsize: size of palette data in bytes
// colors: number of colors (typically 16 for 4bpp)
// vramAddr: VRAM address for tile data (word address)
void bgInitTileSet(uint8_t bg, const void *tiles, const void *palette, 
                   uint16_t tileoffset, uint16_t tilesize, uint16_t palsize, 
                   uint16_t colors, uint16_t vramAddr) {
    // Load tiles to VRAM
    // LoadVram expects a byte address and divides by 2 internally to convert to word address
    // vramAddr is a word address, so we multiply by 2 to convert to byte address
    if(tiles && tilesize > 0) {
        uint16_t byteAddr = vramAddr << 1;
        // Log only the first 20 VRAM writes
        if (vram_log_count < 20) {
            // Log the LoadVram call (24-bit pointer: bank + offset)
            uint32_t ptr = (uint32_t)(uintptr_t)tiles;
            uint8_t bank = (ptr >> 16) & 0xFF;
            uint16_t offset = ptr & 0xFFFF;
            // Format with leading zeros manually since minimal printf doesn't support width specifiers
            char bank_str[3], offset_str[5], dest_str[5];
            uint_to_str(bank_str, bank, 16);
            uint_to_str(offset_str, offset, 16);
            uint_to_str(dest_str, byteAddr, 16);
            char bank_padded[3], offset_padded[5], dest_padded[5];
            pad_hex_str(bank_padded, bank_str, 2);
            pad_hex_str(offset_padded, offset_str, 4);
            pad_hex_str(dest_padded, dest_str, 4);
            consoleNocashMessage("LoadVram[%u]: bgInitTileSet bg=%u src=0x%s:%s dest=0x%s size=%u\n", 
                                vram_log_count, bg, bank_padded, offset_padded, dest_padded, tilesize);
            vram_log_count++;
        }
        LoadVram((const unsigned char *)tiles, byteAddr, tilesize);
    }
    
    // Load palette to CGRAM
    // Calculate CGRAM offset: typically palette slot is bg * 16 colors
    // Each color is 2 bytes, so offset = bg * colors * 2
    if(palette && palsize > 0) {
        uint16_t cgram_offset = bg * colors * 2;
        LoadCGRam((const unsigned char *)palette, cgram_offset, palsize);
    }
    
    // Note: tileoffset and other parameters may need additional handling
    // depending on how the game uses them. This is a basic implementation.
}