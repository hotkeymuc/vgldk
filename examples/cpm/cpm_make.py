#!/usr/bin/python3
"""
CP/M Make in Python
===================

Although there exsits a rudimentary CP/M build process using Make files,
creating a CP/M image requires several steps. A lot of care has to be put into
placing the different modules (BINT/BIOS/BDOS/CCP) in memory space, while
keeping the physical memory layout and banks in mind:

Stock memory layout of GL4000:
	Memory Bank	Usage
	0000 - 3FFF	Internal ROM (bank switched via port 0x00)
	4000 - 7FFF	Internal ROM (bank switched via port 0x01, even cartridge!)
	8000 - 9FFF	Cartridge ROM 0000 - 1FFF (bank switched via port 0x03?)
	A000 - BFFF	Cartridge ROM 2000 - 3FFF (bank switched via port 0x02)
	C000 - DFFF	Internal RAM 0000 - 1FFF
	E000 - FFFF	Internal RAM 0000 - 1FFF copy / unused

In order to get CP/M running, 32KB of additional RAM have to be
"mounted" to the base of address space, "overshadowing" internal system ROM.
This can be achieved by connecting the internal ROM's ~CS HIGH while
connecting its signal to the external SRAM ~CS.

Modified memory layout for CP/M:
	Memory Bank	Usage
	0000 - 3FFF	Cartridge RAM 0000 - 3FFF
	4000 - 7FFF	Cartridge RAM 4000 - 7FFF
	8000 - 9FFF	Cartridge ROM 0000 - 1FFF (bank switched via port 0x03?)
	A000 - BFFF	Cartridge ROM 2000 - 3FFF (bank switched via port 0x02)
	C000 - DFFF	Internal RAM 0000 - 1FFF
	E000 - FFFF	Internal RAM 0000 - 1FFF copy / unused


How to start up CP/M:
	New Method: Boot stock system and swap ROM/RAM at run-time:
		* Enable internal ROM (stock configuration)
		* Insert the (E)EPROM cartridge containing the CP/M code
		* Remove RAM cartridge (or leave it connected and tie its ~CS HIGH to +5V)
		* Boot the stock firmware, start into the cartridge
		* CP/M BIOS should boot, BDOS should pause at "RAM check..."
		* While the system is running: Disable internal ROM (tie its ~CS HIGH to +5V)
		* While the system is running: Enable RAM on internal ROM space (bind its ~CS to internal ROM ~CS)
		* BDOS should detect the RAM and start up into the CCP prompt
	
	Old Method: Use cartridge as internal ROM for bootstrapping:
		* Diable internal stock ROM
		* Mount ROM cartridge into internal ROM space (bind its ~CS to internal ROM ~CS). PREPARE code must be compiled for LOC_CODE=0x0000
		* Mount RAM cartridge into cartridge space (bind its ~CS to cartridge ~CS - factory default)
		* Boot the ROM cartridge (it replaces stock firmware and boots into PREPARE code)
		* Prepare the RAM (copy code from cartridge to RAM -or- use serial loader to download the code to RAM)
		(* Power off system - optional, but recommended to keep RAM contents)
		* Mount RAM cartridge into to internal ROM space (bind its ~CS to internal ROM ~CS)
		* Mount ROM cartridge into cartridge space (bind its ~CS to cartridge ~CS - factory default)
		* Boot the system from RAM cartridge into CP/M


2023-08-23 Bernhard "HotKey" Slawik
"""

# Have a look at these settings:
#VGLDK_SERIES = 4000	# System to compile for (e.g. 4000 for VTech Genius Leader 4000 series)
VGLDK_SERIES = 6000	# System to compile for (e.g. 6000 for VTech Genius Leader 6000/700x SL series)
COMPILE_CPM = True	# Compile BINT/BIOS/BDOS (main)
COMPILE_CCP = True	# Compile CCP (command line processor)
EMULATE_CCP_IN_YAZE = False	# Run the CCP in YAZE emulator to test cross-compatibility
DISASSEMBLE_BDOS = False	# Disassemble the BDOS segment using z80dasm
GENERATE_LOWSTORAGE = True	# Re-generate lowstorage.h after successful compilation of CP/M for next time
UPLOAD_TO_MONITOR = False	# Upload the resulting CP/M image to real hardware running the MONITOR cartridge
GENERATE_MAME_ROM = True	# Generate a fake system ROM for use in MAME (required for MAME emulation)
EMULATE_IN_MAME = True	# Start MAME and switch to BDOS host mode


# Default local paths to serve as CP/M drives using bdos_host.py
# first entry = A: = default
BDOS_MOUNTS = [
	'out',	# Output directory is A: (CCP.COM should be there)
	#'programs',
	#'programs/ASCOM22',
	#'programs/BBCBASIC',	# Works!
	#'programs/CATCHUM',
	#'programs/CBASIC2',
	#'programs/LADDER',
	#'programs/STDCPM22',	# good for testing
	#'programs/TEX',
	#'programs/TP300',	#@FIXME: reboots when invoked
	#'programs/VG04',
	#'programs/WRDMASTR',
	#'programs/WS30',
	'programs/ZORK123',	# Works!
]



import time
import os	# For running files at the command line
import datetime	# For nice date
import zipfile	# For creating MAME system ROM zip file

# Import VGLDK tools
import sys
sys.path.append('../../tools')
from rel2app import *
import calcsize
import sdcc	# for custom compilation

# Import MONITOR tools for uploading output to real hardware
sys.path.append('../monitor')
import monitor


def put(t):
	print(t)

def hexdump(data, addr=0):
	put(monitor.hexdump(data, addr=addr))


def cpm_make():
	"""Main CP/M make process"""
	
	vgldk_series = VGLDK_SERIES	# Model of VTech Genius Leader to build for
	src_path = '.'	# source directory
	out_path = 'out'	# Output directory for the compiler
	lib_paths = []
	include_paths = [
		'../../include/arch/gl%d'%vgldk_series,	# Architecture specific drivers
		'../../include',	# General hardware drivers
	]
	rom_out_path = 'roms'
	
	### Set up the memory layout
	loc_cart = 0x8000
	loc_internal_ram = 0xc000
	loc_transient = 0x0100	# Start of CP/M transient area (defined as being at 0x0100. Do not change.)
	loc_transient_top = 0x7FFF	# Last byte of transient area (low 32KB)
	
	#@FIXME: I cannot mount the upper 16kb (0x4000-7FFF) in MAME at the moment...
	if vgldk_series == 6000: loc_transient_top = 0x3FFF	# Last byte of transient area (end of lower 16KB)
	
	# Set-up CP/M layout
	cart_eeprom_size = 8192	# Size of EEPROM you are planning to use
	#cpm_code_size_estimate = 0x1440	# Approx size of the generated CP/M code segment (to determine optimal layout). Must be LARGER OR EQUAL to actual binary size.
	#cpm_code_size_estimate = 0x1800	# Approx size of the generated CP/M code segment (to determine optimal layout). Must be LARGER OR EQUAL to actual binary size.
	cpm_code_size_estimate = 0x1FE0	# Approx size of the generated CP/M code segment (to determine optimal layout). Must be LARGER OR EQUAL to actual binary size.
	cpm_data_size_estimate = 0x0a00	# Approx size of CPM RAM usage
	#cpm_loc_code = 0x8000 - cpm_code_size_estimate	# Put CP/M as far up as possible in lower RAM bank
	#cpm_loc_code = 0xc000 - cpm_code_size_estimate	# Put CP/M as far up in cartridge space (0x8000-BFFF) as possible
	#cpm_loc_code = loc_cart + cart_eeprom_size - cpm_code_size_estimate	# Put CP/M as far up in cartridge EPROM (0x8000-BFFF) as possible
	cpm_loc_code = loc_cart + 0x0020	# Put CP/M at start of cartridge EPROM (0x8000-BFFF)
	cpm_loc_data = loc_internal_ram	# Use stock system RAM at 0xC000-0xDFFF. Static variable data and gsinit-code will be put to this offset in binary file. Monitor uses 0xd000 for its data
	
	# Set-up CCP layout
	ccp_code_size_estimate = 0x0b80	# Approx size of generated CCP code data (to determine optimal layout). Must be LARGER OR EQUAL to actual binary size
	#ccp_data_size_estimate = 0x0600	# Don't know... Just a guess...
	ccp_merge_into_rom = False	# Merge CCP binary into ROM, so BDOS can jump to it without loading
	#ccp_loc_code = 0x0100 + 0x20	# Regular transient COM file (add some bytes for CRT0 header)
	#ccp_loc_code = 0x4000	# Arbitrary location in transient area (must be in cartridge area if it should be merged into ROM)
	#ccp_loc_code = loc_transient_top - ccp_data_size_estimate	# At top of transient area
	#ccp_loc_code = cpm_loc_code - ccp_code_size_estimate	# Put CCP below BDOS
	#ccp_loc_code = loc_cart + 0x0020	# Put CCP at start of cartridge (leave ~0x20 bytes for cart header)
	ccp_loc_code = 0xd000
	#ccp_loc_data = ccp_loc_code - ccp_data_size_estimate	# Don't collide with BDOS/BIOS (or optional MONITOR which may be still resident)
	#ccp_loc_data = loc_internal_ram + cpm_data_size_estimate	# Use stock system RAM to keep CP/M RAM as free as possible. But don't collide with other CP/M modules.
	#ccp_loc_data = cpm_loc_data + cpm_data_size_estimate	# Use stock system RAM to keep CP/M RAM as free as possible. But don't collide with other CP/M modules.
	ccp_loc_data = ccp_loc_code + ccp_code_size_estimate
	
	
	# Configure CP/M defines (features and options)
	softuart_baud = 9600	# 9600 or 19200. 9600 is more reliable.
	
	cpm_defines = {	# Main "#define"s for the CP/M code base
		
		## Configure VGLDK hardware and drivers
		'VGLDK_SERIES': vgldk_series,	# e.g. 4000 for GL4000
		#'KEYBOARD_MINIMAL': 1,	# Use minimal keyboard (no modifiers, no buffer) to save code space
		'SOFTUART_BAUD': softuart_baud,	# SoftUART baud rate. Currently supported (2023-08) are 9600 or 19200
		
		## Configure BIOS
		#'BIOS_SCROLL_WAIT': 1,	# Wait after 1 page of text	#@TODO: Make this runtime-changable!
		#'BIOS_SHOW_BANNER': 1,	# Show CP/M text banner and version on reset
		
		#@TODO: Make aux device changable at runtime (using the "iobyte"!)
		#'BIOS_SHOW_PAPER_TAPE_MAPPING': 1,	# Print the configured paper tape configuration on boot
		'BIOS_PAPER_TAPE_TO_DISPLAY': 1,	# Redirect paper tape functions to display
		#'BIOS_PAPER_TAPE_TO_SOFTUART': 1,	# Redirect paper tape functions to SoftUART
		#'BIOS_PAPER_TAPE_TO_SOFTSERIAL': 1,	# Redirect paper tape functions to SoftSerial
		#'BIOS_PAPER_TAPE_TO_MAME': 1,	# Redirect paper tape functions to MAME
		
		
		## Configure BDOS
		#'BDOS_SHOW_BANNER': 1,	# Show "BDOS" on boot (helpful for debugging)
		'BDOS_WAIT_FOR_RAM': 1,	# Wait until RAM is writable before proceeding (recommended)
		'BDOS_RESTORE_LOWSTORAGE': 1,	# Restore/fix the lower memory area on each start
		'BDOS_RESTORE_BDOS_VECTOR': 1,	# Restore/fix BDOS vector at 0x0005 on each start
		'BDOS_RESTORE_BINT_VECTORS': 1,	# Restore/fix the interrupt vectors 0x0008...0x0038 on each start
		
		#@FIXME: Currently 0x4000-0x7FFF are not writable on GL6000SL (in emulation). "loc_transient_top" must be set to 0x3FFF... :-(
		'BDOS_PATCHED_ENTRY_ADDRESS': (loc_transient_top+1 - 3),	# Patch the BDOS vector at 0x0005 to point to the highest usable RAM bytes in transient area
		#'BDOS_PATCHED_ENTRY_ADDRESS': 0x3ffd,	# Patch the BDOS vector at 0x0005 to point to the highest usable RAM bytes in transient area
		
		# CCP handling
		'BDOS_AUTOSTART_CCP': 1,	# Start CCP on BDOS startup without asking the user (disable for debugging)
		'BDOS_LOAD_CCP_FROM_DISK': 1,	# Do not assume CCP is in ROM, but load it from disk (using BDOS file functions)
		'BDOS_CCP_LOAD_ADDRESS': ccp_loc_code,	#0x100,	# Where to load the CPM binary to
		'BDOS_CCP_JUMP_ADDRESS': ccp_loc_code,	#0x100,	#ccp_loc_code,	# Where to jump after loading
		
		## BDOS file access is not handled by BDOS itself (yet) and must be re-directed to an external host ("BDOS HOST")
		'BDOS_USE_HOST': 1,	# Re-direct file access to a host (see bdos_host.h). Recommended as there is no "internal" storage, yet.
	#	'BDOS_HOST_ACTIVITY_LED': 1,	# Light up LED on BDOS host activity
	
		#'HOST_DRIVER_PAPER_TAPE': 1,	# Re-direct to BIOS paper tape routines (and let BIOS decide what to do)
	#	'HOST_DRIVER_SOFTUART': 1,	# Re-direct to SoftUART (for use with real hardware)
	#	'HOST_DRIVER_SOFTSERIAL': 1,	# Re-direct to SoftSerial (for use with real hardware)
	'HOST_DRIVER_MAME': 1,	# Re-direct to MAME (for use in emulation)
		
		# Protocol to use for HOST communication (frame level; serial usually requires some sort of error correction and might not support 8bit)
	'HOST_PROTOCOL_BINARY': 1,	# Send using 8bit binary (e.g. for MAME)
	#	'HOST_PROTOCOL_BINARY_SAFE': 1,	# Send using binary, but with checksum and retransmission (e.g. for SoftUART)
		#'HOST_PROTOCOL_HEX': 1,	# Send using hex text (e.g. when serial host does not support 8bit data)
		
	}
	
	
	### Prepare directory
	# Make sure the output directory exists
	if not os.path.isdir(out_path):
		put('Creating directory "%s" because it does not exist...' % out_path)
		os.mkdir(out_path)
	
	
	### Compile CP/M (CRT0, BIOS, BDOS, BINT)
	cpm_hex_filename = '%s/cpm_%d.hex' % (out_path, vgldk_series)
	cpm_bin_filename = '%s/cpm_%d.bin' % (out_path, vgldk_series)
	cpm_ram_filename = '%s/cpm_%d_ram.bin' % (out_path, vgldk_series)
	cpm_cart_filename = '%s/cpm_%d_cart.bin' % (out_path, vgldk_series)
	
	put('Compiling CP/M (CRT0, BINT, BIOS, BDOS) for entry at 0x%04X...' % cpm_loc_code)
	cpm_data = sdcc.compile(
		# The .s file(s) get(s) compiled to a .rel file, which gets pre-pended to .c source
		crt_s_files = [
			#'%s/cpm_crt0.s' % src_path
			'%s/cpm_crt0_%d.s' % (src_path, vgldk_series)
		],
		#crt_rel_file = '%s/cpm_crt0.rel' % out_path,
		crt_rel_file = '%s/cpm_crt0.rel' % out_path,
		
		source_files = [
			# The crt_rel_file is automatically prepended by compile()
			'%s/cpm.c' % src_path
		],
		output_hex_file = cpm_hex_filename,
		output_bin_file = cpm_bin_filename,
		
		lib_paths = lib_paths,	# [ '../../lib' ]
		include_paths = include_paths,	# [ '../../include' ]
		loc_code = cpm_loc_code,	#0x8000 - code_size_estimate	# Put CPM as far up as possible
		loc_data = cpm_loc_data,	# static variable data and gsinit-code will be put to this address in binary file
		
		defines = cpm_defines
	)
	
	# Check for overflow (code too big)
	check_before = 0	# Check some bytes before actual end
	check_after = 8	# Check some bytes after (for overflow)
	for a in range(cpm_loc_code + cpm_code_size_estimate - check_before, cpm_loc_code + cpm_code_size_estimate + check_after):
		if cpm_data[a] != 0x00:
			put('CPM seems quite big: It should fill 0x%04X...0x%04X, but around the end (0x%04X) it is not empty. Please adjust memory layout, code estimate or reduce code size.' % (cpm_loc_code, cpm_loc_code+cpm_code_size_estimate, a))
			# Try guessing how big it is...
			guess_max = 0x800	# How far to check
			for a2 in range(a, a + guess_max):
				if cpm_data[a2] != 0: continue
				sum = 0
				for a3 in range(a2, a2+16):	# Look ahead
					sum += cpm_data[a3]
				if sum == 0:
					s = a2 - cpm_loc_code
					put('I am guessing the CPM binary is %d bytes (0x%04X) in size, instead of the estimated %d bytes (0x%04X).' % (s, s, cpm_code_size_estimate, cpm_code_size_estimate))
					break
			sys.exit(5)
	
	# Estimate actual size
	a = cpm_code_size_estimate
	while a > 0:
		if cpm_data[cpm_loc_code + a] != 0:
			break
		a -= 1
	put('CPM size is estimated to be %d bytes (0x%04X)' % (a, a))
	
	#hexdump(cpm_data[:0x0120], 0x0000)
	#hexdump(cpm_data[cpm_loc_code:0x8000], cpm_loc_code)
	
	
	if COMPILE_CCP:
		### Compile CCP (as a simple CP/M .com file)
		#loc_transient = 0x0100	# Start of CP/M transient area (defined as being 0x0100)
		#ccp_loc_code = 0x6000	# Must be known by BDOS in order to start up CCP!
		#ccp_loc_data = 0x4000	# Don't collide with BDOS/BIOS (or optional MONITOR which may be still resident)
		ccp_bin_filename = '%s/ccp.bin' % out_path
		ccp_com_filename = '%s/ccp.com' % out_path
		
		put('Compiling CCP for entry at 0x%04X...' % ccp_loc_code)
		ccp_data = sdcc.compile(
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
			
			lib_paths = lib_paths,	# [ '../../lib' ]
			include_paths = include_paths,	# [ '../../include' ]
			loc_code = ccp_loc_code,	#0x8000 - code_size_estimate	# Put CPM as far up as possible
			loc_data = ccp_loc_data,	# static variable data and gsinit-code will be put to this address in binary file
			
			defines = {
				#'VGLDK_SERIES': vgldk_series	#@FIXME: CP/M binaries should be completely architecture agnostic!
				#'CCP_SHOW_BANNER': 1,	# Show "CCP" on startup
			}
		)
		
		### Extract CCP code area as stand-alone CP/M .COM binary (starting at transient area 0x100...)
		# Beware of file offsets! File offset 0x5F00 corresponds to memory 0x6000, because only the transient area is included in the .com file!
		#ccp_data = ccp_data[ccp_loc_code - loc_transient:]	# Note! The file data does not start at memory location 0x0000, but 0x100 (CP/M program transient area!)
		ccp_data = ccp_data[ccp_loc_code:]	# Extract only the code
		ccp_code_size = len(ccp_data)	# Determine the actually generated binary size
		if ccp_code_size > ccp_code_size_estimate:
			put('Compiled CCP code size (%d bytes / 0x%04X) is larger than estimated size "ccp_code_size_estimate" (%d bytes / 0x%04X). Code blocks will likely collide/overlap. Please adjust!' % (ccp_code_size,ccp_code_size, ccp_code_size_estimate,ccp_code_size_estimate))
			sys.exit(1)
		#hexdump(ccp_data, ccp_loc_code)
		# Save to .COM file
		put('Saving CCP code to "%s"...' % ccp_com_filename)
		with open(ccp_com_filename, 'wb') as h:
			h.write(ccp_data)
		
		if ccp_merge_into_rom:
			### Merge CCP binary into CP/M image
			put('Merging CCP binary (%d bytes / 0x%04X) into CP/M image at 0x%04X-0x%04X...' % (ccp_code_size, ccp_code_size, ccp_loc_code, ccp_loc_code+ccp_code_size))
			
			# Check if area is empty
			for a in range(ccp_loc_code, ccp_loc_code+ccp_code_size):
				if cpm_data[a] != 0x00:
					put('Destination of CCP (0x%04X) is not empty! Please adjust memory layout or reduce code size.' % a)
					sys.exit(5)
				
			#cpm_data[ccp_loc_code:ccp_loc_code+len(ccp_data)] = ccp_data[:]
			cpm_data = cpm_data[:ccp_loc_code] + ccp_data + cpm_data[ccp_loc_code+ccp_code_size:]
			
			# Write merged output CCP CP/M .COM binary
			with open(cpm_bin_filename, 'wb') as h:
				h.write(cpm_data)
		#
	#
	
	### Debugging
	
	
	if EMULATE_CCP_IN_YAZE:
		### Simulate CCP binary (standalone) in YAZE emulator
		os.system('cp %s %s/CCPRUN.COM' % (ccp_bin_filename, out_path))
		cmd = './ccp_sim.sh'
		put('>> %s' % cmd)
		os.system(cmd)
	
	
	"""
	## Disassemble using z80dasm
	cmd = 'z80dasm --address --labels --source --origin=0000h %s' % cpm_bin_filename
	os.system(cmd)
	"""
	
	if DISASSEMBLE_BDOS:
		## Disassemble BDOS area to investigate "stack confusion" bugs
		bdos_addr = cpm_data[6] + cpm_data[7] * 256	# Get BDOS vector at location 0x0005-0x0007 ("JP xxxx")
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
	
	### Write separate system ROM and cart ROM files
	# Lower 32KB to system RAM file
	with open(cpm_ram_filename, 'wb') as h:
		h.write(cpm_data[:0x8000])	# CP/M RAM is 0x0000...0x7FFF
	
	# Upper code to cartridge ROM file (usually 0x8000...0xBFFF)
	with open(cpm_cart_filename, 'wb') as h:
		h.write(cpm_data[loc_cart:loc_cart+cart_eeprom_size])
	
	
	if GENERATE_LOWSTORAGE:
		### Generate new version of lowstorage.h (used for restoring on boot)
		lowstorage_origin = 0x0000	# Start at 0
		lowstorage_size = 0x5c	# We only need the lowest 0x5c bytes, because rest is dynamic (0x5c=def_fcb, 0x80=dma, 0x100=transient)
		lowstorage_h_file = '%s/lowstorage.h' % src_path
		
		# Extract lowstorage from freshly built CPM binary image
		lowstorage_data = cpm_data[lowstorage_origin:lowstorage_origin+lowstorage_size]
		
		# Generate C code...
		#lowstorage_h = 'const byte lowstorage[%d] = {%s};\n' % (', '.join(['0x%02x' % b for b in lowstorage_data]))
		lowstorage_h = '#ifndef __LOWSTORAGE_H__\n'
		lowstorage_h += '#define __LOWSTORAGE_H__\n'
		lowstorage_h += '// Auto-generated by %s on %s. Do not change!\n\n' % (__file__, str(datetime.datetime.now()))
		lowstorage_h += 'const word lowstorage_origin = 0x%04X;\n' % lowstorage_origin
		lowstorage_h += 'const word lowstorage_size = 0x%04X;\n' % lowstorage_size
		lowstorage_h += 'const byte lowstorage_data[%d] = {' % lowstorage_size
		cols = 16
		col = 0
		for i,b in enumerate(lowstorage_data):
			if (i > 0): lowstorage_h += ', '
			if (i% cols) == 0: lowstorage_h += '\n	'
			lowstorage_h += '0x%02x' % b
		lowstorage_h += '\n};\n'
		lowstorage_h += '#endif // __LOWSTORAGE_H__\n\n'
		
		# Re-write .h file
		with open(lowstorage_h_file, 'w') as h:
			h.write(lowstorage_h)
	
	
	if UPLOAD_TO_MONITOR:
		### Upload to hardware running serial MONITOR
		put('Uploading CP/M image...')
		cpm_upload(cpm_data)
	
	
	# MAME settings (used for system ROM generation and emulation)
	mame_sys = 'gl%d' % vgldk_series	# MAME system name
	
	if GENERATE_MAME_ROM:
		### Create MAME sys ROM for emulation
		put('Creating MAME sys ROM for emulation...')
		if vgldk_series == 4000:
			mame_sys = 'gl4000'	# MAME system name
			output_file_sysrom = '27-5480-00'	# This filename is specific to each MAME system!
			output_file_sysromzip = '%s/%s.zip' % (rom_out_path, mame_sys)
		elif vgldk_series == 6000:
			mame_sys = 'gl6000sl'	# MAME system name
			output_file_sysrom = '27-5894-01'	# This filename is specific to each MAME system!
			#mame_sys = 'gl7007sl'	# MAME system name
			#output_file_sysrom = '27-6060-00'	# This filename is specific to each MAME system!
			output_file_sysromzip = '%s/%s.zip' % (rom_out_path, mame_sys)
		else:
			put('At the moment, only generation of gl4000/gl6000sl ROMs is implemented.')
			sys.exit(1)
		
		# Make sure ROMs directory exists
		if not os.path.isdir(rom_out_path):
			put('Creating ROMs directory "%s", because it does not exits, yet...' % rom_out_path)
			os.mkdir(MAME_ROMS_DIR)
			put('Don\'t forget to copy the required additional driver ROMs (e.g. hd44780_a00.zip) there, too!') 
		
		
		# Generate the system ROM .zip file
		put('Generating zip file "%s"...' % output_file_sysromzip)
		with zipfile.ZipFile(output_file_sysromzip, 'w') as z:
			
			# Write dynamic info file
			with z.open('_this_is_cpm.txt', 'w') as h:
				h.write(b'This is a fake system image to emulate a bootstrapped CP/M SRAM mounted in system ROM area.\n')
				
				now = datetime.datetime.now()
				h.write(b'File created by %b on %b.' % (bytes(__file__, 'ascii'), bytes(str(now), 'ascii')))
			
			# Write properly named system ROM (must match machine specific name)
			with z.open(output_file_sysrom, 'w') as h_sysrom:
				#h_sysrom.write(cpm_data)	# Write the whole address space to sysrom
				
				# Copy CP/M RAM to system ROM file
				with open(cpm_ram_filename, 'rb') as h_rom:
					h_sysrom.write(h_rom.read())
				#
			#
		#
	#
	
	if EMULATE_IN_MAME:
		## Emulate in MAME, connect to bdos_host.py
		"""
		# Run MAME manually
		MAMECMD = '/z/data/_code/_c/mame.git/mame64'
		#MAMECMD = '/z/data/_code/_c/mame.git/mamehtk64'	# Writable ROM version (needed for CP/M)
		cmd = MAMECMD
		cmd += ' -nodebug'
		cmd += ' -rompath "%s"' % MAME_ROMS_DIR
		cmd += ' %s' % MAME_SYS
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
		
		### Emulate using bdos_host.py (Start MAME and serve files to bdos_host)
		put('Starting BDOS host...')
		import bdos_host
		#bdos_host.SHOW_TRAFFIC = True	# Debug traffic
		#bdos_host.SHOW_TRAFFIC_BYTES = True	# Debug traffic byte by byte
		
		# Start bdos_host in MAME mode...
		
		# Chose a driver
		if 'HOST_DRIVER_MAME' in cpm_defines:
			mame_roms_dir = rom_out_path	# Use the newly created sysrom
			#mame_roms_dir = None	# Use stock ROM
			
			driver = bdos_host.Driver_MAME(rompath=mame_roms_dir, emusys=mame_sys, cart_file=cpm_cart_filename)
		elif 'HOST_DRIVER_SOFTUART' in cpm_defines:
			driver = bdos_host.Driver_serial(baud=softuart_baud, stopbits=2)	# More stopbits = more time to process?
		elif 'HOST_DRIVER_SOFTSERIAL' in cpm_defines:
			driver = bdos_host.Driver_serial(baud=9600, stopbits=1)
		else:
			driver = bdos_host.Driver()
		
		# Chose a protocol
		if 'HOST_PROTOCOL_BINARY' in cpm_defines:
			protocol = bdos_host.Protocol_binary()
		elif 'HOST_PROTOCOL_BINARY_SAFE' in cpm_defines:
			protocol = bdos_host.Protocol_binary_safe()
		elif 'HOST_PROTOCOL_HEX' in cpm_defines:
			protocol = bdos_host.Protocol_hex()
		else:
			protocol = bdos_host.Protocol()
		
		# Start host
		put('Host driver=%s, protocol=%s' % (str(driver.__class__.__name__), str(protocol.__class__.__name__)))
		host = bdos_host.BDOS_Host(driver=driver, protocol=protocol, mounts=BDOS_MOUNTS)
		host.open()
		
		if not host.is_open:
			put('Connection could not be opened. Aborting.')
			sys.exit(4)
		
		#put('Loading binary file "%s"...' % (bin_filename))
		#host.upload(bin_filename)
		host.run()
	#
#




def cpm_upload(data):
	"""Upload binary to real hardware running the MONITOR ROM"""
	
	#comp = monitor.Monitor()
	#comp = monitor.Monitor(baud=19200)	# Requires monitor to be compiled using SoftUART and 19200 baud
	comp = monitor.Monitor(baud=9600)	# Requires monitor to be compiled using SoftSerial or SoftUART at 9600 baud
	comp.open()
	
	monitor.SHOW_TRAFFIC = False	# disable verbose traffic
	comp.wait_for_monitor()
	
	comp.ver()
	
	# GL4000
	
	# Set banks
	put('Setting bank switching ports (GL4000)...')
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
	
	
	ram_found = False
	addr = 0x0000
	put(f'Testing for writable RAM at 0x{addr:04X}...')
	while not ram_found:
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
	size = 0x8000	# 32KB (0x0000-0x7FFF). Do not upload to ROM cartridge at 0x8000-0xBFFF - it won't work ;-)
	
	# Clear area with zeros
	comp.memset(dest=dest, pat=0x00, size=size)
	
	#comp.upload(filename='out/cpm.bin', src_addr=0, dest_addr=0x0000, max_size=0x200, chunk_size=32, verify=True)
	#comp.upload(data=data, src_addr=0x0000, dest_addr=0x0000, chunk_size=16, skip_zeros=True, verify=True)
	comp.upload(data=data[:size], src_addr=0x0000, dest_addr=0x0000, chunk_size=56, skip_zeros=True, verify=not True)
	
	
	dest = 0x0000	# Entry jump
	put('Disabling serial and calling 0x%04X...' % dest)
	comp.write('sio;call %04x\n' % dest)
	


if __name__ == '__main__':
	#@TODO: Add argparse command line arguments!
	cpm_make()
	
