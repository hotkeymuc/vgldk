#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Serial companion for the VGL CP/M BDOS

This script communicates with a VTech Genius Leader notebook.
There are multiple methods of communication:
	* Using softserial (9600/19200 baud)
	* Using HEX strings in STDIN/STDOUT to talk to a modified version of MAME (port 0x13 mapped to STDIN/STDOUT)

2019-11-15 Bernhard "HotKey" Slawik
"""

import serial

import subprocess


PYTHON3 = True

#from thread import start_new_thread	# Python 2
from _thread import start_new_thread	# Python 3

import time
import sys	# for sys.argv and sys.exit(status)
import getopt
import os

# MAME
#MAME_CMD = '../../../mame.git/mame64'
MAME_CMD = '/z/data/_code/_c/mame.git/mame64'

# FTDI
SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_BAUD = 19200	#9600
SHOW_TRAFFIC = not True


"""
# Arduino MEGA
#SERIAL_PORT = '/dev/ttyACM0'
SERIAL_PORT = '/dev/ttyACM1'
SERIAL_BAUD = 115200	# If connected to an Arduino+BusBuddy which buffers the serial stream
"""


TOP_OF_STACK = 0x0e07	# 0x0e07 for ZORK, "None" for unlimited

# Maximum frame sizes (sending more might corrupt RAM on VGL and lead to severe glitches)
MAX_LINE = 255
MAX_DATA = 128	# for HEX serial: 128 
MAX_SEND = 120	# for HEX serial: < 128-header

REPLY_THROTTLE = 0.05	# We need to wait a bit before sending a reply, because VGL does not have interrupt based serial and needs to be ready for data or it'll simply miss it
DMA_THROTTLE = 0.1

DEFAULT_DAT_FILENAME = None

#DEFAULT_BIN_FILENAME = 'out/app_busBuddy2.app.bin'
#DEFAULT_BIN_FILENAME = 'out/app_parallelBuddy.app.bin'
#DEFAULT_BIN_FILENAME = 'out/app_hello.app.bin'
#DEFAULT_BIN_FILENAME = 'out/app_test.app.bin'
#DEFAULT_BIN_FILENAME = 'programs/STDCPM22/DUMP.COM'
#DEFAULT_BIN_FILENAME = 'programs/STDCPM22/GENMOD.COM'
#DEFAULT_BIN_FILENAME = 'programs/STDCPM22/SYSGEN.COM'
#DEFAULT_BIN_FILENAME = 'programs/VG04/ASM.COM'
#DEFAULT_BIN_FILENAME = 'programs/VG04/CBASIC.COM'
#DATA_PATH = 'programs/VG04'
#DEFAULT_BIN_FILENAME = 'programs/CBASIC2/CBAS2.COM'
#DATA_PATH = 'programs/CBASIC2'
#DEFAULT_BIN_FILENAME = 'programs/XDIR.COM'
#DEFAULT_BIN_FILENAME = 'programs/CATCHUM/CATCHUM.COM'	# > 16kb will break the system (bank switching?)
#DATA_PATH = 'programs/CATCHUM'
#DEFAULT_BIN_FILENAME = 'programs/BBCBASIC/BBCBASIC.COM'
#DATA_PATH = 'programs/BBCBASIC'
#DEFAULT_BIN_FILENAME = 'programs/SARGON.COM'
#DATA_PATH = 'programs'
#DEFAULT_BIN_FILENAME = 'programs/WS30/WS.COM'
#DATA_PATH = 'programs/WS30'

DEFAULT_BIN_FILENAME = 'programs/ZORK123/ZORK1.COM'

DATA_PATH = 'programs/ZORK123'

DATA_PATHS = [
	# first entry = A: = default
	#'programs',
	
	#'programs/ASCOM22',
	#'programs/BBCBASIC',
	#'programs/CATCHUM',
	#'programs/CBASIC2',
	#'programs/LADDER',
	#'programs/STDCPM22',
	#'programs/TEX',
	#'programs/TP300',
	#'programs/VG04',
	#'programs/WRDMASTR',
	#'programs/WS30',
	'programs/ZORK123',
	
]

#DEFAULT_BIN_FILENAME = 'test/out/test.com'
#DATA_PATH = 'test/out'


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


def my_ceil(x, y):
	# int + ceil division
	return x//y + (1 if ((x % y) > 0) else 0)


class Host:
	"The VGL CP/M BDOS talks to this counter part as a virtual disk and/or monitor"
	
	def __init__(self, throttle=None, paths=DATA_PATHS):
		self.throttle = throttle
		self.running = False
		self.is_open = False
		
		self.paths = paths
		
		# Open bin file
		with open(DEFAULT_BIN_FILENAME, 'rb') as h:
			self.bin_data = h.read()
		
		self.dat_data = None
		
		self.dir_list = None
		self.dir_i = 0
	
	def run(self):
		pass
	
	def reply_frame(self, data):
		pass
	
	
	def handle_frame(self, data):
		"This is binary proof, checksummed data"
		
		#put('frame: %s' % (str(data)))
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
			if (REPLY_THROTTLE >= 0): time.sleep(REPLY_THROTTLE)
			
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
			
		
		elif realm == ord('F'):
			# FCB based file stuff
			num = data[1]
			addr = data[2] * 256 + data[3]
			
			#fcb = data[4:]
			fcb = data[4:4+36]
			
			fcb_dr = fcb[0]
			fcb_name = ''.join(chr(c) for c in fcb[1:1+8])
			fcb_typ = ''.join(chr(c) for c in fcb[1+8:1+8+3])
			fcb_ex = fcb[12]
			fcb_s1 = fcb[13]
			fcb_s2 = fcb[14]
			fcb_rc = fcb[15]
			fcb_d = fcb[16:32]	# Allocation
			fcb_cr = fcb[32]
			fcb_r0 = fcb[33]
			fcb_r1 = fcb[34]
			fcb_r2 = fcb[35]
			
			
			if chr(0) in fcb_name: fcb_name = fcb_name[:fcb_name.index(chr(0))]	# Remove 0 bytes
			filename = (fcb_name.strip() + '.' + fcb_typ.strip())
			put('! FCB function #%d to FCB @ %04X: file drive=%d "%s.%s": ex=%02X, s1=%02X, s2=%02X, rc=%02X, cr=%02X' % (num, addr, fcb_dr, str_hex(fcb_name), str_hex(fcb_typ), fcb_ex, fcb_s1, fcb_s2, fcb_rc, fcb_cr))
			
			#@TODO: When using sfirst: dr = '?' (0x3f) means: Show metainfo / labels etc.
			if fcb_dr == 0x3f: fcb_dr = 0
			if fcb_dr > len(self.paths):
				put('Unknown FCB DR=%d - using default drive' % fcb_dr)
				path = self.paths[0]
			else:
				path = self.paths[fcb_dr]
			
			if (num == 15): # Open file
				#full_filename = os.path.join(DATA_PATH, filename)
				full_filename = os.path.join(path, filename)
				put('F: Open file %s:%s ("%s")...' % (chr(ord('A') + fcb_dr), filename, full_filename))
				
				# Try lower case
				if not os.path.isfile(full_filename):
					full_filename = os.path.join(path, filename.lower())
				
				fcb_s2 = fcb[14] = fcb_s2 | 0x80	# Seems to be set, no matter if file found or not
				
				if os.path.isfile(full_filename):
					# File was found, OK
					with open(full_filename, 'rb') as h:
						self.dat_data = h.read()
					
					file_size = len(self.dat_data)
					file_ofs = 0
					
					# Alter FCB
					#@FIXME: use myceil()!!!
					fcb_ex = fcb[12] = 0	#((file_size - file_ofs) % 524288) // 16384	# Current extent
					fcb_s1 = fcb[13] = 2	# ? Number of extents?
					fcb_s2 = fcb[14] = ((file_size - file_ofs) // 524288) | 0x80	# Extent high byte; 0x80 is an "open" flag?
					fcb_rc = fcb[15] = min(0x80, ((file_size - file_ofs) // 128) )	# Record count in current extent, 0x80=continues...
					#fcb_d = ...	# Image of the second half of the directory entry, containing the file's allocation (which disc blocks it owns).
					fcb_d[:] = fcb[16:32] = [0x10, 0x00, 0x11, 0x00, 0x12, 0x00, 0x13, 0x00, 0x14, 0x00, 0x15, 0x00, 0x16, 0x00, 0x17, 0x00]
					fcb_cr = fcb[32] = 0	#(file_ofs % 16384) // 128	# Current record within extent
					fcb_r0 = fcb[33] = 0
					fcb_r1 = fcb[34] = 0
					fcb_r2 = fcb[35] = 0
					
					time.sleep(0.05)
					self.reply_frame([0x00] + fcb)
					return
					
				else:
					# File was not found
					put('File "%s" was NOT found!' % full_filename)
					time.sleep(0.05)
					self.reply_frame([0xff])
					return
			
			if (num == 16): # Close file
				put('F: Close file "%s"' % filename)
				self.dat_data = None
				return
			
			if (num == 17): # Search first
				# rewind!
				path_list = sorted(os.listdir(path))
				self.dir_list = [f for f in path_list if os.path.isfile(os.path.join(path, f))]
				put('Complete dir list of "%s": %s' % (path, str(self.dir_list)))
				self.dir_i = 0
			
			if (num == 17) or (num == 18): # Search next (and first)
				
				if (self.dir_list is None):
					self.reply_frame([0xff])
					return
				
				if (self.dir_i >= len(self.dir_list)):
					self.dir_list = None
					self.reply_frame([0xff])
					return
				
				#filename = 'test.txt'
				filename = self.dir_list[self.dir_i]
				filename_local = '%s/%s' % (path, filename)
				put('Returning dir_i=%d, filename="%s"' % (self.dir_i, filename))
				self.dir_i += 1
				
				# Get file stats
				if os.path.isfile(filename_local): file_size = os.path.getsize(filename_local)
				else: file_size = 0
				
				# Prepare CP/M filename
				filename = filename.upper()
				try:
					name, ext = filename.split('.')
				except ValueError:	# No "." in name
					name = filename
					ext = ''
				
				# Limit length to 8+3
				name = name[:8]
				ext = ext[:3]
				
				# Fill out result FCB
				# e.g. CCPRUN.COM (27.254 bytes)
				# 00A0|00 43 43 50 52 55 4E 20 20 43 4F 4D 01 00 00 55 |.CCPRUN  COM...U
				# 00B0|10 00 11 00 12 00 13 00 14 00 15 00 16 00 00 00 |................
				# 00C0|00 43 50 4D                                     |.CPM            
				
				fcb[0] = 0	# fcb_dr	# DRive; 0=default, 1=A, 0=DR = Drive:	^0x20 = deleted file
				
				# Clear filename + ext
				fcb[1:1+8+3] = [ 0x00 ] * 11
				# Fill filename + ext
				fcb[1:1+len(name)] = [ ord(c) for c in name ]
				fcb[1+8:1+8+len(ext)] = [ ord(c) for c in ext]
				
				#@TODO: Fill out more data!
				ex = my_ceil(file_size, 16384)
				rc = my_ceil((file_size % 16384), 128)
				fcb[12] = ex	# ex = Current Extent = (file pointer % 524288) / 16384
				fcb[13] = 0	# s1 = reserved
				fcb[14] = 0	# s2 = reserved
				fcb[15] = rc	# rc = RC (Record Count for extent "ex"); 0x80 = big file, more entries follow
				fcb[16:16+16] = [0,0] * 8	# d = Allocation of current file
				#for i in range(rc):
				#	# Fake some allocation
				#	fcb[16+i*2] = 0x10 + i
				#	fcb[16+i*2+1] = 0
				
				# Bytes 32-36 are not part of 32-byte directory entry
				#fcb[32] =	# cr = Current Record to r/w = (file pointer % 16384)  / 128
				#fcb[33] =	# r0 = Random access record number, lowest byte
				#fcb[34] =	# r1 = middle byte of rn
				#fcb[35] =	# r2 = highest byte of rn (for CP/M 3)
				
				fcb = fcb[:32]	# Only 32 bytes for directories!
				
				time.sleep(0.05)
				self.reply_frame([0x00] + fcb)	# Result = 0-3 = OK, 0xff = error
				return
			
			
			ofs = 0
			if (num == 20):	# Read sequencial
				#put('Read sequencial...')
				ofs = (fcb_ex * 16384) + (fcb_cr * 128)
			
			if (num == 33):	# Read rand
				#put('Read random...')
				fcb_rr = fcb_r0 + (fcb_r1 << 8) + ((fcb_r2 & 1) << 16)
				ofs = fcb_rr * 128	# Convert "record num" to file offset
				
			if (num == 20) or (num == 33):	# Read
				if self.dat_data is None:
					put('Read without data / closed file!')
					self.reply_frame([0xff])
					return
				
				l_all = len(self.dat_data)
				
				put('Read data from file %s:"%s" at ofs=%d / %d' % (chr(ord('A') + fcb_dr), filename, ofs, l_all))
				
				data = []
				
				max_read = 128
				#max_read = 64	# Limit for testing
				
				l = min(max_read, l_all - ofs)	# Read at max 128 bytes
				
				#put('Preparing %d bytes of payload' % l)
				is_eof = (l == 0)
				for i in range(l):
					if (ofs + i < l_all):
						#b = ord(self.dat_data[ofs + i])	# Python 2
						b = self.dat_data[ofs + i]	# Python 3
						data.append(b)
					else:
						is_eof = True
						break
						#data.append(26)	# ESC/EOF
				
				# Send in smaller chunks
				#time.sleep(0.05)
				o = 0
				l2 = 32	# 32 works
				#l2 = 64
				while (l2 > 0):
					if self.throttle is not None: time.sleep(self.throttle)
					put('Sending chunk o=%d / %d' % (o, len(data)))
					d = data[o:o+l2]
					l2 = len(d)
					if (l2 == 0): break	# Only send zero-length frame on EOF
					
					self.reply_frame([0x20] + d)	# Prepend dummy byte, so we never send zero packages
					o += l2
					
				#
				if is_eof:
					put('Sending EOF')
					self.reply_frame([0x1A])
				put('DMA sent.')
			#
		else:
			put('Unknown realm 0x%02X / "%s"' % (realm, chr(realm)))
		
	

class Host_MAME(Host):
	"Host implementation when using MAME"
	
	#@FIXME: Use tools/mame.py! It can run MAME and redirect output
	
	def __init__(self, emusys='gl4000', rompath='./roms', cart_file=None, buffer_size=3, *args, **kwargs):
		
		#@FIXME: Use tools/mame.py to handle the MAME communication
		self.proc = None
		
		Host.__init__(self, *args, **kwargs)
		#Host.__init__(self, throttle=DMA_THROTTLE)
		
		self.emusys = emusys
		self.rompath = rompath
		self.cart_file = cart_file
		
		self.buffer_size = buffer_size	# HEX HEX newline = 3 bytes
	
	def __del__(self):
		self.mame_close()
	
	def open(self):
		put('Starting MAME...')
		start_new_thread(self.mame_open, ())
		while(not self.is_open):
			time.sleep(0.5)
	
	def mame_open(self):
		self.is_open = False
		
		cmd = MAME_CMD
		cmd += ' -nodebug'
		if self.rompath is not None: cmd += ' -rompath %s' % self.rompath
		cmd += ' %s' % self.emusys
		if self.cart_file is not None: cmd += ' -cart %s' % self.cart_file
		cmd += ' -window'
		cmd += ' -nomax'
		cmd += ' -nofilter'
		cmd += ' -sleep'
		cmd += ' -volume -24'
		#cmd += ' -skip_disclaimer'	# When set: process ends immediately :-(
		cmd += ' -skip_gameinfo'
		cmd += ' -speed %.2f' % 1.0	#2.0
		cmd += ' -nomouse'
		
		#self.proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		### https://stackoverflow.com/questions/1606795/catching-stdout-in-realtime-from-subprocess
		#self.proc = subprocess.Popen('stdbuf -o0 '+ cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		self.proc = subprocess.Popen(
			#cmd,
			'stdbuf -i%d -o%d %s' % (self.buffer_size, self.buffer_size, cmd),	# use with bufsize=0
			
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			shell=True,
			
			#close_fds=False,	# Use close_fds=False on unix, close_fds=True on linux
			
			#bufsize=self.buffer_size
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
		put('Return code so far: %s' % str(self.proc.returncode))
		self.is_open = True
		
	
	def mame_close(self):
		if self.proc is not None:
			self.proc.communicate()	# Close PIPE
	
	def run(self):
		self.mame_run()
	
	def mame_run(self):
		"Main loop"
		self.running = True
		
		#h = self.proc.stdout
		#put(str(dir(h)))
		
		while self.running:
			
			r = self.proc.poll()
			if (r is None):
				# Still running
				
				l = self.mame_read_byte()
				data = []
				for i in range(l):
					data.append(self.mame_read_byte())
				
				if (SHOW_TRAFFIC):
					#put('<< Got %d bytes' % l)
					put('<< Got %d bytes: %s' % (l, ' '.join(['%02X' % b for b in data]) ))
				self.handle_frame(data)
				
			else:
				# Process ended!
				put('Poll returned: %s' % str(r))
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
	
	def mame_read_byte(self):
		#s = self.proc.stdout.read(2)
		s = self.proc.stdout.readline()	# Read 2-digit HEX with trailing newline
		#put('read="%s"' % str(s))
		try:
			v = int(s, 16)
		except ValueError:
			put('Got non-hex data from stdout: %s' % str_hex(s))
			v = 0	# Error
		
		return v
	
	def mame_send_byte(self, b):
		self.proc.stdin.write(bytes('%2X\n' % b, 'ascii'))
		
	def reply_frame(self, data):
		self.mame_reply_frame(data)
		
	def mame_reply_frame(self, data):
		if (SHOW_TRAFFIC):
			#put('>> Replying %d bytes' % len(data))
			put('>> Replying %d bytes: (%02X) %s' % (len(data), len(data), ' '.join(['%02X' % b for b in data]) ))
		
		"""
		self.mame_send_byte(len(data))
		for c in data:
			self.mame_send_byte(c)
		"""
		
		self.proc.stdin.write(b'%02X\n' % len(data))	# Length
		#time.sleep(.1)
		
		for c in data:
			self.proc.stdin.write(b'%02X\n' % c)	# Data
			#time.sleep(.1)
		
		# Flush
		for i in range(4):
			#self.proc.stdin.write(b'xx\n')	# This is OK, because it gets ignored by pc2000.cpp stdin parser
			self.proc.stdin.write(b'00\n')	#@FIXME: Do not do this! Destroys sync!
			#time.sleep(.01)
		
	

class Host_Serial_hex(Host):
	"""Using serial protocol, with synch and checksum"""
	def __init__(self, port=SERIAL_PORT, baud=SERIAL_BAUD, *args, **kwargs):
		Host.__init__(self, throttle=DMA_THROTTLE, *args, **kwargs)
		
		# Serial state
		self.port = port
		self.baud = baud
		self.ser = None
	
	def __del__(self):
		self.serial_close()
	
	def open(self):
		self.serial_open()
	
	def serial_open(self):
		self.ser = None
		port = self.port
		put('Opening "%s"...' % port)
		
		try:
			self.ser = serial.Serial(port=port, baudrate=self.baud, bytesize=8, parity='N', stopbits=1, timeout=3, xonxoff=0, rtscts=0)
			put('Open!')
			self.is_open = True
			return True
		except serial.serialutil.SerialException as e:
			#except e:
			put('Error opening serial device: %s' % str(e))
			self.is_open = False
			return False
	
	def serial_close(self):
		self.running = False
		time.sleep(0.2)
		
		if (self.ser is not None):
			self.ser.close()
			self.ser = None
	
	def serial_write(self, data):
		if self.ser is None:
			put('! Cannot send data, because serial is not open!')
			return False
		
		if (SHOW_TRAFFIC): put('>>> "%s"' % str_hex(data.strip()))
		self.ser.write(data)
		#self.ser.flush()	#?
	
	def serial_readline(self, timeout=1):
		r = ''
		start_time = time.time()
		end_time = start_time + timeout
		data = ''
		while (time.time() < end_time):
			if (self.ser.in_waiting > 0):
				data = self.ser.readline().strip()
				if (SHOW_TRAFFIC): put('<<< "%s"' % str_hex(data))
				
				# Weird... I get a lot of 0xFF....
				#while (len(data) > 0) and (ord(data[0]) == 0xff):
				#	data = data[1:]
				#put('<<< "%s"' % str_hex(data))
				
				return data
				#end_time = time.time() + timeout	# Extend timeout
			else:
				time.sleep(0.05)
		put('Timeout!')
		return None
	
	def run(self):
		self.serial_run()
	
	def serial_run(self):
		"Main loop"
		self.running = True
		
		put('Ready.')
		line = ''
		while(self.running):
			if (self.ser.in_waiting > 0):
				l = self.serial_readline()
				if l is not None:
					self.serial_handle_frame(l)
			else:
				# Idle
				time.sleep(0.01)
	
	def reply_frame(self, data):
		"Reply data inside a frame"
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
			self.write(line)
			
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
	
	def serial_handle_frame(self, line):
		"Handle incoming raw data, check for validity, acknowledge and pass on to parser"
		#put('Parsing serial frame...')
		
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
		#put('Ack...')
		# Throttle...
		#time.sleep(0.05)
		self.write('%02X' % (check_actual))
		
		#put('Serial frame has been received OK! l=%d, data="%s"' % (len(data), str_hex(''.join(chr(b) for b in data) ) ))
		#put('Serial frame has been received OK! l=%d, data="%s"' % (len(data), str_hex(''.join(chr(b) for b in data) ) ))
		self.handle_frame(data)
		
	

class Host_Serial_binary(Host):
	"""Using just binary frames (error prone)"""
	def __init__(self, port=SERIAL_PORT, baud=SERIAL_BAUD, *args, **kwargs):
		Host.__init__(self, throttle=DMA_THROTTLE, *args, **kwargs)
		
		# Serial state
		self.port = port
		self.baud = baud
		self.ser = None
	
	def __del__(self):
		self.serial_close()
	
	def open(self):
		self.serial_open()
	
	def serial_open(self):
		self.ser = None
		port = self.port
		put('Opening "%s"...' % port)
		
		try:
			self.ser = serial.Serial(port=port, baudrate=self.baud, bytesize=8, parity='N', stopbits=1, timeout=3, xonxoff=0, rtscts=0)
			put('Open!')
			self.is_open = True
			return True
		except serial.serialutil.SerialException as e:
			#except e:
			put('Error opening serial device: %s' % str(e))
			self.is_open = False
			return False
	
	def serial_close(self):
		self.running = False
		time.sleep(0.2)
		
		if (self.ser is not None):
			self.ser.close()
			self.ser = None
	
	def serial_write(self, data):
		if self.ser is None:
			put('! Cannot send data, because serial is not open!')
			return False
		
		if (SHOW_TRAFFIC): put('>>> "%s"' % str_hex(data.strip()))
		self.ser.write(data)
		#self.ser.flush()	#?
	
	def run(self):
		self.serial_run()
	
	def serial_run(self):
		"Main loop"
		self.running = True
		
		put('Ready.')
		line = ''
		while(self.running):
			if (self.ser.in_waiting > 0):
				f = self.serial_read_frame()
				if f is not None:
					self.serial_handle_frame(f)
			else:
				# Idle
				time.sleep(0.01)
	
	def reply_frame(self, data):
		"Reply data inside a frame"
		
		
		frame = [ len(data) ] + data
		
		# Prepend and pad with zero bytes (for synching, they get ignored as zero-length frames)
		#frame = [ 0 ] * 8 + [ len(data) ] + data + [ 0 ] * 8
		
		# Pre-padding (to give some time and sync)
		self.ser.write(bytes( [0] * 4 ))
		
		# Send without checksum check
		self.ser.write(bytes(frame))
		
		# Post padding (to give some time)
		self.ser.write(bytes( [0] * 4 ))
		
	
	def serial_read_frame(self):
		while (self.ser.in_waiting == 0): time.sleep(0.01)
		l = self.ser.read(1)[0]
		while (self.ser.in_waiting < l): time.sleep(0.01)
		data = self.ser.read(l)
		return list(data)	# Convert bytes to list
	
	def serial_handle_frame(self, f):
		"Handle incoming raw data"
		
		self.handle_frame(f)
	


class Host_Serial_binary_safe(Host):
	"""Using just binary frames with checksum and retransmission"""
	def __init__(self, port=SERIAL_PORT, baud=SERIAL_BAUD, *args, **kwargs):
		Host.__init__(self, throttle=DMA_THROTTLE, *args, **kwargs)
		
		# Serial state
		self.port = port
		self.baud = baud
		self.ser = None
	
	def __del__(self):
		self.serial_close()
	
	def open(self):
		self.serial_open()
	
	def serial_open(self):
		self.ser = None
		port = self.port
		put('Opening "%s"...' % port)
		
		try:
			self.ser = serial.Serial(port=port, baudrate=self.baud, bytesize=8, parity='N', stopbits=1, timeout=3, xonxoff=0, rtscts=0)
			put('Open!')
			self.is_open = True
			return True
		except serial.serialutil.SerialException as e:
			#except e:
			put('Error opening serial device: %s' % str(e))
			self.is_open = False
			return False
	
	def serial_close(self):
		self.running = False
		time.sleep(0.2)
		
		if (self.ser is not None):
			self.ser.close()
			self.ser = None
	
	def serial_write(self, data):
		if self.ser is None:
			put('! Cannot send data, because serial is not open!')
			return False
		
		if (SHOW_TRAFFIC): put('>>> "%s"' % str_hex(data.strip()))
		self.ser.write(data)
		#self.ser.flush()	#?
	
	def run(self):
		self.serial_run()
	
	def serial_run(self):
		"Main loop"
		self.running = True
		
		put('Ready.')
		line = ''
		while(self.running):
			if (self.ser.in_waiting > 0):
				f = self.serial_read_frame()
				if f is not None:
					self.serial_handle_frame(f)
			else:
				# Idle
				time.sleep(0.01)
				
				# Flush
				#self.ser.write(bytes([ 0x00 ]))	# 0x00 is ignored as a frame start
				
	
	def reply_frame(self, data):
		"Reply data inside a frame"
		
		put('>>> %d bytes...' % len(data))
		
		# Calculate checksum
		check = 0x55
		for b in data:
			check ^= b
		
		frame = [ len(data) ] + data + [ check ]
		
		#self.ser.write(bytes( [0] * 4 ))	# Pre-padding to sync
		
		while True:
			#while (self.ser.in_waiting > 0): self.ser.read()	# Flush inputs
			time.sleep(0.02)
			
			# Send frame
			#self.ser.write(bytes( [0] * 16 ))	# Pre-padding to sync
			self.ser.write(bytes(frame))
			#self.ser.write(bytes( [0] * 16 ))	# Post-padding to flush
			
			# Wait for ACK/NACK
			#put('Waiting for ACK...')
			timeout = 50
			while (self.ser.in_waiting == 0) and (timeout > 0):
				time.sleep(0.01)
				#self.ser.write(bytes( [0] * 2 ))	# Flush
				timeout -= 1
			
			# Check for timeout
			if timeout <= 0:
				put('TX: Timeout waiting for ACK/NACK! Re-transmitting...')
				time.sleep(0.1)
				continue
				#put('TX: Timeout waiting for ACK/NACK! Stopping TX!')
				#return
			
			#@FIXME: I have no clue why it always keeps sending 0x28!
			#while r not in (0xaa, 0x99):
			#r = self.ser.read(1)[0]
			
			r = self.ser.read()
			put('TX: received ACK/NACK [ %s ]' % (' '.join(['0x%02X'%b for b in r ])))
			r = r[-1]
			
			# Check for ACK
			if r == 0xAA: break	# ACK received!
			
			#if r != 0x99:
			#	put('TX: Invalid response!!!! Stopping TX...')
			#	return
			
			# No ACK. Repeat!
			put('TX: NACK received (0x%02X)!' % r)
			time.sleep(0.1)
			#self.ser.write(bytes( [0] * 4 ))	# Flush
			while (self.ser.in_waiting > 0): self.ser.read()	# Flush inputs
			put('TX: Re-transmitting...')
		#
		put('TX: OK, ACK received.')
		
	
	def serial_read_frame(self):
		
		while True:
			# Receive length
			while (self.ser.in_waiting == 0): time.sleep(0.01)
			l = self.ser.read(1)[0]
			
			# Receive data
			while (self.ser.in_waiting < l): time.sleep(0.01)
			data = self.ser.read(l)
			
			# Receive checksum
			while (self.ser.in_waiting == 0): time.sleep(0.01)
			check_received = self.ser.read(1)[0]
			
			# Calculate checksum
			check = 0x55
			for b in data:
				check ^= b
			
			# Check
			if check_received == check: break	# Checksums OK!
			
			# Not OK: Send NACK
			put('RX: Checksum mismatch rx=0x%02X != 0x%02X! Sending NACK...' % (check_received, check))
			time.sleep(0.02)
			#self.ser.write(bytes( [0] * 16 ))	# Pre-padding to sync
			self.ser.write(bytes([ 0x99 ]))	# Anything but 0xAA
			#self.ser.write(bytes( [0] * 16 ))	# Post-padding to flush
		#
		
		# Send ACK
		put('RX: OK, sending ACK')
		time.sleep(0.02)
		#self.ser.write(bytes( [0] * 4 ))	# Pre-padding to sync
		self.ser.write(bytes([ 0xAA ]))
		#self.ser.write(bytes( [0] * 16 ))	# Post-padding to flush
		#time.sleep(0.05)	# Throttle
		
		return list(data)	# Convert bytes to list
	
	def serial_handle_frame(self, f):
		"Handle incoming raw data"
		
		self.handle_frame(f)
	


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
	
	bin_filename = DEFAULT_BIN_FILENAME
	
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
		elif opt in ('-i', '--input'):
			bin_filename = arg
	
	#comp = Host_Serial_hex(port=port, baud=baud)
	#comp = Host_Serial_binary(port=port, baud=baud)
	comp = Host_Serial_binary_safe(port=port, baud=baud)
	#comp = Host_MAME()
	
	comp.open()
	
	if not comp.is_open:
		put('Connection could not be opened. Aborting.')
		sys.exit(4)
	
	#put('Loading binary file "%s"...' % (bin_filename))
	#comp.upload(bin_filename)
	comp.run()
	
	
	put('End.')
