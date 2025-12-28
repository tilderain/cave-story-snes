#include "vdp.h"
#include <snes.h> // Needed for consoleInit and consoleDrawText

// --- Initialization & Control ---

void vdp_init() {
    // Redirect to SNES console init for debugging
    consoleInit();
    consoleDrawText(1, 1, "VDP Init Called");
}

void vdp_vsync() {
    WaitForVBlank();
}

void vdp_set_display(uint8_t enabled) {
    if(enabled) setScreenOn();
    else setScreenOn(); // Keep screen on for debug text even if game requests off
}

void vdp_set_autoinc(uint8_t val) {}
void vdp_set_scrollmode(uint8_t hoz, uint8_t vert) {}
void vdp_set_highlight(uint8_t enabled) {}
void vdp_set_backcolor(uint8_t index) {}
void vdp_set_window(uint8_t x, uint8_t y) {}

// --- DMA / Memory Transfer ---

//void vdp_dma_vram(uint32_t from, uint16_t to, uint16_t len) {}
//void vdp_dma_cram(uint32_t from, uint16_t to, uint16_t len) {}
//void vdp_dma_vsram(uint32_t from, uint16_t to, uint16_t len) {}

// --- Tile Management ---

//void vdp_tiles_load(volatile const uint32_t *data, uint16_t index, uint16_t num) {}
//void vdp_tiles_load_from_rom(volatile const uint32_t *data, uint16_t index, uint16_t num) {}

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
void vdp_color(uint16_t index, uint16_t color) {}
void vdp_colors_next(uint16_t index, const uint16_t *values, uint16_t count) {}
void vdp_color_next(uint16_t index, uint16_t color) {}

uint16_t vdp_fade_step() {
    return 0; // Return 0 to indicate fade is "done" immediately
}

void vdp_fade(const uint16_t *src, const uint16_t *dst, uint16_t speed, uint8_t async) {}

// --- Scroll ---
uint16_t prev_h = 0;
uint16_t prev_v = 0;

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
	bgSetScroll(plan, hscroll, prev_v);
    prev_h = hscroll;
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


// --- Sprites ---

void vdp_sprite_add(const VDPSprite *spr) {}
void vdp_sprites_add(const VDPSprite *spr, uint16_t num) {}
void vdp_sprites_clear() {}
void vdp_sprites_update() {}

// --- Text / Fonts ---

//void vdp_font_load(const uint32_t *tiles) {}
void vdp_font_pal(uint16_t pal) {}

void vdp_puts(uint16_t plan, const char *str, uint16_t x, uint16_t y) {
    // Map Genesis VDP coordinates to SNES Console
    // Genesis 40x28, SNES 32x28
    if (x < 32 && y < 28) {
        consoleDrawText(x, y, (char*)str);
    }
}

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len) {
    // Crude implementation to erase text on SNES console
    if (y >= 28) return;
	uint16_t i;
    for( i=0; i<len; i++) {
        if(x+i < 32) consoleDrawText(x+i, y, " ");
    }
}