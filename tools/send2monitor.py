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


if PYTHON3:
	from _thread import start_new_thread	# Python 3
else:
	from thread import start_new_thread	# Python 2

import serial
#import subprocess

import time
import sys	# for sys.argv and sys.exit(status)
#import getopt
from optparse import OptionParser
import os


# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	# Software Serial currrently only supports 9600 baud (fixed)
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



class Hardware:
	"Actual hardware connected via serial (monitor.c using SoftSerial on VTech Genius Leader)"
	
	def __init__(self, port=SERIAL_PORT, baud=SERIAL_BAUD):
		self.is_open = False
		self.running = False
		
		# Serial state
		self.port = port
		self.baud = baud
		self.ser = None
	
	def __del__(self):
		#self.serial_close()
		pass
	
	def put(self, s):
		put('Hardware	%s' % s)
	
	def open(self):
		self.serial_open()
		
	def serial_open(self):
		self.ser = None
		
		self.put('Opening "%s" at %d baud...' % (self.port, self.baud))
		
		try:
			self.ser = serial.Serial(
				port=self.port,
				baudrate=self.baud,
				bytesize=serial.EIGHTBITS,
				parity=serial.PARITY_NONE,
				stopbits=serial.STOPBITS_ONE,
				timeout=SERIAL_TIMEOUT,
				#exclusive=True,
				#xonxoff=0,
				#rtscts=0,
				#inter_byte_timeout=0.1
				)
			self.put('Open!')
			self.is_open = True
		except serial.serialutil.SerialException as e:
			#except e:
			self.put('Error opening serial device: %s' % str(e))
			self.is_open = False
			raise e
			return False
		
		#self.run()
	
	def close(self):
		self.running = False
		time.sleep(0.2)
		
		if (self.ser is not None):
			self.ser.close()
			self.ser = None
	
	def write(self, s):
		if self.ser is None:
			self.put('! Cannot send data, because serial is not open!')
			return False
		
		if (SHOW_TRAFFIC): self.put('>>> "%s"' % str_hex(s))
		
		if PYTHON3:
			self.ser.write(bytes(s, 'ascii'))	# Python3
		else:
			self.ser.write(s)	# Python2
		
		self.ser.flush()	# Wait until all data is written
		
	
	def readline(self, timeout=SERIAL_TIMEOUT):
		"""
		data = ''
		while (self.ser.in_waiting > 0):
			data += ''.join([chr(b) for b in self.ser.readline()])
		"""
		if PYTHON3:
			data = ''.join([chr(b) for b in self.ser.readline()])
		else:
			data = self.ser.readline()
		
		#data = self.ser.readline()
		
		if (SHOW_TRAFFIC): self.put('<<< "%s"' % str_hex(data))
		return data
		
	
	def run(self):
		self.running = True
		
		line = ''
		while(self.running):
			l = self.readline()
			if l is not None:
				#self.serial_handle_frame(l)
				self.put('<<< "%s"' % l)
			else:
				# Idle
				time.sleep(0.01)
		
	
	# Monitor specific functions
	
	def wait_for_monitor(self):
		"Wait for prompt on beginning, activate console"
		"""
		self.put('Waiting for monitor...')
		while True:
			if (self.ser.in_waiting == 0):
				time.sleep(0.5)
			else:
				s = self.readline()
				if (s.startswith('CR to activate')):
					break
		"""
		
		self.put('Activating console... (Issue command "SIO" on monitor if nothing happens)')
		while True:
			self.write('\n')
			
			# Get banner
			s = self.readline()
			if (s == '>'): break
		
		#self.put('Waiting for prompt...')
		#self.wait_for_prompt()
		self.put('Connected to monitor!')
		time.sleep(0.1)
		
	def wait_for_prompt(self):
		
		self.ser.timeout = SERIAL_TIMEOUT_SHORT
		while True:
			s = self.readline()
			if s.endswith('>'):
				break
			self.write('\n')
		
		self.ser.timeout = SERIAL_TIMEOUT
		return True
	
	def ver(self):
		self.write('ver\n')
		s = self.readline()
		return s.strip()
	
	def port_in(self, p):
		ps = '%02X' % p
		
		self.write('in %s\n' % ps)
		s = self.readline()
		if (len(s) < 7):
			self.put('Erroneous answer (too short)')
			return 0xff
		
		# Strip prompt
		if (s[0] == '>'): s = s[1:]
		
		# Check answer
		if (s[0:2] != ps):
			self.put('Erroneous answer: Answer "%s" does not match port "%s".' % (s, ps))
			return 0xff
		
		vs = s[5:7]
		vi = int(vs, 16)
		return vi
		
	
	def port_out(self, p, v):
		self.write('out %02x %02x\n' % (p, v))
		s = self.readline()
		#put('out result: "%s"' % s)
	
	def ints(self):
		# Let interrupts happen
		#self.write('ei\ndi\n')
		self.write('ints\n')
		self.readline()
		self.write('\n')
		self.readline()
		self.readline()
		
	
	def peek(self, a, l=1):
		self.ser.timeout = SERIAL_TIMEOUT
		self.write('peek %04x %02x\n' % (a, l))
		s = self.readline()
		#put(s)
		while (len(s) > 0) and (s[0] == '>'): s = s[1:]
		
		if len(s) < 7:
			raise Exception('Erroneous peek result (too short)')
		
		
		
		if (s[0:4] != ('%04X'%a)):
			raise Exception('Erroneous peek return address (expected 0x%04X, got "%s")' % (a, s[0:4]))
		
		data = s.strip()[5:]
		#put(data)
		if len(data) != l*2:
			raise Exception('Erroneus peek result length (expected %d, got %d)' % (l*2, len(data)))
		
		r = []
		for i in range(l):
			v = int(data[i*2:i*2+2], 16)
			r.append(v)
		
		return r
	
	def dump(self, a, l=16):
		put(hexdump(self.peek(a, l), a))
		
	def poke(self, a, data):
		if PYTHON3:
			self.write('poke %04x %s\n' % (a, ''.join(['%02x'%b for b in data]) ))
		else:
			self.write('poke %04x %s\n' % (a, ''.join(['%02x'%ord(b) for b in data]) ))
		
		#time.sleep(0.1)
		#r = self.readline()
		#if r == '>': return True
		#put('Returned "%s"!' % str(r))
		#return False
		#self.wait_for_prompt()
		#return True
		return True
		
	
	def call(self, a):
		self.write('call %04x\n' % a)
		#self.readline()
	
	def upload(self, filename, dest_addr, src_addr=0):
		# Upload an app
		self.put('Uploading app "%s"...' % filename)
		#dest_addr = 0xc800	# LOC_DATA
		#app_addr_start = dest_addr	#0xd6ad	# Location of vgldk_init()
		#app_addr_start = 0xd6ad	# Just for testing (known entry point)
		
		with open(filename, 'rb') as h:
			data = [ b for b in h.read() ]
		
		# Extract portion
		app_data = data[src_addr:]
		
		l = len(app_data)
		self.put('App size: %d bytes' % l)
		
		chunk_size = UPLOAD_BUFFER_SIZE
		t_start = time.time()
		
		o = 0
		addr = dest_addr
		while (o < l):
			
			chunk = app_data[o:o+chunk_size]
			self.put('Pushing %.1f%% (%d / %d bytes)...' % (o*100.0/l, o, l))
			#hex = ''.join(['%02x'%b for b in chunk])
			#r = 'poke %04x %s' % (addr, hex)
			#put(r)
			
			
			success = False
			while not success:
				
				#self.write('\n')
				#self.wait_for_prompt()
				#time.sleep(0.1)
				
				success = self.poke(addr, chunk)
				self.wait_for_prompt()
				if not success:
					continue
				
				#time.sleep(0.1)
				
				success = False
				try:
					check = self.peek(addr, len(chunk))
					self.wait_for_prompt()
					if check == chunk:
						success = True
						break
					else:
						put('Expected	%s\nbut got 	%s!' % (str(chunk), str(check)))
						#@TODO: Increment as much as possible
				except Exception:
					put('Error')
				
				
			addr += chunk_size
			o += chunk_size
		
		t_end = time.time()
		t_delta = t_end - t_start
		self.put('Upload OK (%d bytes in %d seconds = %.2f B/s).' % (l, t_delta, l/t_delta))
	

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
	
	
	parser = OptionParser(
		description='Upload a binary file to a computer running monitor.c',
		usage='Usage: %prog [options] filename'
	)
	parser.add_option('-p', '--port', dest='port', default=port,
		action='store', type='string', help='Serial port to use (%s)' % port)
	parser.add_option('-b', '--baud', dest='baud', default=baud,
		action='store', type='int', help='Baud rate (%d)' % baud)
	parser.add_option('-d', '--dest', dest='dest', default=dest,
		action='store', type='int', help='Destination address (0x%04X)' % dest)
	#parser.add_option('-f', '--file',
	#	action='store', type='string', dest='filename', help='Binary file to send')
	opt, args = parser.parse_args()
	
	port = opt.port
	baud = opt.baud
	dest = opt.dest
	#filename = opt.filename
	# Positional
	if len(args) > 0:
		filename = args[0]
	
	if filename is None:
		put('No filename given')
		parser.print_help()
		sys.exit(1)
	
	put('Uploading "%s" on port "%s" using %d baud to memory 0x%04X...' % (filename, port, baud, dest))
	comp = Hardware(port=port, baud=baud)
	comp.open()
	
	# Wait for the monitor logo/prompt
	comp.wait_for_monitor()
	
	# Disable interrupts
	comp.write('di\n')
	comp.wait_for_prompt()
	
	# Upload app binary
	comp.upload(filename, dest)
	
	# Call to function
	put('Calling 0x%04X...' % dest)
	# Call right away (having serial I/O as stdio)
	#comp.call(dest)
	
	# Switch from serial I/O back to REAL I/O and call
	comp.write('sio;call %04x\n' % dest)
	
	# Flush some residual output
	for i in range(10):
		s = comp.readline()
	
	put('End.')
	
