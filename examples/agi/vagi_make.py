#!/usr/bin/python3
"""
VAGI Make in Python
===================

Like CP/M, the VAGI runtime is quite big, so things get complicated.
VAGI's code is ~32KB (at least bigger than 16KB) so it won't fit a cartridge.
at least not without bank switching.

So I am trying to create a VAGI base system (32KB) mounted as system ROM,
and the game data mounted as an external cartridge.

2024-09-26 Bernhard "HotKey" Slawik
"""

# Have a look at these settings:
#VGLDK_SERIES = 4000	# System to compile for (e.g. 4000 for VTech Genius Leader 4000 series)
VGLDK_SERIES = 6000	# System to compile for (e.g. 6000 for VTech Genius Leader 6000/700x SL series)
GENERATE_MAME_ROM = True	# Generate a fake system ROM for use in MAME (required for MAME emulation)
CODE_SEGMENTED = True	# Create a segmented/banked ROM? Might not fit into a single 32KB one....

EMULATE_IN_MAME = True	# Start MAME
EMULATION_SPEED = 8.0	#2.0

MAME_ROMS_DIR = './roms'
if   VGLDK_SERIES == 4000: MAME_SYS = 'gl4000'	# MAME system name
elif VGLDK_SERIES == 6000: MAME_SYS = 'gl6000sl'	# MAME system name
elif VGLDK_SERIES == 7007: MAME_SYS = 'gl7007sl'	# MAME system name
else:
	put('At the moment, only generation of gl4000/gl6000sl/gl7007sl ROMs is implemented, not %s' % VGLDK_SERIES)
	sys.exit(1)


GAMES_PATH = '/z/apps/_games/_SCUMM'
#GAME_ID = 'KQ1'
#GAME_ID = 'KQ2'
#GAME_ID = 'KQ3'
#GAME_ID = 'LSL1'
#GAME_ID = 'CAULDRON'
#GAME_ID = 'SQ1'
GAME_ID = 'SQ2'	# my fav! (AGIv2)
#GAME_ID = 'PQ1'
# Not working:
##GAME_ID = 'Enclosure'
##GAME_ID = 'uriquest'
##GAME_ID = 'SpaceQuest0__rep_104'
##GAME_ID = 'SpaceQuest_NewAdventuresOfRogerWilco'
##GAME_ID = 'SpaceQuestX_TheLostChapter'

GAME_PATH = f'{GAMES_PATH}/{GAME_ID}'	# Where to find the game
GAME_CART_FILENAME = f'out/DATA_{GAME_ID}.bin'	# Where to put the bundled game ROM

import time
import os	# For running files at the command line
import datetime	# For nice date


# Import VGLDK tools
import sys
sys.path.append('../../tools')
#from rel2app import *
#import calcsize
import sdcc	# for custom compilation
import mame	# for creating MAME system ROMs
import romfs_gen	# For creating the game ROM
import noi2h	# For extracting segment information for multi-segment code

# Import MONITOR tools for uploading output to real hardware
#sys.path.append('../monitor')
#import monitor

def put(t):
	print(t)

#def hexdump(data, addr=0):
#	put(monitor.hexdump(data, addr=addr))

def game_make(game_path, cart_filename):
	# Pack one AGI game into a ROM file
	filespecs = [
		f'{game_path}/LOGDIR',
		f'{game_path}/PICDIR',
		f'{game_path}/SNDDIR',
		f'{game_path}/VIEWDIR',
		f'{game_path}/WORDS.TOK',
		f'{game_path}/VOL.*',
	]
	
	put('Generating game ROM "%s" from file specs: %s...' % (cart_filename, filespecs))
	romfs_gen.generate_rom(
		filename_src = None,	# Use empty buffer
		filename_dst = cart_filename,
		
		#filename_h = None,	# We need no C header file
		filename_h = 'romfs_data.h',
		
		filespecs = filespecs,	# Positional arguments
		
		file_offset = 0x0000,	# 0x8000,
		chip_offset = 0x0000,	# 0x8000,
		mem_offset = 0x8000,	# 0x8000,
		mem_bank_size = 0x4000,
		mem_bank_start = 0,
		align_size = 0x0010,	#max(1, opt.align_size),	#0x2000
		pad = 0xff,
		verbose = False,
		fix_crossing = False,	#True
	)
	


def vagi_make(out_name='vagi', more_defines={}):
	"""Main VAGI make process"""
	
	vgldk_series = VGLDK_SERIES	# Model of VTech Genius Leader to build for
	
	src_path = '.'	# source directory
	out_path = 'out'	# Output directory for the compiler
	lib_paths = []
	include_paths = [
		#'../../include/arch/gl%d'%vgldk_series,	# Architecture specific drivers
		'../../include/arch/%s' % MAME_SYS,	# Architecture specific drivers
		'../../include',	# General hardware drivers
	]
	
	### Set up the memory layout
	loc_rom = 0x0000
	loc_cart = 0x8000
	loc_internal_ram = 0xc000
	
	#loc_code = 0x100	# Put compiled code after the CRT0 zero page
	loc_code = 0x080	# Put compiled code after the CRT0
	#loc_data = loc_internal_ram	# Put heap in regular banked RAM
	loc_data = 0xebb8	# Put static heap in non-banked upper segment, right after VRAM
	#loc_data = 0xec00	# Put static heap in non-banked upper segment
	#if ('CODE_SEGMENT' in more_defines) and (more_defines['CODE_SEGMENT'] == 1):
	#	# Re-locate data of segment 1 out of reach of segment 0
	#	loc_data = 0xf900
	
	# Set-up layout
	#cart_eeprom_size = 8192	# Size of EEPROM you are planning to use
	
	# Configure CP/M defines (features and options)
	#softuart_baud = 9600	# 9600 or 19200. 9600 is more reliable.
	
	defines = {	# Main "#define"s for the CP/M code base
		
		## Configure VGLDK hardware and drivers
		'VGLDK_SERIES': vgldk_series,	# e.g. 4000 for GL4000
		#'KEYBOARD_MINIMAL': 1,	# Use minimal keyboard (no modifiers, no buffer) to save code space
		#'SOFTUART_BAUD': softuart_baud,	# SoftUART baud rate. Currently supported (2023-08) are 9600 or 19200
		
		**more_defines	# Optional arguments
	}
	
	
	### Prepare directory
	# Make sure the output directory exists
	if not os.path.isdir(out_path):
		put('Creating directory "%s" because it does not exist...' % out_path)
		os.mkdir(out_path)
	
	### Compile
	hex_filename  = '%s/%s.hex' % (out_path, out_name)
	bin_filename  = '%s/%s.bin' % (out_path, out_name)
	#ram_filename  = '%s/vagi_%d_ram.bin' % (out_path, vgldk_series)
	#cart_filename = '%s/vagi_%d_cart.bin' % (out_path, vgldk_series)
	#cart_filename = '%s/vagi.cart.1024kb_SQ2.bin' % out_path
	cart_filename = GAME_CART_FILENAME
	
	put('Compiling VAGI for entry at 0x%04X...' % loc_code)
	data = sdcc.compile(
		# The .s file(s) get(s) compiled to a .rel file, which gets pre-pended to .c source
		crt_s_files = [
			#'%s/_crt0_%d.s' % (src_path, vgldk_series)
			#'../../include/arch/standalone_crt0.s'
			'../../include/arch/%s/standalone_crt0.s' % MAME_SYS
		],
		crt_rel_file = '%s/%s_standalone_crt0.rel' % (out_path, out_name),
		
		source_files = [
			# The crt_rel_file is automatically prepended by compile()
			'%s/vagi.c' % src_path
		],
		output_hex_file = hex_filename,
		output_bin_file = bin_filename,
		
		lib_paths = lib_paths,	# [ '../../lib' ]
		include_paths = include_paths,	# [ '../../include' ]
		loc_code = loc_code,	#0x8000 - code_size_estimate	# Put CPM as far up as possible
		loc_data = loc_data,	# static variable data and gsinit-code will be put to this address in binary file
		
		defines = defines
	)
	
	"""
	# Check for overflow (code too big)
	check_before = 0	# Check some bytes before actual end
	check_after = 8	# Check some bytes after (for overflow)
	for a in range(loc_code + code_size_estimate - check_before, cpm_loc_code + cpm_code_size_estimate + check_after):
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
	"""
	
	
	### Debugging
	"""
	## Disassemble using z80dasm
	cmd = 'z80dasm --address --labels --source --origin=0000h %s' % cpm_bin_filename
	os.system(cmd)
	"""
	
	### Write separate system ROM and cart ROM files
	"""
	# Lower 32KB to system RAM file
	with open(cpm_ram_filename, 'wb') as h:
		h.write(cpm_data[:0x8000])	# CP/M RAM is 0x0000...0x7FFF
	"""
	"""
	# Upper code to cartridge ROM file (usually 0x8000...0xBFFF)
	with open(cpm_cart_filename, 'wb') as h:
		h.write(cpm_data[loc_cart:loc_cart+cart_eeprom_size])
	"""
	
	
	# MAME settings (used for system ROM generation and emulation)
	
	
#


def vagi_emulate(cart_filename=None):
	
	## Emulate in MAME
	vgldk_series = VGLDK_SERIES	# Model of VTech Genius Leader to build for
	
	# Run MAME manually
	MAMECMD = '/z/data/_code/_c/mame.git/mame64'
	#MAMECMD = '/z/data/_code/_c/mame.git/mamehtk64'	# Writable ROM version (needed for CP/M)
	cmd = MAMECMD
	cmd += ' -nodebug'
	cmd += ' -rompath "%s"' % MAME_ROMS_DIR	#MAME_ROMS_DIR
	cmd += ' %s' % MAME_SYS
	if cart_filename is not None: cmd += ' -cart "%s"' % cart_filename
	cmd += ' -window'
	cmd += ' -nomax'
	cmd += ' -nofilter'
	cmd += ' -sleep'
	cmd += ' -volume -24'
	cmd += ' -skip_gameinfo'
	#cmd += ' -speed 2.00'
	cmd += ' -speed %.2f' % EMULATION_SPEED
	cmd += ' -nomouse'
	put('"%s"...' % cmd)
	os.system(cmd)
	
	'''
	### Emulate using bdos_host.py (Start MAME and serve files to bdos_host)
	put('Starting BDOS host...')
	import bdos_host
	#bdos_host.SHOW_TRAFFIC = True	# Debug traffic
	#bdos_host.SHOW_TRAFFIC_BYTES = True	# Debug traffic byte by byte
	
	# Start bdos_host in MAME mode...
	
	# Chose a driver
	if 'HOST_DRIVER_MAME' in cpm_defines:
		mame_roms_dir = MAME_ROMS_DIR	# Use the newly created sysrom
		#mame_roms_dir = None	# Use stock ROM
		
		driver = bdos_host.Driver_MAME(rompath=mame_roms_dir, emusys=MAME_SYS, cart_file=cpm_cart_filename)
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
	'''

if __name__ == '__main__':
	#@TODO: Add argparse command line arguments!
	
	# Compile the game data ROM
	game_make(game_path=GAME_PATH, cart_filename=GAME_CART_FILENAME)
	
	# Compile the VAGI runtime
	
	if CODE_SEGMENTED:
		# Compile extended code segment
		put('Compiling extended segment binary...')
		vagi_make('vagi_segment_1', {'CODE_SEGMENT': 1, 'CODE_SEGMENT_1': 1})
		# Extract entry addresses
		noi2h.noi2h('out/vagi_segment_1.noi', 'code_segment_1.h', 'code_segment_1_')
		
		# Compile main code segment
		put('Compiling main segment binary...')
		vagi_make('vagi_segment_0', {'CODE_SEGMENT': 0, 'CODE_SEGMENT_0': 1})
		
		# Get system ROM data
		put('Combining binaries...')
		with open('out/vagi_segment_0.bin', 'rb') as h:
			data_segment_0 = h.read(0x8000)
			if len(data_segment_0) < 0x8000: data_segment_0 += bytes([0xff] * (0x8000 - len(data_segment_0)))
		with open('out/vagi_segment_1.bin', 'rb') as h:
			data_segment_1 = h.read(0x8000)
			if len(data_segment_1) < 0x8000: data_segment_1 += bytes([0xff] * (0x8000 - len(data_segment_1)))
		
		# Combine
		data_sysrom = data_segment_0[:0x8000] + data_segment_1[:0x8000]
		with open('out/vagi_combined.bin', 'wb') as h:
			h.write(data_sysrom)
	
	else:
		# Compile a non-segmented, single binary
		put('Compiling single-segment binary...')
		vagi_make('vagi_all', {'CODE_SEGMENT':1, 'CODE_SEGMENT_0': 1, 'CODE_SEGMENT_1': 1})	# Compile all
		
		# Get system ROM data
		with open('out/vagi_all.bin', 'rb') as h:
			data_sysrom = h.read()	#(0x8000)
	
	# Create final image
	if GENERATE_MAME_ROM:
		### Create MAME sys ROM for emulation
		put('Creating MAME sys ROM for emulation...')
		
		# Make sure ROMs directory exists
		if not os.path.isdir(MAME_ROMS_DIR):
			put('Creating ROMs directory "%s", because it does not exits, yet...' % MAME_ROMS_DIR)
			os.mkdir(MAME_ROMS_DIR)
			put('Don\'t forget to copy the required additional driver ROMs (e.g. hd44780_a00.zip) there, too!') 
		
		# Create the ZIP file
		output_file_sysromzip = '%s/%s.zip' % (MAME_ROMS_DIR, MAME_SYS)
		put('Creating MAME system ROM zip at "%s"...' % output_file_sysromzip)
		mame.create_sysrom_zip(
			rom_data=data_sysrom,
			mame_sys=MAME_SYS,
			zip_filename=output_file_sysromzip
		)
	#
	
	# Emulate
	if EMULATE_IN_MAME:
		vagi_emulate(cart_filename=GAME_CART_FILENAME)
	
