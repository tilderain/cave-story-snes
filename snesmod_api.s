;*
;* Copyright 2009 Mukunda Johnson (mukunda.com)
;* 
;* This file is part of SNESMOD - gh.mukunda.com/snesmod
;*
;* VASM Conversion (C-Callable / PVSnesLib Compatible)
;*

    ; Global exports
    xdef spcBoot
    xdef spcSetBank
    xdef spcLoad
    xdef spcTest
    xdef spcPlay
    xdef spcStop
    xdef spcPauseMusic
    xdef spcResumeMusic
    xdef spcReadStatus
    xdef spcReadPosition
    xdef spcGetCues

    xdef spcSetModuleVolume
    xdef spcFadeModuleVolume
    xdef spcLoadEffect
    xdef spcEffect

    xdef spcFlush
    xdef spcProcess

    xdef spcSetSoundTable
    xdef spcAllocateSoundRegion
    xdef spcPlaySound
    xdef spcPlaySoundV
    xdef spcPlaySoundEx
    xdef spcGetMusicPosition

    xref CART_HEADER
    xref SM_SPC

;----------------------------------------------------------------------
; soundbank defs
;----------------------------------------------------------------------

    ifdef HIROM
SB_SAMPCOUNT    = $0000
SB_MODCOUNT     = $0002
SB_MODTABLE     = $0004
SB_SRCTABLE     = $0184
    else
SB_SAMPCOUNT    = $8000
SB_MODCOUNT     = $8002
SB_MODTABLE     = $8004
SB_SRCTABLE     = $8184
    endif

REG_APUIO0      = $2140 ; Sound Register                        1B/RW
REG_APUIO1      = $2141 ; Sound Register                        1B/RW
REG_APUIO2      = $2142 ; Sound Register                        1B/RW
REG_APUIO3      = $2143 ; Sound Register                        1B/RW
REG_SLHV        = $2137 ; Software Latch For H/V Counter        1B/R
REG_OPVCT       = $213D ; Y Scanline Location                   1B/R D

REG_NMI_TIMEN   = $4200


;----------------------------------------------------------------------
; spc commands
;----------------------------------------------------------------------

CMD_LOAD    = $00
CMD_LOADE   = $01
CMD_VOL     = $02
CMD_PLAY    = $03
CMD_STOP    = $04
CMD_MVOL    = $05
CMD_FADE    = $06
CMD_RES     = $07
CMD_FX      = $08
CMD_TEST    = $09
CMD_SSIZE   = $0A
CMD_PAUSE   = $0A
CMD_RESUME  = $0B

;----------------------------------------------------------------------

; process for 5 scanlines
PROCESS_TIME = 5
INIT_DATACOPY = 13

SPC_BOOT = $0400 ; spc entry/load address

;======================================================================
    section "zpage", "aurwz"
;======================================================================

spc_ptr:    ds 3
spc_v:      ds 1
spc_bank:   ds 1

spc1:       ds 2
spc2:       ds 2

spc_fread:  ds 1
spc_fwrite: ds 1

; port record [for interruption]
spc_pr:     ds 4

digi_src:   ds 3
digi_src2:  ds 3

SoundTable: ds 3

;======================================================================
    section "_bss.far.snesmod", "aurw"
;======================================================================

spc_fifo:       ds 256  ; 128-byte command fifo
spc_sfx_next:   ds 1
spc_q:          ds 1

digi_init:      ds 1
digi_pitch:     ds 1
digi_vp:        ds 1
digi_remain:    ds 2
digi_active:    ds 1
digi_copyrate:  ds 1


;======================================================================
    section "text", "acrx"
;======================================================================

    x16     ; Tell vasm Index regs are 16-bit
    a8      ; Tell vasm Accumulator is 8-bit

;**********************************************************************
;* void spcBoot(void)
;* C-Callable
;**********************************************************************
spcBoot:            
    php               
    sei
    phb               
    sep #$20
    lda #$0
    sta REG_NMI_TIMEN 
    pha
    plb             ; change bank address to 0

    x16
    a8

;----------------------------------------------------------------------
.wwait:
    ldx REG_APUIO0  ; wait for 'ready signal from SPC
    cpx #$BBAA      ;
    bne .wwait      ;--------------------------------------
    stx REG_APUIO1  ; start transfer:
    ldx #SPC_BOOT   ; port1 = !0
    stx REG_APUIO2  ; port2,3 = transfer address
    lda #$CC        ; port0 = 0CCh
    sta REG_APUIO0  ;--------------------------------------
.wspc:
    cmp REG_APUIO0  ; wait for SPC
    bne .wspc       ;
;----------------------------------------------------------------------
; ready to transfer
;----------------------------------------------------------------------
    ; Use > to force long addressing (24-bit)
    lda >SM_SPC    ; read first byte
    xba         ;
    lda #0      ;
    ldx #1      ;
    bra sb_start    ;
;----------------------------------------------------------------------
; transfer data
;----------------------------------------------------------------------
sb_send:
;----------------------------------------------------------------------
    xba                 ; swap DATA into A
    lda >SM_SPC, x ; read next byte
    inx                 ; swap DATA into B
    xba                 ;--------------------------------------
.wspc2:
    cmp REG_APUIO0      ; wait for SPC
    bne .wspc2          ;--------------------------------------
    inc                 ; increment counter (port0 data)
;----------------------------------------------------------------------
sb_start:
;----------------------------------------------------------------------
    rep #$20        ; write port0+port1 data
    a16             ; Tell vasm A is 16-bit
    sta REG_APUIO0  ;
    sep #$20        ;--------------------------------------
    a8              ; Tell vasm A is 8-bit
    
    ; Note: Assuming SM_SPC_SIZE is defined in your symbols or calculated
    cpx #SM_SPC_SIZE   
    
    bcc sb_send             ;
;----------------------------------------------------------------------
; all bytes transferred
;----------------------------------------------------------------------
.wspc3:
    cmp REG_APUIO0  ; wait for SPC
    bne .wspc3      ;--------------------------------------
    inc             ; add 2 or so... 
    inc             ; 
                ; mask data so invalid 80h message wont get sent
    stz REG_APUIO1  ; port1=0
    ldx #SPC_BOOT   ; port2,3 = entry point
    stx REG_APUIO2  ;
    sta REG_APUIO0  ; write P0 data
                ;--------------------------------------
.fsync:
    cmp REG_APUIO0  ; final sync
    bne .fsync      ;--------------------------------------
    stz REG_APUIO0
    
    ; Exhaustive initialization (matching File 2)
    stz spc_v
    stz spc_q
    stz spc_fwrite
    stz spc_fread
    stz spc_sfx_next
    
    stz spc_pr+0
    stz spc_pr+1
    stz spc_pr+2
    stz spc_pr+3

    stz spc_ptr+0
    stz spc_ptr+1
    stz spc_ptr+2
    stz spc_bank

    stz spc1+0
    stz spc1+1
    stz spc2+0
    stz spc2+1

    stz digi_src+0
    stz digi_src+1
    stz digi_src+2
    stz digi_src2+0
    stz digi_src2+1
    stz digi_src2+2

    stz SoundTable+0
    stz SoundTable+1
    stz SoundTable+2

    stz spc_sfx_next

    stz digi_init
    stz digi_pitch
    stz digi_vp
    stz digi_remain+0
    stz digi_remain+1
    stz digi_active
    stz digi_copyrate
    
    ; Clear FIFO
    ldx #$0
    lda #$0
.fifo_clear:
    sta spc_fifo,x
    inx
    cpx #$ff
    bne .fifo_clear

;----------------------------------------------------------------------
; driver installation successful
;----------------------------------------------------------------------
    plb
    cli
    plp
    rtl         ; Return Long
;----------------------------------------------------------------------

;**********************************************************************
; void spcSetBank(u8 *bank)
;**********************************************************************
spcSetBank:
    php
    phb
    sep #$20
    lda #$0
    pha
    plb ; change bank address to 0
    
    ; Stack: [00][OldB][OldP][RetL][RetH][RetB]
    ; Arg u8* bank is at 6,s (L), 7,s (H), 8,s (Bank)
    
    lda 8,s     ; index of bank -> 6..7 adrr, 8 bank
    sta spc_bank
    
    plb
    plp
    rtl
    
; increment memory pointer by 2
incptr macro
    iny
    iny
    
    ifndef HIROM
    bmi \@_catch_overflow
    inc spc_ptr+2
    ldy #$8000
    else
    bne \@_catch_overflow
    inc spc_ptr+2
    endif

\@_catch_overflow:
    endm

;**********************************************************************
; void spcLoad(u16 musIndex)
;**********************************************************************
spcLoad:
;----------------------------------------------------------------------
    php
    phb

    sep #$20
    lda #$0
    pha
    plb ; change bank address to 0

    rep #$30
    a16
    lda 6,s     ; module_id
    tax
    sep #$20
    a8

    phx             ; flush fifo!
    jsr xspcFlush        ;
    plx             ;
    
    phx
    ldy #SB_MODTABLE
    sty spc2
    jsr get_address
    rep #$20
    a16             ; Tell vasm A is 16-bit
    lda [spc_ptr], y    ; X = MODULE SIZE
    tax
    
    incptr
    
    lda [spc_ptr], y    ; read SOURCE LIST SIZE
    
    incptr
    
    sty spc1        ; pointer += listsize*2
    asl         ;
    adc spc1        ;
    ifndef HIROM
    bmi .hirom_chk      ;
    ora #$8000      ; FIXED: A is 16-bit here
    else
    bcc .hirom_chk
    endif
    inc spc_ptr+2   ;
.hirom_chk:
    tay         ;
    
    sep #$20        ;
    a8              ; Tell vasm A is 8-bit
    lda spc_v       ; wait for spc
    pha         ;
.wloop1:
    cmp REG_APUIO1  ;
    bne .wloop1     ;------------------------------
    lda #CMD_LOAD   ; send LOAD message
    sta REG_APUIO0  ;
    pla         ;
    eor #$80        ;
    ora #$01        ;
    sta spc_v       ;
    sta REG_APUIO1  ;------------------------------
.wloop2:
    cmp REG_APUIO1  ; wait for spc
    bne .wloop2     ;------------------------------
    jsr do_transfer
    
    ;------------------------------------------------------
    ; transfer sources
    ;------------------------------------------------------
    
    plx
    ldy #SB_MODTABLE
    sty spc2
    jsr get_address
    incptr
    
    rep #$20        ; x = number of sources
    a16             ; Tell vasm A is 16-bit
    lda [spc_ptr], y    ;
    tax         ;
    
    incptr
    
transfer_sources:
    
    lda [spc_ptr], y    ; read source index
    sta spc1        ;
    
    incptr
    
    phy         ; push memory pointer
    sep #$20        ; and counter
    a8              ; Tell vasm A is 8-bit
    lda spc_ptr+2   ;
    pha         ;
    phx         ;
    
    jsr transfer_source
    
    plx         ; pull memory pointer
    pla         ; and counter
    sta spc_ptr+2   ;
    ply         ;
    
    dex
    bne transfer_sources
.no_more_sources:

    stz REG_APUIO0  ; end transfers
    lda spc_v       ;
    eor #$80        ;
    sta spc_v       ;
    sta REG_APUIO1  ;-----------------
.wloop3:
    cmp REG_APUIO1  ; wait for spc
    bne .wloop3     ;-----------------
    sta spc_pr+1
    stz spc_sfx_next    ; reset sfx counter
    
    plb
    plp
    rtl
    
;--------------------------------------------------------------
; spc1 = source index
;--------------------------------------------------------------
transfer_source:
;--------------------------------------------------------------
    
    ldx spc1
    ldy #SB_SRCTABLE
    sty spc2
    jsr get_address
    
    lda #$01        ; port0=01h
    sta REG_APUIO0  ;
    rep #$20        ; x = length (bytes->words)
    a16             ; Tell vasm A is 16-bit
    lda [spc_ptr], y    ;
    incptr          ;
    inc             ; 
    lsr         ;
    tax         ;
    lda [spc_ptr], y    ; port2,3 = loop point
    sta REG_APUIO2
    incptr
    sep #$20
    a8              ; Tell vasm A is 8-bit
    
    lda spc_v       ; send message
    eor #$80        ;   
    ora #$01        ;
    sta spc_v       ;
    sta REG_APUIO1  ;-----------------------
.wloop:
    cmp REG_APUIO1  ; wait for spc
    bne .wloop      ;-----------------------
    cpx #0
    beq end_transfer    ; if datalen != 0
    bra do_transfer ; transfer source data
    
;--------------------------------------------------------------
; spc_ptr+y: source address
; x = length of transfer (WORDS)
;--------------------------------------------------------------
transfer_again:
    eor #$80        ;
    sta REG_APUIO1  ;
    sta spc_v       ;
    incptr          ;
.wloop:
    cmp REG_APUIO1  ;
    bne .wloop      ;
;--------------------------------------------------------------
do_transfer:
;--------------------------------------------------------------

    rep #$20        ; transfer 1 word
    a16             ; Tell vasm A is 16-bit
    lda [spc_ptr], y    ;
    sta REG_APUIO2  ;
    sep #$20        ;
    a8              ; Tell vasm A is 8-bit
    lda spc_v       ;
    dex         ;
    bne transfer_again  ;
    
    incptr

end_transfer:
    lda #0      ; final word was transferred
    sta REG_APUIO1  ; write p1=0 to terminate
    sta spc_v       ;
.wloop:
    cmp REG_APUIO1  ;
    bne .wloop      ;
    sta spc_pr+1
    rts

;--------------------------------------------------------------
; spc2 = table offset
; x = index
;
; returns: spc_ptr = 0,0,bank, Y = address
get_address:
;--------------------------------------------------------------

    lda spc_bank    ; spc_ptr = bank:SB_MODTABLE+module_id*3
    sta spc_ptr+2   ;
    rep #$20        ;
    a16             ; Tell vasm A is 16-bit
    stx spc1        ;
    txa         ;
    asl         ;
    adc spc1        ;
    adc spc2        ;
    sta spc_ptr     ;
    
    lda [spc_ptr]   ; read address
    pha         ;
    sep #$20        ;
    a8              ; Tell vasm A is 8-bit
    ldy #2      ;
    lda [spc_ptr],y ; read bank#
    
    clc         ; spc_ptr = long address to module
    adc spc_bank    ;
    sta spc_ptr+2   ;
    ply         ;
    stz spc_ptr
    stz spc_ptr+1
    rts         ;
    
;**********************************************************************
;* void spcLoadEffect(u16 sfxIndex)
;*
;* load effect into memory
;**********************************************************************
spcLoadEffect:
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb ; change bank address to 0

    rep #$30
    a16
    lda 6,s     ; id
    tax
    sep #$20
    a8

    ldy #SB_SRCTABLE    ; get address of source
    sty spc2        ;
    jsr get_address ;--------------------------------------
    lda spc_v       ; sync with SPC
.wloop1:
    cmp REG_APUIO1  ;
    bne .wloop1     ;--------------------------------------
    lda #CMD_LOADE  ; write message
    sta REG_APUIO0  ;--------------------------------------
    lda spc_v       ; dispatch message and wait
    eor #$80        ;
    ora #$01        ;
    sta spc_v       ;
    sta REG_APUIO1  ;
.wloop2:
    cmp REG_APUIO1  ;
    bne .wloop2     ;--------------------------------------
    rep #$20        ; x = length (bytes->words)
    a16             ; Tell vasm A is 16-bit
    lda [spc_ptr], y    ;
    inc             ; 
    lsr         ;
    incptr          ;
    tax         ;--------------------------------------
    incptr          ; skip loop
    sep #$20        ;--------------------------------------
    a8              ; Tell vasm A is 8-bit
    jsr do_transfer ; transfer data
                ;--------------------------------------
    lda spc_sfx_next    ; return sfx index
    inc spc_sfx_next    ;
    
    plb
    plp
    rtl
    
;**********************************************************************
; a = id
; spc1 = params
;**********************************************************************
QueueMessage:
    sei             ; disable IRQ in case user 
                    ; has spcProcess in irq handler
            
    sep #$10            ; queue data in fifo
    x8              ; Tell vasm X/Y are 8-bit
    ldx spc_fwrite      ;
    sta spc_fifo, x     ;
    inx             ;
    lda spc1            ;
    sta spc_fifo, x     ;
    inx             ;
    lda spc1+1          ;
    sta spc_fifo, x     ;
    inx             ;
    stx spc_fwrite      ;
    rep #$10            ;
    x16             ; Tell vasm X/Y are 16-bit
    cli             ;
    
    plb
    plp
    rtl

;**********************************************************************
; void spcFlush(void)
;**********************************************************************
spcFlush:
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

.loop:
    lda spc_fread       ; call spcProcess until
    cmp spc_fwrite      ; fifo becomes empty
    beq .exit           ;
    jsr xspcProcessMessages  ;
    bra .loop        ;
.exit:  
    plb
    plp
    rtl
    
;**********************************************************************
; Internal spcFlush for spcLoad (no banking overhead)
;**********************************************************************
xspcFlush:
;----------------------------------------------------------------------
    lda spc_fread       ; call spcProcess until
    cmp spc_fwrite      ; fifo becomes empty
    beq .exit           ;
    jsr xspcProcessMessages  ;
    bra xspcFlush        ;
.exit:  rts             ;
    
;**********************************************************************
; Internal ProcessMessages (no banking overhead)
;**********************************************************************
xspcProcessMessages:

    sep #$10            ; 8-bit index during this function
    x8              ; Tell vasm X/Y are 8-bit
    lda spc_fwrite      ; exit if fifo is empty
    cmp spc_fread       ;
    beq .exit           ;------------------------------
    ldy #PROCESS_TIME       ; y = process time
;----------------------------------------------------------------------
.process_again:
;----------------------------------------------------------------------
    lda spc_v           ; test if spc is ready
    cmp REG_APUIO1      ;
    bne .next           ; no: decrement time
                    ;------------------------------
    ldx spc_fread       ; copy message arguments
    lda spc_fifo, x     ; and update fifo read pos
    sta REG_APUIO0      ;
    sta spc_pr+0
    inx             ;
    lda spc_fifo, x     ;
    sta REG_APUIO2      ;
    sta spc_pr+2
    inx             ;
    lda spc_fifo, x     ;
    sta REG_APUIO3      ;
    sta spc_pr+3
    inx             ;
    stx spc_fread       ;------------------------------
    lda spc_v           ; dispatch message
    eor #$80            ;
    sta spc_v           ;
    sta REG_APUIO1      ;------------------------------
    sta spc_pr+1
    lda spc_fread       ; exit if fifo has become empty
    cmp spc_fwrite      ;
    beq .exit           ;
;----------------------------------------------------------------------
.next:
;----------------------------------------------------------------------
    lda REG_SLHV        ; latch H/V and test for change
    lda REG_OPVCT       ;------------------------------
    cmp spc1            ; we will loop until the VCOUNT
    beq .process_again      ; changes Y times
    sta spc1            ;
    dey             ;
    bne .process_again      ;
;----------------------------------------------------------------------
.exit:
;----------------------------------------------------------------------
    rep #$10            ; restore 16-bit index
    x16             ; Tell vasm X/Y are 16-bit
    rts             ;
    
;**********************************************************************
; void spcProcess(void)
;**********************************************************************
spcProcess:
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

    lda digi_active
    beq spcProcessMessages
    jsr spcProcessStream

spcProcessMessages:

    sep #$10            ; 8-bit index during this function
    x8              ; Tell vasm X/Y are 8-bit
    lda spc_fwrite      ; exit if fifo is empty
    cmp spc_fread       ;
    beq .exit           ;------------------------------
    ldy #PROCESS_TIME       ; y = process time
;----------------------------------------------------------------------
.process_again:
;----------------------------------------------------------------------
    lda spc_v           ; test if spc is ready
    cmp REG_APUIO1      ;
    bne .next           ; no: decrement time
                    ;------------------------------
    ldx spc_fread       ; copy message arguments
    lda spc_fifo, x     ; and update fifo read pos
    sta REG_APUIO0      ;
    sta spc_pr+0
    inx             ;
    lda spc_fifo, x     ;
    sta REG_APUIO2      ;
    sta spc_pr+2
    inx             ;
    lda spc_fifo, x     ;
    sta REG_APUIO3      ;
    sta spc_pr+3
    inx             ;
    stx spc_fread       ;------------------------------
    lda spc_v           ; dispatch message
    eor #$80            ;
    sta spc_v           ;
    sta REG_APUIO1      ;------------------------------
    sta spc_pr+1
    lda spc_fread       ; exit if fifo has become empty
    cmp spc_fwrite      ;
    beq .exit           ;
;----------------------------------------------------------------------
.next:
;----------------------------------------------------------------------
    lda REG_SLHV        ; latch H/V and test for change
    lda REG_OPVCT       ;------------------------------
    cmp spc1            ; we will loop until the VCOUNT
    beq .process_again      ; changes Y times
    sta spc1            ;
    dey             ;
    bne .process_again      ;
;----------------------------------------------------------------------
.exit:
;----------------------------------------------------------------------
    rep #$10            ; restore 16-bit index
    x16             ; Tell vasm X/Y are 16-bit
    plb
    plp
    rtl
    
;**********************************************************************
; void spcPlay(u8 startPos)
;**********************************************************************
spcPlay:
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    lda 6,s             ; module_id
    sta spc1+1          ; id -- xx
    lda #CMD_PLAY       ;
    jmp QueueMessage        ;
    
;**********************************************************************
; void spcStop(void)
;**********************************************************************
spcStop:
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

    lda #CMD_STOP
    jmp QueueMessage

;**********************************************************************
; void spcPauseMusic(void)
;**********************************************************************
spcPauseMusic:
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    lda #CMD_PAUSE
    jmp QueueMessage

;**********************************************************************
; void spcResumeMusic(void)
;**********************************************************************
spcResumeMusic:
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    lda #CMD_RESUME
    jmp QueueMessage

;-------test function-----------;
spcTest:            ;#
    lda spc_v       ;#
.wloop:
    cmp REG_APUIO1  ;#
    bne .wloop      ;#
    xba         ;#
    lda #CMD_TEST   ;#
    sta REG_APUIO0  ;#
    xba         ;#
    eor #$80        ;#
    sta spc_v       ;#
    sta REG_APUIO1  ;#
    rtl         ;#
;--------------------------------#
; ################################

;**********************************************************************
; read status register
;**********************************************************************
spcReadStatus:
    ldx #5          ; read PORT2 with stability checks
    lda REG_APUIO2      ; 
.loop:                  ;
    cmp REG_APUIO2      ;
    bne spcReadStatus       ;
    dex             ;
    bne .loop           ;
    rtl             ; 
    
;**********************************************************************
; read position register
;**********************************************************************
spcReadPosition:
    ldx #5          ; read PORT3 with stability checks
    lda REG_APUIO2      ;
.loop:                  ;
    cmp REG_APUIO2      ;
    bne spcReadPosition     ;
    dex             ;
    bne .loop           ;
    rtl             ;

;**********************************************************************
; spcGetCues
;**********************************************************************
spcGetCues:
;**********************************************************************
    lda spc_q
    sta spc1
    jsr spcReadStatus
    and #$0F
    sta spc_q
    sec
    sbc spc1
    bcs .no_wrap
    adc #16
.no_wrap:
    rtl

;**********************************************************************
; void spcSetModuleVolume(u8 vol)
;**********************************************************************
spcSetModuleVolume:
;**********************************************************************
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

    lda 6,s             ; volume
    sta spc1+1          ; id -- vv
    lda #CMD_MVOL       ;
    jmp QueueMessage        ;

;**********************************************************************
; void spcFadeModuleVolume(u16 vol, u16 fadespeed)
;**********************************************************************
spcFadeModuleVolume:
;**********************************************************************
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    rep #$20
    a16
    lda 6,s     ; speed
    tay
    lda 8,s     ; target volume
    tax
    sep #$20
    a8

    txa             ;queue:
    sta spc1+1          ; id xx yy
    tya             ;
    sta spc1            ;
    lda #CMD_FADE
    jmp QueueMessage

;**********************************************************************
; void spcEffect(u16 pitch, u16 sfxIndex, u8 volpan)
;**********************************************************************
spcEffect:
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

    rep #$20
    a16
    lda 6,s     ; pitch
    tay
    lda 8,s     ; id
    tax
    sep #$20
    a8
    lda 10,s    ; v*16 + p

    sta spc1            ; spc1.l = "vp"
    sty spc2            ; spc1.h = "sh"
    txa             ;
    asl             ;
    asl             ;
    asl             ;
    asl             ;
    ora spc2            ;
    sta spc1+1          ;------------------------------
    lda #CMD_FX         ; queue FX message
    jmp QueueMessage        ;
;----------------------------------------------------------------------

;---------------------------------------------------------------------------------
; u8 spcGetMusicPosition(void)
;---------------------------------------------------------------------------------
spcGetMusicPosition: 
    php
    sep #$20
    ; Note: PVSnesLib C compilers usually expect return values in A (8-bit) or X/Y
    ; The original WLA code stored to tcc__r0. 
    ; For standard ASM to C return, A is usually fine for 8-bit.
    ; We will just read to A.
    lda REG_APUIO3
    rep #$20
    plp
    rtl

;======================================================================
;
; STREAMING
;
;======================================================================

;======================================================================
; void spcSetSoundTable(char *sndTableAddr);
;======================================================================
spcSetSoundTable:
;======================================================================
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    rep #$20
    a16
    lda 6,s         ; src (lower 16 bits)
    sta SoundTable
    sep #$20
    a8
    lda 8,s         ; src bank
    sta SoundTable+2
    
    plb
    plp
    rtl

;======================================================================
; void spcAllocateSoundRegion(u8 size)
;======================================================================
spcAllocateSoundRegion:
; a = size of buffer
;----------------------------------------------------------------------
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    lda 6,s         ; size of buffer
    pha             ; flush command queue
    jsr xspcFlush        ;
                    ;
    lda spc_v           ; wait for spc
.wloop:
    cmp REG_APUIO1      ;
    bne .wloop          ;
;----------------------------------------------------------------------
    pla             ; set parameter
    sta REG_APUIO3      ;
;----------------------------------------------------------------------
    lda #CMD_SSIZE      ; set command
    sta REG_APUIO0      ;
    sta spc_pr+0        ;
;----------------------------------------------------------------------
    lda spc_v           ; send message
    eor #128            ;
    sta REG_APUIO1      ;
    sta spc_v           ;
    sta spc_pr+1        ;
;----------------------------------------------------------------------
    plb
    plp
    rtl

;----------------------------------------------------------------------
; void spcPlaySound(u8 sndIndex)
;======================================================================
spcPlaySound:
;======================================================================
    php
    phb
    sep #$20
    lda #$0
    pha
    plb
    
    lda 6,s     ; index of sound
    
    xba
    lda #128
    xba
    ldx #255
    ldy #255
    jmp spcPlaySoundEx
    
;======================================================================
; void spcPlaySoundV(u8 sndIndex, u16 volume)
;======================================================================
spcPlaySoundV:
;======================================================================
    php
    phb
    sep #$20
    lda #$0
    pha
    plb

    rep #$20
    a16
    lda 6,s     ; volume of sound
    tay
    sep #$20
    a8
    lda 8,s     ; index of sound
    
    xba
    lda #128
    xba
    ldx #255
    jmp spcPlaySoundEx
    
;----------------------------------------------------------------------
; a = index
; b = pitch
; y = vol (0-15)
; x = pan (0-15, 8=center)
;======================================================================
spcPlaySoundEx:
;======================================================================
    sep #$10            ; push 8bit vol,pan on stack
    x8              ; Tell vasm X/Y are 8-bit
    phy             ;
    phx             ;
;----------------------------------------------------------------------------
    rep #$30            ; um
    a16             ; Tell vasm A is 16-bit
    x16             ; Tell vasm X/Y are 16-bit
    pha             ; 
;----------------------------------------------------------------------------
    and #$0FF           ; y = sound table index 
    asl             ;
    asl             ;
    asl             ;
    tay             ;
;----------------------------------------------------------------------------
    pla             ; a = rate
    xba             ;
    and #255            ; clear B
    sep #$20            ;
    a8              ; Tell vasm A is 8-bit
;----------------------------------------------------------------------------
    cmp #0          ; if a < 0 then use default
    bmi .use_default_pitch  ; otherwise use direct  
    sta digi_pitch      ;
    bra .direct_pitch       ;
.use_default_pitch:         ;
    lda [SoundTable], y     ;
    sta digi_pitch      ;
.direct_pitch:              ;
;----------------------------------------------------------------------------
    tax             ; set transfer rate
    lda >digi_rates, x       ;
    sta digi_copyrate       ;
;----------------------------------------------------------------------------
    iny             ; [point to PAN]
    pla             ; if pan <0 then use default
    bmi .use_default_pan    ; otherwise use direct
    sta spc1
    bra .direct_pan
.use_default_pan:
    lda [SoundTable], y
    sta spc1
.direct_pan:
;----------------------------------------------------------------------------
    iny             ; [point to VOL]
    pla             ; if vol < 0 then use default
    bmi .use_default_vol    ; otherwise use direct
    bra .direct_vol
.use_default_vol:
    lda [SoundTable], y
.direct_vol:
;----------------------------------------------------------------------------
    asl             ; vp = (vol << 4) | pan
    asl             ;
    asl             ;       
    asl             ;
    ora spc1            ;
    sta digi_vp         ;
;----------------------------------------------------------------------------
    iny             ; [point to LENGTH]
    rep #$20            ; copy length
    a16             ; Tell vasm A is 16-bit
    lda [SoundTable], y     ;
    sta digi_remain     ;
;----------------------------------------------------------------------------
    iny             ; [point to SOURCE]
    iny             ;
    lda [SoundTable], y     ; copy SOURCE also make +2 copy
    iny             ;
    iny             ;
    sta digi_src        ;
    inc             ; 
    inc             ; 
    sta digi_src2       ;
    sep #$20            ;
    a8              ; Tell vasm A is 8-bit
    lda [SoundTable], y     ;
    sta digi_src+2      ;
    sta digi_src2+2     ;
;----------------------------------------------------------------------------
    lda #1          ; set flags
    sta digi_init       ;
    sta digi_active     ; 
;----------------------------------------------------------------------------
    plb
    plp
    rtl
    
;============================================================================
spcProcessStream:
;============================================================================
    rep #$20            ; test if there is data to copy
    a16             ; Tell vasm A is 16-bit
    lda digi_remain     ;
    bne .continue           ;
    sep #$20            ;
    a8              ; Tell vasm A is 8-bit
    stz digi_active     ;
    rts             ;
.continue:  sep #$20            ;
    a8              ; Tell vasm A is 8-bit
;-----------------------------------------------------------------------
    lda spc_pr+0        ; send STREAM signal
    ora #128            ;
    sta REG_APUIO0      ;
;-----------------------------------------------------------------------
.wloop:
    bit REG_APUIO0      ; wait for SPC
    bpl .wloop          ;
;-----------------------------------------------------------------------
    stz REG_APUIO1      ; if digi_init then:
    lda digi_init       ;   clear digi_init
    beq .no_init        ;   set newnote flag
    stz digi_init       ;   copy vp
    lda digi_vp         ;   copy pan
    sta REG_APUIO2      ;   copy pitch
    lda digi_pitch      ;
    sta REG_APUIO3      ;
    lda #1          ;
    sta REG_APUIO1      ;
    lda digi_copyrate       ; copy additional data
    clc             ;
    adc #INIT_DATACOPY      ;
    bra .newnote        ;
.no_init:               ;
;-----------------------------------------------------------------------
    lda digi_copyrate       ; get copy rate
.newnote:
    rep #$20            ; saturate against remaining length
    a16             ; Tell vasm A is 16-bit
    and #$0FF           ; 
    cmp digi_remain     ;
    bcc .nsatcopy       ;
    lda digi_remain     ;
    stz digi_remain     ;
    bra .copysat        ;
.nsatcopy:              ;
;-----------------------------------------------------------------------
    pha             ; subtract amount from remaining
    sec             ;
    sbc digi_remain     ;
    eor #$0FFFF         ; FIXED: A is 16-bit
    inc             ; 
    sta digi_remain     ;
    pla             ;
.copysat:               ;
;-----------------------------------------------------------------------
    sep #$20            ; send copy amount
    a8              ; Tell vasm A is 8-bit
    sta REG_APUIO0      ;
;-----------------------------------------------------------------------
    sep #$10            ; spc1 = nn*3 (amount of tribytes to copy)
    x8              ; Tell vasm X/Y are 8-bit
    tax             ; x = vbyte
    sta spc1            ;
    asl             ;
    clc             ;
    adc spc1            ;
    sta spc1            ;
    ldy #0          ;
;-----------------------------------------------------------------------


.next_block:
        
    lda [digi_src2], y
    sta spc2
    rep #$20            ; read 2 bytes
    a16             ; Tell vasm A is 16-bit
    lda [digi_src], y       ;
.sync:
    cpx REG_APUIO0      ;-sync with spc
    bne .sync           ;
    inx             ; increment v
    sta REG_APUIO2      ; write 2 bytes
    sep #$20            ;
    a8              ; Tell vasm A is 8-bit
    lda spc2            ; copy third byte
    sta REG_APUIO1      ;
    stx REG_APUIO0      ; send data
    iny             ; increment pointer
    iny             ;
    iny             ;
    dec spc1            ; decrement block counter
    bne .next_block     ;
;-----------------------------------------------------------------------
.wsync:
    cpx REG_APUIO0      ; wait for spc
    bne .wsync          ;
;-----------------------------------------------------------------------    
    lda spc_pr+0        ; restore port data
    sta REG_APUIO0      ;
    lda spc_pr+1        ;
    sta REG_APUIO1      ;
    lda spc_pr+2        ;
    sta REG_APUIO2      ;
    lda spc_pr+3        ;
    sta REG_APUIO3      ;
;-----------------------------------------------------------------------
    tya             ; add offset to source
    rep #$31            ;
    a16             ; Tell vasm A is 16-bit
    x16             ; Tell vasm X/Y are 16-bit
    and #255            ;
    adc digi_src        ;
    sta digi_src        ;
    inc             ; 
    inc             ; 
    sta digi_src2       ;
    sep #$20            ;
    a8              ; Tell vasm A is 8-bit
;-----------------------------------------------------------------------
    rts
    
digi_rates:
    db 0, 3, 5, 7, 9, 11, 13

    include "snesmod_blob.s"