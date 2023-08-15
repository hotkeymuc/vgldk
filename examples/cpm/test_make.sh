#!/bin/sh

NAME=test

INPUT_FILE=${NAME}.c

OUT_DIR=out

OUTPUT_FILE_HEX=${OUT_DIR}/${NAME}.hex
OUTPUT_FILE_HEXBIN=${OUT_DIR}/${NAME}.hex.bin
OUTPUT_FILE_COM=${OUT_DIR}/${NAME}.com

sh ./test_compile.sh
if [ -f "${OUTPUT_FILE_COM}" ]; then
  sh ./test_sim.sh
fi
