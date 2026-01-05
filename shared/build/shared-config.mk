    
# Shared SNES C Compiler Build Configuration
# This file contains all compiler configurations, source definitions, and build rules

# Set default shell
SHELL = /bin/sh

# Set default paths (can be overridden by individual projects)
SHARED_SRC_DIR ?= ../src
SHARED_PORT_DIR ?= ../port
BUILD_DIR ?= build

# =============================================================================
# AUDIO CONFIGURATION (SNESMOD)
# =============================================================================

SMCONV      := $(PVSNESLIB_HOME)/devkitsnes/tools/smconv

# List your .it files here in order
AUDIOFILES  := res/WANPAK2.it res/Gestation.it

SOUNDBANK_BNK := res/soundbank.bnk
SOUNDBANK_ASM_WRAPPER := res/soundbank_vasm.s
SOUNDBANK_OUTPUT   := res/soundbank
# SMCONV Flags:
# -s: silent, -o: output path, -v: verbose
# -b 5: Bank size (5=32k, often safer for compatibility)
# -i: HiROM (remove this flag if compiling for LoROM)
SMCONVFLAGS = -s -i -V -b 3 -o $(SOUNDBANK_OUTPUT)

# =============================================================================
# TOOLCHAIN PATHS (Modify these or add to your $PATH)
# =============================================================================
# Defaults assume tools are in your $PATH. Override via command line or env vars.
WDC_PATH ?= /opt/wdc
CALYPSI_PATH ?= /opt/calypsi/bin
CC65_PATH ?= /usr/bin
LLVM_MOS_PATH ?= /usr/local
VBCC_PATH ?= /opt/vbcc

# Check if compiler is specified (skip for help, clean, and convenience targets)
ifneq ($(filter help clean wdc vbcc calypsi llvm-mos cc65 jcc816 tcc816,$(MAKECMDGOALS)),)
# Skip compiler check for help, clean, and convenience targets
else
ifeq ($(COMPILER),)
$(error Please specify a compiler. Usage: make COMPILER=wdc816cc, make COMPILER=vbcc65816, make COMPILER=calypsi, make COMPILER=llvm-mos, make COMPILER=cc65, make COMPILER=jcc816, or make COMPILER=tcc816 (case-insensitive))
endif
endif

# Normalizing Compiler Name (Lower case)
COMPILER_LOWER = $(shell echo $(COMPILER) | tr A-Z a-z)

# =============================================================================
# COMPILER CONFIGURATIONS
# =============================================================================

# WDC816CC Configuration (Requires Wine on Linux)
ifeq ($(COMPILER_LOWER),wdc816cc)
	# WDC Tools usually need to run via Wine on Linux
	CC = wine $(WDC_PATH)/bin/wdc816cc.exe
	AS = wine $(WDC_PATH)/bin/wdc816as.exe
	LD = wine $(WDC_PATH)/bin/wdcln.exe
	
	CCFLAGS = -WL -SM -MK -MT -ML -WP -MU -MV -SI -SP -D__WDC816CC__=1
	ASFLAGS = 
	
	# Path adjustments for Wine may be needed for includes
	INCLUDES = -I"$(WDC_PATH)/Tools/include" -I"$(SHARED_SRC_DIR)" -I"lib" -I"include"
	
	ifeq ($(USE_FLOATING_POINT),1)
		LDFLAGS = -HB -ML -B -E -T -C018000,008000 $(PROJECT_OBJECTS) $(BUILD_DIR)/vectors.obj -C028000,010000 $(BUILD_DIR)/kernel.obj $(BUILD_DIR)/initsnes.obj -D7E2000,18000 -K048000,20000 -Lml -Lcl -O$(BUILD_DIR)/mainBankZero.bin
	else
		LDFLAGS = -HB -ML -B -E -T -C018000,008000 $(PROJECT_OBJECTS) $(BUILD_DIR)/vectors.obj -C028000,010000 $(BUILD_DIR)/kernel.obj $(BUILD_DIR)/initsnes.obj -D7E2000,18000 -K048000,20000 -Lcl -O$(BUILD_DIR)/mainBankZero.bin
	endif
	
	OUTPUT_EXT = .bin
	POST_LINK = if [ -f "$(BUILD_DIR)/mainBankZero.bin" ]; then cp "$(BUILD_DIR)/mainBankZero.bin" "$(BUILD_DIR)/mainBankZero_wdc816cc.smc"; fi
	COMPILER_NAME = wdc816cc
endif
    
export PATH := $(PATH):$(VBCC)/bin

# VBCC65816 Configuration
ifeq ($(COMPILER_LOWER),vbcc65816)
    # 1. Standard tools
    CC = $(VBCC)/bin/vc
    AS = $(VBCC)/bin/vasm6502_oldstyle
    
    # 2. Point LD directly to vlink (bypass vc for linking)
    LD = $(VBCC)/bin/vlink
    
    # 3. Compiler Flags (Compilation only)
    CCFLAGS = +snes-hi -lm -maxoptpasses=300 -O3 -inline-depth=1000 -unroll-all -fp-associative -force-statics -range-opt -I"$(SHARED_SRC_DIR)" -I"lib" -I"include" -I"elua-0.9/inc" -I"elua-0.9/inc/snes" -I"elua-0.9/src/lua" -I"elua-0.9/inc/newlib" -D__VBCC__=1 -DLUA_CROSS_COMPILER -D__VBCC65816__ -c
    ASFLAGS = -816 -quiet -nowarn=62 -opt-branch -ldots -Fvobj -underscore
    
    # 4. Linker Flags (Direct vlink arguments)
    # We explicitly include the startup.o, standard libs, and ROM size here
    LDFLAGS = -b rawbin1 -nowarn=22 -Cvbcc \
              -T$(VBCC)/targets/65816-snes/vlink-hi.cmd \
              -L$(VBCC)/targets/65816-snes/lib \
              $(VBCC)/targets/65816-snes/lib/startup.o \
              -DROMSIZE=0x400000 \
              -symfile $(BUILD_DIR)/mainBankZero_vbcc65816.sym \
              -M$(BUILD_DIR)/mainBankZero_vbcc65816.map
    
    INCLUDES = 
    OUTPUT_EXT = .smc
    # This script checks if the file exists, then replaces all colons with spaces
    POST_LINK = if [ -f $(BUILD_DIR)/mainBankZero_vbcc65816.sym ]; then \
                    sed -i -e 's/:/ /g' -e 's/0x//g' $(BUILD_DIR)/mainBankZero_vbcc65816.sym; \
                    echo "Processed $(BUILD_DIR)/mainBankZero_vbcc65816.sym"; \
                fi
    COMPILER_NAME = vbcc65816
endif

# 1. NEW: VBCC Classic Configuration (Supports -O4)
ifeq ($(COMPILER_LOWER),vbcc_classic)
    CC = $(VBCC)/bin/vc
    AS = $(VBCC)/bin/vasm6502_oldstyle
    # In classic mode, we use vc as the linker to handle LTO (-O4)
    LD = $(VBCC)/bin/vc
    
    # Enable -O4 for whole-program optimization
    CCFLAGS = +snes-hi -lm -maxoptpasses=300 -O4 -inline-depth=1000 -unroll-all -fp-associative -force-statics -range-opt -I"$(SHARED_SRC_DIR)" -I"lib" -I"include" -I"elua-0.9/inc" -I"elua-0.9/inc/snes" -I"elua-0.9/src/lua" -I"elua-0.9/inc/newlib" -D__VBCC__=1 -DLUA_CROSS_COMPILER -D__VBCC65816__ -c
    ASFLAGS = -816 -quiet -nowarn=62 -opt-branch -ldots -Fvobj -underscore
    
    # Linker Flags:
    # 1. We use --M to generate a map file. The 'vc' driver strips one '-' and passes it to vlink.
    # 2. We avoid -symfile here because the 'vc' driver reorders paths with spaces, breaking the link.
    # 3. --DROMSIZE is passed through to the linker script.
    LDFLAGS = +snes-hi -lm -maxoptpasses=300 -O3 -inline-depth=1000 -unroll-all -fp-associative -force-statics -range-opt \
              -I"$(SHARED_SRC_DIR)" -I"lib" -I"include" -I"elua-0.9/inc" -I"elua-0.9/inc/snes" -I"elua-0.9/src/lua" -I"elua-0.9/inc/newlib" \
              -D__VBCC__=1 -DLUA_CROSS_COMPILER -D__VBCC65816__ \
              --M$(BUILD_DIR)/mainBankZero_vbcc65816.map \
              --DROMSIZE=0x400000
    
    OUTPUT_EXT = .smc
    
    # Post-link Logic:
    # 1. Check if the Map file exists.
    # 2. Extract lines containing "0x" (the symbols).
    # 3. Use awk to take the Address ($1) and Symbol Name ($2).
    # 4. Strip the "0x" prefix to match your required .sym format.
    POST_LINK = if [ -f $(BUILD_DIR)/mainBankZero_vbcc65816.map ]; then \
                    grep "0x" $(BUILD_DIR)/mainBankZero_vbcc65816.map | grep -v "ROM" | awk '{print $$1 " " $$2}' | sed 's/0x//g' > $(BUILD_DIR)/mainBankZero_vbcc65816.sym; \
                    echo "Generated $(BUILD_DIR)/mainBankZero_vbcc65816.sym from Map file"; \
                fi
                
    COMPILER_NAME = vbcc_classic
endif
# Calypsi Configuration
ifeq ($(COMPILER_LOWER),calypsi)
	CC = $(CALYPSI_PATH)/cc65816
	AS = $(CALYPSI_PATH)/cc65816
	LD = $(CALYPSI_PATH)/ln65816
	
	ifeq ($(ROM_TYPE),huge)
		CCFLAGS += --core=65816 -O0 --code-model=large --data-model=huge --target=SNES --list-file=$(BUILD_DIR)/calypsi.lst -D__CALYPSI__=1
		STDLIB = $(CALYPSI_PATH)/../lib-huge/clib-huge.a
		LDFLAGS = --raw-multiple-memories --rom-code --no-tree-shaking --no-copy-initialize huge
	else
		CCFLAGS += --core=65816 -O2 --speed --code-model=large --data-model=large --target=SNES --list-file=$(BUILD_DIR)/calypsi.lst -D__CALYPSI__=1
		STDLIB = $(CALYPSI_PATH)/../lib/clib-lc-ld-snes.a
		LDFLAGS = --raw-multiple-memories --rom-code
	endif
	
	ASFLAGS =
	INCLUDES = -I"$(SHARED_SRC_DIR)" -I"lib" -I"include"
	OUTPUT_EXT = .smc
	
	ifeq ($(ROM_TYPE),huge)
		ifeq ($(ROM_MAPPING),HiROM)
			LINKER_SCRIPT = $(SHARED_PORT_DIR)/calypsi/linker-large-large-HiROM.scm
			POST_LINK = python3 $(SHARED_PORT_DIR)/calypsi/ConvertIntelHex_HiROM.py $(BUILD_DIR)/calypsi.hex $(BUILD_DIR)/mainBankZero_calypsi.smc
		else
			LINKER_SCRIPT = $(SHARED_PORT_DIR)/calypsi/linker-large-large-LoROM.scm
			POST_LINK = python3 $(SHARED_PORT_DIR)/calypsi/ConvertIntelHex_LoROM.py $(BUILD_DIR)/calypsi.hex $(BUILD_DIR)/mainBankZero_calypsi.smc
		endif
	else
		ifeq ($(ROM_TYPE),HiROM)
			LINKER_SCRIPT = $(SHARED_PORT_DIR)/calypsi/linker-large-large-HiROM.scm
			POST_LINK = python3 $(SHARED_PORT_DIR)/calypsi/ConvertIntelHex_HiROM.py $(BUILD_DIR)/calypsi.hex $(BUILD_DIR)/mainBankZero_calypsi.smc
		else
			LINKER_SCRIPT = $(SHARED_PORT_DIR)/calypsi/linker-large-large-LoROM.scm
			POST_LINK = python3 $(SHARED_PORT_DIR)/calypsi/ConvertIntelHex_LoROM.py $(BUILD_DIR)/calypsi.hex $(BUILD_DIR)/mainBankZero_calypsi.smc
		endif
	endif
	COMPILER_NAME = calypsi
endif

# LLVM-Mos Configuration
ifeq ($(COMPILER_LOWER),llvm-mos)
	# Root directory of the SDK
	LLVM_MOS_SDK_ROOT ?= /media/gamesSteam/llvm-mos-sdk-snes

	# Directory where the compiler binary is located
	LLVM_MOS_BIN_DIR ?= $(LLVM_MOS_SDK_ROOT)/_deps/llvm-mos-src/bin

	# Directory where the compiled headers and libs are located
	# (Matches the path found in your grep output)
	LLVM_MOS_PLATFORM_ROOT ?= $(LLVM_MOS_SDK_ROOT)/mos-platform/build/install/mos-platform

	CC = $(LLVM_MOS_BIN_DIR)/mos-clang
	AS = $(LLVM_MOS_BIN_DIR)/mos-clang
	LD = $(LLVM_MOS_BIN_DIR)/mos-clang

	# FLAGS
	# Point includes to the 'install' directory
	CCFLAGS = -mcpu=mosw65816 \
	          -I"$(LLVM_MOS_PLATFORM_ROOT)/common/include" \
	          -I"$(LLVM_MOS_PLATFORM_ROOT)/snes/include" \
	          -I"$(SHARED_SRC_DIR)" -Iinclude -Oz -flto -fnonreentrant -ffast-math \
	          -funroll-loops -finline-functions -fomit-frame-pointer \
	          -fno-stack-protector -fdata-sections -ffunction-sections 

	ASFLAGS = 

	# Linker Flags
	# Point library paths (-L) to the 'install' directory
	LDFLAGS = -T $(SHARED_PORT_DIR)/llvm-mos/linker.ld \
	          -L"$(LLVM_MOS_PLATFORM_ROOT)/common/lib" \
	          -L"$(LLVM_MOS_PLATFORM_ROOT)/snes/lib" \
	          -lzero-bss -lcopy-data -lcopy-zp-data -linit-stack -lc -lexit-loop

	INCLUDES = 
	OUTPUT_EXT = .smc
	POST_LINK = 
	COMPILER_NAME = llvm-mos
endif

# CC65 Configuration
ifeq ($(COMPILER_LOWER),cc65)
	CC = cc65
	AS = ca65
	LD = ld65
	CCFLAGS = -t none -O -I$(SHARED_SRC_DIR) -Iinclude -D__CC65__=1
	ASFLAGS = -t none
	LDFLAGS = -C $(SHARED_PORT_DIR)/cc65/snes.cfg -o -m $(BUILD_DIR)/mainBankZero_cc65.map --no-smc
	INCLUDES = 
	OUTPUT_EXT = .smc
	POST_LINK = 
	COMPILER_NAME = cc65
endif

# JCC816 Configuration
ifeq ($(COMPILER_LOWER),jcc816)
	CC = python3 $(SHARED_PORT_DIR)/jcc816/compile.py
	AS = python3 $(SHARED_PORT_DIR)/jcc816/compile.py
	LD = python3 $(SHARED_PORT_DIR)/jcc816/compile.py
	CCFLAGS = -l example=$(SHARED_PORT_DIR)/jcc816/exampleHeader.xml -O 0 -D 2 -V 2 -r build
	ASFLAGS = 
	LDFLAGS = 
	INCLUDES = 
	OUTPUT_EXT = .sfc
	POST_LINK = 
	COMPILER_NAME = jcc816
endif

# TCC816 (pvsneslib) Configuration
ifeq ($(COMPILER_LOWER),tcc816)
	CC = python3 $(SHARED_PORT_DIR)/tcc816/compile.py
	AS = python3 $(SHARED_PORT_DIR)/tcc816/compile.py
	LD = python3 $(SHARED_PORT_DIR)/tcc816/compile.py
	CCFLAGS = -c -I$(SHARED_SRC_DIR) -Iinclude -I.
	ASFLAGS = 
	LDFLAGS = 
	INCLUDES = 
	OUTPUT_EXT = .obj
	POST_LINK = 
	COMPILER_NAME = tcc816
endif

# =============================================================================
# SOURCE CONFIGURATIONS
# =============================================================================

# WDC816CC Source Configuration
ifeq ($(COMPILER_LOWER),wdc816cc)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_PORT_DIR)/wdc816cc/lorom/kernel.c $(SHARED_SRC_DIR)/initsnes.c
	ASM_SOURCES = $(SHARED_PORT_DIR)/wdc816cc/lorom/vectors.asm
	PROJECT_OBJECTS = $(addprefix $(BUILD_DIR)/,$(addsuffix .obj,$(basename $(PROJECT_C_FILES))))
	OBJECTS = $(PROJECT_OBJECTS) $(BUILD_DIR)/kernel.obj $(BUILD_DIR)/initsnes.obj $(BUILD_DIR)/vectors.obj
	vpath %.c $(SHARED_PORT_DIR)/wdc816cc/lorom $(SHARED_SRC_DIR) .
	vpath %.asm $(SHARED_PORT_DIR)/wdc816cc/lorom
	vpath %.h .
endif

# VBCC65816 Source Configuration
ifneq ($(filter vbcc65816 vbcc_classic,$(COMPILER_LOWER)),)
    PROJECT_C_FILES = $(wildcard *.c)
    C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c
    ASM_SOURCES = snesmod_api.s
    PROJECT_OBJECTS = $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(PROJECT_C_FILES))))
    
    # NEW: Add the soundbank object specifically
    AUDIO_OBJ =
    ifneq ($(strip $(AUDIOFILES)),)
        AUDIO_OBJ = $(BUILD_DIR)/soundbank_vasm.o
    endif

    OBJECTS = $(PROJECT_OBJECTS) $(BUILD_DIR)/initsnes.o \
              $(BUILD_DIR)/snesmod_api.o $(AUDIO_OBJ)
              
    vpath %.c $(SHARED_SRC_DIR) .
    vpath %.s res/  # Ensure vpath can find your new .s file
endif

# Calypsi Source Configuration
ifeq ($(COMPILER_LOWER),calypsi)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c
	ASM_SOURCES = 
	PROJECT_OBJECTS = $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(PROJECT_C_FILES))))
	OBJECTS = $(PROJECT_OBJECTS) $(BUILD_DIR)/initsnes.o
	vpath %.c $(SHARED_SRC_DIR) .
	vpath %.asm 
	vpath %.h .
endif

# LLVM-Mos Source Configuration
ifeq ($(COMPILER_LOWER),llvm-mos)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c $(SHARED_PORT_DIR)/llvm-mos/putchar_stub.c
	ASM_SOURCES = $(SHARED_PORT_DIR)/llvm-mos/startup.s $(SHARED_PORT_DIR)/llvm-mos/vectors.s
	OBJECTS = 
	vpath %.c $(SHARED_SRC_DIR) $(SHARED_PORT_DIR)/llvm-mos .
	vpath %.s $(SHARED_PORT_DIR)/llvm-mos
	vpath %.asm 
	vpath %.h .
endif

# CC65 Source Configuration
ifeq ($(COMPILER_LOWER),cc65)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c $(SHARED_PORT_DIR)/cc65/putchar_stub.c
	ASM_SOURCES = $(SHARED_PORT_DIR)/cc65/snes_header.s $(SHARED_PORT_DIR)/cc65/runtime_stubs.s
	PROJECT_OBJECTS = $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(PROJECT_C_FILES))))
	OBJECTS = $(PROJECT_OBJECTS) $(BUILD_DIR)/initsnes.o $(BUILD_DIR)/putchar_stub.o $(BUILD_DIR)/snes_header.o $(BUILD_DIR)/runtime_stubs.o
	vpath %.c $(SHARED_SRC_DIR) $(SHARED_PORT_DIR)/cc65 .
	vpath %.asm $(SHARED_PORT_DIR)/cc65
	vpath %.h .
endif

# JCC816 Source Configuration
ifeq ($(COMPILER_LOWER),jcc816)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c
	ASM_SOURCES = 
	OBJECTS = 
	vpath %.c $(SHARED_SRC_DIR) .
	vpath %.asm 
	vpath %.h .
endif

# TCC816 Source Configuration
ifeq ($(COMPILER_LOWER),tcc816)
	PROJECT_C_FILES = $(wildcard *.c)
	C_SOURCES = $(PROJECT_C_FILES) $(SHARED_SRC_DIR)/initsnes.c
	ASM_SOURCES = 
	PROJECT_OBJECTS = $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(PROJECT_C_FILES))))
	OBJECTS = $(PROJECT_OBJECTS)
	vpath %.c $(SHARED_SRC_DIR) .
	vpath %.asm 
	vpath %.h .
endif

# =============================================================================
# BUILD RULES
# =============================================================================

# Default target
all: $(BUILD_DIR)
ifeq ($(COMPILER_LOWER),wdc816cc)
	@$(MAKE) $(OBJECTS)
	$(LD) $(LDFLAGS)
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),calypsi)
	@$(MAKE) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LINKER_SCRIPT) $(STDLIB) --list-file=$(BUILD_DIR)/calypsi.lst --cross-reference --output-format=intel-hex -o $(BUILD_DIR)/calypsi.hex
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),llvm-mos)
	@echo "Compiling with LLVM-MOS..."
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $(BUILD_DIR)/mainBankZero_llvm-mos$(OUTPUT_EXT) $(C_SOURCES) $(ASM_SOURCES)
	@echo "Compilation completed successfully"
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),cc65)
	@$(MAKE) $(OBJECTS)
	$(LD) -C $(SHARED_PORT_DIR)/cc65/snes.cfg -o $(BUILD_DIR)/mainBankZero_cc65$(OUTPUT_EXT) -m $(BUILD_DIR)/mainBankZero_cc65.map $(OBJECTS) lib/none.lib
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),jcc816)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(C_SOURCES)
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),tcc816)
	$(CC) $(CCFLAGS) $(INCLUDES) $(C_SOURCES)
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),vbcc_classic)
	@echo "Compiling with VBCC Classic (-O4 LTO enabled)..."
	@$(MAKE) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(BUILD_DIR)/mainBankZero_vbcc65816$(OUTPUT_EXT)
	$(POST_LINK)
else
ifeq ($(COMPILER_LOWER),vbcc65816)
	@echo "Compiling with VBCC65816 (incremental compilation)..."
	@$(MAKE) $(OBJECTS)
	@echo "Linking object files with vbcc..."
	@echo "OBJECTS variable: $(OBJECTS)"
	@echo "Compiling and linking all sources together..."
	$(LD) $(LDFLAGS) $(OBJECTS) -lvc -lm  -o $(BUILD_DIR)/mainBankZero_vbcc65816$(OUTPUT_EXT)
	@echo "Compilation completed successfully"
	$(POST_LINK)
else
	@echo "Unknown compiler: $(COMPILER)"
	@echo "Available compilers: wdc816cc, vbcc65816, calypsi, llvm-mos, cc65, jcc816, tcc816"
	@exit 1
endif
endif
endif
endif
endif
endif
endif

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# WDC816CC specific rules
ifeq ($(COMPILER_LOWER),wdc816cc)
$(BUILD_DIR)/%.obj: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/%.obj: %.asm
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<
endif

# Calypsi specific rules
ifeq ($(COMPILER_LOWER),calypsi)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ $<
endif

# TCC816 specific rules
ifeq ($(COMPILER_LOWER),tcc816)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(INCLUDES) $<
endif

# VBCC65816 specific rules
ifneq ($(filter vbcc65816 vbcc_classic,$(COMPILER_LOWER)),)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ $<
endif
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<
endif

# CC65 specific rules
ifeq ($(COMPILER_LOWER),cc65)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$*.s $<
	$(AS) $(ASFLAGS) -o $@ $(BUILD_DIR)/$*.s

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/snes_header.o: $(SHARED_PORT_DIR)/cc65/snes_header.s
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/runtime_stubs.o: $(SHARED_PORT_DIR)/cc65/runtime_stubs.s
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<
endif

# Clean target
clean: cleanAudio
	rm -rf $(BUILD_DIR)
	rm -f *.obj *.o *.bin *.bnk *.map *.smc PROG.LINK *.sfc
	@echo Clean complete!

# Convenience targets
wdc: clean
	@$(MAKE) COMPILER=wdc816cc

vbcc65816: clean
	@$(MAKE) COMPILER=vbcc65816

calypsi: clean
	@$(MAKE) COMPILER=calypsi

llvm-mos: clean
	@$(MAKE) COMPILER=llvm-mos

cc65: clean
	@$(MAKE) COMPILER=cc65

jcc816: clean
	@$(MAKE) COMPILER=jcc816

tcc816: clean
	@$(MAKE) COMPILER=tcc816

# Show current compiler
info:
	@echo Current compiler: $(COMPILER)
	@echo C sources: $(C_SOURCES)
	@echo Objects: $(OBJECTS)

# Help target
help:
	@echo Available targets:
	@echo "  make COMPILER=wdc816cc  - Build with WDC816CC compiler (Requires Wine)"
	@echo "  make COMPILER=vbcc65816 - Build with VBCC65816 compiler"
	@echo "  make COMPILER=calypsi   - Build with Calypsi compiler"
	@echo "  make COMPILER=llvm-mos  - Build with LLVM-Mos compiler"
	@echo "  make COMPILER=cc65      - Build with CC65 compiler"
	@echo "  make COMPILER=jcc816    - Build with JCC816 compiler"
	@echo "  make COMPILER=tcc816    - Build with TCC816 (pvsneslib) compiler"
	@echo "  make clean              - Clean build artifacts"
	@echo ""
	@echo "Note: Ensure your tools are in PATH or edit the Variables at the top of the Makefile."
	@echo ""
	@echo Calypsi ROM type options:
	@echo "  ROM_TYPE=HiROM          - Use HiROM mapping (default: LoROM)"
	@echo "  ROM_TYPE=huge           - Use huge data/code model with --enable-huge-attribute"
	@echo "  ROM_MAPPING=HiROM       - Use HiROM mapping when ROM_TYPE=huge (default: LoROM)"

# Phony targets
.PHONY: all clean wdc vbcc65816 calypsi llvm-mos cc65 jcc816 tcc816 info help



# 1. Run smconv to get the .bnk and .h (ignore the .asm it generates)
$(SOUNDBANK_BNK): $(AUDIOFILES)
	@echo "--- Generating Soundbank ---"
	@mkdir -p res
	@# Verify files exist to catch path errors early
	@for f in $(AUDIOFILES); do \
		if [ ! -f $$f ]; then echo "ERROR: File $$f not found!"; exit 1; fi; \
	done
	@# Run smconv from the project root
	$(SMCONV) $(SMCONVFLAGS) $(AUDIOFILES)
	@if [ ! -f $(SOUNDBANK_BNK) ]; then \
		echo "ERROR: smconv failed to create $(SOUNDBANK_BNK)"; \
		exit 1; \
	fi

$(BUILD_DIR)/soundbank_vasm.o: $(SOUNDBANK_ASM_WRAPPER) $(SOUNDBANK_BNK)
	@mkdir -p $(BUILD_DIR)
	@echo "Assembling VASM Soundbank Wrapper..."
	$(AS) $(ASFLAGS) -Ires -o $@ $<

# Update cleanAudio to clean the binary and header
cleanAudio:
	@echo "Cleaning Audio..."
	rm -f res/soundbank.asm res/soundbank.h res/soundbank.bnk