#!/bin/sh

NAME=ccp

INPUT_FILE=${NAME}.c

OUT_DIR=out

OUTPUT_FILE_HEX=${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_HEXBIN=${OUT_DIR}/${NAME}.hex.bin
OUTPUT_FILE_COM=${OUT_DIR}/${NAME}.com

sh ./ccp_compile.sh
if [ -f "${OUTPUT_FILE_COM}" ]; then
  sh ./ccp_sim.sh
fi
