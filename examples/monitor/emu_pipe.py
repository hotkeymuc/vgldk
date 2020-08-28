#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Emu companion

This script is used to start MAME and trap some port accesses (requires patched MAME).

It can then be used to re-direct port-accesses via serial to the real hardware.
Goal is to find out more about undocumented hardware (e.g. audio chip)

2020-05-27 Bernhard "HotKey" Slawik
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


# MAME
MAME_CALL = '/z/data/_code/_c/mame.git/mame64'
#MAME_CALL = '../../../mame.git/mame64'
#MAME_EMUSYS = 'gl6000sl'
MAME_EMUSYS = 'gl4000'

MAME_CMD = MAME_CALL
MAME_CMD += ' ' + MAME_EMUSYS
#MAME_CMD += ' -nodebug'
#MAME_CMD += ' -rompath ./roms'
#MAME_CMD += ' -cart out/cpm.cart.bin'
MAME_CMD += ' -cart out/monitor.cart.16kb.bin'
MAME_CMD += ' -window'
MAME_CMD += ' -nomax'
MAME_CMD += ' -nofilter'
MAME_CMD += ' -sleep'
MAME_CMD += ' -volume -24'
MAME_CMD += ' -skip_gameinfo'
MAME_CMD += ' -speed 2.00'


# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	# Software Serial currrently only supports 9600 baud (fixed)
SERIAL_TIMEOUT = 0.1


# General
SHOW_TRAFFIC = not True


def put(txt):
	print('host: %s' % txt)

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




class MAME:
	"Host implementation when using MAME"
	
	def __init__(self):
		self.is_open = False
		self.running = False
		
		self.proc = None
		
		self.on_port_out = None
		self.on_port_in = None
	
	def __del__(self):
		self.mame_close()
	
	def put(self, s):
		put('MAME	%s' % s)
	
	def open(self):
		self.put('Starting MAME...')
		start_new_thread(self.mame_open, ())
		while(not self.is_open):
			time.sleep(0.5)
	
	def close(self):
		self.mame_close()
		self.running = False
	
	def mame_open(self):
		self.is_open = False
		
		cmd = MAME_CMD
		
		#self.proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		### https://stackoverflow.com/questions/1606795/catching-stdout-in-realtime-from-subprocess
		#self.proc = subprocess.Popen('stdbuf -o0 '+ cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		self.proc = subprocess.Popen(
			#'stdbuf -i2 -o2 '+ cmd,
			cmd,
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			shell=True,
			#close_fds=False,	# Use close_fds=False on unix, close_fds=True on linux
			#bufsize=2
			bufsize=0	# 0=unbuffered, 1=line buffered, n=buffer ~n bytes
			)
		
		"""
		h = self.proc.stdout
		
		for l in iter(h.readline, b''):	# iter() is needed!
			if len(l) > 0:
				put('"%s"' % (l.strip()))
				self.on_mame_line(l)
		
		self.proc.communicate()	# Close PIPE
		
		#put('Exit with returncode="%s"' % (str(p.returncode)))
		return self.proc.returncode
		"""
		self.put('Return code so far: %s' % str(self.proc.returncode))
		self.is_open = True
		
		self.run()
		
	
	def mame_close(self):
		self.proc.communicate()	# Close PIPE
	
	def run(self):
		"Main loop"
		self.running = True
		
		#h = self.proc.stdout
		#put(str(dir(h)))
		
		while self.running:
			
			r = self.proc.poll()
			if (r is None):
				# Still running
				
				s = self.readline()
				s = ''.join([chr(b) for b in s])
				self.handle_data(s)
				
			else:
				# Process ended!
				self.put('Poll returned: %s' % str(r))
				self.running = False
			
			time.sleep(0.01)	# Throttle a little
		
		"""
		for l in iter(h.readline, b''):	# iter() is needed!
			if len(l) > 0:
				put('"%s"' % (l.strip()))
				self.on_mame_line(l)
		
		self.proc.communicate()	# Close PIPE
		"""
		
		#put('Exit with returncode="%s"' % (str(p.returncode)))
		#return self.proc.returncode
	
	def readline(self):
		#s = self.proc.stdout.read(64)
		s = self.proc.stdout.readline()
		if (SHOW_TRAFFIC): self.put('<<< "%s"' % s)
		return s
	
	def write(self, s):
		if (SHOW_TRAFFIC): self.put('>>> "%s"' % s)
		#self.proc.stdin.write(bytes('%2X\n' % b, 'ascii'))
		#self.proc.stdin.write(s)
		try:
			self.proc.stdin.write(bytes(s, 'ascii'))
		except BrokenPipeError:
			self.running = False
	
	def handle_data(self, s):
		#put('Need to handle data "%s"' % s)
		
		if (len(s) < 7): return
		
		if s[0] == 'W':
			ps = s[2:4]
			pi = int(ps, 16)
			vs = s[5:7]
			vi = int(vs, 16)
			if self.on_port_out is not None:
				self.on_port_out(pi, vi)
			else:
				self.put('port_out callback not set!')
			
		elif s[0] == 'R':
			ps = s[2:4]
			pi = int(ps, 16)
			if self.on_port_in is not None:
				vi = self.on_port_in(pi)
			else:
				self.put('port_in callback not set!')
				vi = 0xff
			self.write('%02X\n' % vi)
		

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
				put('Serial got "%s"' % l)
			else:
				# Idle
				time.sleep(0.01)
		
	
	# Specific functions
	def port_in(self, p):
		ps = '%02X' % p
		
		self.write('in %s\n' % ps)
		s = self.readline()
		if (len(s) < 7):
			put('Erroneous answer')
			return 0xff
		
		# Strip prompt
		if (s[0] == '>'): s = s[1:]
		
		# Check answer
		if (s[0:2] != ps):
			put('Erroneous answer: Answer "%s" does not match port "%s".' % (s, ps))
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
		
	


def show_help():
	put(__doc__)
	put('emu_pipe.py ...')

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
	
	
	mame = MAME()
	
	def my_port_out(p, v):
		put('MAME -> HW\tport 0x%02X := 0x%02X' % (p, v))
		comp.port_out(p, v)
		
		#comp.ints()	# Let interrupts happen
		
	def my_port_in(p):
		v = comp.port_in(p)
		put('MAME <- HW\tport 0x%02X == 0x%02X' % (p, v))
		
		#comp.ints()	# Let interrupts happen
		
		return v
	
	# If MAME needs to output to a port: Redirect to Hardware
	#mame.on_port_out = comp.port_out
	mame.on_port_out = my_port_out
	# If MAME wants to know the state of a port: Ask the hardware
	#mame.on_port_in = comp.port_in
	mame.on_port_in = my_port_in
	
	mame.open()
	while mame.running:
		time.sleep(1)
	
	
	put('End.')
