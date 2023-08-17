#!/bin/sh

NAME=ccp

# CCP can/should live somewhere else than 0x100, because it has to load other programs to that address!
# Transient area starts at 0x100. BDOS is at top of RAM - so we need to be below that.
# LOC_CODE must leave enough space for CRT0 code AND not collide with BDOS/BIOS
#LOC_CODE=0x0180
#LOC_CODE=0x0108
LOC_CODE=0x6000

# Data could also be stored in INTERNAL RAM (0xc000 - 0xdfff)
LOC_DATA=0x6a00

OUT_DIR=out
INC_DIR=`realpath ../../include`
#LIB_DIR=`realpath ../../include`
#INC_DIR=${LIB_DIR}
LIB_DIR=`realpath .`

INPUT_FILE=${NAME}.c
CRT_S_FILE=program_crt0.s
CRT_REL_FILE=${OUT_DIR}/${NAME}_crt0.rel

OUTPUT_FILE_HEX=${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_HEXBIN=${OUT_DIR}/${NAME}.hex.bin
OUTPUT_FILE_COM=${OUT_DIR}/${NAME}.com

# Clean
if [ ! -d ${OUT_DIR} ]; then
  mkdir ${OUT_DIR}
fi
if [ -f ${OUTPUT_FILE_HEX} ]; then
  rm ${OUTPUT_FILE_HEX}
fi
if [ -f ${OUTPUT_FILE_HEXBIN} ]; then
  rm ${OUTPUT_FILE_HEXBIN}
fi
if [ -f ${OUTPUT_FILE_COM} ]; then
  rm ${OUTPUT_FILE_COM}
fi

# --code-loc 0x8000
# --stack-loc 0x20
# --data-loc 0x30
# --idata-loc 0x80
# --xram-loc 0xc000
# --model-small
# --no-std-crt0
# --nostdlib
# --verbose

# Using default crt0
#sdcc -mz80 --vc --code-loc 0x8000 -o ${OUTPUT_FILE_HEX} ${INPUT_FILE}

# Using a custom crt0
echo Compiling CRT "${CRT_S_FILE}"...
sdasz80 -o ${CRT_REL_FILE} ${CRT_S_FILE}
RETVAL=$?
[ $RETVAL -eq 0 ] && echo CRT0 compile: Success
[ $RETVAL -ne 0 ] && echo CRT0 compile: Failure

echo Compiling "${INPUT_FILE}"...
# "code-loc" specifies where the C code whould reside. Put it past the CRT code which begins at 0x8000
#sdcc -mz80 --no-std-crt0 --lib-path ${LIB_DIR} -I ${INC_DIR} --vc --code-loc 0x8030 --data-loc 0xc000 -o ${OUTPUT_FILE_HEX} ${CRT_REL_FILE} ${INPUT_FILE}
sdcc -mz80 --no-std-crt0 --lib-path ${LIB_DIR} -I ${INC_DIR} --vc --code-loc ${LOC_CODE} --data-loc ${LOC_DATA} -o ${OUTPUT_FILE_HEX} ${CRT_REL_FILE} ${INPUT_FILE}
#sdcc -mz80 --no-std-crt0 --lib-path ${LIB_DIR} --vc --code-loc ${LOC_CODE} --data-loc ${LOC_DATA} -o ${OUTPUT_FILE_HEX} ${CRT_REL_FILE} ${INPUT_FILE}

RETVAL=$?
[ $RETVAL -eq 0 ] && echo SDCC compile: Success
[ $RETVAL -ne 0 ] && echo SDCC compile: Failure

# Convert .hex to .bin
if [ -f ${OUTPUT_FILE_HEX} ]; then
  echo Building binary "${OUTPUT_FILE_HEXBIN}"...
  objcopy -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_HEXBIN}
  
  echo Extracting "${OUTPUT_FILE_COM}"...
  #dd bs=1024 skip=256 iflag=skip_bytes if=${OUTPUT_FILE_HEXBIN} of=${OUTPUT_FILE_COM}
  cp ${OUTPUT_FILE_HEXBIN} ${OUTPUT_FILE_COM}
  
fi
