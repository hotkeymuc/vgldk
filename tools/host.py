#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Serial companion for VGLDK

This script communicates with a VTech Genius Leader notebook
using the host.h functions.

There are multiple methods of communication:
	* Using softserial (9600/19200 baud)
	* Using HEX strings in STDIN/STDOUT to talk to a modified version of MAME (port 0x13 mapped to STDIN/STDOUT)

Driver	---- bytes ---->	Protocol	---- frames ---->	Host.handle_frame()
Driver	<--- bytes -----	Protocol	<--- frames -----	Host.reply_frame()


2019-11-15 Bernhard "HotKey" Slawik
"""

#from thread import start_new_thread	# Python 2
from _thread import start_new_thread	# Python 3

import time
import sys	# for sys.argv and sys.exit(status)
import getopt
import os

import serial
import subprocess


PYTHON3 = True

# MAME
#MAME_CMD = '../../../mame.git/mame64'
MAME_CMD = '/z/data/_code/_c/mame.git/mame64'

# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 9600	#19200	# 9600 or 19200
SHOW_TRAFFIC = not True
SHOW_TRAFFIC_BYTES = not True


"""
# Arduino MEGA
#SERIAL_PORT = '/dev/ttyACM0'
SERIAL_PORT = '/dev/ttyACM1'
SERIAL_BAUD = 115200	# If connected to an Arduino+BusBuddy which buffers the serial stream
"""


TOP_OF_STACK = 0x0e07	# for debugging: 0x0e07 for ZORK, "None" for unlimited

# Maximum frame sizes (sending more might corrupt RAM on VGL and lead to severe glitches)
MAX_LINE = 255
MAX_DATA = 128	# for HEX serial: 128 
MAX_SEND = 120	# for HEX serial: < 128-header

def put(txt):
	print('host: %s' % txt)


class Host:
	"""The VGL CP/M BDOS talks to this counter part as a virtual disk and/or monitor"""
	
	def __init__(self, driver, protocol):
		self.driver = driver
		self.protocol = protocol
		
		self.running = False
		self.is_open = False
		
	
	def __del__(self):
		self.driver.close()
	
	def open(self):
		# Wire up
		self.protocol.read_byte = self.driver.read_byte
		self.protocol.write_byte = self.driver.write_byte
		self.protocol.finish_frame = self.driver.finish_frame
		
		self.driver.open()
		self.is_open = True
	
	def run(self):
		# Run (blocking)
		self.running = True
		while self.running:
			self.update()
	
	def update(self):
		# Keep driver active
		if not self.driver.update():
			self.running = False
			return
		
		# Ask protocol for new frame (blocking)
		try:
			data = self.protocol.receive_frame()
		except Exception as e:
			put('Receiving frames failed (%s / %s). Quitting...' % (e.__class__.__name__, str(e)))
			self.running = False
			return
		
		# Handle frame
		self.handle_frame(data)
	
	def reply_frame(self, data):
		if SHOW_TRAFFIC: put('>> (%d) %s' % (len(data), ' '.join([ '0x%02X'%b for b in data])))
		self.protocol.send_frame(data)
	
	def handle_frame(self, data):
		"Handle one complete FRAME of data"
		
		#put('frame: %s' % (str(data)))
		if SHOW_TRAFFIC: put('<< frame: (%d) [%s]' % (len(data), ' '.join([ '0x%02X'%b for b in data])))
		if (len(data) == 0): return False
		
		realm = data[0]
		if realm == ord('L'):
			# Load
			addr = (data[1] << 8) + data[2]
			put('L: Load from addr %04X...' % addr)
			l_max = len(self.bin_data)
			l = l_max - addr
			if (l < 0): l = 0
			elif (l > MAX_SEND): l = MAX_SEND
			
			#put('Replying with %d bytes' % l)
			
			# Throttle...
			if REPLY_THROTTLE is not None: time.sleep(REPLY_THROTTLE)
			
			data = []
			for i in range(l):
				#b = ord(self.bin_data[addr + i])	# Python 2
				b = self.bin_data[addr + i]
				data.append(b)
			self.reply_frame(data)
		
		elif realm == ord('D'):
			# Debug stuff
			reg_a = data[1]
			reg_b = data[2]
			reg_c = data[3]
			reg_d = data[4]
			reg_e = data[5]
			reg_h = data[6]
			reg_l = data[7]
			
			reg_s = data[8]
			reg_p = data[9]
			stack_count = data[10]
			stack_data = data[11:]	#11+stack_count]
			
			reg_sp = reg_s * 256 + reg_p
			put('D: Debug:	SP=%04X	A=%02X	B=%02X C=%02X	D=%02X E=%02X	H=%02X L=%02X' % (reg_sp, reg_a, reg_b, reg_c, reg_d, reg_e, reg_h, reg_l))
			for i in range(stack_count):
				sp = reg_sp + 2 * i
				#a = stack_data[i*2] * 256 + stack_data[i*2+1]	# Physical
				a = stack_data[i*2 + 1] * 256 + stack_data[i*2 + 0]	# Readable addresses
				put('	#%02d %04X: %04X' % (i, sp, a))
				if (sp == TOP_OF_STACK):
					# Reached top of stack
					break
			
		
		else:
			put('Unknown realm 0x%02X / "%s"' % (realm, chr(realm)))
		
	


class Protocol:
	"""Abstract protocol"""
	def __init(self, read_byte=None, write_byte=None, finish_frame=None):
		self.read_byte = read_byte
		self.write_byte = write_byte
		self.finish_frame = finish_frame
	
	def receive_frame(self):
		"""Receive new frame from driver (blocking)"""
		return b''
	
	def send_frame(self, data):
		"""Send new frame to driver"""
		if callable(self.finish_frame): self.finish_frame()
		pass


class Protocol_binary(Protocol):
	"""Simple binary protocol"""
	
	def receive_frame(self):
		"""Receive new frame from driver (blocking)"""
		# Length
		l = self.read_byte()
		
		# Data
		data = []
		for i in range(l):
			data.append(self.read_byte())
		
		return data
	
	def send_frame(self, data):
		"""Send new frame to driver"""
		# Length
		self.write_byte(len(data))
		
		# Data
		for b in data:
			self.write_byte(b)
		
		if callable(self.finish_frame): self.finish_frame()
	


# Binary_safe protocol
def bdos_host_checksum(data):
	"""
	check = 0x55
	for b in data:
		check ^= b
	"""
	
	"""
	check = BDOS_HOST_CHECK_INIT
	for b in data:
		check = ((check << 1) & 0xff) ^ b	# Shift and XOR
	"""
	
	check = BDOS_HOST_CHECK_INIT
	for b in data:
		check = ((check << 1) & 0xffff) ^ b	# Shift and XOR
	
	return check


class Protocol_binary_safe(Protocol):
	"""Binary protocol with checksum and retransmission"""
	def receive_frame(self):
		
		while True:	# Re-transmission loop
			
			# Receive length
			l = self.read_byte()
			
			# Receive data
			data = []
			for i in range(l):
				data.append(self.read_byte())
			
			# Receive checksum
			check_received = self.read_byte()
			check_received = check_received * 256 + self.read_byte()	# 16 bit
			
			# Calculate checksum
			check = bdos_host_checksum(data)
			
			# Check
			if check_received == check: break	# Checksums OK!
			
			# Not OK: Send NAK
			put('RX: Checksum mismatch rx=0x%02X != 0x%02X! Sending NAK...' % (check_received, check))
			self.write_byte(BDOS_HOST_STATUS_NAK)	# Anything but 0xAA
			
			if callable(self.finish_frame): self.finish_frame()
		#
		
		# Send ACK
		self.write_byte(BDOS_HOST_STATUS_ACK)
		
		if callable(self.finish_frame): self.finish_frame()
		
		return list(data)	# Convert bytes to list
	
	
	def send_frame(self, data):
		"Reply data inside a frame"
		
		#if SHOW_TRAFFIC: put('>>> Frame: %d bytes' % len(data))
		
		# Calculate checksum
		check = bdos_host_checksum(data)
		
		#frame = [ len(data) ] + data + [ check ]	# 8 bit checksum
		frame = [ len(data) ] + data + [ check >> 8, check & 0xff ]	# 16 bit checksum
		
		while True:
			
			# Send frame
			d = [0]*16 + frame	# Include pre-padding in same call (works great!)
			for b in d: self.write_byte(b)
			if callable(self.finish_frame): self.finish_frame()
			#self.serial_write(bytes( [0] * 16 ))	# Post-padding to flush
			
			# Wait for ACK/NAK
			"""
			put('Waiting for ACK/NAK...')
			timeout = 50
			while (self.ser.in_waiting == 0) and (timeout > 0):
				time.sleep(0.01)
				#self.serial_write(bytes( [0] * 2 ))	# Flush
				timeout -= 1
			
			# Check for timeout
			if timeout <= 0:
				put('TX: Timeout waiting for ACK/NAK! Re-transmitting...')
				#time.sleep(0.1)
				continue
				#put('TX: Timeout waiting for ACK/NAK! Stopping TX!')
				#return
			
			#@FIXME: I have no clue why it always keeps sending 0x28!
			#while r not in (0xaa, 0x99):
			#r = self.ser.read(1)[0]
			"""
			r = self.read_byte()
			
			# Check for ACK
			if r == BDOS_HOST_STATUS_ACK: break	# ACK received!
			
			if r != BDOS_HOST_STATUS_NAK:
				put('!! TX: Invalid ACK/NAK response!!! (0x%02X / %d)!' % (r, r))
				time.sleep(2)
				#put('TX: Invalid response!!!! Stopping TX...')
				#return
			
			# No ACK. Repeat!
			put('TX: NAK received (0x%02X)! Re-transmitting...' % r)
		#
		
		#put('TX: ACK received.')
		
	


class Protocol_hex(Protocol):
	"""Hex protocol with sync, checksum and retransmission"""
	
	def readline(self):
		# Receive one line of text
		data = []
		while True:
			b = self.read_byte()
			if b in [ 10, 13 ]:
				break
			#@TODO: Filter out non-ascii / non-hex characters?
			data.append(b)
		
		#@TODO: Try/catch if corrupted bytes
		line = bytes(data).decode('ascii')
	
	
	def receive_frame(self):
		"""Receive new frame from driver (blocking)"""
		
		line = self.readline()
		lline = len(line)
		
		# Skip garbage until sync
		while ((lline > 0) and (line[0] != 'U')):
			line = line[1:]
			lline -= 1
		
		# Skip sync padding
		while ((lline > 0) and (line[0] == 'U')):
			line = line[1:]
			lline -= 1
		
		if (lline < (2+2)):
			put('Ignoring short answer: %d < 4' % lline)
			return False
		
		# Get length
		try:
			l = int(line[0:2], 16)
		except ValueError:
			return False
		#put('Given length: %d' % l)
		
		if (l != (lline - 4)//2):
			put('Length mismatch: given=%d, actual=%d' % (l, (lline-4)//2))
			return False
		
		data = []
		check_actual = l
		for i in range(l):
			b = int(line[2 + i*2:2 + i*2 + 2], 16)
			data.append(b)
			check_actual ^= b
		
		check_given = int(line[-2:], 16)
		if (check_given != check_actual):
			put('Checksum mismatch: given=0x%02X, actual=0x%02X' % (check_given, check_actual))
			return False
		
		# Acknowledge
		ca = b'%02X' % check_actual
		self.write_byte(ca[0])
		self.write_byte(ca[1])
		#put('Serial frame has been received OK! l=%d, data="%s"' % (len(data), str_hex(''.join(chr(b) for b in data) ) ))
		
		return data
	
	
	def send_frame(self, data):
		"""Send new frame to driver"""
		
		frame = []
		l = len(data)
		frame.append(l)
		
		check = l
		for i in range(l):
			b = data[i]
			frame.append(b)
			check ^= b
		
		frame.append(check)
		
		line = 'UU' + ''.join(('%02X' % b) for b in frame) + '\n'
		acked = False
		check_str = '%02X' % check
		
		# Send without checksum check
		#self.write(line)
		
		# Check checksum, resend if wrong/not rcvd
		
		while not acked:
			#self.write(line)
			for c in line:
				self.write_byte(ord(c))
			
			# Wait for checksum answer!
			answer = self.readline()
			
			# Strip "U"
			while ((answer is not None) and (len(answer) > 0) and (answer[0] == 'U')):
				answer = answer[1:]
			#put('Compare %02X vs "%s"' % (check, answer))
			if (answer == check_str):
				acked = True
			else:
				put('ACK wrong (expected "%s", got "%s"). Re-sending...' % (check_str, answer))
				acked = False
				time.sleep(0.05)
		#
		
		if callable(self.finish_frame): self.finish_frame()
	



### Drivers (i.e. hardware abstraction)
class Driver:
	"""Abstract driver, e.g. serial or MAME"""
	def open(self):
		pass
	def close(self):
		pass
	def update(self):
		return True	# While running
	def read_byte(self):
		#if SHOW_TRAFFIC_BYTES: put('< 0x%02X' % b)
		#return b
		pass
	def write_byte(self, b):
		#if SHOW_TRAFFIC_BYTES: put('> 0x%02X' % b)
		pass
	def finish_frame(self):
		pass


import mame
class Driver_MAME(Driver):
	"""MAME Emulator driver (using stdin/stdout for communication)"""
	
	def __init__(self, emusys='gl4000', rompath=None, cart_file=None, buffer_size=3):
		Driver.__init__(self)
		
		mame.SHOW_TRAFFIC = SHOW_TRAFFIC
		self.mame = mame.MAME(rompath=rompath, emusys=emusys, cart=cart_file, buffer_size=buffer_size)
	
	def open(self):
		put('Starting MAME...')
		self.mame.open(run=False)	# Do not "run", because this requests data from emulator and will block/freeze
		put('MAME is open.')
	
	def close(self):
		self.mame.close()
	
	def read_byte(self):
		while True:
			# Do not hang
			self.mame.mame_keep_alive()
			
			s = self.mame.read()
			#put('read="%s"' % str(s))
			try:
				v = int(s, 16)
				if SHOW_TRAFFIC_BYTES: put('< 0x%02X' % v)
				return v
			except ValueError:
				put('Got non-hex data from stdout: %s' % str_hex(s))
			#
			time.sleep(0.01)
		#
	
	def write_byte(self, b):
		if SHOW_TRAFFIC_BYTES: put('> 0x%02X' % b)
		self.mame.write(bytes('%2X\n' % b, 'ascii'))
	
	def finish_frame(self):
		# Flush STDIN (or else the last byte(s) might not get through to MAME)
		for i in range(3):
			self.write_byte(0)	#@FIXME: Do not do this! Might destroy sync!
		pass
	
	def update(self):
		
		self.mame.mame_keep_alive()
		
		#if (r is None):
		if self.mame.is_open:
			# Still running
			
			# Flush while idle?
			#for i in range(4): self.write_byte(0)	#@FIXME: Do not do this! Might destroy sync!
			
			return True	# True = driver is running
			
		else:
			# Process ended!
			#put('MAME ended! Poll returned: %s' % str(r))
			put('MAME ended!')
			return False	# False = driver stopped
		
		#put('Exit with returncode="%s"' % (str(p.returncode)))
		#return self.proc.returncode
	

class Driver_serial(Driver):
	"""Serial driver"""
	def __init__(self, port=SERIAL_PORT, baud=SERIAL_BAUD, stopbits=1):
		Driver.__init__(self)
		
		# Serial state
		self.port = port
		self.baud = baud
		self.stopbits = stopbits
		self.ser = None
		self.is_open = False
	
	def open(self):
		self.ser = None
		put('Opening "%s" at %d baud...' % (self.port, self.baud))
		
		try:
			self.ser = serial.Serial(
				port=self.port,
				baudrate=self.baud,
				bytesize=8,
				parity='N',
				stopbits=self.stopbits,
				timeout=3,
				xonxoff=0,
				rtscts=0
			)
			
			put('Open!')
			self.is_open = True
			return True
		except serial.serialutil.SerialException as e:
			#except e:
			#put('Error opening serial device: %s' % str(e))
			self.is_open = False
			self.close()
			raise e
			return False
	
	def close(self):
		if (self.ser is not None):
			self.ser.close()
			self.ser = None
	
	def read_byte(self):
		# Block until data is there
		while (self.ser.in_waiting == 0): time.sleep(0.01)
		r = self.ser.read(1)[0]
		
		if SHOW_TRAFFIC_BYTES: put('< 0x%02X' % r)
		return r
	
	def write_byte(self, b):
		if self.ser is None:
			put('! Cannot send data, because serial is not open!')
			return False
		
		if SHOW_TRAFFIC_BYTES: put('> 0x%02X' % b)
		
		self.ser.write(bytes([ b ]))
		#self.ser.flush()	#?
	
	def finish_frame(self):
		pass
	
	def update(self):
		if (self.ser.in_waiting == 0): time.sleep(0.01)
		return True
	



### Main

def show_help():
	put(__doc__)
	put('host.py ...')

if __name__ == '__main__':
	#put(__doc__)
	
	argv = sys.argv[1:]
	try:
		opts, args = getopt.getopt(argv, 'pbhni:', ['port=','baud=','input=','no-result'])
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
	
	# Chose a driver
	driver = Driver_serial(port=port, baud=baud, stopbits=2)	# More stopbits = more time to process?
	#driver = Driver_MAME(cart_file='./out/cpm_cart.bin')
	
	# Chose a protocol
	#protocol = Protocol_binary()	# For MAME
	protocol = Protocol_binary_safe()	# For serial. Recommended.
	#protocol = Protocol_hex()	# Old text-only protocol
	
	# Start host
	host = Host(driver=driver, protocol=protocol)
	host.open()
	
	if not host.is_open:
		put('Connection could not be opened. Aborting.')
		sys.exit(4)
	
	host.run()
	
	put('End.')

