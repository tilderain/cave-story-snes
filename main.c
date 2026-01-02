#include <stdint.h>
#include "snes_regs_xc.h"
#include "data_converted_part1.h"
#include "data_converted_part2.h"
#include "vdp.h"

//---------------------------------------------------------------------------------
int main(void)
{
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
