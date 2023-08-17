#!/bin/bash
#make app
# "upload" also creates the app
make upload

# Go to receiver right away
#picocom --baud 9600 --imap 8bithex,spchex /dev/ttyUSB0

#picocom --baud 19200 --imap 8bithex,spchex,nrmhex /dev/ttyUSB0
picocom --baud 19200 --imap spchex,tabhex,crhex,lfhex,8bithex,nrmhex /dev/ttyUSB0