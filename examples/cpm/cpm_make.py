#!/usr/bin/python3
"""
CP/M Make in Python
===================

I have a rudimentary CP/M build process up and running using Make files.

Since producing a valid CP/M image with all its bootstrapping and RAM preparation,
it might be a better idea to do it using a Python script.

I already have created a VTech builder for Z88DK for HAUL3.


Starting up:
	Method A: Use cartridge as internal ROM while preparing
		* Diable internal ROM
		* Mount cartridge in internal ROM space
		* Mount RAM to cartridge space
		* Boot cartridge (internal ROM space)
		* Prepare RAM (in cartridge space)
		(* Power off)
		* Mount RAM to internal ROM space
		* Mount cartridge in cartridge space
	Method B: Boot prepare as cartridge, overlay RAM to internal ROM
		(* Enable internal ROM, allowing to boot stock firmware to cartridge)
		* Remove RAM cartridge or disable its ~OE while its mounted to internal ROM space
		* Boot stock system to cartridge
		* While running: Disable internal ROM (bind its ~CS HIGH)
		* While running: Enable RAM on internal ROM space (enable ~OE and bind its ~CS to internal ROM ~CS)
	* OUT 2, 0x7F	-> Mount RAM page 1 to 0x0000-0x3FFF, RAM page 2 to 0x4000-0x7FFF
	* Boot RAM in internal ROM space

2023-08-10 Bernhard "HotKey" Slawik
"""

import time
import os
import shutil
import subprocess

import sys
sys.path.append('../../tools')
from rel2app import *
import calcsize

sys.path.append('../monitor')
import monitor


def put(t):
	print(t)

def hexdump(data, addr=0):
	put(monitor.hexdump(data, addr=addr))

def start_emu():
	"""
	put('Launching MESS emulator...')
	
	#"%MESSPATH%\mess.exe" -rompath "%ROMPATH%" %EMUSYS% -cart "%PROGNAME%.bin" -window -nomax -nofilter -sleep -volume -10 -skip_gameinfo -speed 2.00
	
	cmd = '"%s"' % os.path.join(mess_path, 'mess.exe')
	cmd += ' -rompath "%s"' % (mess_rom_path)
	cmd += ' %s' % (mess_sys)
	cmd += ' -cart "%s"' % (os.path.abspath(os.path.join(self.output_path, bin_filename)))
	cmd += ' -window'
	cmd += ' -nomax'
	cmd += ' -nofilter'
	cmd += ' -sleep'
	cmd += ' -volume -20'
	cmd += ' -skip_gameinfo'
	cmd += ' -speed 2.00'
	#cmd += ' -debug'	# Attach debug console and STEP
	
	self.chdir(self.staging_path)	# Change to staging dir (MESS creates some messy files wherever it is called)
	r = self.command(cmd)
	self.chdir(start_path)	# Change back
	
	#@TODO: Delete the config file that MESS is creating (CFG)
	"""
	pass


def compile():
	src_path = '.'
	out_path = 'out'
	
	### Process .s file, generate .rel file
	crt_s_files = [ '%s/cpm_crt0.s' % src_path ]
	crt_rel_file = '%s/cpm_crt0.rel' % out_path
	
	source_files = [
		crt_rel_file,	# Start with .rel file for SDCC
		'%s/cpm.c' % src_path
	]
	output_hex_file = '%s/cpm.hex' % out_path
	output_bin_file = '%s/cpm.bin' % out_path
	
	lib_path = None	#'../../include'
	include_path = '../../include'
	loc_code = 0x8000 - 0x0F00	#0x8000 for cart code, < 0x8000 for CP/M
	loc_data = 0xc000	# static variable data and gsinit-code will be put to this address in binary file
	loc_idata = None	#0xc800	# ?
	defines = {
		'VGLDK_SERIES': 4000
	}
	
	"""
	### Compile source file(s) using Z88DK, generate .bin file
	
	my_env = os.environ.copy()
	my_env['PATH'] = os.environ['PATH'] + ';' + os.path.join(z88dk_path, 'bin')
	my_env['OZFILES'] = z88dk_lib_path	# Normally these files reside inside the z88dk, but we can hijack them and use our minimal local version
	my_env['ZCCCFG'] = os.path.abspath(z88dk_lib_path + '/config')
	
	#cmd = os.path.join(z88dk_path, 'bin', 'zcc')
	#cmd = 'zcc +vgl -vn -clib=sdcc_iy -SO3 --max-allocs-per-node200000 %PROGNAME%.c -o %PROGNAME% -create-app'
	#cmd = 'zcc +vgl -v -clib=sdcc_iy -SO3 --max-allocs-per-node200000 %PROGNAME%.c -o %PROGNAME% -create-app'
	cmd = 'zcc'
	cmd += ' +' + z88dk_target
	cmd += ' -vn'
	cmd += ' -clib=new'	# "new" or "sdcc_iy" or ...
	cmd += ' -subtype=' + z88dk_subtype
	#cmd += ' -I' + os.path.abspath(libs_path)
	#cmd += ' -I' + os.path.abspath(os.path.join(z88dk_path, 'include'))
	#cmd += ' -lm'
	#cmd += ' -l' + 'gen_math'
	#cmd += ' -l' + 'zx80_clib'
	#cmd += ' -lndos'
	#cmd += ' -l' + 'z88_clib'
	#cmd += ' -l' + 'z88_math'
	cmd += ' %s' % ' '.join(source_files)	# may only be a single .c file
	cmd += ' -o ' + output_bin_file
	cmd += ' -create-app'
	"""
	
	
	cmd = 'sdasz80 -o %s %s' % (crt_rel_file, ' '.join(crt_s_files))
	env = None
	put('%s...' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Compile source file(s) using SDCC, generate .hex file
	cmd = 'sdcc -mz80'
	
	# --model-small
	# --no-std-crt0
	# --nostdlib
	cmd += ' --no-std-crt0'	# Provide our own crt0 .rel
	
	if lib_path is not None: cmd += ' --lib-path %s' % lib_path
	if include_path is not None: cmd += ' -I %s' % include_path
	
	cmd += ' --code-loc 0x%04X' % loc_code
	if loc_data is not None: cmd += ' --data-loc 0x%04X' % loc_data
	if loc_idata is not None: cmd += ' --idata-loc 0x%04X' % loc_idata
	
	# --xram-loc 0xc000
	# --verbose
	#cmd += ' --vc'	# vc = messages are compatible with Micro$oft visual studio
	
	for k,v in defines.items():
		cmd += ' -D %s=%s' % (k, v)
	
	cmd += ' -o %s' % output_hex_file
	cmd += ' %s' % ' '.join(source_files)	# .rel, .c, .c ...
	put('%s...' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	# Show intermediate .rel file
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Convert .hex, generate .bin file
	cmd = 'objcopy -Iihex -Obinary %s %s' % (output_hex_file, output_bin_file)
	put('%s...' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#@FIXME: The .bin file also contains some data at the RAM area... (e.g. file offset 0xc000)
	
	### Show stats
	process_rel_file(crt_rel_file, output_bin_file)
	
	calcsize.analyze(output_bin_file)
	
	"""
	cpm_compile.sh:
		#!/bin/sh
		NAME=cpm
		# Maximum size of prepare.bin segment
		# Keep in sync with LOC_CODE! (LOC_CODE > 0x8000+PREPARE_BIN_SIZE)
		#PREPARE_BIN_SIZE=1536
		#PREPARE_BIN_SIZE=1280
		#PREPARE_BIN_SIZE=1024
		#PREPARE_BIN_SIZE=896
		#PREPARE_BIN_SIZE=512
		PREPARE_BIN_SIZE=384
		
		# Low Storage size in bytes (!)
		OUTPUT_RAM_SIZE=128
		#OUTPUT_RAM_SIZE=96
		
		# LOC_CODE must leave space for the prepare code, but should be as close as possible
		# Keep in mind, that in the final binary, prepare code and cpm code share one ROM
		# Keep in sync with PREPARE_BIN_SIZE! (LOC_CODE > 0x8000+PREPARE_BIN_SIZE)
		#LOC_CODE=0x8200
		LOC_CODE=0x8180
		
		# LOC_DATA is where the RAM is at (0xc000 is Genius Leader on-board RAM)
		LOC_DATA=0xc000
		
		# Output cartridge bin file in kilobytes (!)
		#  8 =  8KB = 0x2000 = AT28C64B
		# 16 = 16KB = 0x4000 = AT28C128 (?)
		# 32 = 32KB = 0x8000 = AT28C256
		#OUTPUT_CART_SIZE=8
		#OUTPUT_CART_SIZE=16
		OUTPUT_CART_SIZE=32
		
		OUTPUT_SYS=gl4000
		#OUTPUT_SYS=gl4000htk
		
		OUT_DIR=out
		LIB_DIR=`realpath ../includes`
		INC_DIR=${LIB_DIR}
		ROM_DIR=roms
		
		# Convert .hex to .bin
			echo Building full binary "${OUTPUT_FILE_HEXBIN}"...
			objcopy -Iihex -Obinary ${OUTPUT_FILE_HEX} ${OUTPUT_FILE_HEXBIN}
			
			echo Extracting ${OUTPUT_CART_SIZE}kB cartridge "${OUTPUT_FILE_CART}"...
			dd bs=1024 count=${OUTPUT_CART_SIZE} skip=32768 iflag=skip_bytes if=${OUTPUT_FILE_HEXBIN} of=${OUTPUT_FILE_CART}
			
			echo Extracting bootstrap RAM contents "${OUTPUT_FILE_RAM}"...
			dd bs=${OUTPUT_RAM_SIZE} count=1 skip=0 iflag=skip_bytes if=${OUTPUT_FILE_HEXBIN} of=${OUTPUT_FILE_RAM}
			
			echo Creating header file for RAM bootstrapping "${OUTPUT_FILE_RAM_H}"...
			#hexdump -v -e '16/1 "_x%02X" "\n"' ${OUTPUT_FILE_RAM} | sed 's/_/\\/g; s/\\x  //g; s/.*/    "&"/'
			#xxd --include ${OUTPUT_FILE_RAM} ${OUTPUT_FILE_RAM_H}
			#cat ${OUTPUT_FILE_RAM} | xxd --include - ${OUTPUT_FILE_RAM_H}
			### Use xxd to convert to .h file
			#echo "// This file has been auto-generated by cpm_compile.sh" > ${OUTPUT_FILE_ZERO_H}
			#echo "// It contains the first ${OUTPUT_RAM_SIZE} bytes of the file ${OUTPUT_FILE_RAM}." >> ${OUTPUT_FILE_ZERO_H}
			#echo "// It is used by prepare.c to bootstrap the SRAM cartridge so it can be booted from." >> ${OUTPUT_FILE_ZERO_H}
			#echo "" >> ${OUTPUT_FILE_ZERO_H}
			#echo -n "const " >> ${OUTPUT_FILE_ZERO_H}
			#cp ${OUTPUT_FILE_RAM} ${OUTPUT_FILE_ZERO_NAME}
			#xxd --include ${OUTPUT_FILE_RAM_NAME} - >>${OUTPUT_FILE_ZERO_H}
			#rm ${OUTPUT_FILE_ZERO_NAME}
			### Use python script to convert RAM image to .h file
			python data2h.py ${OUTPUT_FILE_RAM} ${OUTPUT_FILE_RAM_H} ${OUTPUT_FILE_RAM_NAME}
			
			RETVAL=$?
			[ $RETVAL -eq 0 ] && echo BIN extraction: Success
			[ $RETVAL -ne 0 ] && echo BIN extraction: Failure
			
			
			echo Compiling prepare program...
			sh ./prepare_compile.sh
			
			echo Combining ${PREPARE_FILE_BIN} and ${OUTPUT_FILE_CART} to final PROM image ${OUTPUT_FILE_PROM}...
			dd bs=${PREPARE_BIN_SIZE} count=1 skip=0 iflag=skip_bytes if=${PREPARE_FILE_BIN} of=${PREPARE_FILE_BIN_TMP}
			# Pad until max size
			dd if=/dev/zero of=${PREPARE_FILE_BIN_TMP} bs=1 count=1 seek=$((PREPARE_BIN_SIZE -1 ))
			
			# Pad to fill EEPROM
			dd if=/dev/zero of=${OUTPUT_FILE_CART} bs=1 count=1 seek=$((OUTPUT_CART_SIZE * 1024 -1 ))
			
			# Extract without start
			dd bs=1024 count=${OUTPUT_CART_SIZE} skip=${PREPARE_BIN_SIZE} iflag=skip_bytes if=${OUTPUT_FILE_CART} of=${OUTPUT_FILE_PROM_TMP}
			
			# Combine
			cat ${PREPARE_FILE_BIN_TMP} ${OUTPUT_FILE_PROM_TMP} > ${OUTPUT_FILE_PROM}
			
			# Clean up
			rm ${OUTPUT_FILE_PROM_TMP}
			rm ${PREPARE_FILE_BIN_TMP}
			
			if [ -f ${OUTPUT_FILE_SYSROM} ]; then
				rm ${OUTPUT_FILE_SYSROM}
			fi
			if [ -f ${OUTPUT_FILE_SYSROMZIP} ]; then
				rm ${OUTPUT_FILE_SYSROMZIP}
			fi
			
			echo Creating fake ROM image for MAME "${OUTPUT_FILE_SYSROMZIP}"...
			echo "This is a fake system image to emulate a bootstrapped SRAM in system ROM area" > ${OUTPUT_FILE_INFO}
			cp ${OUTPUT_FILE_RAM} ${OUTPUT_FILE_SYSROM}
			zip ${OUTPUT_FILE_SYSROMZIP} ${OUTPUT_FILE_SYSROM} ${OUTPUT_FILE_INFO}
			rm ${OUTPUT_FILE_SYSROM}
			rm ${OUTPUT_FILE_INFO}
		fi
	"""
	
	# Return pure binary
	with open(output_bin_file, 'rb') as h:
		data = h.read()
	return data


def cpm_upload(data):
	comp = monitor.Monitor()	#(port=port, baud=baud)
	comp.open()
	
	monitor.SHOW_TRAFFIC = False	# disable verbose traffic
	comp.wait_for_monitor()
	
	comp.ver()
	
	# GL4000
	
	# Set banks
	put('Setting bank switching ports...')
	comp.port_out(0, 0x00)	# Set 0x0000-0x3FFF to ROM 0x0000
	comp.port_out(1, 0x01)	# Set 0x4000-0x7FFF to ROM 0x4000
	#comp.port_out(2, 0x00)	# ?
	comp.port_out(3, 0x80)	# Set 0x8000-0xBfff to CART 0x0000
	#comp.port_out(4, 0x00)	# ?
	#comp.port_out(5, 0x00)	# ?
	
	#l = 64
	#comp.dump(0x0000, l)
	#comp.dump(0x4000, l)
	#comp.dump(0x8000, l)
	#comp.dump(0xC000, l)
	#comp.dump(0xE000, l)
	
	put('Testing for RAM at 0x0000...')
	
	ram_found = False
	while not ram_found:
		addr = 0x0000
		# Get old value
		old = comp.peek(addr)[0]
		# Write new value
		new = old ^ 0xff
		comp.poke(addr, [new])
		# Read it back
		check = comp.peek(addr)[0]
		
		if check == new:
			put('RAM found!')
			ram_found = True
			break
		elif check == old:
			put('ROM found! (was 0x%02X, wrote 0x%02X, read back 0x%02X)' % (old, new, check))
		else:
			put('Floating state found! (was 0x%02X, wrote 0x%02X, read back 0x%02X)' % (old, new, check))
		time.sleep(1)
	
	time.sleep(0.5)
	
	put('Preparing RAM...')
	#comp.upload(filename='out/cpm.bin', src_addr=0, dest_addr=0x0000, max_size=0x200, chunk_size=32, verify=True)
	comp.upload(data=data, src_addr=0x0000, dest_addr=0x0000, chunk_size=16, skip_zeros=True, verify=True)
	
	
	#sys.exit(0)
	
	put('Disabling serial and calling 0x%04X...' % dest)
	dest = 0x0000
	comp.write('sio;call %04x\n' % dest)
	


if __name__ == '__main__':
	
	data = compile()
	hexdump(data[:0x0120], 0x0000)
	hexdump(data[0x7000:0x8000], 0x7000)
	
	#bin_filename = 'out/cpm.bin'
	#cmd = 'z80dasm --address --labels --source --origin=0000h %s' % bin_filename
	#os.system(cmd)
	
	#cpm_upload(data)
	
	#cpm_run()
	