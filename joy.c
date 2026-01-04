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

// Read pad state for port (0-1)
// Returns 16-bit button state
// SNES format: REG_JOY1L = axlr0000, REG_JOY1H = byetUDLR (active low)
// We need to invert and map to match KEY_* constants
uint16_t padsCurrent(uint8_t port) {
    uint8_t joy_l, joy_h;
    uint16_t result = 0;
    
    if(port == 0) {
        // REG_JOY1L ($4218) contains AXLR0000
        joy_l = REG_JOY1L;  
        // REG_JOY1H ($4219) contains BYetUDLR
        joy_h = REG_JOY1H;  
    } else {
        joy_l = REG_JOY2L;
        joy_h = REG_JOY2H;
    }
    
    // STOP: Do NOT invert (~) these registers.
    // The SNES hardware $4218 registers are already Active High.
    // 1 = Pressed, 0 = Released.
    
    // Map to KEY_* constants format
    
    // Directional buttons from high byte ($4219: BYetUDLR)
    // Masks should match bits 0-3 (R, L, D, U)
    if(joy_h & JOY_RIGHT_MASK) result |= KEY_RIGHT;
    if(joy_h & JOY_LEFT_MASK)  result |= KEY_LEFT;
    if(joy_h & JOY_DOWN_MASK)  result |= KEY_DOWN;
    if(joy_h & JOY_UP_MASK)    result |= KEY_UP;
    
    // Action buttons from high byte ($4219: BYetUDLR)
    // Masks should match bits 4-7 (Start, Sel, Y, B)
    if(joy_h & JOY_START_MASK)  result |= KEY_START;
    if(joy_h & JOY_SELECT_MASK) result |= KEY_SELECT;
    if(joy_h & JOY_Y_MASK)      result |= KEY_Y;
    if(joy_h & JOY_B_MASK)      result |= KEY_B;
    
    // Shoulder buttons from low byte ($4218: AXLR0000)
    // Masks should match bits 4-7 (R, L, X, A)
    // Note: The lower 4 bits are ID codes (usually 0), so we ignore them by using correct masks.
    if(joy_l & JOY_R_MASK) result |= KEY_R;
    if(joy_l & JOY_L_MASK) result |= KEY_L;
    if(joy_l & JOY_X_MASK) result |= KEY_X;
    if(joy_l & JOY_A_MASK) result |= KEY_A;
    
    return result;
}

void joy_update() {
	//iprintf("joy_update %d %d\n", joystate, oldstate);
	oldstate = joystate;
	joystate = padsCurrent(0);
	return;

}
