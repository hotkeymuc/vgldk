#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
MAME piper

This script is used to start MAME and trap some port accesses (requires patched MAME).

It then re-directs port-accesses via serial to the real hardware.
Goal is to find out more about undocumented hardware (e.g. audio chip)

2020-05-27 Bernhard "HotKey" Slawik
"""


import sys
PYTHON3 = (sys.version_info.major == 3)

import time
import sys	# for sys.argv and sys.exit(status)
import getopt
import os


# MAME
sys.path.append(os.path.join('..', '..', 'tools'))
import mame
#MAME_COMMAND = './mame64'
#MAME_COMMAND = '../../../mame.git/mame64'
MAME_COMMAND = '/z/data/_code/_c/mame.git/mame64'
#MAME_ROMPATH = None
MAME_ROMPATH = '/z/apps/_emu/_roms'
MAME_EMUSYS = 'gl4000'
MAME_CART = 'out/monitor.cart.16kb.bin'
mame.SHOW_TRAFFIC = False


# Real hardware
import monitor
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	# Software Serial currrently only supports 9600 baud (fixed)


def put(txt):
	print('mame_pipe: %s' % txt)


def show_help():
	put(__doc__)
	put('mame_pipe.py ...')

if __name__ == '__main__':
	#put(__doc__)
	
	argv = sys.argv[1:]
	try:
		opts, args = getopt.getopt(argv, 'pbh:', ['port=','baud='])
	except getopt.GetoptError:
		show_help()
		sys.exit(2)
	
	port = SERIAL_PORT
	baud = SERIAL_BAUD
	
	for opt, arg in opts:
		if opt == '-h':
			show_help()
			sys.exit()
		elif opt in ('-p', '--port'):
			port = arg
		elif opt in ('-b', '--baud'):
			baud = int(arg)
	
	
	comp = monitor.Monitor(port=port, baud=baud)
	comp.open()
	
	#comp.write('ver\n')
	#s = comp.readline()
	
	#put('Sending "in 05"...')
	"""
	comp.port_out(0x05, 0x41)
	for i in range(5):
		v = comp.port_in(0x05)
		put('Got value 0x%02X' % v)
		#comp.write('in 05\n')
		#s = comp.readline()
		#put('Result is: "%s"' % s)
	"""
	
	emu = mame.MAME(command=MAME_COMMAND, rompath=MAME_ROMPATH, emusys=MAME_EMUSYS, cart=MAME_CART)
	
	def my_port_out(p, v):
		put('MAME -> HW\tport 0x%02X := 0x%02X' % (p, v))
		comp.port_out(p, v)
		
		#comp.ints()	# Let interrupts happen
		
	def my_port_in(p):
		v = comp.port_in(p)
		put('MAME <- HW\tport 0x%02X == 0x%02X' % (p, v))
		#comp.ints()	# Let interrupts happen
		return v
	
	def my_data(s):
		if (len(s) < 7): return
		s = ''.join([chr(b) for b in s])
		
		if s[0] == 'W':
			ps = s[2:4]
			pi = int(ps, 16)
			vs = s[5:7]
			vi = int(vs, 16)
			my_port_out(pi, vi)
			
		elif s[0] == 'R':
			ps = s[2:4]
			pi = int(ps, 16)
			vi = my_port_in(pi)
			emu.write('%02X\n' % vi)
	
	# If MAME needs to output to a port: Redirect to Hardware
	emu.on_data = my_data
	
	emu.open()
	while emu.running:
		time.sleep(1)
	
	
	put('End.')
