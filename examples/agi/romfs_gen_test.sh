#!/bin/sh
# Test symlinks:
#GAMES_PATH=/z/data/_code/_c/V-Tech/vgldk.git/examples/agi/__apps__games___SCUMM
# Test relative paths:
GAMES_PATH=/z/apps/_games/_SCUMM/SQ1/..
# Test absolute path:
#GAMES_PATH=/z/apps/_games/_SCUMM

GAME_PATH=${GAMES_PATH}/SQ2

# Bash will evaluate the wildcard before calling!
#./romfs_gen.py ${GAME_PATH}/VOL.*

# Putting it in quotes will send the wildcard string as-is
#./romfs_gen.py --help
#./romfs_gen.py --file-offset=0x4000 --chip-offset=0x4000 --mem-offset=0x4000 --mem-bank-start=1 --mem-bank-size=0x4000 --align-size 0x0010 "${GAME_PATH}/VOL.*"
./romfs_gen.py\
 --file-offset=0x4000 --chip-offset=0x4000 --mem-offset=0x4000 --mem-bank-start=1 --mem-bank-size=0x4000\
 --align-size 0x0010\
 --file-h=romfs_data.h\
 --file-src=out/vagi.cart.32kb.bin\
 --file-dst=out/vagi.cart.32kb_extended.bin\
 "${GAME_PATH}/VOL.*"
