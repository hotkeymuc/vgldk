# Makefile to create a cartridge ROM file


#NAME=test

ifndef VGLDK_SERIES
$(error VGLDK_SERIES is not set)
	#VGLDK_SERIES=0
	#VGLDK_SERIES=1000
	#VGLDK_SERIES=2000
	#VGLDK_SERIES=4000
	#VGLDK_SERIES=6000
endif

#ifndef VGLDK_DIRECTIO
##$(error VGLDK_DIRECTIO is not set)
#	#VGLDK_DIRECTIO=0
#	VGLDK_DIRECTIO=1
#endif


# Start up / boot options
#	
#	CRT_NAME=cart_pc1000_crt0
#		Specifies which crt0-file to prepend to the actual code binary
#		This contains some "magic" bytes that make the cartridge auto-run
#	
#	LOC_CODE=8010
#		Where the code will reside in address space while running
#		Cartridges will get mapped to 0x8000
#		This must leave some space for the CRT0 code
#	
#	LOC_DATA=c000
#		Where variables are stored
#		Series 1000     : 0x4000 - 0x47ff
#		Series 2000/4000: 0xc000 - 0xdfff
#	
# 


#
# Systems / Architectures
#

#ifeq (${VGLDK_SERIES},0)
#	ARCH_ID=app
#	EMU_SYS=gl4004
#	CRT_NAME=app_crt0
#	
#	ifndef LOC_CODE
#		LOC_CODE=c400
#	endif
#	ifndef LOC_DATA
#		LOC_DATA=c000
#	endif
#endif

ifeq (${VGLDK_SERIES},0)
	ARCH_ID=plain
	EMU_SYS=gl6000sl
	#EMU_SYS=gl7007sl
	
	CRT_NAME=plain_crt0
	ifndef LOC_CODE
		LOC_CODE=c800
	endif
	ifndef LOC_DATA
		LOC_DATA=c000
	endif
endif
ifeq (${VGLDK_SERIES},1000)
	ARCH_ID=pc1000
	EMU_SYS=pc1000
	
	ifndef LOC_CODE
		LOC_CODE=8020
	endif
	ifndef LOC_DATA
		LOC_DATA=4000
	endif
endif
ifeq (${VGLDK_SERIES},2000)
	ARCH_ID=gl2000
	EMU_SYS=gl2000
endif
ifeq (${VGLDK_SERIES},3000)
	ARCH_ID=gl3000s
	EMU_SYS=gl3000s
endif
ifeq (${VGLDK_SERIES},4000)
	ARCH_ID=gl4000
	EMU_SYS=gl4000
	#EMU_SYS=gl4004
endif
ifeq (${VGLDK_SERIES},5000)
	ARCH_ID=gl5000
	EMU_SYS=gl5000
endif
ifeq (${VGLDK_SERIES},6000)
	ARCH_ID=gl6000sl
	EMU_SYS=gl6000sl
	#EMU_SYS=gl7007sl
endif



#
# Defaults
#

ifndef ARCH_ID
	ARCH_ID=gl${VGLDK_SERIES}
endif
ifndef EMU_SYS
	EMU_SYS=${ARCH_ID}
endif
ifndef LOC_CODE
	LOC_CODE=8010
endif
ifndef LOC_DATA
	LOC_DATA=c000
endif


ifndef CRT_NAME
	CRT_NAME=cart_crt0
endif

# Cartridge output options
#  8 =  8KB = 0x2000 = AT28C64B:
# 16 = 16KB = 0x4000 = AT28C128 (?)
# 32 = 32KB = 0x8000 = AT28C256
OUTPUT_CART_SIZE_KB=8
#OUTPUT_CART_SIZE_KB=16
#OUTPUT_CART_SIZE_KB=32

# EEPROM options
#PART=CAT28C64B
PART=AT28C64B
#PART=AT28C256


# Directories
#LIB_DIR=`realpath ../include`
INC_DIR=../../include
ARCH_DIR=${INC_DIR}/arch/${ARCH_ID}
LIB_DIR=${INC_DIR}
OUT_DIR=out
EMU_ROM_DIR="/z/apps/_emu/_roms"


# Filenames
INPUT_FILE=${NAME}.c
CRT_S_FILE=${ARCH_DIR}/${CRT_NAME}.s
CRT_REL_FILE=${OUT_DIR}/${NAME}.${CRT_NAME}.rel
OUTPUT_FILE_HEX=${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_BIN64K=${OUT_DIR}/${NAME}.64k.bin
OUTPUT_FILE_BIN=${OUT_DIR}/${NAME}.bin
OUTPUT_FILE_CART=${OUT_DIR}/${NAME}.cart.bin

OUTPUT_FILE_APP=${OUT_DIR}/${NAME}.app.${LOC_CODE}.bin

# Emulation options

# Commands
MKDIR_P=mkdir -p
CC=sdcc
SDASZ80=sdasz80
OBJCOPY=objcopy
DD=dd
MAME=mame


# Targets
all: cart

cart: ${OUTPUT_FILE_CART}
app: ${OUTPUT_FILE_APP}

${CRT_REL_FILE}: ${CRT_S_FILE}
	${MKDIR_P} ${OUT_DIR}
	${SDASZ80} -o ${CRT_REL_FILE} ${CRT_S_FILE}

# %.hex: %.c ${CRT_REL_FILE}
${OUTPUT_FILE_HEX}: ${INPUT_FILE} ${CRT_REL_FILE}
	${MKDIR_P} ${OUT_DIR}
	${CC} -mz80 --no-std-crt0 --vc \
	--code-loc 0x${LOC_CODE} --data-loc 0x${LOC_DATA} \
	--lib-path ${LIB_DIR} -I ${INC_DIR} -I ${ARCH_DIR} \
	-D VGLDK_SERIES=${VGLDK_SERIES} \
	-o ${OUTPUT_FILE_HEX} \
	${CRT_REL_FILE} ${INPUT_FILE}

${OUTPUT_FILE_BIN64K}: ${OUTPUT_FILE_HEX}
	# Build usable cartridge ROM bin file (64K memory dump)
	#${OBJCOPY} -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN64K}
	makebin -p -s 65536 ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN64K}

${OUTPUT_FILE_BIN}: ${OUTPUT_FILE_BIN64K}
	# Extract cartridge section (0x8000 and up)
	#${DD} bs=1024 count=${OUTPUT_CART_SIZE_KB} skip=32768 iflag=skip_bytes if=$< of=$@
	${DD} \
	skip=32768 \
	iflag=skip_bytes \
	bs=1024 count=${OUTPUT_CART_SIZE_KB} \
	if=${OUTPUT_FILE_BIN64K} \
	of=${OUTPUT_FILE_BIN}

${OUTPUT_FILE_CART}: ${OUTPUT_FILE_BIN}
	# Create empty EEPROM binary (full cart size)
	#dd if=/dev/zero of=${OUTPUT_FILE_CART} bs=$(OUTPUT_CART_SIZE * 1024) count=1
	#dd if=/dev/zero of=${OUTPUT_FILE_CART} bs=1 count=1 seek=$((OUTPUT_CART_SIZE * 1024 -1 ))
	dd if=/dev/zero ibs=1k count=${OUTPUT_CART_SIZE_KB} | tr "\000" "\377" >${OUTPUT_FILE_CART} 
	
	# Copy bin data into it
	dd if=${OUTPUT_FILE_BIN} of=${OUTPUT_FILE_CART} conv=notrunc

# App
${OUTPUT_FILE_APP}: ${OUTPUT_FILE_BIN64K}
	# Extract code section
	#LOC_CODE_DECIMAL=$(( 16# LOC_CODE ))
	cp ${OUTPUT_FILE_BIN64K} ${OUTPUT_FILE_APP}

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

.PHONY: clean

clean:
	rm -f ${OUT_DIR}/${NAME}.*

