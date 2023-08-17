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
#import getopt
from optparse import OptionParser
import os


# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	#19200	# Software Serial currrently only supports 9600 baud (and 19200 on some models)
SERIAL_TIMEOUT = 0.05	#0.1

DEST_DEFAULT = 0xc800

MONITOR_PROMPT = '>'

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
		if (c >= 0x20) and (c < 128):
			r += chr(c)
		else:
			r += ' [0x%02X] ' % c
	return r



class Monitor:
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
		put('Monitor	%s' % s)
	
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
				inter_byte_timeout=0.1
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
		
		#if (SHOW_TRAFFIC): self.put('>>> "%s"' % str_hex(s))
		if (SHOW_TRAFFIC): self.put('>>> %s' % str_hex(s))
		
		if PYTHON3:
			#self.ser.write(bytes(s, 'ascii'))	# Python3
			# s must be bytes
			if type(s) is bytes:
				self.ser.write(s)
			else:
				self.ser.write(bytes(s, 'ascii'))	# Convert to bytes
		else:
			self.ser.write(s)	# Python2
		
		#self.ser.flush()	# Wait until all data is written
		
	def read(self):
		b = self.ser.read()
		
		if (b is None) or (len(b) == 0):
			# Just ignore empty inputs (b'' means "nothing received")
			return None
		
		if (SHOW_TRAFFIC): self.put('<<< "%s"' % str_hex(b))
		return b
		
	def readline(self, timeout=SERIAL_TIMEOUT):
		"""
		data = ''
		while (self.ser.in_waiting > 0):
			data += ''.join([chr(b) for b in self.ser.readline()])
		"""
		data = '\n'
		while data == '\n':
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
		
		self.put('Activating console... (Enter "SIO" at the prompt if nothing happens)')
		while True:
			
			# Get banner
			s = self.readline()
			if (s.strip().endswith(MONITOR_PROMPT)):
				break
			
			#self.write('\n')
			self.write(bytes('\n', 'ascii'))
		
		#self.wait_for_prompt()
		#s = '\n'
		#while len(s) > 0: s = self.readline()
		
		self.put('Connected to monitor!')
		
	def wait_for_prompt(self):
		if SHOW_TRAFFIC: self.put('Waiting for prompt...')
		
		retries_before_pressing_enter = 2
		retries = retries_before_pressing_enter
		while True:
			s = self.readline()
			if (s.strip().endswith(MONITOR_PROMPT)):
				return True
			retries -= 1
			if retries <= 0:
				self.write('\n')
				retries = retries_before_pressing_enter
	
	### Monitor commands
	def ver(self):
		self.write('ver\n')
		self.wait_for_prompt()
		s = self.readline()
		return s.strip()
	
	def port_in(self, p):
		ps = '%02X' % p
		
		self.write('in %s\n' % ps)
		self.wait_for_prompt()
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
		self.wait_for_prompt()
		#s = self.readline()
		#put('out result: "%s"' % s)
	
	def ints(self):
		# Let interrupts happen
		#self.write('ei\ndi\n')
		self.write('ints\n')
		#self.readline()
		self.wait_for_prompt()
		self.write('\n')
		self.readline()
		self.readline()
		
	
	def peek(self, a, l=1):
		
		while True:
			s = ''
			while s in ['', '\n']:
				self.write('peek %04x %02x\n' % (a, l))
				s = self.readline()
			
			self.wait_for_prompt()
			#put(s)
			#while (len(s) > 0) and (s[0] == '>'): s = s[1:]
			
			if len(s) < 7:
				#raise Exception('Erroneous peek result (too short): "%s"' % s)
				put('Erroneous peek result (too short): "%s"' % s)
				continue
			
			if (s[0:4] != ('%04X'%a)):
				#raise Exception('Erroneous peek return address (expected 0x%04X, got "%s")' % (a, s[0:4]))
				put('Erroneous peek return address (expected 0x%04X, got "%s")' % (a, s[0:4]))
				continue
			
			data = s.strip()[5:]
			#put(data)
			if len(data) != l*2:
				put('Erroneus peek result length (expected %d, got %d)' % (l*2, len(data)))
				continue
			
			# OK! End loop
			break
		#
		
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
		
		self.wait_for_prompt()
	
	def call(self, a):
		self.write('call %04x\n' % a)
		#self.readline()
	
	def upload(self, filename=None, data=None, dest_addr=0x0000, src_addr=0, skip_zeros=False, skip_trailing=True, max_size=None, chunk_size=56, verify=False):
		# Upload an app
		self.put('Uploading "%s" to 0x%04X...' % (filename, dest_addr))
		#dest_addr = 0xc800	# LOC_DATA
		#app_addr_start = dest_addr	#0xd6ad	# Location of vgldk_init()
		#app_addr_start = 0xd6ad	# Just for testing (known entry point)
		
		if filename is None and data is None:
			raise Exception('Must specify either data or filename')
		
		if filename is not None:
			with open(filename, 'rb') as h:
				data = h.read()
		
		self.put('File size: %d bytes' % len(data))
		
		# Extract portion
		app_data = data[src_addr:]
		
		if max_size is not None:
			app_data = app_data[:max_size]
		self.put('Sliced data size: %d bytes' % len(app_data))
		
		if skip_trailing:
			len_old = len(data)
			i = len_old
			while(i > 0):
				if data[i-1] not in [0xff, 0x00]: break
				i -= 1
			data = data[:i]
			self.put('Stripped trailing FF/00: %d - %d = %d bytes' % (len_old, len_old-len(data), len(data)))
		
		l = len(app_data)
		put('Upload size: %d bytes' % l)
		
		#chunk_size = 56	# Keep it < monitor's MAX_INPUT-6/2
		o = 0
		addr = dest_addr
		while (o < l):
			chunk = app_data[o:o+chunk_size]
			if skip_zeros:
				for c in chunk:
					if c != 0: break
				else:
					o += len(chunk)
					addr += len(chunk)
					self.put('Skipping %d zeros...' % len(chunk))
					continue
			self.put('Pushing %d / %d bytes (%d%%)...' % (o, l, o*100.0/l))
			#hex = ''.join(['%02x'%b for b in chunk])
			#r = 'poke %04x %s' % (addr, hex)
			#put(r)
			self.poke(addr, chunk)
			
			if verify:
				check = bytes(self.peek(addr, len(chunk)))
				if check != chunk:
					put('sent    : %s' % chunk)
					put('received: %s' % check)
					
					v = 0
					while check[v] == chunk[v]:
						v += 1
					put('Verify failed. Re-sending from byte %d...' % v)
					addr += v
					o += v
					continue
			
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

def test_bankswitch(comp):
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
	
	# Named args
	port = opt.port
	baud = opt.baud
	dest = opt.dest
	#filename = opt.filename
	
	# Positional args
	if len(args) > 0:
		filename = args[0]
	
	
	### Upload file to memory
	if filename is None:
		put('No filename given')
		parser.print_help()
		sys.exit(1)
	
	# Initialize monitor link
	comp = Monitor(port=port, baud=baud)
	comp.open()
	
	comp.wait_for_monitor()
	
	# Perform tests
	#test_bankswitch(comp)
	
	#comp.write('ver\n')
	#s = comp.readline()
	#put(s)
	#put('Version: "%s"' % comp.ver())
	#put(hexdump([ ord(b) for b in 'Hello world! This is hexdump']))
	
	
	put('Uploading "%s" on port "%s" using %d baud to memory 0x%04X...' % (filename, port, baud, dest))
	comp.upload(filename, dest)
	
	# Call right away (having serial I/O as stdio)
	#put('Calling 0x%04X...' % dest)
	#comp.call(dest)
	
	# Switch from serial I/O back to REAL I/O and call
	comp.write('sio;call %04x\n' % dest)
	
	# Flush some residual output
	for i in range(10):
		s = comp.readline()
	
	
	put('End.')
