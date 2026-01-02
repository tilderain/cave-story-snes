STACKLEN   =?    0x400;
WRAMSTART  =? 0x7E0000;
WRAMSIZE   =?  0x20000;
LORAMSTART =?   0x0000;
LORAMSIZE  =?   0x2000;

/* CHANGE: Increased ROMSIZE from 0x80000 (512KB) to 0x400000 (4MB) */
ROMSIZE    =?  0x400000;

MEMORY
{
 /* 'out' file container size */
 out : org = 0x0000, len = 0x1000000

 zero : org = 0 , len = 256

 loram : org = LORAMSTART + 256, len = (LORAMSIZE-256-STACKLEN)
 wram : org = (WRAMSTART + LORAMSIZE), len = WRAMSIZE-LORAMSIZE
 header : org = 0xFFC0, len = 0x20
 vectors : org = 0xFFE0, len = 0x20

 /* Bank $40 (First 32KB of HiROM) */
 xrom : org = 0x400000, len = 0x8000
 
 /* Bank $00 (Lower 32KB mirror for startup/interrupts) */
 lrom : org = 0x008000, len = 0x8000 - 0x40
 
 /* 
    HiROM Data Area (Banks $41 through $7F). 
    Starts at 0x410000. 
    Length = Total Size (4MB) - First Bank (64KB alignment adjustment) 
 */
 hrom : org = 0x410000, len = ROMSIZE - 0x10000
}

SECTIONS
{
  /* Put startup code and critical far data in the first bank (xrom) */
  xrom  : {*(.ctors) *(.dtors) *(*_text.far.*) *(*_rodata.far.*) *(*_text.huge.*) *(*_rodata.huge.*) } >xrom AT>out
  fillx : {.=0x408000;} >xrom AT>out
  
  /* Near code/data goes to low mirror */
  nrom  : {*(*_text.startup) *(*_text.near.*) *(*_rodata.near.*) *(.ctors) *(.dtors) *(text) *(*_text*) *(*_rodata.*) } >lrom AT>out
  fill0 : {.=0xFFC0;} >lrom AT>out
  header : {EXTERN(___header_hirom_ntsc) *(*header)} >header AT>out
  vectors : {EXTERN(___vecs) *(*vectors)} >vectors AT>out

  /* 
     Bulk Assets:
     This is where your map data, graphics, and scripts go.
     It now writes to the expanded 'hrom' region.
  */
  from (BANKSIZE=65536): {*(*_text.far.*) *(*_rodata.far.*)} >hrom AT>out
  hrom : {*(*_text.huge.*) *(*_rodata.huge.*) } >hrom AT>out

  /* RAM definitions */
  ndata: {*(*_data.near.*)} >loram AT>out
  nbss (NOLOAD): {*(*_bss.near.*)} >loram
  fdata (BANKSIZE=65536): { *(SORT_BY_SIZE(*_data.far.*))} >wram AT>out
  fbss (BANKSIZE=65536) (NOLOAD): { *(SORT_BY_SIZE(*_bss.far.*))} >wram
  hdata: { *(*_data.huge.*)} >wram AT>out
  hbss (NOLOAD): { *(*_bss.huge.*)} >wram

  fill : { BYTE(0); .=ALIGN(32768)-1; BYTE(0); } >out

  zpage (NOLOAD) : {*(zpage) *(zp1) *(zp2)} >zero

  ___heap = MAX(ADDR(fbss) + SIZEOF(fbss),ADDR(hbss)+SIZEOF(hbss));
  ___heapend = WRAMSTART+WRAMSIZE;

  __NDS = ADDR(ndata);
  __NDE = ADDR(ndata) +  SIZEOF(ndata);
  __NDC = MAX(ADDR(from)+SIZEOF(from),ADDR(hrom)+SIZEOF(hrom));

  __NBS = ADDR(nbss);
  __NBE = ADDR(nbss) + SIZEOF(nbss);

  __FDS = ADDR(fdata);
  __FDE = ADDR(fdata) +  SIZEOF(fdata);
  __FDC = __NDC+SIZEOF(ndata);

  __FBS = ADDR(fbss);
  __FBE = ADDR(fbss) + SIZEOF(fbss);

  __HDS = ADDR(hdata);
  __HDE = ADDR(hdata) +  SIZEOF(hdata);
  __HDC = __FDC+SIZEOF(fdata);

  __HBS = ADDR(hbss);
  __HBE = ADDR(hbss) + SIZEOF(hbss);

  __DBR_init = 0;
}
