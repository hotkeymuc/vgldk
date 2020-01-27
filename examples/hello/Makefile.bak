
#NAME=hello
NAME=test

VGLDK_SERIES=2000
VGLDK_DIRECTIO=1

# Memory layout
# Where the code will reside in address space once loaded
# This must leave some space for the CRT0 code
LOC_CODE=0x8010
#LOC_CODE=0x8020

# Where variables are stored
LOC_DATA=0xc000

# Cartridge output options
#  8 =  8KB = 0x2000 = AT28C64B:
# 16 = 16KB = 0x4000 = AT28C128 (?)
# 32 = 32KB = 0x8000 = AT28C256
#OUTPUT_CART_SIZE_KB=8
#OUTPUT_CART_SIZE_KB=16
OUTPUT_CART_SIZE_KB=32

# Directories
#LIB_DIR=`realpath ../include`
LIB_DIR=../../include
INC_DIR=${LIB_DIR}
OUT_DIR=out
EMU_ROM_DIR="/z/apps/_emu/_roms"

# Filenames
INPUT_FILE=${NAME}.c
CRT_S_FILE=${LIB_DIR}/arch/cart_crt0.s
CRT_REL_FILE=${OUT_DIR}/${NAME}.crt0.rel
OUTPUT_FILE_HEX=${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_BIN=${OUT_DIR}/${NAME}.bin
OUTPUT_FILE_CART=${OUT_DIR}/${NAME}.cart.bin

# Emulation
#EMU_SYS=pc1000
#EMU_SYS=gl2000
#EMU_SYS=gl3000s
#EMU_SYS=gl4000
#EMU_SYS=gl4004
#EMU_SYS=gl5000
#EMU_SYS=gl3000s
#EMU_SYS=${ARCH}
EMU_SYS=gl${VGLDK_SERIES}
ifeq (${VGLDK_SERIES},1000)
	EMU_SYS=pc${VGLDK_SERIES}
endif
ifeq (${VGLDK_SERIES},3000)
	EMU_SYS=gl${VGLDK_SERIES}s
endif


# Commands
MKDIR_P=mkdir -p
CC=sdcc
SDASZ80=sdasz80
OBJCOPY=objcopy
DD=dd

all: cart

cart: ${OUTPUT_FILE_CART}

${CRT_REL_FILE}: ${CRT_S_FILE}
	${MKDIR_P} ${OUT_DIR}
	${SDASZ80} -o ${CRT_REL_FILE} ${CRT_S_FILE}

# %.hex: %.c ${CRT_REL_FILE}
${OUTPUT_FILE_HEX}: ${INPUT_FILE} ${CRT_REL_FILE}
	${MKDIR_P} ${OUT_DIR}
	${CC} -mz80 --no-std-crt0 --vc \
	--code-loc ${LOC_CODE} --data-loc ${LOC_DATA} \
	--lib-path ${LIB_DIR} -I ${INC_DIR} \
	-D VGLDK_SERIES=${VGLDK_SERIES} \
	-D VGLDK_DIRECTIO=${VGLDK_DIRECTIO} \
	-o ${OUTPUT_FILE_HEX} \
	${CRT_REL_FILE} ${INPUT_FILE}

${OUTPUT_FILE_BIN}: ${OUTPUT_FILE_HEX}
	#${OBJCOPY} -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN}
	makebin -p -s 65536 ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN}

${OUTPUT_FILE_CART}: ${OUTPUT_FILE_BIN}
	#${DD} bs=1024 count=${OUTPUT_CART_SIZE_KB} skip=32768 iflag=skip_bytes if=$< of=$@
	${DD} \
	bs=1024 count=${OUTPUT_CART_SIZE_KB} \
	skip=32768 \
	iflag=skip_bytes \
	if=${OUTPUT_FILE_BIN} \
	of=${OUTPUT_FILE_CART}


#hello: hello.c
#	${MKDIR_P} ${OUT_DIR}
#	${SDASZ80} -o ${CRT_REL_FILE} ${CRT_S_FILE}
#	${CC} -mz80 --no-std-crt0 --lib-path ${LIB_DIR} -I ${INC_DIR} --vc --code-loc ${LOC_CODE} --data-loc ${LOC_DATA} -o ${OUTPUT_FILE_HEX} ${CRT_REL_FILE} ${INPUT_FILE}
#	${OBJCOPY} -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_BIN}
#	${DD} bs=1024 count=${OUTPUT_CART_SIZE_KB} skip=32768 iflag=skip_bytes if=${OUTPUT_FILE_BIN} of=${OUTPUT_FILE_CART}

emu: ${OUTPUT_FILE_CART}
	mame \
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

.PHONY: clean

clean:
	rm -f ${OUT_DIR}/${NAME}.*

