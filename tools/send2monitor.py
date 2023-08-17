#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Monitor Host
============

This script communicates with a VTech Genius Leader running the "monitor.c" cartridge.

You may have to enter "serial I/O" mode using command "sio" if the auto-detection does not work.

2020-06-01 Bernhard "HotKey" Slawik
"""

import sys
PYTHON3 = (sys.version_info.major == 3)


import os
sys.path.append(os.path.join('..', 'monitor'))
sys.path.append(os.path.join('..', 'examples', 'monitor'))
import monitor

import time
#import getopt
from optparse import OptionParser
import os


# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
#SERIAL_BAUD = 9600	# Stable
SERIAL_BAUD = 19200	# Experimental (using SoftUART.h)
SERIAL_TIMEOUT = 0.05
SERIAL_TIMEOUT_SHORT = 0.05

# Keep it < monitor's MAX_INPUT-10/2
#UPLOAD_BUFFER_SIZE = 56	# Used to work fine, but doesn't any more
#UPLOAD_BUFFER_SIZE = 32
UPLOAD_BUFFER_SIZE = 24
#UPLOAD_BUFFER_SIZE = 16	# Should work quite reliably
DEST_DEFAULT = 0xc800


# General
SHOW_TRAFFIC = not True


def put(txt):
	#print('host: %s' % txt)
	print(txt)


def str_hex(s):
	"Safe string conversion (encoding invalid chars to hex numbers)"
	if PYTHON3:
		if type(s) not in [str]:	# Python 3
			s = str(s)
	else:
		if type(s) not in [str, unicode]:	# Python 2
			s = str(s)
	
	r = ''
	for b in s:
		c = ord(b)
		if (c >= 0x20) and (c < 128):
			r += chr(c)
		else:
			r += ' [0x%02X] ' % c
	return r




def hexdump(data, addr=0x0000, width=16):
	o = addr
	l = len(data)
	i = 0
	r = ''
	while i < l:
		count = min(width, l-i)
		d = data[i:i+count]
		r += '%04X' % o
		r += ' '
		r += ' '.join(['%02X'%b for b in d])
		if count < width: r += ' ..' * (width-count)
		
		r += ' | '
		r += ''.join([ chr(b) if b >= 20 and b < 0x80 else '.' for b in d])
		r += '\n'
		
		i += count
		o += count
		
	return r

def show_help():
	put(__doc__)
	put('host.py ...')

if __name__ == '__main__':
	#put(__doc__)
	
	port = SERIAL_PORT
	baud = SERIAL_BAUD
	#filename = '../hello/out/hello.app.c800.bin'
	filename = None
	dest = DEST_DEFAULT
	keep_sio = False
	reset = False
	
	parser = OptionParser(
		description='Upload a binary file to a computer running monitor.c',
		usage='Usage: %prog [options] filename'
	)
	parser.add_option('-p', '--port', dest='port', default=port, action='store', type='string', help='Serial port to use (%s)' % port)
	parser.add_option('-b', '--baud', dest='baud', default=baud, action='store', type='int', help='Baud rate (%d)' % baud)
	parser.add_option('-d', '--dest', dest='dest', default=dest, action='store', type='int', help='Destination address (0x%04X)' % dest)
	parser.add_option('-k', '--keep-sio', dest='keep_sio', default=keep_sio, action='store_true', help='Keep IO on serial when calling')
	parser.add_option('-r', '--reset', dest='reset', default=reset, action='store_true', help='Reset after call')
	
	#parser.add_option('-f', '--file',
	#	action='store', type='string', dest='filename', help='Binary file to send')
	opt, args = parser.parse_args()
	
	port = opt.port
	baud = opt.baud
	dest = opt.dest
	keep_sio = opt.keep_sio
	reset = opt.reset
	
	#filename = opt.filename
	# Positional
	if len(args) > 0:
		filename = args[0]
	
	if filename is None:
		put('No filename given')
		parser.print_help()
		sys.exit(1)
	
	put('Uploading "%s" on port "%s" using %d baud to memory 0x%04X...' % (filename, port, baud, dest))
	comp = monitor.Monitor(port=port, baud=baud)
	comp.open()
	
	# Quiet output
	monitor.SHOW_TRAFFIC = False
	
	# Wait for the monitor logo/prompt
	comp.wait_for_monitor()
	
	# Disable interrupts
	#comp.write('di\n')
	#comp.wait_for_prompt()
	
	# Upload app binary
	#comp.upload(filename=filename, dest_addr=dest)
	comp.upload(filename=filename, dest_addr=dest, verify=True)
	
	# Wait a bit
	#time.sleep(0.2)
	comp.wait_for_prompt()
	
	### Call to function
	put('Calling 0x%04X...' % dest)
	
	# Call right away (having serial I/O as stdio)
	#comp.call(dest)
	
	# Switch from serial I/O back to REAL I/O and call (must be done in one line, because no more commands can be issued over serial after "sio")
	#comp.write('sio;call %04x\n' % dest)
	
	# Either just call (keeping I/O on serial) or pre-pend a "sio" to switch back to display/keyboard
	cmd = 'call %04x' % dest
	
	if not keep_sio:
		cmd = 'sio;' + cmd	# Must be called in one line, because after "sio" nothing can be issued after it!
	
	if reset:
		cmd += ';call 0000'
	
	cmd += '\n'
	comp.write(cmd)
	
	
	# Flush some residual output
	for i in range(10):
		time.sleep(0.05)
		s = comp.readline()
	
	put('End.')
	
