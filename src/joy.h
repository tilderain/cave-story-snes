#include "snes_regs_xc.h"

/*#define BUTTON_UP       0x0001
#define BUTTON_DOWN     0x0002
#define BUTTON_LEFT     0x0004
#define BUTTON_RIGHT    0x0008
#define BUTTON_A        0x0040
#define BUTTON_B        0x0010
#define BUTTON_C        0x0020
#define BUTTON_START    0x0080
#define BUTTON_X        0x0400
#define BUTTON_Y        0x0200
#define BUTTON_Z        0x0100
#define BUTTON_MODE     0x0800 */

#define BUTTON_UP       0x0800
#define BUTTON_DOWN     0x0400
#define BUTTON_LEFT     (1<<5)
#define BUTTON_RIGHT    (1<<4)
#define BUTTON_A        0x0040
#define BUTTON_B        (1<<15) //GBA B
#define BUTTON_C        (1<<7) //GBA A
#define BUTTON_START    (1<<12) //GBA Start
#define BUTTON_X        (1<<13) //GBA Select
#define BUTTON_Y        (1<<5) //GBA L
#define BUTTON_Z        (1<<4) //GBA R
#define BUTTON_MODE     0x0800

#define BUTTON_DIR      0x000F
#define BUTTON_BTN      0x0FF0

// SNES button definitions (compatible with PVSneslib KEY_* constants)
// These map to SNES joypad register bits

    
// Native SNES Controller Bitmasks
// Matches the 16-bit Auto-Read register ($4218) directly.

// Low Byte ($4218): A, X, L, R, 0, 0, 0, 0
#define KEY_R       0x0010  // Bit 4
#define KEY_L       0x0020  // Bit 5
#define KEY_X       0x0040  // Bit 6
#define KEY_A       0x0080  // Bit 7

// High Byte ($4219): B, Y, Select, Start, Up, Down, Left, Right
#define KEY_RIGHT   0x0100  // Bit 8
#define KEY_LEFT    0x0200  // Bit 9
#define KEY_DOWN    0x0400  // Bit 10
#define KEY_UP      0x0800  // Bit 11
#define KEY_START   0x1000  // Bit 12
#define KEY_SELECT  0x2000  // Bit 13
#define KEY_Y       0x4000  // Bit 14
#define KEY_B       0x8000  // Bit 15

  
#define JOY_TYPE_PAD3           0x00
#define JOY_TYPE_PAD6           0x01

extern const uint16_t btn[12];
extern const char btnName[12][4];

#define joy_pressed(b) (((joystate&(b))&&((~oldstate)&(b))))
#define joy_released(b) ((((~joystate)&(b))&&(oldstate&(b))))
#define joy_down(b) ((joystate&(b)))

extern uint8_t joytype;
extern uint16_t joystate, oldstate;

void joy_init();
void joy_update();
uint16_t padsCurrent(uint8_t port); // Read pad state for port (0-1) - returns 16-bit button state
