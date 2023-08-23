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
#import shutil
#import subprocess
import zipfile	# For creating MAME system ROM zip file
import datetime	# For nice date

# Import VGLDK tools
import sys
sys.path.append('../../tools')
from rel2app import *
import calcsize

# Import MONITOR tools for uploading output to real hardware
sys.path.append('../monitor')
import monitor


def put(t):
	print(t)

def hexdump(data, addr=0):
	put(monitor.hexdump(data, addr=addr))


def compile(
	## Defaults...
	crt_s_files = [
		'./crt0.s'
	],
	crt_rel_file = 'out/crt0.rel',
	source_files = [
		'./main.c'	# CRT0 rel file is prepended automatically
	],
	output_hex_file = 'out/main.hex',
	output_bin_file = 'out/main.bin',
	lib_path = None,	#'../../lib'
	include_path = None,	#'../../include',
	
	loc_code = 0x8000,
	#loc_stack = 0x0020,	# ?
	loc_data = 0xc000,
	#loc_idata = None,	#0xc800	# ?
	#loc_xram = None,	# ?
	
	defines = {
		'VGLDK_SERIES': 4000
	}):
	
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
	
	put('>> %s' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Compile source file(s) using SDCC, generate .hex file
	cmd = 'sdcc -mz80'
	
	#cmd += ' --model-small'	# model-small = default, but not supported on Z80
	cmd += ' --no-std-crt0'	# Provide our own crt0 .rel
	#cmd += ' --nostdlib'
	
	#@TODO: Allow multiple paths
	if lib_path is not None: cmd += ' --lib-path %s' % lib_path
	if include_path is not None: cmd += ' -I %s' % include_path
	
	cmd += ' --code-loc 0x%04X' % loc_code
	#if loc_stack is not None: cmd += ' --stack-loc 0x%04X' % loc_stack
	if loc_data is not None: cmd += ' --data-loc 0x%04X' % loc_data
	#if loc_idata is not None: cmd += ' --idata-loc 0x%04X' % loc_idata
	#if loc_xram is not None: cmd += ' --xram-loc 0x%04X' % loc_xram
	
	# --xram-loc 0xc000
	# --verbose
	#cmd += ' --vc'	# vc = messages are compatible with Micro$oft visual studio
	# Add "#define"s
	for k,v in defines.items():
		cmd += ' -D %s=%s' % (k, v)
	
	#cmd += ' -dD'	# Tell the preprocessor to pass all macro definitions into the output, in their proper sequence in the rest of the output.
	#cmd += ' --verbose'	# Show stages of compilation
	
	cmd += ' -o %s' % output_hex_file
	cmd += ' %s %s' % (crt_rel_file, ' '.join(source_files))	# .rel, .c, .c ...
	
	put('>> %s' % cmd)
	r = os.system(cmd)
	if r != 0:
		put('Result: %s' % r)
		sys.exit(1)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	# Show intermediate .rel file
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	if output_bin_file is not None:
	### Convert .hex, generate .bin file
		cmd = 'objcopy -Iihex -Obinary %s %s' % (output_hex_file, output_bin_file)
		put('>> %s' % cmd)
		r = os.system(cmd)
		if r != 0:
			put('Result: %s' % r)
			sys.exit(1)
		#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
		
		#@FIXME: The .bin file also contains some data at the RAM area... (e.g. file offset 0xc000)
	
	### Show stats
	#process_rel_file(crt_rel_file, output_bin_file)
	
	### Show cartridge size statistics
	#calcsize.analyze(output_bin_file)
	
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
	
	vgldk_series = 4000	# Model of VTech Genius Leader
	src_path = '.'
	out_path = 'out'
	lib_path = None
	include_path = '../../include'
	
	# Compile CP/M (CRT0, BIOS, BDOS, BINT)
	put('Compiling CP/M (CRT0, BINT, BIOS, BDOS)...')
	transient = 0x0100	# Start of CP/M transient area (defined as being 0x0100)
	
	cpm_code_size_estimate = 0x1200	# Approx size of CPM code data (to determine optimal offset). Must LARGER OR EQUAL to actual binary size
	#cpm_loc_code = 0x8000 - cpm_code_size_estimate	# Put CPM as far up as possible in lower RAM bank
	cpm_loc_code = 0xc000 - cpm_code_size_estimate	# Put CPM as far up as possible in cartridge
	cpm_loc_data = 0xc000	# Use stock system RAM. Static variable data and gsinit-code will be put to this address in binary file. Monitor uses 0xd000 for its data
	cpm_bin_filename = '%s/cpm.bin' % out_path
	cpm_rom_filename = '%s/cpm_rom.bin' % out_path
	cpm_cart_filename = '%s/cpm_cart.bin' % out_path
	
	ccp_code_size_estimate = 0x0d00	# Approx size of CCP code data (to determine optimal offset). Must LARGER OR EQUAL to actual binary size
	ccp_data_size_estimate = 0x0600	# Don't know actually...
	ccp_loc_code = cpm_loc_code - ccp_code_size_estimate	# Put CCP below BDOS
	#ccp_loc_data = ccp_loc_code - ccp_data_size_estimate	# Don't collide with BDOS/BIOS (or optional MONITOR which may be still resident)
	ccp_loc_data = 0xc800	# Use stock system RAM, but don't collide with other modules
	
	cpm_data = compile(
		# The .s file(s) get(s) compiled to a .rel file, which gets pre-pended to .c source
		crt_s_files = [
			'%s/cpm_crt0.s' % src_path
		],
		crt_rel_file = '%s/cpm_crt0.rel' % out_path,
		
		source_files = [
			# The crt_rel_file is automatically prepended by compile()
			'%s/cpm.c' % src_path
		],
		output_hex_file = '%s/cpm.hex' % out_path,
		output_bin_file = cpm_bin_filename,
		
		lib_path = lib_path,	#'../../lib'
		include_path = include_path,	#'../../include'
		loc_code = cpm_loc_code,	#0x8000 - code_size_estimate	# Put CPM as far up as possible
		loc_data = cpm_loc_data,	# static variable data and gsinit-code will be put to this address in binary file
		
		defines = {
			## Configure hardware
			'VGLDK_SERIES': vgldk_series,
			'SOFTUART_BAUD': 19200,
			
			## Configure BIOS
			#'BIOS_SCROLL_WAIT': 1,	# Wait after 1 page of text	#@TODO: Make this runtime-changable!
			'BIOS_SHOW_BANNER': 1,	# Show CP/M banner and version on boot
			
			# Paper tape is display by default. But it can be changed at compile time (in the future maybe at runtime using the "iobyte")
			#@TODO: Make this runtime-changable!
			#'BIOS_PAPER_TAPE_TO_SOFTUART': 1,	# Redirect paper tape to SoftUART
			#'BIOS_PAPER_TAPE_TO_MAME': 1,	# Redirect paper tape to MAME
			#'BIOS_SHOW_PAPER_TAPE_MAPPING': 1,	# Print the configured paper tape configuration on boot
			
			## Configure BDOS
			'BDOS_PATCHED_ENTRY_ADDRESS': (0x8000 - 3),	# Patch the BDOS vector at 0x0005 to point to the highest usable RAM bytes in transient area
			'BDOS_AUTOSTART_CCP': 1,	# Start CCP wihtout asking
			
			## BDOS file accesses are not handled in BDOS itself (yet).
			'BDOS_USE_HOST': 1,	# Re-direct file accesses to a host (see bdos_host.h)
			#'BDOS_HOST_TO_PAPER_TAPE': 1,	# Re-direct to BIOS paper tape routines and let BIOS decide what to do
			#'BDOS_HOST_TO_SOFTUART': 1,	# Re-direct to SoftUART (might be linked statically - you might want to use paper tape)
			'BDOS_HOST_TO_MAME': 1,	# Re-direct to MAME
			
			
			## Configure CCP
			'CCP_LOC_CODE': '0x%04X'%ccp_loc_code	# Tell BDOS where to find CCP
		}
	)
	
	#hexdump(cpm_data[:0x0120], 0x0000)
	#hexdump(cpm_data[cpm_loc_code:0x8000], cpm_loc_code)
	
	
	## Compile CCP (as a simple program)
	put('Compiling CCP for entry at 0x%04X...' % ccp_loc_code)
	ccp_bin_filename = '%s/ccp.com' % out_path
	#transient = 0x0100	# Start of CP/M transient area (defined as being 0x0100)
	#ccp_loc_code = 0x6000	# Must be known by BDOS in order to start up CCP!
	#ccp_loc_data = 0x4000	# Don't collide with BDOS/BIOS (or optional MONITOR which may be still resident)
	
	ccp_data = compile(
		# The .s file(s) get(s) compiled to a .rel file, which gets pre-pended to .c source
		crt_s_files = [
			'%s/program_crt0.s' % src_path
		],
		crt_rel_file = '%s/ccp_crt0.rel' % out_path,
		
		source_files = [
			# The crt_rel_file is automatically prepended by compile()
			'%s/ccp.c' % src_path
		],
		output_hex_file = '%s/ccp.hex' % out_path,
		output_bin_file = ccp_bin_filename,
		
		lib_path = lib_path,	#'../../lib'
		include_path = include_path,	#'../../include'
		loc_code = ccp_loc_code,	#0x8000 - code_size_estimate	# Put CPM as far up as possible
		loc_data = ccp_loc_data,	# static variable data and gsinit-code will be put to this address in binary file
		
		defines = {
			#'VGLDK_SERIES': vgldk_series	#@FIXME: CP/M binaries should be completely architecture agnostic!
		}
	)
	
	## Extract CCP code area as stand-alone CP/M .COM binary (starting at transient area 0x100...)
	# Beware of file offsets! File offset 0x5F00 corresponds to memory 0x6000, because only the transient area is included in the .com file!
	ccp_data = ccp_data[ccp_loc_code - transient:]	# Note! The binary data does not start at memory location 0x0000, but 0x100 (CP/M program transient area!)
	ccp_code_size = len(ccp_data)
	if ccp_code_size > ccp_code_size_estimate:
		put('Compiled CCP code size (%d bytes / 0x%04X) is larger than estimated size "ccp_code_size_estimate" (%d bytes / 0x%04X). Code blocks will likely collide/overlap. Please adjust!' % (ccp_code_size,ccp_code_size, ccp_code_size_estimate,ccp_code_size_estimate))
		sys.exit(1)
	#hexdump(ccp_data, ccp_loc_code)
	
	
	## Merge CCP binary into CP/M image
	put('Merging CCP binary (%d bytes / 0x%04X) into CP/M image at 0x%04X-0x%04X...' % (ccp_code_size, ccp_code_size, ccp_loc_code, ccp_loc_code+ccp_code_size))
	#cpm_data[ccp_loc_code:ccp_loc_code+len(ccp_data)] = ccp_data[:]
	cpm_data = cpm_data[:ccp_loc_code] + ccp_data + cpm_data[ccp_loc_code+ccp_code_size:]
	
	# Write merged output CCP CP/M .COM binary
	with open(cpm_bin_filename, 'wb') as h:
		h.write(cpm_data)
	
	
	## Simulate CCP in YAZE
	os.system('cp %s %s/CCPRUN.COM' % (ccp_bin_filename, out_path))
	cmd = './ccp_sim.sh'
	put('>> %s' % cmd)
	os.system(cmd)
	
	
	"""
	## Decompile using z80dasm
	cmd = 'z80dasm --address --labels --source --origin=0000h %s' % cpm_bin_filename
	os.system(cmd)
	"""
	
	### Debugging
	
	DISASSEMBLE_BDOS = False
	if DISASSEMBLE_BDOS:
		## Disassemble BDOS area to investigate "stack confusion" bugs
		bdos_addr = cpm_data[6] + cpm_data[7] * 0x100	# Get BDOS vector at location 0x0005-0x0007 ("JP xxxx")
		put('Extracted BDOS vector at 0x0005: @bdos = 0x%04X' % bdos_addr)
		put('Debugging BDOS area')
		hexdump(cpm_data[bdos_addr:bdos_addr+128], bdos_addr)
		
		dasm_tmp = '/tmp/z80dasm.asm'
		cmd = 'z80dasm --address --labels --source --origin=%04Xh %s >%s' % (0x0000, cpm_bin_filename, dasm_tmp)
		os.system(cmd)
		started = False
		start_label = 'l%04xh:' % bdos_addr	# Label to start output from
		max_lines = 100
		line_count = 0
		for l in open(dasm_tmp, 'r'):
			if l.startswith(start_label):
				started = True
			if started:
				put(l[:-1])
				line_count += 1
				if line_count >= max_lines:
					put('(Stopping output after %d lines.)' % line_count)
					break
		#sys.exit(1)
	#
	
	## Write separate system ROM and cart ROM files
	with open(cpm_rom_filename, 'wb') as h:
		h.write(cpm_data[:0x8000])
	with open(cpm_cart_filename, 'wb') as h:
		h.write(cpm_data[0x8000:])
	
	## Generate new version of lowstorage.h (used for restoring on boot)
	lowstorage_origin = 0x0000	# Start at 0
	lowstorage_size = 0x5c	# We only need the lowest 0x5c bytes, because rest is dynamic (0x5c=def_fcb, 0x80=dma, 0x100=transient)
	lowstorage_h_file = '%s/lowstorage.h' % src_path
	lowstorage_data = cpm_data[lowstorage_origin:lowstorage_origin+lowstorage_size]
	
	# Generate
	#lowstorage_h = 'const byte lowstorage[%d] = {%s};\n' % (', '.join(['0x%02x' % b for b in lowstorage_data]))
	lowstorage_h = '#ifndef __LOWSTORAGE_H__\n'
	lowstorage_h = '#define __LOWSTORAGE_H__\n'
	lowstorage_h += '// Auto-generated by %s on %s. Do not change!\n\n' % (__file__, str(datetime.datetime.now()))
	lowstorage_h += 'const word lowstorage_origin = 0x%04X;\n' % lowstorage_origin
	lowstorage_h += 'const word lowstorage_size = 0x%04X;\n' % lowstorage_size
	lowstorage_h += 'const byte lowstorage_data[%d] = {'
	cols = 16
	col = 0
	for i,b in enumerate(lowstorage_data):
		if (i > 0): lowstorage_h += ', '
		if (i% cols) == 0: lowstorage_h += '\n	'
		lowstorage_h += '0x%02x' % b
	lowstorage_h += '};\n'
	lowstorage_h += '#endif // __LOWSTORAGE_H__\n\n'
	
	# Re-write .h file
	with open(lowstorage_h_file, 'w') as h:
		h.write(lowstorage_h)
	
	
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
	
	"""
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
	"""
	
	with zipfile.ZipFile(OUTPUT_FILE_SYSROMZIP, 'w') as z:
		
		# Write dynamic info file
		with z.open(OUTPUT_FILE_INFO, 'w') as h:
			h.write(b'This is a fake system image to emulate a bootstrapped CP/M SRAM mounted in system ROM area.\n')
			
			now = datetime.datetime.now()
			h.write(b'File created by %b on %b.' % (bytes(__file__, 'ascii'), bytes(str(now), 'ascii')))
		
		# Write properly named system ROM (must match machine specific name)
		with z.open(OUTPUT_FILE_SYSROM, 'w') as h_sysrom:
			#h_sysrom.write(cpm_data)	# Write the whole address space to sysrom
			
			# Copy rom area only
			with open(cpm_rom_filename, 'rb') as h_rom:
				h_sysrom.write(h_rom.read())
			
		
	
	
	## Emulate
	EMUSYS = OUTPUT_SYS	# 'gl4000'
	#CART_FILE = ccp_bin_filename	#'out/ccp.com'
	
	"""
	# Run MAME manually
	MAMECMD = '/z/data/_code/_c/mame.git/mame64'
	#MAMECMD = '/z/data/_code/_c/mame.git/mamehtk64'	# Writable ROM version (needed for CP/M)
	cmd = MAMECMD
	cmd += ' -nodebug'
	cmd += ' -rompath "%s"' % ROM_DIR
	cmd += ' %s' % OUTPUT_SYS
	cmd += ' -cart "%s"' % cpm_cart_filename
	cmd += ' -window'
	cmd += ' -nomax'
	cmd += ' -nofilter'
	cmd += ' -sleep'
	cmd += ' -volume -24'
	cmd += ' -skip_gameinfo'
	cmd += ' -speed 2.00'
	cmd += ' -nomouse'
	put('"%s"...' % cmd)
	os.system(cmd)
	"""
	
	# Use BDDOS_HOST to start MAME
	import bdos_host
	
	#bdos_host.SHOW_TRAFFIC = True	# Debug traffic
	
	# Default local paths to serve as CP/M drives
	paths = [
		# first entry = A: = default
		#'programs',
		
		#'programs/ASCOM22',
		'programs/BBCBASIC',
		#'programs/CATCHUM',
		#'programs/CBASIC2',
		#'programs/LADDER',
		#'programs/STDCPM22',
		#'programs/TEX',
		#'programs/TP300',
		#'programs/VG04',
		#'programs/WRDMASTR',
		#'programs/WS30',
		'programs/ZORK123',
	]
	#comp = bdos_host.Host_Serial(port=port, baud=baud, paths=paths)
	comp = bdos_host.Host_MAME(rompath=ROM_DIR, emusys=OUTPUT_SYS, cart_file=cpm_cart_filename, paths=paths)
	
	comp.open()
	
	if not comp.is_open:
		put('Connection could not be opened. Aborting.')
		sys.exit(4)
	
	#put('Loading binary file "%s"...' % (bin_filename))
	#comp.upload(bin_filename)
	comp.run()
	
	