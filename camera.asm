;vcprmin=10000
	section	"DONTMERGE_text.far.camera_init.0","acrx"
	a16
	x16
	global	_camera_init
_camera_init:
	lda	#<_player
	sta	>28+_camera
	lda	#^(_player)
	sta	>30+_camera
	lda	#0
	sta	>16+_camera
	sta	>18+_camera
	stz	_cameraShake
	stz	r2
	stz	r2+2
	stz	r4
	stz	r4+2
	lda	_stageWidth
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	#4096
	sta	r6
	txa
	sbc	#1
	sta	r6+2
	lda	r6
	cmp	#0
	lda	r6+2
	sbc	#0
	bvc	l448
	eor	#32768
l448:
	bpl	l179
	lda	r6
	sta	r4
	lda	r6+2
	sta	r4+2
l179:
	lda	_stageHeight
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	#61440
	sta	r0
	txa
	sbc	#0
	sta	r0+2
	lda	r0
	cmp	#0
	lda	r0+2
	sbc	#0
	bvc	l449
	eor	#32768
l449:
	bpl	l181
	lda	r0
	sta	r2
	lda	r0+2
	sta	r2+2
l181:
	lda	r4
	cmp	#4096
	lda	r4+2
	sbc	#1
	bvc	l450
	eor	#32768
l450:
	bpl	l183
	lda	#4096
	sta	r4
	lda	#1
	sta	r4+2
l183:
	lda	r2
	cmp	#61440
	lda	r2+2
	sbc	#0
	bvc	l451
	eor	#32768
l451:
	bpl	l185
	lda	#61440
	sta	r2
	stz	r2+2
l185:
	lda	r4
	sta	>8+_camera
	lda	r4+2
	sta	>10+_camera
	lda	>8+_camera
	sta	>_camera
	lda	>10+_camera
	sta	>2+_camera
	lda	r2
	sta	>12+_camera
	lda	r2+2
	sta	>14+_camera
	lda	>12+_camera
	sta	>4+_camera
	lda	>14+_camera
	sta	>6+_camera
	lda	r4+1
	sta	r31
	lda	r4+3
	and	#255
	bit	#128
	beq	l452
	eor	#65280
l452:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#128
	sta	>24+_camera
	lda	r2+1
	sta	r31
	lda	r2+3
	and	#255
	bit	#128
	beq	l453
	eor	#65280
l453:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#112
	sta	>26+_camera
	sec
	lda	>_camera
	sbc	#49152
	sta	_camera_xmin
	lda	>2+_camera
	sbc	#1
	sta	2+_camera_xmin
	lda	#16384
	sta	_camera_xsize
	lda	#3
	sta	2+_camera_xsize
	sec
	lda	>4+_camera
	sbc	#40960
	sta	_camera_ymin
	lda	>6+_camera
	sbc	#1
	sta	2+_camera_ymin
	stz	_camera_ysize
	lda	#3
	sta	2+_camera_ysize
	rtl
; stacksize=0+??
;vcprmin=10000
	section	"DONTMERGE_text.far.camera_set_position.0","acrx"
	a16
	x16
	global	_camera_set_position
_camera_set_position:
	sta	r2
	stx	r2+2
	lda	4,s
	sta	r4
	lda	6,s
	sta	r4+2
	lda	_stageWidth
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	#4096
	sta	r0
	txa
	sbc	#1
	sta	r0+2
	lda	r0
	cmp	r2
	lda	r0+2
	sbc	r2+2
	bvc	l460
	eor	#32768
l460:
	bpl	l21
	lda	r0
	sta	r2
	lda	r0+2
	sta	r2+2
l21:
	lda	_stageHeight
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	#61440
	sta	r6
	txa
	sbc	#0
	sta	r6+2
	lda	r6
	cmp	r4
	lda	r6+2
	sbc	r4+2
	bvc	l461
	eor	#32768
l461:
	bpl	l23
	lda	r6
	sta	r4
	lda	r6+2
	sta	r4+2
l23:
	lda	r2
	cmp	#4096
	lda	r2+2
	sbc	#1
	bvc	l462
	eor	#32768
l462:
	bpl	l25
	lda	#4096
	sta	r2
	lda	#1
	sta	r2+2
l25:
	lda	r4
	cmp	#61440
	lda	r4+2
	sbc	#0
	bvc	l463
	eor	#32768
l463:
	bpl	l27
	lda	#61440
	sta	r4
	stz	r4+2
l27:
	lda	r2
	sta	>8+_camera
	lda	r2+2
	sta	>10+_camera
	lda	>8+_camera
	sta	>_camera
	lda	>10+_camera
	sta	>2+_camera
	lda	r4
	sta	>12+_camera
	lda	r4+2
	sta	>14+_camera
	lda	>12+_camera
	sta	>4+_camera
	lda	>14+_camera
	sta	>6+_camera
	lda	r2+1
	sta	r31
	lda	r2+3
	and	#255
	bit	#128
	beq	l464
	eor	#65280
l464:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#128
	sta	>24+_camera
	lda	r4+1
	sta	r31
	lda	r4+3
	and	#255
	bit	#128
	beq	l465
	eor	#65280
l465:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#112
	sta	>26+_camera
	sec
	lda	>_camera
	sbc	#49152
	sta	_camera_xmin
	lda	>2+_camera
	sbc	#1
	sta	2+_camera_xmin
	lda	#16384
	sta	_camera_xsize
	lda	#3
	sta	2+_camera_xsize
	sec
	lda	>4+_camera
	sbc	#40960
	sta	_camera_ymin
	lda	>6+_camera
	sbc	#1
	sta	2+_camera_ymin
	stz	_camera_ysize
	lda	#3
	sta	2+_camera_ysize
	rtl
; stacksize=0+??
;vcprmin=10000
	section	"DONTMERGE_text.far.camera_shake.0","acrx"
	a16
	x16
	global	_camera_shake
_camera_shake:
	tax
	stx	_cameraShake
	rtl
; stacksize=0+??
;vcprmin=10000
	section	"DONTMERGE_text.far.camera_update.0","acrx"
	a16
	x16
	global	_camera_update
_camera_update:
	pei	(r16)
	pei	(r18)
	pei	(r19)
	pei	(r20)
	pei	(r21)
	phy
	phy
	phy
	phy
	lda	>28+_camera
	ora	>30+_camera
	beq	l161
	tsc
	clc
	adc	#5
	sta	r20
	stz	r20+2
	tsc
	ina
	sta	r18
	stz	r18+2
	lda	>30+_camera
	cmp	#^(_player)
	bne	l327
	lda	>28+_camera
	cmp	#<_player
	bne	l327
	sep	#32
	a8
	lda	>61+_player
	a16
	rep	#32
	bne	l371
	sec
	lda	>16+_camera
	sbc	#512
	sta	>16+_camera
	lda	>18+_camera
	sbc	#0
	sta	>18+_camera
	lda	>16+_camera
	cmp	#40960
	lda	>18+_camera
	sbc	#65535
	bvc	l490
	eor	#32768
l490:
	bpl	l376
	lda	#40960
	sta	>16+_camera
	lda	#65535
	sta	>18+_camera
	bra	l376
l371:
	clc
	lda	>16+_camera
	adc	#512
	sta	>16+_camera
	lda	>18+_camera
	adc	#0
	sta	>18+_camera
	lda	#24576
	cmp	>16+_camera
	lda	#0
	sbc	>18+_camera
	bvc	l491
	eor	#32768
l491:
	bpl	l376
	lda	#24576
	sta	>16+_camera
	lda	#0
	sta	>18+_camera
l376:
	sep	#32
	a8
	lda	_controlsLocked
	a16
	rep	#32
	bne	l378
	lda	_joystate
	and	#64
	sep	#32
	a8
	cmp	#0
	a16
	rep	#32
	beq	l378
	sec
	lda	>20+_camera
	sbc	#512
	sta	>20+_camera
	lda	>22+_camera
	sbc	#0
	sta	>22+_camera
	lda	>20+_camera
	cmp	#40960
	lda	>22+_camera
	sbc	#65535
	bvc	l492
	eor	#32768
l492:
	bpl	l393
	lda	#40960
	sta	>20+_camera
	lda	#65535
	sta	>22+_camera
	bra	l393
l378:
	sep	#32
	a8
	lda	_controlsLocked
	a16
	rep	#32
	bne	l384
	lda	_joystate
	and	#128
	sep	#32
	a8
	cmp	#0
	a16
	rep	#32
	beq	l384
	clc
	lda	>20+_camera
	adc	#512
	sta	>20+_camera
	lda	>22+_camera
	adc	#0
	sta	>22+_camera
	lda	#24576
	cmp	>20+_camera
	lda	#0
	sbc	>22+_camera
	bvc	l493
	eor	#32768
l493:
	bpl	l393
	lda	#24576
	sta	>20+_camera
	lda	#0
	sta	>22+_camera
	bra	l393
l384:
	lda	#0
	cmp	>20+_camera
	sbc	>22+_camera
	bvc	l494
	eor	#32768
l494:
	bpl	l390
	sec
	lda	>20+_camera
	sbc	#512
	sta	>20+_camera
	lda	>22+_camera
	sbc	#0
	sta	>22+_camera
	bra	l393
l390:
	lda	>20+_camera
	cmp	#0
	lda	>22+_camera
	sbc	#0
	bvc	l495
	eor	#32768
l495:
	bpl	l393
	clc
	lda	>20+_camera
	adc	#512
	sta	>20+_camera
	lda	>22+_camera
	adc	#0
	sta	>22+_camera
l393:
	lda	>28+_camera
	sta	r0
	lda	>30+_camera
	sta	r0+2
	ldy	#33
	lda	[r0],y ;am(33)
	and	#65024
	pha
	ldy	#35
	lda	[r0],y ;am(33)
	tax
	pla
	clc
	adc	>16+_camera
	pha
	txa
	adc	>18+_camera
	tax
	pla
	sec
	sbc	>_camera
	pha
	txa
	sbc	>2+_camera
	tax
	pla
	sta	r31
	txa
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	tax
	lda	r31
	clc
	adc	>_camera
	sta	1,s
	txa
	adc	>2+_camera
	sta	3,s
	lda	>28+_camera
	sta	r0
	lda	>30+_camera
	sta	r0+2
	ldy	#37
	lda	[r0],y ;am(37)
	and	#65024
	pha
	ldy	#39
	lda	[r0],y ;am(37)
	tax
	pla
	clc
	adc	>20+_camera
	pha
	txa
	adc	>22+_camera
	tax
	pla
	sec
	sbc	>4+_camera
	pha
	txa
	sbc	>6+_camera
	tax
	pla
	sta	r31
	txa
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	tax
	lda	r31
	clc
	adc	>4+_camera
	sta	5,s
	txa
	adc	>6+_camera
	sta	7,s
	bra	l328
l327:
	lda	#0
	sta	>16+_camera
	sta	>18+_camera
	sta	>20+_camera
	sta	>22+_camera
	lda	>28+_camera
	sta	r0
	lda	>30+_camera
	sta	r0+2
	ldy	#33
	lda	[r0],y ;am(33)
	and	#65024
	pha
	ldy	#35
	lda	[r0],y ;am(33)
	tax
	pla
	clc
	adc	>16+_camera
	pha
	txa
	adc	>18+_camera
	tax
	pla
	sec
	sbc	>_camera
	pha
	txa
	sbc	>2+_camera
	tax
	pla
	sta	r31
	txa
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	tax
	lda	r31
	clc
	adc	>_camera
	sta	1,s
	txa
	adc	>2+_camera
	sta	3,s
	lda	>28+_camera
	sta	r0
	lda	>30+_camera
	sta	r0+2
	ldy	#37
	lda	[r0],y ;am(37)
	and	#65024
	pha
	ldy	#39
	lda	[r0],y ;am(37)
	tax
	pla
	clc
	adc	>20+_camera
	pha
	txa
	adc	>22+_camera
	tax
	pla
	sec
	sbc	>4+_camera
	pha
	txa
	sbc	>6+_camera
	tax
	pla
	sta	r31
	txa
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	tax
	lda	r31
	clc
	adc	>4+_camera
	sta	5,s
	txa
	adc	>6+_camera
	sta	7,s
l328:
	sec
	lda	1,s
	sbc	>_camera
	pha
	lda	5,s
	sbc	>2+_camera
	tax
	lda	1,s
	cmp	#61441
	txa
	sbc	#65535
	bvc	l497
	eor	#32768
l497:
	bpl	l496
	pla
	sec
	lda	>_camera
	sbc	#4095
	sta	1,s
	lda	>2+_camera
	sbc	#0
	sta	3,s
l332:
	sec
	lda	5,s
	sbc	>4+_camera
	pha
	lda	9,s
	sbc	>6+_camera
	tax
	lda	1,s
	cmp	#61441
	txa
	sbc	#65535
	bvc	l499
	eor	#32768
l499:
	bpl	l498
	pla
	sec
	lda	>4+_camera
	sbc	#4095
	sta	5,s
	lda	>6+_camera
	sbc	#0
	sta	7,s
l334:
	sec
	lda	1,s
	sbc	>_camera
	pha
	lda	5,s
	sbc	>2+_camera
	tax
	lda	1,s
	sta	r31
	lda	#4095
	cmp	r31
	stx	r31
	lda	#0
	sbc	r31
	bvc	l501
	eor	#128
l501:
	bpl	l500
	pla
	clc
	lda	>_camera
	adc	#4095
	sta	1,s
	lda	>2+_camera
	adc	#0
	sta	3,s
l336:
	sec
	lda	5,s
	sbc	>4+_camera
	pha
	lda	9,s
	sbc	>6+_camera
	tax
	lda	1,s
	sta	r31
	lda	#4095
	cmp	r31
	stx	r31
	lda	#0
	sbc	r31
	bvc	l503
	eor	#128
l503:
	bpl	l502
	pla
	clc
	lda	>4+_camera
	adc	#4095
	sta	5,s
	lda	>6+_camera
	adc	#0
	sta	7,s
l338:
	lda	_cameraShake
	beq	l342
	lda	#2
	sta	r0
	bra	l343
l342:
	lda	#8
	sta	r0
l343:
	lda	r0
	sta	r15
	lda	_stageID
	cmp	#18
	bne	l345
	sep	#32
	a8
	lda	_pal_mode
	a16
	rep	#32
	bne	l345
	lda	#4096
	sta	1,s
	lda	#1
	sta	3,s
	lda	#0
	sta	5,s
	lda	#1
	sta	7,s
	bra	l357
l345:
	lda	r15
	sta	r1
	clc
	adc	#128
	sta	r0
	ldx	#0
	sta	r31
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	sta	r4
	lda	btmp3+1
	sta	r4+2
	lda	1,s
	cmp	r4
	lda	3,s
	sbc	r4+2
	bvc	l504
	eor	#32768
l504:
	bpl	l349
	lda	r4
	sta	1,s
	lda	r4+2
	sta	3,s
	bra	l352
l349:
	lda	_stageWidth
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	r4
	sta	r12
	txa
	sbc	r4+2
	sta	r12+2
	lda	r12
	cmp	1,s
	lda	r12+2
	sbc	3,s
	bvc	l505
	eor	#32768
l505:
	bpl	l352
	lda	r12
	sta	1,s
	lda	r12+2
	sta	3,s
l352:
	lda	r1
	clc
	adc	#112
	sta	r0
	ldx	#0
	sta	r31
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	sta	r2
	lda	btmp3+1
	sta	r2+2
	lda	5,s
	cmp	r2
	lda	7,s
	sbc	r2+2
	bvc	l506
	eor	#32768
l506:
	bpl	l354
	lda	r2
	sta	5,s
	lda	r2+2
	sta	7,s
	bra	l357
l354:
	lda	_stageHeight
	ldx	#0
	sta	btmp3
	stx	btmp3+2
	xba
	and	#65280
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	asl
	rol	btmp3+1
	ldx	btmp3+1
	sec
	sbc	r2
	sta	r10
	txa
	sbc	r2+2
	sta	r10+2
	lda	r10
	cmp	5,s
	lda	r10+2
	sbc	7,s
	bvc	l507
	eor	#32768
l507:
	bpl	l357
	lda	r10
	sta	5,s
	lda	r10+2
	sta	7,s
l357:
	lda	_cameraShake
	beq	l366
	dec	_cameraShake
	lda	_cameraShake
	and	#1
	sep	#32
	a8
	cmp	#0
	a16
	rep	#32
	beq	l366
	jsl	>_random
	and	#2047
	sec
	sbc	#1024
	sta	r16
	jsl	>_random
	and	#2047
	sec
	sbc	#1024
	sta	r14
	lda	1,s
	and	#61440
	pha
	lda	#0
	tax
	pla
	sta	r31
	lda	r16
	sta	r0
	ldy	#0
	cmp	#0
	bpl	l508
	dey
l508:
	sty	r0+2
	lda	r31
	clc
	lda	r0
	adc	1,s
	sta	r8
	lda	r0+2
	adc	3,s
	sta	r8+2
	lda	r8
	and	#61440
	sta	r0
	lda	#0
	sta	r0+2
	lda	r31
	cpx	r0+2
	bne	l364
	cmp	r0
	bne	l364
	lda	r8
	sta	1,s
	lda	r8+2
	sta	3,s
l364:
	lda	5,s
	and	#61440
	pha
	lda	#0
	tax
	pla
	sta	r31
	lda	r14
	sta	r0
	ldy	#0
	cmp	#0
	bpl	l509
	dey
l509:
	sty	r0+2
	lda	r31
	clc
	lda	r0
	adc	5,s
	sta	r6
	lda	r0+2
	adc	7,s
	sta	r6+2
	lda	r6
	and	#61440
	sta	r0
	lda	#0
	sta	r0+2
	lda	r31
	cpx	r0+2
	bne	l366
	cmp	r0
	bne	l366
	lda	r6
	sta	5,s
	lda	r6+2
	sta	7,s
l366:
	lda	2,s
	sta	r31
	lda	4,s
	and	#255
	bit	#128
	beq	l510
	eor	#65280
l510:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#128
	sta	>24+_camera
	lda	6,s
	sta	r31
	lda	8,s
	and	#255
	bit	#128
	beq	l511
	eor	#65280
l511:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#112
	sta	>26+_camera
	sec
	lda	>_camera
	sbc	#49152
	sta	_camera_xmin
	lda	>2+_camera
	sbc	#1
	sta	2+_camera_xmin
	lda	#16384
	sta	_camera_xsize
	lda	#3
	sta	2+_camera_xsize
	sec
	lda	>4+_camera
	sbc	#40960
	sta	_camera_ymin
	lda	>6+_camera
	sbc	#1
	sta	2+_camera_ymin
	stz	_camera_ysize
	lda	#3
	sta	2+_camera_ysize
	pei	(r20+2)
	pei	(r20)
	ldx	r18+2
	lda	r18
	jsl	>l107
	lda	5,s
	sta	>_camera
	lda	7,s
	sta	>2+_camera
	lda	9,s
	sta	>4+_camera
	lda	11,s
	sta	>6+_camera
	lda	6,s
	sta	r31
	lda	8,s
	and	#255
	bit	#128
	beq	l512
	eor	#65280
l512:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#128
	sta	>24+_camera
	lda	10,s
	sta	r31
	lda	12,s
	and	#255
	bit	#128
	beq	l513
	eor	#65280
l513:
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#112
	sta	>26+_camera
	ply
	ply
l161:
	ply
	ply
	ply
	ply
	plx
	stx	r21
	plx
	stx	r20
	plx
	stx	r19
	plx
	stx	r18
	plx
	stx	r16
	rtl
l502:
	pla
	bra	l338
l500:
	pla
	bra	l336
l498:
	pla
	bra	l334
l496:
	pla
	bra	l332
; stacksize=0+??
;vcprmin=10000
	section	"DONTMERGE_text.far.camera_handle_morphing.107","acrx"
	a16
	x16
l107:
	pei	(r16)
	pei	(r17)
	pei	(r18)
	pei	(r19)
	pei	(r20)
	pei	(r21)
	pei	(r22)
	pei	(r23)
	pei	(r24)
	pei	(r25)
	pei	(r26)
	pei	(r27)
	phy
	phy
	phy
	phy
	sta	r16
	stx	r16+2
	lda	36,s
	sta	r22
	lda	38,s
	sta	r22+2
	ldy	#1
	lda	[r16],y
	sta	r31
	ldy	#3
	lda	[r16],y
	and	#255
	bit	#128
	beq	l530
	eor	#65280
l530:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	3,s
	lda	r31
	sta	1,s
	lda	>1+_camera
	sta	r31
	lda	>3+_camera
	and	#255
	bit	#128
	beq	l531
	eor	#65280
l531:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	lda	1,s
	sta	r31
	lda	r0
	sta	r2
	lda	r31
	sep	#32
	a8
	sec
	sbc	r2
	sta	_morphingColumn
	a16
	ldy	#1
	rep	#32
	lda	[r22],y
	sta	r31
	ldy	#3
	lda	[r22],y
	and	#255
	bit	#128
	beq	l532
	eor	#65280
l532:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	7,s
	lda	r31
	sta	5,s
	lda	>5+_camera
	sta	r31
	lda	>7+_camera
	and	#255
	bit	#128
	beq	l533
	eor	#65280
l533:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	lda	5,s
	sta	r31
	lda	r0
	sta	r2
	lda	r31
	sep	#32
	a8
	sec
	sbc	r2
	sta	_morphingRow
	a16
	rep	#32
	lda	_morphingColumn
	eor	#128
	and	#255
	sec
	sbc	#128
	sta	r2
	lda	_morphingRow
	eor	#128
	and	#255
	sec
	sbc	#128
	ora	r2
	beq	l291
	lda	_stageTileset
	and	#255
	sta	r31
	lda	#14
	sta	r1
	lda	r31
	sta	r0
	jsl	>___mulint16snes
	sta	r31
	tax
	lda	>10+_tileset_info,x ;am(r31)
	sta	r24
	ldx	r31
	lda	>12+_tileset_info,x ;am(r31)
	sta	r24+2
	sep	#32
	a8
	lda	_morphingColumn
	a16
	rep	#32
	beq	l290
	sep	#32
	a8
	lda	_morphingRow
	a16
	rep	#32
	beq	l115
	sep	#32
	a8
	inc	_diag_tick
	a16
	rep	#32
	lda	_diag_tick
	and	#1
	sep	#32
	a8
	cmp	#0
	a16
	rep	#32
	beq	l115
	sep	#32
	a8
	stz	_morphingColumn
	a16
	rep	#32
	lda	>_camera
	sta	[r16]
	lda	>2+_camera
	ldy	#2
	sta	[r16],y
	bra	l290
l115:
	sep	#32
	a8
	lda	_morphingColumn
	cmp	#1
	a16
	rep	#32
	bne	l119
	lda	#16
	sta	r11
	bra	l120
l119:
	lda	#65520
	sta	r11
l120:
	lda	1,s
	clc
	adc	r11
	sta	r5
	lda	5,s
	sec
	sbc	#10
	sta	r20
	lda	r5
	sec
	sbc	#65504
	bvc	l534
	eor	#32768
l534:
	bmi	l290
	lda	_stageWidth
	clc
	adc	#32
	asl
	tax
	clc
	sbc	r5
	bvc	l536
	eor	#32768
l536:
	bmi	l535
	lda	#31
	sta	r18
	lda	r5
	sta	r27
l288:
	pei	(r24+2)
	pei	(r24)
	pei	(r20)
	lda	r27
	jsl	>_stage_draw_tile
	inc	r20
	lda	r18
	dec	r18
	ply
	ply
	ply
	cmp	#0
	bne	l288
l290:
	sep	#32
	a8
	lda	_morphingRow
	a16
	rep	#32
	beq	l291
	sep	#32
	a8
	lda	_morphingColumn
	a16
	rep	#32
	beq	l131
	lda	_diag_tick
	and	#255
	and	#1
	bne	l131
	sep	#32
	a8
	stz	_morphingRow
	a16
	rep	#32
	lda	>4+_camera
	sta	[r22]
	lda	>6+_camera
	ldy	#2
	sta	[r22],y
	bra	l291
l131:
	ldy	#1
	lda	[r22],y
	sta	r31
	ldy	#3
	lda	[r22],y
	and	#255
	bit	#128
	beq	l537
	eor	#65280
l537:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	r12+2
	lda	r31
	sta	r12
	sep	#32
	a8
	lda	_morphingRow
	cmp	#1
	a16
	rep	#32
	bne	l135
	lda	#14
	sta	r10
	bra	l136
l135:
	lda	#65522
	sta	r10
l136:
	lda	r12
	clc
	adc	r10
	sta	r4
	ldy	#1
	lda	[r16],y
	sta	r31
	ldy	#3
	lda	[r16],y
	and	#255
	bit	#128
	beq	l538
	eor	#65280
l538:
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	cmp	#32768
	ror
	ror	r31
	sta	r0+2
	lda	r31
	sta	r0
	sec
	sbc	#16
	sta	r19
	lda	r4
	sec
	sbc	#65504
	bvc	l539
	eor	#32768
l539:
	bmi	l291
	lda	_stageHeight
	clc
	adc	#32
	asl
	tax
	clc
	sbc	r4
	bvc	l541
	eor	#32768
l541:
	bmi	l540
	lda	#63
	sta	r21
	lda	r4
	sta	r26
l289:
	pei	(r24+2)
	pei	(r24)
	pei	(r26)
	lda	r19
	jsl	>_stage_draw_tile
	inc	r19
	lda	r21
	dec	r21
	ply
	ply
	ply
	cmp	#0
	bne	l289
l291:
	sep	#32
	a8
	lda	_morphingColumn
	a16
	rep	#32
	bne	l145
	sec
	lda	>8+_camera
	sbc	[r16]
	sta	r0
	lda	>10+_camera
	ldy	#2
	sbc	[r16],y
	sta	r0+2
	lda	r0
	cmp	#0
	lda	r0+2
	sbc	#0
	bvc	l542
	eor	#32768
l542:
	bpl	l149
	sec
	lda	#0
	sbc	r0
	sta	r6
	lda	#0
	sbc	r0+2
	sta	r6+2
	bra	l150
l149:
	lda	r0
	sta	r6
	lda	r0+2
	sta	r6+2
l150:
	lda	#8191
	cmp	r6
	lda	#0
	sbc	r6+2
	bvc	l543
	eor	#32768
l543:
	bmi	l144
	sec
	lda	>12+_camera
	sbc	[r22]
	sta	r2
	lda	>14+_camera
	ldy	#2
	sbc	[r22],y
	sta	r2+2
	lda	r2
	cmp	#0
	lda	r2+2
	sbc	#0
	bvc	l544
	eor	#32768
l544:
	bpl	l152
	sec
	lda	#0
	sbc	r2
	sta	r8
	lda	#0
	sbc	r2+2
	sta	r8+2
	bra	l153
l152:
	lda	r2
	sta	r8
	lda	r2+2
	sta	r8+2
l153:
	lda	#8191
	cmp	r8
	lda	#0
	sbc	r8+2
	bvc	l545
	eor	#32768
l545:
	bpl	l145
l144:
	lda	[r16]
	sta	>8+_camera
	ldy	#2
	lda	[r16],y
	sta	>10+_camera
	lda	[r22]
	sta	>12+_camera
	lda	[r22],y
	sta	>14+_camera
l145:
	ply
	ply
	ply
	ply
	plx
	stx	r27
	plx
	stx	r26
	plx
	stx	r25
	plx
	stx	r24
	plx
	stx	r23
	plx
	stx	r22
	plx
	stx	r21
	plx
	stx	r20
	plx
	stx	r19
	plx
	stx	r18
	plx
	stx	r17
	plx
	stx	r16
	rtl
l540:
	txa
	bra	l291
l535:
	txa
	bra	l290
; stacksize=0+??
	global	_camera_ymin
	section	"DONTMERGE_data.near.camera_ymin.0","adrw"
_camera_ymin:
	dd	0
	global	_camera_ysize
	section	"DONTMERGE_data.near.camera_ysize.0","adrw"
_camera_ysize:
	dd	0
	global	_camera
	section	"DONTMERGE_data.far.camera.0","adrw"
_camera:
	dd	0
	reserve	4
	reserve	4
	reserve	4
	reserve	4
	reserve	4
	reserve	2
	reserve	2
	reserve	4
	global	___mulint16snes
	global	_random
	global	_joystate
	global	_player
	global	_controlsLocked
	global	_stageID
	global	_stageWidth
	global	_stageHeight
	global	_stageTileset
	global	_morphingRow
	global	_morphingColumn
	global	_stage_draw_tile
	global	_tileset_info
	global	_camera_xmin
	section	"DONTMERGE_bss.near.camera_xmin.0","aurw"
_camera_xmin:
	reserve	4
	global	_camera_xsize
	section	"DONTMERGE_bss.near.camera_xsize.0","aurw"
_camera_xsize:
	reserve	4
	global	_cameraShake
	section	"DONTMERGE_bss.near.cameraShake.0","aurw"
_cameraShake:
	reserve	2
	global	_mapbuf
	section	"DONTMERGE_bss.far.mapbuf.0","aurw"
_mapbuf:
	reserve	128
	global	_diag_tick
	section	"DONTMERGE_bss.near.diag_tick.0","aurw"
_diag_tick:
	reserve	1
	global	_pal_mode
	zpage	r0
	zpage	r1
	zpage	r2
	zpage	r3
	zpage	r4
	zpage	r5
	zpage	r6
	zpage	r7
	zpage	r8
	zpage	r9
	zpage	r10
	zpage	r11
	zpage	r12
	zpage	r13
	zpage	r14
	zpage	r15
	zpage	r16
	zpage	r17
	zpage	r18
	zpage	r19
	zpage	r20
	zpage	r21
	zpage	r22
	zpage	r23
	zpage	r24
	zpage	r25
	zpage	r26
	zpage	r27
	zpage	r28
	zpage	r29
	zpage	r30
	zpage	r31
	zpage	btmp0
	zpage	btmp1
	zpage	btmp2
	zpage	btmp3
