#!/bin/sh
# Custom Makefile

# Command line tools setup
MKDIR_P='mkdir -p'
CC='sdcc'
SDASZ80='sdasz80'
OBJCOPY='objcopy'
DD='dd'
MAME='mame'
MINIPRO='minipro'
REL2APP='python3 $(TOOLS_DIR)/rel2app.py'
NOI2H='./noi2h.py'
SEND2MONITOR='python $(TOOLS_DIR)/send2monitor.py'

# Directories
#OUT_DIR=`realpath out`
OUT_DIR='out'
#INC_DIR=`realpath ../include`
INC_DIR='../../include'
ARCH_DIR='${INC_DIR}/arch/${ARCH_ID}'

#	TOOLS_DIR = ../../tools
#	LIB_DIR = $(INC_DIR)
#EMU_ROM_DIR='/z/apps/_emu/_roms'
EMU_ROM_DIR='roms'


# Explanation
#	
#	ADDR_CART = 0x8000
#		Where the cartridge ROM will reside at
#	
#	ADDR_RAM = 0xC000
#		Where the RAM is located
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
#		Series 6000     : 0xc000 - 0xffff
#	


# Target setup
#VGLDK_SERIES=1000
#VGLDK_SERIES=2000
VGLDK_SERIES=4000
#VGLDK_SERIES=6000

# Determine target system properties
case ${VGLDK_SERIES} in
	1000)
		ARCH_ID=pc1000
		ADDR_RAM='0x4000'
		ADDR_CART='0x8000'
		EMU_SYS=pc1000
		EMU_ROM=27-00780-002-002.u4
		;;
	
	2000)
		ARCH_ID=gl2000
		ADDR_RAM='0xc000'
		ADDR_CART='0x8000'
		EMU_SYS=gl2000
		EMU_ROM=lh532hez_9416_d.bin
		;;
	
	4000)
		ARCH_ID=gl4000
		ADDR_RAM='0xc000'
		ADDR_CART='0x8000'
		EMU_SYS=gl4000
		EMU_ROM=27-5480-00
		#EMU_SYS=gl4004
		#EMU_ROM=27-5762-00.u2
		;;
	
	6000)
		ARCH_ID=gl6000sl
		ADDR_RAM='0xc000'
		ADDR_CART='0x8000'
		EMU_SYS=gl6000sl
		EMU_ROM=27-5894-01
		#EMU_SYS=gl7007sl
		#EMU_ROM=27-6060-00
		;;
	
	*)
		echo "Unknown architecture ${VGLDK_SERIES}"
		exit 2
		;;
esac


# Convert addresses to from hex to dec
#ADDR_RAM_DECIMAL=`printf "%d" ${ADDR_RAM}`
#ADDR_CART_DECIMAL=`printf "%d" ${ADDR_CART}`

# Addresses
#CRT_SIZE=0
LOC_CODE=${ADDR_CART}
LOC_DATA=${ADDR_RAM}
#	LOC_CODE_MAIN_DECIMAL = $(shell echo $(LOC_CODE_DECIMAL) + $(CRT_SIZE) | bc )
#	LOC_CODE_MAIN = $(shell printf "0x%04x" $(LOC_CODE_MAIN_DECIMAL) )


# Create output dir
if ! [ -d ${OUT_DIR} ]; then
	echo Creating output directory \"${OUT_DIR}\"...
	${MKDIR_P} ${OUT_DIR}
fi


function compile {
	local C_FILE=$1
	local IHX_FILE=$2
	
	local LOC_CODE=$3
	local LOC_DATA=$4
	
	echo Compiling \"${C_FILE}\" to \"${IHX_FILE}\"...
		# Compile $< and $(CRT_REL_FILE) to $@
		# --out-fmt-ihx
		# --iram-size 4000
		# --lib-path ${LIB_DIR}
		${CC} \
		-mz80 \
		--no-std-crt0 \
		--code-loc ${LOC_CODE} \
		--data-loc ${LOC_DATA} \
		-D VGLDK_SERIES=${VGLDK_SERIES} \
		-I ${INC_DIR} \
		-I ${ARCH_DIR} \
		-o ${IHX_FILE} \
		${C_FILE}
		
		#	${CC} \
		#	-mz80 \
		#	--no-std-crt0 \
		#	-D VGLDK_SERIES=${VGLDK_SERIES} \
		#	--code-loc ${LOC_CODE_MAIN} \
		#	--data-loc ${LOC_DATA} \
		#	--lib-path ${LIB_DIR} \
		#	-I ${INC_DIR} \
		#	-I ${ARCH_DIR} \
		#	-o ${IHX_FILE} \
		#	${CRT_REL_FILE} \
		#	$<
	
}

function hex2bin {
	local HEX_FILE=$1
	local BIN_FILE=$2
	local BIN_SIZE=65536
	
	# Pad-fill empty file
	#dd if=/dev/zero ibs=1k count=$(CART_SIZE_KB) status=none | tr "\000" "\377" >
	
	# Build memory dump binary $@
	#$(OBJCOPY) -Iihex -Obinary $< $@
	#makebin -s ${BIN_SIZE} $< $@
	#      -p = only extract minimal amount
	#makebin -p -s ${BIN_SIZE} ${HEX_FILE} ${BIN_FILE}
	makebin -s ${BIN_SIZE} ${HEX_FILE} ${BIN_FILE}
}

function binextract {
	local BIN_FILE_IN=$1
	local BIN_FILE_OUT=$2
	local EXTRACT_START=`printf "%d" $3`
	local EXTRACT_SIZE=`printf "%d" $4`
	
	# Extract section from bin file
	#${DD} iflag=skip_bytes skip=${EXTRACT_START} bs=1024 count=${CART_SIZE_KB} if=$< of=$@ status=none
	${DD} iflag=skip_bytes skip=${EXTRACT_START} bs=${EXTRACT_SIZE} count=1 if=${BIN_FILE_IN} of=${BIN_FILE_OUT} status=none
	
	# Create empty EPROM file filled with FF (full cart size)
	#dd if=/dev/zero ibs=1k count=$(CART_SIZE_KB) status=none | tr "\000" "\377" >$@
	## Copy bin data into it
	#dd if=$< of=$@ conv=notrunc
}

function emu {
	local CART_FILE=$1
	
	# Start MAME using the specified cartridge file
	${MAME} \
	${EMU_SYS} \
	-rompath "${EMU_ROM_DIR}" \
	-cart "${CART_FILE}" \
	-window -nomax -nofilter -sleep \
	-speed 2.00 -volume -24 \
	-skip_gameinfo -nomouse
	# -debug	# Attach debug console and STEP
	
	# Remove MAME history directory
	rm -r history
	
	# Remove MESS config directory that is created
	#rm -r cfg
}

function build_rom {
	local BIN_FILE=$1
	local ROM_DIR=${EMU_ROM_DIR}
	local OUTPUT_FILE_SYSROM=${EMU_ROM}
	local OUTPUT_FILE_SYSROMZIP=${ROM_DIR}/${EMU_SYS}.zip
	local OUTPUT_FILE_INFO=_this_is_a_development_rom.txt
	
	if [ -f ${OUTPUT_FILE_SYSROM} ]; then
		rm ${OUTPUT_FILE_SYSROM}
	fi
	if [ -f ${OUTPUT_FILE_SYSROMZIP} ]; then
		rm ${OUTPUT_FILE_SYSROMZIP}
	fi
	
	echo Creating fake ROM image for MAME "${OUTPUT_FILE_SYSROMZIP}"...
	
	echo "This is a fake system ROM auto-created by VGLDK make." > ${OUTPUT_FILE_INFO}
	cp ${BIN_FILE} ${OUTPUT_FILE_SYSROM}
	zip ${OUTPUT_FILE_SYSROMZIP} ${OUTPUT_FILE_SYSROM} ${OUTPUT_FILE_INFO}
	
	rm ${OUTPUT_FILE_SYSROM}
	rm ${OUTPUT_FILE_INFO}
}

function burn {
	local BIN_FILE=$1
	
	#	# Cartridge output options
	#	#  8 =  8KB = 0x2000 = AT28C64B
	#	# 32 = 32KB = 0x8000 = AT28C256
	#	CART_SIZE_KB = 8
	#	#CART_SIZE_KB = 32
	#	
	#	# EEPROM burning options
	#	ifeq ($(CART_SIZE_KB),8)
	#		#EEPROM_PART = CAT28C64B
	#		EEPROM_PART = AT28C64B
	#	endif
	#	ifeq ($(CART_SIZE_KB),32)
	#		EEPROM_PART = AT28C256
	#	endif
	
	# Burn image "$(OUTPUT_FILE_CART)" to (E)EPROM of type "$(CART_PART)"
	# Use -s = no warning for file size mismatch
	${MINIPRO} -p "${EEPROM_PART}" -w ${BIN_FILE}
}

function upload {
	local BIN_FILE=$1
	local LOC_CODE=$2
	
	${SEND2MONITOR} --dest=${LOC_CODE} ${BIN_FILE}
}

# Actually do stuff

# Compile a segment
#SEGMENT_SIZE=0x4000
SEGMENT_SIZE_KB=16
SEGMENT_SIZE_BYTES=`echo ${SEGMENT_SIZE_KB} \* 1024 | bc`
ADDR_BANK1=0x4000

#compile "out/main.rel segment1.c" out/segment1.ihx ${ADDR_BANK1} 0xc200
compile common.c out/common.ihx 0x1000 0xc800
hex2bin out/common.ihx out/common.64k.bin
binextract out/common.64k.bin out/main.seg0b.bin 0x1000 0x3000
${NOI2H} out/common.noi common_addr.h

compile segment1.c out/segment1.ihx ${ADDR_BANK1} 0xc200
#compile "out/common.rel segment1.c" out/segment1.ihx ${ADDR_BANK1} 0xc200
hex2bin out/segment1.ihx out/segment1.64k.bin
binextract out/segment1.64k.bin out/main.seg1.bin ${ADDR_BANK1} ${SEGMENT_SIZE_BYTES}
${NOI2H} out/segment1.noi segment1_addr.h

compile segment2.c out/segment2.ihx ${ADDR_BANK1} 0xc400
#compile "out/common.rel segment2.c" out/segment2.ihx ${ADDR_BANK1} 0xc400
hex2bin out/segment2.ihx out/segment2.64k.bin
binextract out/segment2.64k.bin out/main.seg2.bin ${ADDR_BANK1} ${SEGMENT_SIZE_BYTES}
${NOI2H} out/segment2.noi segment2_addr.h

# Compile main
#CART_SIZE_KB=8
#CART_SIZE_BYTES=`echo ${CART_SIZE_KB} \* 1024 | bc`
#
#CART_FILE=out/main.cart.${CART_SIZE_KB}k.bin
#compile main.c out/main.ihx ${LOC_CODE} ${LOC_DATA}
#hex2bin out/main.ihx out/main.64k.bin
#binextract out/main.64k.bin ${CART_FILE} ${ADDR_CART} ${CART_SIZE_BYTES}
#emu ${CART_FILE}

compile main.c out/main.ihx 0x0000 ${LOC_DATA}
#compile "out/common.rel out/segment1.rel out/segment2.rel main.c" out/main.ihx 0x0000 ${LOC_DATA} 
hex2bin out/main.ihx out/main.64k.bin
#build_rom out/main.64k.bin
#binextract out/main.64k.bin out/main.seg0.bin 0x0000 ${SEGMENT_SIZE_BYTES}
binextract out/main.64k.bin out/main.seg0a.bin 0x0000 0x1000

# Compile a system ROM
# Combine segments
#cat out/main.seg0.bin out/main.seg1.bin out/main.seg2.bin > out/main.segs.bin
cat out/main.seg0a.bin out/main.seg0b.bin out/main.seg1.bin out/main.seg2.bin > out/main.segs.bin

# Bundle
build_rom out/main.segs.bin
# Emu
emu ${CART_FILE}

