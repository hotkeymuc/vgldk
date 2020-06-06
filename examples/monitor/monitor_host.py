#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Monitor Host
============

This script communicates with a VTech Genius Leader running the "monitor.c" cartridge.

You must enter "serial I/O" mode using command "sio".

2020-06-01 Bernhard "HotKey" Slawik
"""

import sys
PYTHON3 = (sys.version_info.major == 3)

import serial
import subprocess


if PYTHON3:
	from _thread import start_new_thread	# Python 3
else:
	from thread import start_new_thread	# Python 2


import time
import sys	# for sys.argv and sys.exit(status)
import getopt
import os


# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	# Software Serial currrently only supports 9600 baud (fixed)
SERIAL_TIMEOUT = 0.1


# General
SHOW_TRAFFIC = True


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
		#if (c < 0x20) and (c < 128):
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
		#start_new_thread(self.serial_open, ())
		#while(not self.is_open):
		#	time.sleep(0.2)
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
				#inter_byte_timeout=1.0
				)
			self.put('Open!')
			self.is_open = True
		except serial.serialutil.SerialException as e:
			#except e:
			self.put('Error opening serial device: %s' % str(e))
			self.is_open = False
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
			#for c in s:
			#	self.ser.write(bytes([ord(c)]))	# Python3
			#	self.ser.flush()	# Wait until all data is written
			#	#self.ser.read(self.ser.in_waiting)
		else:
			self.ser.write(s)	# Python2
		
		self.ser.flush()	# Wait until all data is written
		
	
	def readline(self, timeout=SERIAL_TIMEOUT):
		"""
		data = ''
		while (self.ser.in_waiting > 0):
			data += ''.join([chr(b) for b in self.ser.readline()])
		"""
		data = ''.join([chr(b) for b in self.ser.readline()])
		
		#data = self.ser.readline()
		
		if (SHOW_TRAFFIC): self.put('<<< "%s"' % str_hex(data))
		return data
		
		"""
		r = ''
		start_time = time.time()
		end_time = start_time + timeout
		data = ''
		while (time.time() < end_time):
			if (self.ser.in_waiting > 0):
				#data = self.ser.read()
				data = self.ser.readline()
				if (SHOW_TRAFFIC): self.put('<<< "%s"' % str_hex(data))
				
				return data
				#end_time = time.time() + timeout	# Extend timeout
			else:
				time.sleep(0.05)
		self.put('Timeout!')
		return None
		"""
	
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
		
		self.put('Activating console...')
		while True:
			self.write('\n')
			
			# Get banner
			s = self.readline()
			if (s == '>'): break
		
		#self.put('Waiting for prompt...')
		#self.wait_for_prompt()
		self.put('Connected to monitor!')
		
	def wait_for_prompt(self):
		while True:
			s = self.readline()
			if s == '>':
				return True
	
	def ver(self):
		comp.write('ver\n')
		s = comp.readline()
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
		#comp.write('ei\ndi\n')
		comp.write('ints\n')
		comp.readline()
		comp.write('\n')
		comp.readline()
		comp.readline()
		
	
	def peek(self, a, l=1):
		comp.write('peek %04x %02x\n' % (a, l))
		s = comp.readline()
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
		put(hexdump(comp.peek(a, l), a))
		
	def poke(self, a, data):
		comp.write('poke %04x %s\n' % (a, ''.join(['%02x'%b for b in data]) ))
		comp.readline()
	
	def call(self, a):
		comp.write('call %04x\n' % a)
		#comp.readline()
	
	def upload(self, filename, app_addr):
		# Upload an app
		self.put('Uploading app "%s"...' % filename)
		#app_addr = 0xc800	# LOC_DATA
		app_addr_start = app_addr	#0xd6ad	# Location of vgldk_init()
		#app_addr_start = 0xd6ad	# Just for testing (known entry point)
		
		with open(filename, 'rb') as h:
			data = h.read()
		
		app_data = data[app_addr:]
		l = len(app_data)
		self.put('App size: %d bytes' % l)
		
		chunk_size = 56	# Keep it < monitor's MAX_INPUT-6/2
		o = 0
		addr = app_addr
		while (o < l):
			chunk = app_data[o:o+chunk_size]
			self.put('Pushing %d / %d bytes...' % (o, l))
			#hex = ''.join(['%02x'%b for b in chunk])
			#r = 'poke %04x %s' % (addr, hex)
			#put(r)
			self.poke(addr, chunk)
			
			addr += chunk_size
			o += chunk_size
		
		self.put('Upload OK.')
	

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
	
	
	comp = Hardware(port=port, baud=baud)
	comp.open()
	
	comp.wait_for_monitor()
	
	
	#comp.upload('../hello/out/hello.app.c800.bin', 0xc800)
	#put('Calling...')
	#comp.call(0xc800)
	#for i in range(10):
	#	s = comp.readline()
	
	
	#comp.write('ver\n')
	#s = comp.readline()
	#put(s)
	#put('Version: "%s"' % comp.ver())
	#put(hexdump([ ord(b) for b in 'Hello world! This is hexdump']))
	
	
	### Scan all banks and dump memory
	#put(hexdump(comp.peek(0x8000, 16), 0xd000))
	
	#comp.dump(0x8000, 64)	# 55 AA 59 45 ...
	
	#comp.dump(0x0000, 32)
	for i in range(0x40, 0x100):
		put('Bank 0x%02X:' % i)
		comp.port_out(0x50, i)
		comp.dump(0x0000, 128)
		
		#comp.port_out(0x51, i)
		#comp.dump(0x4000, 128)
		
		#comp.port_out(0x52, i)	# Cart!
		#comp.dump(0x8000, 128)
		
		#comp.port_out(0x53, i)	# RAM!
		#comp.dump(0xc000, 128)
		
		#comp.port_out(0x54, i)	# RAM2!
		#comp.dump(0xf000, 128)
		
		#comp.port_out(0x55, i)	# Magic...
	
	#comp.poke(0xd000, [1,2,3,4,5,6,7])
	#comp.dump(0xd000)
	
	
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
	
	
	
	put('End.')
