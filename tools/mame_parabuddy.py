#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
MAME test of ParallelBuddy

This script is used to start MAME and trap some port accesses (requires patched MAME).
It requires you to run a cartridge that uses parabuddy.h and PB_USE_MAME switch.

It then interprets all data as "ParallelBuddy" protocol data.

2020-08-29 Bernhard "HotKey" Slawik
"""


import sys
PYTHON3 = (sys.version_info.major == 3)

import time
import sys	# for sys.argv and sys.exit(status)
import getopt
import os


# MAME
import mame
#MAME_COMMAND = './mame64'
#MAME_COMMAND = '../../../mame.git/mame64'
MAME_COMMAND = '/z/data/_code/_c/mame.git/mame64'
#MAME_ROMPATH = None
MAME_ROMPATH = '/z/apps/_emu/_roms'
MAME_EMUSYS = 'gl4000'
MAME_CART = '../examples/monitor/out/monitor.cart.16kb.bin'
mame.SHOW_TRAFFIC = False

# ParallelBuddy
import parabuddy


def put(txt):
	print('mame_parabuddy: %s' % txt)


def show_help():
	put(__doc__)
	put('mame_parabuddy.py ...')

if __name__ == '__main__':
	#put(__doc__)
	
	"""
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
	"""
	
	pb = parabuddy.ParaBuddy()
	emu = mame.MAME(command=MAME_COMMAND, rompath=MAME_ROMPATH, emusys=MAME_EMUSYS, cart=MAME_CART)
	
	def my_data(s):
		
		# Parse hex string from MAME's STDOUT
		data = []
		try:
			for i in range(0, len(s), 3):
				data.append(int(s[i:i+2],16))
		except ValueError:
			put('Unparsable HEX: ' + str(s))
			return False
			
		# Pass it on to ParallelBuddy
		pb.handle_data(data)
	
	def my_send(data):
		#s = ''.join(['%02X\n'%v for v in data])
		#emu.write(bytes(s, 'ascii'))
		for b in data:
			#time.sleep(0.5)
			emu.write(bytes('%02X\n'%b, 'ascii'))
	
	# If MAME needs to output to a port: Redirect to Hardware
	emu.on_data = my_data
	pb.on_send = my_send
	
	emu.open()
	#timeout = 5
	
	while (emu.running):	# and (timeout > 0):
		#put('MAME run timeout: %d' % timeout)
		time.sleep(1)
		#timeout -= 1
	
	emu.close()
	
	put('End.')
