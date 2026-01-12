#include "common.h"
//#include "memory.h"
#include "snes_regs_xc.h"
#include "joy.h"
#include "system.h"
#include "vdp.h"

//#include "gba.h"

/*const uint16_t btn[12] = {
	BUTTON_A, BUTTON_B, BUTTON_START, BUTTON_START,
	BUTTON_RIGHT, BUTTON_LEFT, BUTTON_UP, BUTTON_DOWN,
	BUTTON_Z, BUTTON_Y, BUTTON_X, BUTTON_MODE
};*/

const uint16_t btn[12] = {
	BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,
	BUTTON_B, BUTTON_C, BUTTON_A, BUTTON_START,
	BUTTON_Z, BUTTON_Y, BUTTON_X, BUTTON_MODE
};

const char btnName[12][4] = {
	"Up", "Dn", "Lt", "Rt",
	"B", "C", "A", "St",
	"Z", "Y", "X", "Md"
};

uint8_t joytype = JOY_TYPE_PAD6;
uint16_t joystate, oldstate = 0;

void joy_init() {
	joystate = oldstate = 0;
	return;

}
#define REG_JOYxLH(a) (((volatile uint16_t *)0x4218)[(a)])
// Read pad state for port (0-1)
// Returns 16-bit button state
// SNES format: REG_JOY1L = axlr0000, REG_JOY1H = byetUDLR (active low)
// We need to invert and map to match KEY_* constants
uint16_t padsCurrent(uint8_t port) {
    // 1. Wait for Auto-Read to complete.
    // Documentation: "Official guides advise to check if automatic reading has finished 
    // by reading the HVBJOY register ($4212)."
    // Bit 0 of $4212 is high while the controller is being read.
    while (REG_HVBJOY & 1); 

    uint16_t joy_raw = 0;
    uint16_t result = 0;

    // 2. Read the full 16-bit state at once.
    // Documentation: "The values read from the controllers are made available 
    // via the JOY1-JOY4 registers ($4218-421F)."
    if (port == 0) {
        // Reads $4218 (Low) and $4219 (High) as a single word
        joy_raw = REG_JOYxLH(0); 
    } else {
        // Reads $421A (Low) and $421B (High) as a single word
        joy_raw = REG_JOYxLH(1); 
    }

    // 3. Map bits to KEY constants.
    // In a 16-bit Little Endian read:
    // Bits 15-8 come from the High Byte ($4219): B, Y, Sel, Start, Up, Down, Left, Right
    // Bits 7-0  come from the Low Byte ($4218):  A, X, L, R, 0, 0, 0, 0
    
    // High Byte ($4219)
    if (joy_raw & 0x8000) result |= KEY_B;      // Bit 15
    if (joy_raw & 0x4000) result |= KEY_Y;      // Bit 14
    if (joy_raw & 0x2000) result |= KEY_SELECT; // Bit 13
    if (joy_raw & 0x1000) result |= KEY_START;  // Bit 12
    if (joy_raw & 0x0800) result |= KEY_UP;     // Bit 11
    if (joy_raw & 0x0400) result |= KEY_DOWN;   // Bit 10
    if (joy_raw & 0x0200) result |= KEY_LEFT;   // Bit 9
    if (joy_raw & 0x0100) result |= KEY_RIGHT;  // Bit 8

    // Low Byte ($4218)
    if (joy_raw & 0x0080) result |= KEY_A;      // Bit 7
    if (joy_raw & 0x0040) result |= KEY_X;      // Bit 6
    if (joy_raw & 0x0020) result |= KEY_L;      // Bit 5
    if (joy_raw & 0x0010) result |= KEY_R;      // Bit 4

    return result;
}
void joy_update() {
	//iprintf("joy_update %d %d\n", joystate, oldstate);
	oldstate = joystate;
	joystate = padsCurrent(0);
	return;

}
