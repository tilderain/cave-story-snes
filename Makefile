.PRECIOUS: %.asm

ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

PVSNESLIB_DEBUG := 1

HIROM := 0
FASTROM := 0

# BEFORE including snes_rules :
# list in AUDIOFILES all your .it files in the right order. It will build to generate soundbank file
AUDIOFILES := res/Gestation.it
# then define the path to generate soundbank data. The name can be different but do not forget to update your include in .c file !
export SOUNDBANK := res/soundbank

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

#---------------------------------------------------------------------------------
# ROMNAME is used in snes_rules file
export ROMNAME := cave-story-snes

# to build musics, define SMCONVFLAGS with parameters you want
SMCONVFLAGS	:= -s -o $(SOUNDBANK) -V -b 5
musics: $(SOUNDBANK).obj

all: mariojump.brr mariowalk.brr bitmaps $(ROMNAME).sfc

# 1. Find ALL png files in res/Stage and its subfolders
# This uses the Linux/Unix 'find' command to look recursively
ALL_STAGE_PNGS := $(shell find res/ -name "*.png")

# 2. Extract just the filenames and change extension to .pic
# This removes the directory path so make looks for "PrtEggs2_vert.pic" in the root
STAGE_PICS := $(patsubst %.png,%.pic,$(notdir $(ALL_STAGE_PNGS)))

# 3. Tell 'make' where to look for source files if they aren't in the root
# We find every subdirectory inside res/Stage/ and add it to the search path
VPATH := $(shell find res/ -type d | tr '\n' ':')

# ---------------------------------------------------------------------------------
# CLEAN RULES
# ---------------------------------------------------------------------------------

# Identify ASM files that were generated from C files (so we don't delete manual ASM sources)
GENERATED_ASM := $(patsubst %.c, %.asm, $(wildcard *.c))

clean: cleanBuildRes cleanRom cleanGfx cleanAudio
	@echo Cleaning generated data and generated ASM files...
	@rm -f *.clm mariojump.brr mariowalk.brr $(STAGE_PICS) $(STAGE_PICS_EGGS) $(GENERATED_ASM)

#---------------------------------------------------------------------------------
pvsneslibfont.pic: pvsneslibfont.bmp
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFX4CONV) -s 8 -o 2 -u 16 -p -e 1 -t bmp -i $<

mario_sprite.pic: mario_sprite.bmp
	@echo convert sprites ... $(notdir $@)
	$(GFX4CONV) -s 16 -o 16 -u 16 -p -t bmp -i $<

mariofont.pic: mariofont.bmp
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFX4CONV) -s 8 -o 2 -u 16 -e 1 -p -t bmp -m -R -i $<


# 4. Your catch-all rule (now much simpler)
%.pic: %.png
	@echo converting stage gfx $< ...
	$(GFX4CONV) -s 8 -o 16 -u 16 -p -m -R -i $<

bitmaps : pvsneslibfont.pic PrtCave_vert.pic mariofont.pic mario_sprite.pic $(STAGE_PICS)

%.ps: %.c
	@echo Compiling to .ps ... $(notdir $<)
	$(CC) $(CFLAGS) -Wall -c $< -o $@
