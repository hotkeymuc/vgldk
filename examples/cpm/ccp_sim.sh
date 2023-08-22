#!/bin/sh

# Run a CP/M .com file in YAZE Z80 emulator

# Program config
#NAME=ccp
NAME=ccp

# Must have a different name than "CCP"...
NAME_YAZE=CCPRUN
FILE_ARGS=""
#FILE_ARGS=" DIR"
#FILE_ARGS=" DUMP 0000"
FILE_COM=${NAME}.com
FILE_COM_YAZE=${NAME_YAZE}.COM
OUT_DIR=`realpath out`
OUTPUT_FILE_COM=${OUT_DIR}/${FILE_COM}
#OUTPUT_FILE_DISK=${OUT_DIR}/${NAME}.ydsk

# YAZE config
#YAZE_DIR=/usr/local/lib/yaze
YAZE_DIR=~/cpm
YAZE_BOOT_FILE=${YAZE_DIR}/yaze-cpm3.boot
YAZE_FILES_DIR=${YAZE_DIR}/files
#YAZE_DISKS_DIR=${YAZE_DIR}/disks
YAZE_DISKS_DIR=${YAZE_DIR}
YAZE_RC_FILE=${OUT_DIR}/${NAME}.yazerc
YAZE_AUTORUN_FILE=${OUT_DIR}/AUTORUN.SUB
# If it gets interactive: Kill the process
YAZE_KILL_TIMEOUT=1

# Prepare mounted files (O: in CP/M)
# Copy COM file to files directory
# NO! Keep it in OUT directory!
#cp ${OUTPUT_FILE_COM} ${YAZE_FILES_DIR}/${FILE_COM_YAZE}
cp ${OUTPUT_FILE_COM} ${OUT_DIR}/${FILE_COM_YAZE}

# Creating yazerc (mount commands to get CP/M up and running and mount a local directory)
echo mount a ${YAZE_DISKS_DIR}/BOOT_UTILS.ydsk>${YAZE_RC_FILE}
echo mount b ${YAZE_DISKS_DIR}/CPM3_SYS.ydsk>>${YAZE_RC_FILE}
echo mount o ${OUT_DIR}>>${YAZE_RC_FILE}
echo go>>${YAZE_RC_FILE}

# The program is executed by A:PROFILE.SUB on system startup
# I altered it to skip the info screen (type INFO.TXT), change to O: and run AUTORUN.SUB, then exit using "e"

# Prepare autorun file (which is run by CPM's A:PROFILE.SUB file)
#echo PIP A:PROFILE.SUB=O:PROFILE.SUB>${YAZE_AUTORUN_FILE}
echo ${FILE_COM_YAZE}${FILE_ARGS}>${YAZE_AUTORUN_FILE}


# Run YAZE
#yaze -b ${YAZE_BOOT_FILE} -v ${YAZE_COMMANDS}
#yaze -b ${OUTPUT_FILE_COM} -v
#yaze -b C:${NAME}.com -v
#yaze -v -s ${YAZE_RC_FILE} -b ${OUTPUT_FILE_COM}
#yaze -v -s ${YAZE_RC_FILE} ${YAZE_COMMANDS}
#yaze -s ${YAZE_RC_FILE} ${YAZE_COMMANDS}

## Run YAZE in background, kill the process after some seconds (from foreground)
yaze -s ${YAZE_RC_FILE}&

# Kill YAZE in case of a system crash
echo Waiting ${YAZE_KILL_TIMEOUT} seconds before killing YAZE...
sleep ${YAZE_KILL_TIMEOUT}
echo Killing YAZE...
killall yaze_bin

## Run YAZE in foreground, kill the process after some seconds (from background)
#(sleep ${YAZE_KILL_TIMEOUT}; echo Killing YAZE...; killall yaze_bin)&
#yaze -s ${YAZE_RC_FILE}


echo End.
