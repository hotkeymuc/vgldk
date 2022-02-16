#!/bin/sh
NAME=test
CART=out/${NAME}.bin
#ROMPATH="/z/apps/_emu/_roms"
ROMPATH=`realpath ./roms`

#EMUSYS=gl2000
#EMUSYS=gl4000
#EMUSYS=gl4004
#EMUSYS=gl5000
#EMUSYS=gl3000s
EMUSYS=gl6000sl

# Global MAME:
#MAMECALL=mame

# Custom MAME build:
MAMECALL=/z/data/_code/_c/mame.git/mame64


# Actually run
#stdbuf -i0 -o0 -e0
${MAMECALL} ${EMUSYS} -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00
#${MAMECALL} -rompath "${ROMPATH}" ${EMUSYS} -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00
#${MAMECALL} -rompath "${ROMPATH}" ${EMUSYS} -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00 -debug

#${MAMECALL} -rompath "${ROMPATH}" ${EMUSYS} -cart "${CART}" -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00
#${MAMECALL} -rompath "${ROMPATH}" ${EMUSYS} -cart "${CART}" -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00 -debug
# -debugscript


# Remove MAME history
rm history/*
rmdir history

# Remove MESS config directory that is created
#rm cfg/*.cfg
#rmdir cfg