#!/bin/sh
# This script compiles the VAGI cartridge, then it copies resources to the final binary and starts the emulator.

GAMES_PATH=/z/apps/_games/_SCUMM
#GAME_ID=KQ1
#GAME_ID=KQ2
#GAME_ID=KQ3
#GAME_ID=LSL1
#GAME_ID=CAULDRON
#GAME_ID=SQ1
GAME_ID=SQ2
# Not working:
##GAME_ID=Enclosure
##GAME_ID=uriquest
##GAME_ID=SpaceQuest0__rep_104
##GAME_ID=SpaceQuest_NewAdventuresOfRogerWilco
##GAME_ID=SpaceQuestX_TheLostChapter
GAME_PATH=${GAMES_PATH}/${GAME_ID}

CART_SIZE_KB=1024
CART_ROM_FILE=out/vagi.cart.${CART_SIZE_KB}kb.bin
CART_ROM_FINAL=out/vagi.cart.${CART_SIZE_KB}kb_${GAME_ID}.bin


#ROMFS_SPECS="data/export_*_pic_*.bin"
#ROMFS_SPECS="${GAME_PATH}/*DIR ${GAME_PATH}/VOL.*"
#ROMFS_SPECS="${GAME_PATH}/LOGDIR ${GAME_PATH}/PICDIR ${GAME_PATH}/SNDDIR ${GAME_PATH}/VIEWDIR ${GAME_PATH}/VOL.0 ${GAME_PATH}/VOL.1 ${GAME_PATH}/VOL.2 ${GAME_PATH}/VOL.3"
ROMFS_SPECS="${GAME_PATH}/LOGDIR ${GAME_PATH}/PICDIR ${GAME_PATH}/SNDDIR ${GAME_PATH}/VIEWDIR ${GAME_PATH}/VOL.*"

ROMFS_ARGS=" --file-offset=0x4000 --chip-offset=0x4000\
 --mem-offset=0x4000 --mem-bank-size=0x4000 --mem-bank-start=1\
 --align-size 0x0010\
 --fix-crossing\
 --file-src=${CART_ROM_FILE}\
 --file-dst=${CART_ROM_FINAL}"

echo Creating ROM FS header file...
./romfs_gen.py\
 ${ROMFS_ARGS}\
 --file-h=romfs_data.h\
 --verbose\
 ${ROMFS_SPECS}


echo Compiling cartridge ROM...
make cart
if [ $? -ne 0 ]; then
	echo Make error: Stopping run...
	exit 1
fi


# 	# Merge a data file into ROM
# 	
#	#AGI_DATA_FILE=export_sq2_pic_1.bin
#	#AGI_DATA_FILE=export_sq2_pic_2.bin
#	#AGI_DATA_FILE=export_sq2_pic_3.bin
#	#AGI_DATA_FILE=export_sq2_pic_4.bin
#	AGI_DATA_FILE=export_sq2_pic_5.bin
#	#AGI_DATA_FILE=export_sq2_pic_6.bin
#	#AGI_DATA_FILE=export_sq2_pic_7.bin
#	#AGI_DATA_FILE=export_sq2_pic_8.bin
#	#AGI_DATA_FILE=export_sq2_pic_9.bin
#	#AGI_DATA_FILE=export_sq2_pic_10.bin
#	#AGI_DATA_FILE=export_sq2_pic_11.bin
#	#AGI_DATA_FILE=export_sq2_pic_12.bin
#	#AGI_DATA_FILE=export_sq2_pic_13.bin
#	echo Merging data ${AGI_DATA_FILE} into cartridge ROM...
#	#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_1.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#	#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_2.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#	#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_3.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#	#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_4.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#	#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_5.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#	../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/${AGI_DATA_FILE} out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin

echo Merging game data into cartridge ROM...
./romfs_gen.py\
 ${ROMFS_ARGS}\
 ${ROMFS_SPECS}

if [ $? -ne 0 ]; then
	echo Merge error: Stopping run...
	exit 1
fi


echo Emulating final image...
mame \
gl6000sl \
-rompath "/z/apps/_emu/_roms" \
-cart "${CART_ROM_FINAL}" \
-window -nomax -nofilter -sleep \
-speed 2.00 -volume -24 \
-skip_gameinfo -nomouse
