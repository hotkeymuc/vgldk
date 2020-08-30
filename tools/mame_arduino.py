#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
MAME test of Arduino projects (e.g. ParallelBuddy)

This script is used to start MAME and trap some port accesses (requires patched MAME).

It then routes all traffic to a physical device connected via serial.
This can be used to test an actual Arduino against an emulated machine.

2020-08-29 Bernhard "HotKey" Slawik
"""


import sys
PYTHON3 = (sys.version_info.major == 3)

import time
import getopt

import os
# Use "Monitor" as Arduino stand-in
sys.path.append(os.path.join('..', 'examples', 'monitor'))
import monitor
monitor.SHOW_TRAFFIC = True
ARDUINO_PORT = '/dev/ttyUSB1'
ARDUINO_BAUD = 9600


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



def put(txt):
	print('mame_arduino: %s' % txt)


def show_help():
	put(__doc__)
	put('mame_arduino.py ...')

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
	
	arduino = monitor.Monitor(port=ARDUINO_PORT, baud=ARDUINO_BAUD)
	
	emu = mame.MAME(command=MAME_COMMAND, rompath=MAME_ROMPATH, emusys=MAME_EMUSYS, cart=MAME_CART)
	
	def my_mame_on_data(s):
		
		# Parse hex string from MAME's STDOUT
		data = []
		try:
			for i in range(0, len(s), 3):
				data.append(int(s[i:i+2],16))
		except ValueError:
			put('Unparsable HEX: ' + str(s))
			return False
			
		# Pass it on to serial
		arduino.write(data)
	
	def my_send_to_mame(data):
		# Send a binary array back to the MAME emulation debug port
		#s = ''.join(['%02X\n'%v for v in data])
		#emu.write(bytes(s, 'ascii'))
		for b in data:
			#time.sleep(0.5)
			emu.write(bytes('%02X\n'%b, 'ascii'))
	
	# If MAME needs to output to a port: Redirect to Hardware
	emu.on_data = my_mame_on_data
	
	
	arduino.open()
	emu.open()
	#timeout = 5
	
	while (emu.running):	# and (timeout > 0):
		#put('MAME run timeout: %d' % timeout)
		
		d = arduino.read()
		if (d is not None) and (len(d) > 0):
			#put('Arduino to MAME: ' + str(d))
			my_send_to_mame(d)
			continue
		else:
			# Flush buffer with PB_PREFIX to make data get sent
			my_send_to_mame([ 0xfe for i in range(4)])
		
		time.sleep(.1)
		#timeout -= 1
	
	emu.close()
	arduino.close()
	
	put('End.')
