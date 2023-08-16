#!/usr/bin/python
# -*- coding: utf-8 -*-

"""

Bank-Switching test

Using MONITOR I try switching GL4000's bank switching ports
and dumping each bank, then check what it actually points to.


2023-08-14 Bernhard "HotKey" Slawik
"""


import sys
sys.path.append('../monitor')
from monitor import *

def put(t):
	print(t)

def bank_test(comp):
	"""Probe the bank switching ports and what they actually do to the memory layout"""
	### Scan all banks and dump memory
	#put(hexdump(comp.peek(0x8000, 16), 0xd000))
	
	
	# Set bankswitch ports, then scan all memory banks and see what they map to
	
	
	import json
	
	# Known ROM files to compare against
	rom_filenames = {
		'rom_gl4000q': '/z/data/_devices/VTech_Genius_Lerncomputer/Disassembly/ROM_GL4000Q_27-5480-00.bin',
		#'rom_gl4004l': '/z/data/_devices/VTech_Genius_Lerncomputer/Disassembly/ROM_GL4004L_27-5762-00.bin',
		'cart': '/z/data/_code/_c/V-Tech/vgldk.git/examples/monitor/out/monitor.cart.8kb.bin'
	}
	
	# Pre-populate with known data
	roms = {
		#'ram': b'\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xBF\xFF\xFF\xFF\x0A\x00\x0A\x03\x88\x00\x00\x00\xFF\xFF\xFF\xFF\xDF\xEF\xFF\xFF\x00\x01\x0C\x00\x4D\x6F\x6E\x69\x74\x6F\x72\x20\x31\x2E\x34\x20\x2F\x20\x34\x30\x30\x30\x20\x20\x3E\x53\x49\x4F\x20\x20\x20\x20\x20\x20\x20\x20',
		#'ram': b'\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xBF\xFD\xFF\xFF\x0A\x00\x02\x01\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFD\xEF\x7F\xFF\x00\x00\x0C\x00\x4D\x6F\x6E\x69\x74\x6F\x72\x20\x31\x2E\x34\x20\x2F\x20\x34\x30\x30\x30\x20\x20\x3E\x53\x49\x4F\x20\x20\x20\x20\x20\x20\x20\x20',	# RAM in monitor.c
		'ram': b'\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x66\x99\x4D\x6F\x6E\x69\x74\x6F\x72\x20\x31\x2E\x34\x20\x2F\x20\x34\x30\x30\x30\x20\x20\x3E\x53\x49\x4F\x20\x20\x20\x20\x20\x20\x20\x20',	# RAM in monitor.c
		'nc7eA': b'\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E\x7E',	# unconnected?
		'nc7eB': b'\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFF\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFF\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFF\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFF\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFE\x7E\x7E\x7E\xFF\x7E\x7E\x7E\xFE',	# unconnected?
	}
	
	# Read ROM binaries
	for name,filename in rom_filenames.items():
		with open(filename, 'rb') as h:
			data = h.read()
		roms[name] = data
	
	# Adresses to check
	addrs = [0x0000, 0x2000, 0x4000, 0x6000, 0x8000, 0xa000, 0xc000, 0xe000]
	
	# Size of data to get
	l = 64	#128
	# 0 = 0x0000-0x3fff, 1 = 0x4000-0x7fff, 2 = ???, 3 = (0x8000 - cannot test, because program resides there), 4 = ???, 5 = 
	port = 0
	port_results = {}
	
	# Probe different values for the bank switching port
	for value in [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x10, 0x11, 0x20, 0x21, 0x40, 0x41, 0x70, 0x71, 0x7f, 0x80, 0x81, 0x83, 0x90, 0x91, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xf1, 0xf7, 0xff]:	#range(0x100):
		comp.port_out(port, value)
		comp.readline()
		
		#results = []
		results = {}
		for addr in addrs:
			data = comp.peek(addr, l)
			put(hexdump(data, addr))
			bin = bytes(data)
			
			# Now try to find that data in known ROMs!
			result = None
			for name,rom in roms.items():
				try:
					ofs = rom.index(bin)
					put('Found in "%s" at offset 0x%08X' % (name, ofs))
					result = '%s:0x%08X' % (name, ofs)
				except ValueError:
					# Not found
					pass
			#results.append(result)
			results['0x%04X' % addr] = result
			
			time.sleep(0.2)
		
		# Store results
		port_results['port%d_0x%02X' % (port, value)] = results
		
		# Dump to conosle
		put('port%d_0x%02X = %s' % (port, value, json.dumps(results, indent='\t')))
	
	# Show complete results
	put(str(port_results))
	put('-' * 80)
	put(json.dumps(port_results, indent='\t'))
	
	
	"""
	for i in range(0x00, 0x100):
		put('Bank 0x%02X:' % i)
		#comp.port_out(0x50, i)
		#comp.dump(0x0000, 128)
		
		#comp.port_out(0x51, i)
		#comp.dump(0x4000, 128)
		
		#comp.port_out(0x52, i)	# Cart!
		#comp.dump(0x8000, 128)
		
		#comp.port_out(0x53, i)	# RAM!
		#comp.dump(0xc000, 128)
		
		#comp.port_out(0x54, i)	# RAM2!
		#comp.dump(0xe000, 128)
		
		#comp.port_out(0x55, i)	# Magic...
	#comp.poke(0xd000, [1,2,3,4,5,6,7])
	#comp.dump(0xd000)
	"""
	
	"""
	put('Sending "in 05"...')
	comp.port_out(0x05, 0x41)
	for i in range(5):
		v = comp.port_in(0x05)
		put('Got value 0x%02X' % v)
		#comp.write('in 05\n')
		#s = comp.readline()
		#put('Result is: "%s"' % s)
	"""

if __name__ == '__main__':
	
	# Initialize monitor link
	port = SERIAL_PORT = '/dev/ttyUSB0'
	baud = SERIAL_BAUD = 9600	#19200	# Software Serial currrently only supports 9600 baud (and 19200 on some models)
	comp = Monitor(port=port, baud=baud)
	comp.open()
	
	comp.wait_for_monitor()
	bank_test(comp)
	
