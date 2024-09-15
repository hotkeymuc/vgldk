#!/bin/sh
echo Compiling ROM...
make cart
if [ $? -ne 0 ]; then
	echo Make error: Stopping run...
	exit 1
fi

#AGI_DATA_FILE=export_sq2_pic_1.bin
#AGI_DATA_FILE=export_sq2_pic_2.bin
#AGI_DATA_FILE=export_sq2_pic_3.bin
#AGI_DATA_FILE=export_sq2_pic_4.bin
AGI_DATA_FILE=export_sq2_pic_5.bin
#AGI_DATA_FILE=export_sq2_pic_6.bin
#AGI_DATA_FILE=export_sq2_pic_7.bin
#AGI_DATA_FILE=export_sq2_pic_8.bin
#AGI_DATA_FILE=export_sq2_pic_9.bin
#AGI_DATA_FILE=export_sq2_pic_10.bin
#AGI_DATA_FILE=export_sq2_pic_11.bin
#AGI_DATA_FILE=export_sq2_pic_12.bin
#AGI_DATA_FILE=export_sq2_pic_13.bin
echo Merging data ${AGI_DATA_FILE} into cartridge ROM...
#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_1.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_2.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_3.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_4.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
#../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/export_sq2_pic_5.bin out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
../../tools/binmerge.py --src-ofs=0 --src-num 0x4000 --dst-ofs=0x4000 data/${AGI_DATA_FILE} out/vagi.cart.32kb.bin out/vagi.cart.32kb_extended.bin
if [ $? -ne 0 ]; then
	echo Merge error: Stopping run...
	exit 1
fi


echo Emulating final image...
mame \
gl6000sl \
-rompath "/z/apps/_emu/_roms" \
-cart "out/vagi.cart.32kb_extended.bin" \
-window -nomax -nofilter -sleep \
-speed 2.00 -volume -24 \
-skip_gameinfo -nomouse
