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
	
	### .s file(s) get(s) compiled to a .rel file, which gets pre-pended to .c source
	crt_s_files = [
		'%s/cpm_crt0.s' % src_path
	]
	crt_rel_file = '%s/cpm_crt0.rel' % out_path
	
	source_files = [
		crt_rel_file,	# Start with .rel file for SDCC
		'%s/cpm.c' % src_path
	]
	output_hex_file = '%s/cpm.hex' % out_path
	output_bin_file = '%s/cpm.bin' % out_path
	
	lib_path = None	#'../../include'
	include_path = '../../include'
	
	code_size_estimate = 0x1000	# Approx size of CPM data (to determine optimal offset). Must LARGER OR EQUAL to actual binary size
	loc_code = 0x8000 - code_size_estimate	# Put CPM as far up as possible
	# Monitor uses 0xd000 for data
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
	
	# Compile .s file(s) to .rel file
	cmd = 'sdasz80 -o %s %s' % (crt_rel_file, ' '.join(crt_s_files))
	
	put('%s...' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Compile source file(s) using SDCC, generate .hex file
	cmd = 'sdcc -mz80'
	
	# --code-loc 0x8000
	# --stack-loc 0x20
	# --data-loc 0x30
	# --idata-loc 0x80
	# --xram-loc 0xc000
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
	# Add "#define"s
	for k,v in defines.items():
		cmd += ' -D %s=%s' % (k, v)
	
	cmd += ' -o %s' % output_hex_file
	cmd += ' %s' % ' '.join(source_files)	# .rel, .c, .c ...
	
	put('%s...' % cmd)
	r = os.system(cmd)
	put('Result: %s' % r)
	if r != 0: sys.exit(1)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	# Show intermediate .rel file
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Convert .hex, generate .bin file
	cmd = 'objcopy -Iihex -Obinary %s %s' % (output_hex_file, output_bin_file)
	put('%s...' % cmd)
	r = os.system(cmd)
	put('Result: %s' % r)
	if r != 0: sys.exit(1)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#@FIXME: The .bin file also contains some data at the RAM area... (e.g. file offset 0xc000)
	
	### Show stats
	#process_rel_file(crt_rel_file, output_bin_file)
	
	calcsize.analyze(output_bin_file)
	
	# Return pure binary data
	with open(output_bin_file, 'rb') as h:
		data = h.read()
	return data


def cpm_upload(data):
	#comp = monitor.Monitor()
	comp = monitor.Monitor(baud=19200)	# Requires monitor to be compiled using SoftUART and 19200 baud
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
	dest = 0x0000
	size = 0x8000	# Do not upload to ROM cartridge at 0x8000-0xbfff - it wont work ;-)
	
	# Clear area with zeros
	comp.memset(dest=dest, pat=0x00, size=size)
	
	#comp.upload(filename='out/cpm.bin', src_addr=0, dest_addr=0x0000, max_size=0x200, chunk_size=32, verify=True)
	#comp.upload(data=data, src_addr=0x0000, dest_addr=0x0000, chunk_size=16, skip_zeros=True, verify=True)
	comp.upload(data=data[:size], src_addr=0x0000, dest_addr=0x0000, chunk_size=56, skip_zeros=True, verify=not True)
	
	
	#sys.exit(0)
	
	dest = 0x0000
	put('Disabling serial and calling 0x%04X...' % dest)
	comp.write('sio;call %04x\n' % dest)
	


if __name__ == '__main__':
	
	# Compile CP/M (CRT0, BIOS, BDOS, BINT)
	put('Compiling CP/M (CRT0, BINT, BIOS, BDOS)...')
	cpm_data = compile()
	bin_filename = 'out/cpm.bin'
	#hexdump(cpm_data[:0x0120], 0x0000)
	#hexdump(cpm_data[0x7000:0x8000], 0x7000)
	
	
	## Compile CCP (as a simple program)
	put('Compiling CCP...')
	transient = 0x100	# Start of transient area
	ccp_filename = 'out/ccp.com'
	ccp_loc_code = 0x6000	#LOC_CODE=0x6000 - must match CCP makefile!
	
	#@TODO: Reuse compile()
	r = os.system('/bin/sh ccp_compile.sh')
	put('Result: %s' % r)
	if r != 0: sys.exit(1)
	
	# Merge CCP into image
	with open(ccp_filename, 'rb') as h:
		ccp_data = h.read()
	
	# Extract code only
	ccp_data = ccp_data[ccp_loc_code - transient:]	# Note! Binary does not start at memory 0x0000, but 0x100 (CP/M program!)
	#hexdump(ccp_data, ccp_loc_code)
	
	## Merge into CP/M image
	put('Merging CCP into CP/M image...')
	#cpm_data[ccp_loc_code:ccp_loc_code+len(ccp_data)] = ccp_data[:]
	cpm_data = cpm_data[:ccp_loc_code] + ccp_data + cpm_data[ccp_loc_code+len(ccp_data):]
	
	# Write merged output
	with open(bin_filename, 'wb') as h:
		h.write(cpm_data)
	
	## Decompile using z80dasm
	#cmd = 'z80dasm --address --labels --source --origin=0000h %s' % bin_filename
	#os.system(cmd)
	
	### Debugging
	## Debug BDOS area
	bdos_addr = cpm_data[6] + cpm_data[7] * 0x100
	put('BDOS addr: 0x%04X' % bdos_addr)
	hexdump(cpm_data[bdos_addr:bdos_addr+128], bdos_addr)
	
	dasm_tmp = '/tmp/z80dasm.asm'
	cmd = 'z80dasm --address --labels --source --origin=%04Xh %s >%s' % (0x0000, bin_filename, dasm_tmp)
	os.system(cmd)
	started = False
	start_label = 'l%04xh:' % bdos_addr	# Label to start output from
	for l in open(dasm_tmp, 'r'):
		if l.startswith(start_label):
			started = True
		if started: put(l[:-1])
	sys.exit(1)
	
	
	## Upload to hardware running serial MONITOR
	#put('Uploading CP/M image...')
	#cpm_upload(cpm_data)
	
	
	## Create MAME SYSROM for emulation
	put('Creating MAME SYSROM...')
	OUTPUT_SYS = 'gl4000'
	ROM_DIR = 'roms'
	OUTPUT_FILE_SYSROM = '27-5480-00'
	OUTPUT_FILE_SYSROMZIP = '%s/%s.zip' % (ROM_DIR, OUTPUT_SYS)
	OUTPUT_FILE_INFO = '_this_is_cpm.txt'
	
	# Write dynamic info file
	with open(OUTPUT_FILE_INFO, 'wb') as h:
		h.write(b'This is a fake system image to emulate a bootstrapped SRAM in system ROM area')
	
	# Write correctly named MAME sysrom file
	#cmd = 'cp %s %s' % (OUTPUT_FILE_RAM, OUTPUT_FILE_SYSROM)
	#put('"%s"...' % cmd)
	#os.system(cmd)
	with open(OUTPUT_FILE_SYSROM, 'wb') as h:
		h.write(cpm_data)
	
	# Zip together MAME sysrom zip
	#@TODO: Use python zip so we don't have to create temp files
	cmd = 'zip %s %s %s' % (OUTPUT_FILE_SYSROMZIP, OUTPUT_FILE_SYSROM, OUTPUT_FILE_INFO)
	put('"%s"...' % cmd)
	os.system(cmd)
	# Clean up files after zipping
	os.remove(OUTPUT_FILE_SYSROM)
	os.remove(OUTPUT_FILE_INFO)
	
	
	## Emulate
	MAMECMD = '/z/data/_code/_c/mame.git/mame64'
	EMUSYS = OUTPUT_SYS	#''gl4000'
	CART_FILE = 'out/ccp.com'
	cmd = '%s -nodebug -rompath "%s" %s -cart "%s" -window -nomax -nofilter -sleep -volume -24 -skip_gameinfo -speed 2.00 -nomouse' % (MAMECMD, ROM_DIR, EMUSYS, CART_FILE)
	put('"%s"...' % cmd)
	os.system(cmd)
	
	