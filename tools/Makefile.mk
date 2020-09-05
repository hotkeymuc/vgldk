# Master Makefile
#
#	Examples:
#		make cart
#			Compile a cartridge ROM
#		
#		make cart VGLDK_SERIES=4000
#			Compile a cartridge ROM targeting a specific model
#		
#		make emu
#			Emulate the cartridge
#		
#		make burn
#			Burn the cartridge using MiniPro
#		
#		make app VGLDK_SERIES=0 LOC_CODE=0xc800 LOC_DATA=0xc200
#			Compile a small loadable app at/for given memory location
#		
#		make upload
#			Upload the app binary via serial to a machine running monitor.c
#


# Main file must be specified
#NAME = test

# A target architecture must be specified
#ifndef VGLDK_SERIES
#$(error VGLDK_SERIES is not set)
#	#VGLDK_SERIES = 0
#	#VGLDK_SERIES = 1000
#	#VGLDK_SERIES = 2000
#	#VGLDK_SERIES = 4000
#	#VGLDK_SERIES = 6000
#endif


# Start up / boot options
#	
#	CRT_NAME = cart_pc1000_crt0
#		Specifies which crt0-file to prepend to the actual code binary
#		This contains some "magic" bytes that make the cartridge auto-run
#	
#	ADDR_CART = 0x8000
#		Where the cartridge ROM will reside at
#	
#	LOC_CODE = 0x8000
#		Where the code segment starts
#		Usually the cartridge start address
#	
#	LOC_CODE_MAIN = 0x8020
#		Where the C code section will reside
#		This must leave enough space for the CRT0 code preceding it.
#	
#	LOC_DATA = 0xc000
#		Where variables are stored
#		Usually the RAM start address
#		Series 1000     : 0x4000 - 0x47ff
#		Series 2000/4000: 0xc000 - 0xdfff
#	
# 


#
# Systems / Architectures
#
ARCH_ID ?= gl$(VGLDK_SERIES)
EMU_SYS ?= $(ARCH_ID)
ADDR_RAM ?= 0xc000
ADDR_CART ?= 0x8000

ifeq ($(VGLDK_SERIES),0)
	ARCH_ID = plain
	EMU_SYS = gl6000sl
	#EMU_SYS = gl7007sl
	
	CRT_NAME = plain_crt0
	CRT_SIZE = 0
	#LOC_CODE ?= $(ADDR_RAM)
	#LOC_DATA ?= $(ADDR_RAM)
endif
ifeq ($(VGLDK_SERIES),1000)
	ARCH_ID = pc1000
	EMU_SYS = pc1000
	
	ADDR_RAM ?= 0x4000
endif
ifeq ($(VGLDK_SERIES),2000)
	ARCH_ID = gl2000
	EMU_SYS = gl2000
endif
ifeq ($(VGLDK_SERIES),3000)
	ARCH_ID = gl3000s
	EMU_SYS = gl3000s
endif
ifeq ($(VGLDK_SERIES),4000)
	ARCH_ID = gl4000
	EMU_SYS = gl4000
	#EMU_SYS = gl4004
endif
ifeq ($(VGLDK_SERIES),5000)
	ARCH_ID = gl5000
	EMU_SYS = gl5000
endif
ifeq ($(VGLDK_SERIES),6000)
	ARCH_ID = gl6000sl
	EMU_SYS = gl6000sl
	#EMU_SYS = gl7007sl
endif


# Addresses
LOC_CODE ?= $(ADDR_CART)
LOC_DATA ?= $(ADDR_RAM)

ADDR_RAM_DECIMAL = $(shell printf "%d" $(ADDR_RAM) )
ADDR_CART_DECIMAL = $(shell printf "%d" $(ADDR_CART) )

LOC_CODE_DECIMAL = $(shell printf "%d" $(LOC_CODE) )
LOC_DATA_DECIMAL = $(shell printf "%d" $(LOC_DATA) )

# CRT0 infos
CRT_NAME ?= cart_crt0
CRT_SIZE ?= 32
CRT_S_FILE = $(ARCH_DIR)/$(CRT_NAME).s
CRT_REL_FILE = $(OUT_DIR)/$(NAME).$(CRT_NAME).rel

LOC_CODE_MAIN_DECIMAL = $(shell echo $(LOC_CODE_DECIMAL) + $(CRT_SIZE) | bc )
LOC_CODE_MAIN = $(shell printf "0x%04x" $(LOC_CODE_MAIN_DECIMAL) )


# Cartridge output options
#  8 =  8KB = 0x2000 = AT28C64B
# 32 = 32KB = 0x8000 = AT28C256
CART_SIZE_KB ?= 8
#CART_SIZE_KB = 32

# EEPROM burning options
ifeq ($(CART_SIZE_KB),8)
	CART_PART = CAT28C64B
	#CART_PART = AT28C64B
endif
ifeq ($(CART_SIZE_KB),32)
	CART_PART = AT28C256
endif


# Directories
OUT_DIR = out
TOOLS_DIR = ../../tools
#INC_DIR = `realpath ../include`
INC_DIR = ../../include
ARCH_DIR = $(INC_DIR)/arch/$(ARCH_ID)
LIB_DIR = $(INC_DIR)
EMU_ROM_DIR = /z/apps/_emu/_roms


# Filenames
INPUT_FILE = $(NAME).c

OUTPUT_FILE_HEX = $(OUT_DIR)/$(NAME).ihx
OUTPUT_FILE_HEX_BIN = $(OUT_DIR)/$(NAME).ihx.bin

OUTPUT_FILE_CART = $(OUT_DIR)/$(NAME).cart.$(CART_SIZE_KB)kb.bin
OUTPUT_FILE_APP = $(OUT_DIR)/$(NAME).app.$(LOC_CODE).bin


# Intermediate files to keep (else, they will be removed automatically after make succeeds)
.SECONDARY: $(OUTPUT_FILE_HEX) $(OUTPUT_FILE_HEX_BIN)


# Commands
MKDIR_P = mkdir -p
CC = sdcc
SDASZ80 = sdasz80
OBJCOPY = objcopy
DD = dd
MAME = mame
MINIPRO = minipro
#REL2APP = python $(TOOLS_DIR)/rel2app.py
SEND2MONITOR = python3 $(TOOLS_DIR)/send2monitor.py
CALCSIZE = python $(TOOLS_DIR)/calcsize.py

# Targets
.PHONY: info clean emu burn

all: info

info:
	@echo Compiling \"$(NAME)\"
	@echo VGLDK_SERIES=$(VGLDK_SERIES), ARCH_ID=$(ARCH_ID)
	@echo ADDR_CART=$(ADDR_CART), ADDR_RAM=$(ADDR_RAM)
	@echo VGLDK_TARGET=$(VGLDK_TARGET), CRT0=$(CRT_NAME)
	@echo LOC_CODE=$(LOC_CODE), LOC_DATA=$(LOC_DATA), LOC_CODE_MAIN=$(LOC_CODE_MAIN)
	@#echo Using CRT0 file $(CRT_NAME) at $(CRT_S_FILE)

create_out_dir:
	@if ! [ -d $(OUT_DIR) ]; then $(MKDIR_P) $(OUT_DIR); fi

cart:: VGLDK_TARGET = cart
cart:: info $(OUTPUT_FILE_CART)
	### Info
	@echo Cartridge file $(OUTPUT_FILE_CART) was successfully created.
	@$(CALCSIZE) $(OUTPUT_FILE_CART)

app:: VGLDK_SERIES ?= 0
#app:: ARCH_ID = plain
app:: CRT_NAME = plain_crt0
app:: CRT_SIZE = 0
#app:: CRT_S_FILE = $(ARCH_DIR)/$(CRT_NAME).s
app:: CRT_S_FILE = $(INC_DIR)/arch/plain/plain_crt0.s
app:: CRT_REL_FILE = $(OUT_DIR)/$(NAME).$(CRT_NAME).rel
#app:: LOC_CODE = $(ADDR_RAM)
app:: info $(OUTPUT_FILE_APP)
	@#$(REL2APP) $(OUTPUT_FILE_REL)
	@#$(REL2APP) $(OUT_DIR)/$(NAME)
	@echo App file $(OUTPUT_FILE_APP) was created.
	@$(CALCSIZE) $(OUTPUT_FILE_APP)


$(CRT_REL_FILE): create_out_dir $(CRT_S_FILE)
	### Compiling CRT0 $< to $@
	$(SDASZ80) -o $(CRT_REL_FILE) $(CRT_S_FILE)


$(OUT_DIR)/%.ihx: %.c $(CRT_REL_FILE)
	### Compiling $< and $(CRT_REL_FILE) to $@
	@# --out-fmt-s19
	@# --out-fmt-ihx
	$(CC) -mz80 --no-std-crt0 \
	-D VGLDK_SERIES=$(VGLDK_SERIES) \
	--code-loc $(LOC_CODE_MAIN) --data-loc $(LOC_DATA) \
	--lib-path $(LIB_DIR) -I $(INC_DIR) -I $(ARCH_DIR) \
	-o $@ \
	$(CRT_REL_FILE) $<

%.ihx.bin: %.ihx
	### Building memory dump binary $@
	@#$(OBJCOPY) -Iihex -Obinary $< $@
	@#makebin -s 65536 $< $@
	@#      -p = only extract minimal amount
	makebin -p -s 65536 $< $@

# Cartridge
#%.cart.$(CART_SIZE_KB)kb.bin: %.ihx.bin
$(OUTPUT_FILE_CART): $(OUTPUT_FILE_HEX_BIN)
	### Extracting cartridge section from $< ($(ADDR_CART) and $(CART_SIZE_KB) KB up)
	$(DD) iflag=skip_bytes skip=$(ADDR_CART_DECIMAL) bs=1024 count=$(CART_SIZE_KB) if=$< of=$@ status=none
	
	@# Create empty EPROM file filled with FF (full cart size)
	@#dd if=/dev/zero ibs=1k count=$(CART_SIZE_KB) status=none | tr "\000" "\377" >$@
	@## Copy bin data into it
	@#dd if=$< of=$@ conv=notrunc
	@$(CALCSIZE) $(OUTPUT_FILE_CART)

# App
#%.$(LOC_CODE).app.bin: %.ihx.bin
$(OUTPUT_FILE_APP): $(OUTPUT_FILE_HEX_BIN)
	# Extract code section from $< ($(LOC_CODE) and up)
	$(DD) iflag=skip_bytes skip=$(LOC_CODE_DECIMAL) if=$< of=$@ #status=none


# Helpers
clean:
	rm -f $(OUT_DIR)/$(NAME).*

emu: $(OUTPUT_FILE_CART)
	### Starting MAME using the cartridge file \"$(OUTPUT_FILE_CART)\"
	$(MAME) \
	$(EMU_SYS) \
	-rompath "$(EMU_ROM_DIR)" \
	-cart "$(OUTPUT_FILE_CART)" \
	-window -nomax -nofilter -sleep \
	-speed 2.00 -volume -24 \
	-skip_gameinfo -nomouse
	@# -debug	# Attach debug console and STEP
	
	# Remove MAME history directory
	rm -r history
	
	@# Remove MESS config directory that is created
	@#rm -r cfg

burn: $(OUTPUT_FILE_CART)
	# Burn image "$(OUTPUT_FILE_CART)" to (E)EPROM of type "$(CART_PART)"
	@# Show size analysis
	@$(CALCSIZE) $(OUTPUT_FILE_CART)
	@# Use -s = no warning for file size mismatch
	$(MINIPRO) -p "$(CART_PART)" -w $(OUTPUT_FILE_CART)

upload: $(OUTPUT_FILE_APP)
	@# Show size analysis
	@$(CALCSIZE) $(OUTPUT_FILE_APP)
	$(SEND2MONITOR) --dest=$(LOC_CODE) $(OUTPUT_FILE_APP)