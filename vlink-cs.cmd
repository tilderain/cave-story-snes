STACKLEN   =?    0x400;
WRAMSTART  =? 0x7E0000;
WRAMSIZE   =?  0x20000;
LORAMSTART =?   0x0000;
LORAMSIZE  =?   0x2000;

/* ROMSIZE is 4MB */
ROMSIZE    =?  0x400000;

MEMORY
{
 /* 
    'out' defines the physical file layout.
    We pack sections into this container sequentially.
 */
 out : org = 0x0000, len = 0x1000000

 /* Standard Zero Page / RAM regions */
 zero : org = 0 , len = 256
 loram : org = LORAMSTART + 256, len = (LORAMSIZE-256-STACKLEN)
 wram : org = (WRAMSTART + LORAMSIZE), len = WRAMSIZE-LORAMSIZE
 
 /* 
    FIX: Map 'lrom', 'header', and 'vectors' to Bank $00 addresses.
    This ensures 16-bit vectors/pointers resolve correctly.
 */
 header : org = 0x00FFC0, len = 0x20
 vectors : org = 0x00FFE0, len = 0x20
 
 /* 
    xrom: The first 32KB of the ROM file.
    Mapped to Bank $C0 (FastROM) for 24-bit access.
 */
 xrom : org = 0xC00000, len = 0x8000
 
 /* 
    lrom: The upper 32KB of the first bank.
    Mapped to Bank $00 (SlowROM mirror) to satisfy 16-bit vector requirements.
 */
 lrom : org = 0x008000, len = 0x8000 - 0x40
 
 /* 
    hrom: The rest of the ROM (Banks $C1+).
    Mapped to FastROM addresses.
 */
 hrom : org = 0xC10000, len = ROMSIZE - 0x10000
}

SECTIONS
{
  /* 
     1. Lower 32KB of the first bank (mapped to $C00000).
     Contains constructors/destructors and specific far/huge data.
  */
  xrom  : {*(.ctors) *(.dtors) *(*_text.far.*) *(*_rodata.far.*) *(*_text.huge.*) *(*_rodata.huge.*) } >xrom AT>out
  
  /* Fill up to the 32KB mark ($C08000) */
  fillx : {.=0xC08000;} >xrom AT>out
  
  /* 
     2. Upper 32KB of the first bank (mapped to $008000).
     *** CHANGED *** 
     Only keep startup code, near specific code, and vectors here.
     Standard 'text' has been moved to 'from' (hrom) for FastROM execution.
  */
  nrom  : {
    *(*_text.startup) 
    *(*_text.near.*) 
    *(*_rodata.near.*) 
    /* removed *(text) and *(*_text*) from here */
  } >lrom AT>out
  
  /* Fill up to the header start ($00FFC0) */
  fill0 : {.=0x00FFC0;} >lrom AT>out
  
  header : {EXTERN(___header_hirom_ntsc) *(*header)} >header AT>out
  vectors : {EXTERN(___vecs) *(*vectors)} >vectors AT>out

  /* 
     3. Remaining ROM Banks (mapped to $C10000+).
     Bulk assets and code.
     *** CHANGED ***
     Standard code (*(text)) is now placed here to run in FastROM.
  */
  from (BANKSIZE=65536): {
    *(text)          /* Standard C code moved here */
    *(*_text*)       /* Other text sections moved here */
    *(*_rodata.*)    /* Standard rodata moved here */
    *(*_text.far.*) 
    *(*_rodata.far.*)
  } >hrom AT>out
  
  hrom : {*(*_text.huge.*) *(*_rodata.huge.*) } >hrom AT>out

  /* RAM Section Definitions */
  ndata: {*(*_data.near.*)} >loram AT>out
  nbss (NOLOAD): {*(*_bss.near.*)} >loram
  fdata (BANKSIZE=65536): { *(SORT_BY_SIZE(*_data.far.*))} >wram AT>out
  fbss (BANKSIZE=65536) (NOLOAD): { *(SORT_BY_SIZE(*_bss.far.*))} >wram
  hdata: { *(*_data.huge.*)} >wram AT>out
  hbss (NOLOAD): { *(*_bss.huge.*)} >wram

  /* Final pad to ensure ROM size */
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