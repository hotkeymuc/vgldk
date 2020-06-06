# Master Makefile
#
# make cart
# make cart VGLDK_SERIES=4000
# make app
#

#NAME = test

ifndef VGLDK_SERIES
$(error VGLDK_SERIES is not set)
	#VGLDK_SERIES = 0
	#VGLDK_SERIES = 1000
	#VGLDK_SERIES = 2000
	#VGLDK_SERIES = 4000
	#VGLDK_SERIES = 6000
endif


# Start up / boot options
#	
#	CRT_NAME = cart_pc1000_crt0
#		Specifies which crt0-file to prepend to the actual code binary
#		This contains some "magic" bytes that make the cartridge auto-run
#	
#	LOC_CART = 0x8000
#		Where the cartridge ROM will reside at
#	
#	LOC_CODE = 0x8010
#		Where the C code section will reside in address space while running
#		This must leave enough space for the CRT0 code preceding it!
#	
#	LOC_DATA = 0xc000
#		Where variables are stored
#		Series 1000     : 0x4000 - 0x47ff
#		Series 2000/4000: 0xc000 - 0xdfff
#	
# 


#
# Systems / Architectures
#

#ifeq (${VGLDK_SERIES},0)
#	ARCH_ID = app
#	EMU_SYS = gl4004
#	CRT_NAME = app_crt0
#	
#	LOC_CODE ?= 0xc400
#	LOC_DATA ?= 0xc000
#endif

ifeq (${VGLDK_SERIES},0)
	ARCH_ID = plain
	EMU_SYS = gl6000sl
	#EMU_SYS = gl7007sl
	
	CRT_NAME = plain_crt0
	LOC_CODE ?= 0xc800
	LOC_DATA ?= 0xc000
endif
ifeq (${VGLDK_SERIES},1000)
	ARCH_ID = pc1000
	EMU_SYS = pc1000
	
	LOC_CODE ?= 0x8020
	LOC_DATA ?= 0x4000
endif
ifeq (${VGLDK_SERIES},2000)
	ARCH_ID = gl2000
	EMU_SYS = gl2000
endif
ifeq (${VGLDK_SERIES},3000)
	ARCH_ID = gl3000s
	EMU_SYS = gl3000s
endif
ifeq (${VGLDK_SERIES},4000)
	ARCH_ID = gl4000
	EMU_SYS = gl4000
	#EMU_SYS = gl4004
endif
ifeq (${VGLDK_SERIES},5000)
	ARCH_ID = gl5000
	EMU_SYS = gl5000
endif
ifeq (${VGLDK_SERIES},6000)
	ARCH_ID = gl6000sl
	EMU_SYS = gl6000sl
	#EMU_SYS = gl7007sl
endif



#
# Defaults
#

ARCH_ID ?= gl${VGLDK_SERIES}
EMU_SYS ?= ${ARCH_ID}
LOC_CART ?= 0x8000
LOC_CODE ?= 0x8010
LOC_DATA ?= 0xc000

CRT_NAME ?= cart_crt0

LOC_CART_DECIMAL=$(shell printf "%d" $(LOC_CART) )

# Cartridge output options
#  8 =  8KB = 0x2000 = AT28C64B
# 32 = 32KB = 0x8000 = AT28C256
OUTPUT_CART_SIZE_KB = 8
#OUTPUT_CART_SIZE_KB = 32

# EEPROM burning options
ifeq (${OUTPUT_CART_SIZE_KB},8)
	#PART = CAT28C64B
	PART = AT28C64B
endif
ifeq (${OUTPUT_CART_SIZE_KB},32)
	PART = AT28C256
endif


# Directories
#LIB_DIR = `realpath ../include`
INC_DIR = ../../include
ARCH_DIR = ${INC_DIR}/arch/${ARCH_ID}
LIB_DIR = ${INC_DIR}
OUT_DIR = out
EMU_ROM_DIR = "/z/apps/_emu/_roms"


# Filenames
INPUT_FILE = ${NAME}.c
CRT_S_FILE = ${ARCH_DIR}/${CRT_NAME}.s
CRT_REL_FILE = ${OUT_DIR}/${NAME}.${CRT_NAME}.rel
OUTPUT_FILE_HEX = ${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_BIN64K = ${OUT_DIR}/${NAME}.64k.bin
OUTPUT_FILE_BIN = ${OUT_DIR}/${NAME}.bin
OUTPUT_FILE_CART = ${OUT_DIR}/${NAME}.cart.bin

OUTPUT_FILE_APP = ${OUT_DIR}/${NAME}.app.$(LOC_CODE).bin

# Emulation options

# Commands
MKDIR_P = mkdir -p
CC = sdcc
SDASZ80 = sdasz80
OBJCOPY = objcopy
DD = dd
MAME = mame


# Targets
.PHONY: info clean emu burn

all: cart

info:
	@echo Compiling for VGLDK_SERIES=$(VGLDK_SERIES)

create_out_dir:
	@if ! [ -d $(OUT_DIR) ]; then ${MKDIR_P} ${OUT_DIR}; fi


cart: info ${OUTPUT_FILE_CART}

app: info ${OUTPUT_FILE_APP}

${CRT_REL_FILE}: create_out_dir ${CRT_S_FILE}
	# Compile CRT0.s file to .rel
	${SDASZ80} -o ${CRT_REL_FILE} ${CRT_S_FILE}


$(OUT_DIR)/%.hex: %.c create_out_dir ${CRT_REL_FILE}
	# Compile to .hex
	${CC} -mz80 --no-std-crt0 --vc \
	--code-loc ${LOC_CODE} --data-loc ${LOC_DATA} \
	--lib-path ${LIB_DIR} -I ${INC_DIR} -I ${ARCH_DIR} \
	-D VGLDK_SERIES=${VGLDK_SERIES} \
	-o $@ \
	${CRT_REL_FILE} $<


#${OUTPUT_FILE_BIN64K}: ${OUTPUT_FILE_HEX}
%.64k.bin: %.hex
	# Build usable cartridge ROM bin file (64K memory dump)
	@#${OBJCOPY} -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN64K}
	@#makebin -p -s 65536 @< $@
	makebin -s 65536 $< $@

%.cart.bin: %.64k.bin
	# Extract cartridge section ($(LOC_CART) and up)
	@#${DD} bs=1024 count=${OUTPUT_CART_SIZE_KB} skip=32768 iflag=skip_bytes if=$< of=$@
	${DD} iflag=skip_bytes skip=$(LOC_CART_DECIMAL) \
	bs=1024 count=${OUTPUT_CART_SIZE_KB} \
	if=$< of=$@ status=none
	
	@# Create empty EEPROM binary (full cart size)
	@#dd if=/dev/zero of=${OUTPUT_FILE_CART} bs=$(OUTPUT_CART_SIZE * 1024) count=1
	@#dd if=/dev/zero of=${OUTPUT_FILE_CART} bs=1 count=1 seek=$((OUTPUT_CART_SIZE * 1024 -1 ))
	
	@## Create file filled with FF
	@#dd if=/dev/zero ibs=1k count=${OUTPUT_CART_SIZE_KB} status=none | tr "\000" "\377" >${OUTPUT_FILE_CART} 
	@## Copy bin data into it
	@#dd if=${OUTPUT_FILE_BIN} of=${OUTPUT_FILE_CART} conv=notrunc status=none

# App
%.app.$(LOC_CODE).bin: %.64k.bin
	# Extract code section
	@#cp ${OUTPUT_FILE_BIN64K} ${OUTPUT_FILE_APP}
	cp $< $@


# Helpers
emu: ${OUTPUT_FILE_CART}
	${MAME} \
	${EMU_SYS} \
	-rompath "${EMU_ROM_DIR}" \
	-cart "${OUTPUT_FILE_CART}" \
	-window -nomax -nofilter -sleep \
	-volume -24 \
	-skip_gameinfo \
	-speed 2.00 \
	-nomouse
	# -debug	# Attach debug console and STEP
	
	# Remove MAME history directory
	rm history/*
	rmdir history
	
	# Remove MESS config directory that is created
	#rm cfg/*.cfg
	#rmdir cfg

burn: ${OUTPUT_FILE_CART}
	# Use -s = no warning for file size mismatch
	minipro -p "${PART}" -w ${OUTPUT_FILE_CART}


clean:
	rm -f ${OUT_DIR}/${NAME}.*

