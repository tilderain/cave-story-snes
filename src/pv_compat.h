    
#include "snes_regs_xc.h"


/** \brief The shift to apply to map base when storing it in a tile map location register */
#define SC_BASE_SHIFT 2

/** \brief Macro to set the tile map address in tile map location */
#define SC_TILE_BASE(base) ((base) << SC_BASE_SHIFT)

/* Bit defines for the background control registers */
#define SC_32x32 (0 << 0) /** \brief 32 x 32 tile size */
#define SC_64x32 (1 << 0) /** \brief 64 x 32 tile size */
#define SC_32x64 (2 << 0) /** \brief 32 x 64 tile size */
#define SC_64x64 (3 << 0) /** \brief 64 x 64 tile size */

#define BG_MODE0 (0 << 0) /** \brief 4-color     4-color     4-color     4-color   ;Normal */
//#define BG_MODE1 (1 << 0) /** \brief 16-color    16-color    4-color     -         ;Normal */
#define BG_MODE2 (2 << 0) /** \brief 16-color    16-color    (o.p.t)     -         ;Offset-per-tile */
#define BG_MODE3 (3 << 0) /** \brief 256-color   16-color    -           -         ;Normal */
#define BG_MODE4 (4 << 0) /** \brief 256-color   4-color     (o.p.t)     -         ;Offset-per-tile */
#define BG_MODE5 (5 << 0) /** \brief 16-color    4-color     -           -         ;512-pix-hires */
#define BG_MODE6 (6 << 0) /** \brief 16-color    -           (o.p.t)     -         ;512-pix plus Offs-p-t */
#define BG_MODE7 (7 << 0) /** \brief 256-color   EXTBG       -           -         ;Rotation/Scaling */

#define BG1_TSIZE8x8 (0 << 4)
#define BG2_TSIZE8x8 (0 << 5)
#define BG3_TSIZE8x8 (0 << 6)
#define BG4_TSIZE8x8 (0 << 7)

#define BG1_TSIZE16x16 (1 << 4)
#define BG2_TSIZE16x16 (1 << 5)
#define BG3_TSIZE16x16 (1 << 6)
#define BG4_TSIZE16x16 (1 << 7)

#define BG1_ENABLE (1 << 0)
#define BG2_ENABLE (1 << 1)
#define BG3_ENABLE (1 << 2)
#define BG4_ENABLE (1 << 3)
#define OBJ_ENABLE (1 << 4)

#define BG_4COLORS0 32
#define BG_4COLORS 4
#define BG_16COLORS 16
#define BG_256COLORS 256
#define BG3_MODE1_PRIORITY_HIGH (1 << 3)

#define BG_TIL_PRIO                 (1<<13)     /** \brief 1 for priority in tile attributes */
#define BG_TIL_PAL(n)               (n<<10)     /** \brief 0 to 7 for palette in tile attributes */
#define BG_TIL_NUM(n)               (n<<0)      /** \brief 0 to nnn for tile number in tile attributes */
#define BG_TIL_FLIPX                (1<<14)     /** \brief 1 to flip on X in tile attributes */
#define BG_TIL_FLIPY                (1<<15)     /** \brief 1 to flip on Y in tile attributes */

// Shadow variables to maintain state
static uint16_t bg_gfx_addr[4] = {0, 0, 0, 0}; // Stores VRAM addresses for BG 0-3
static uint8_t bg_enabled_state = 0;           // Stores the TM register state

  
