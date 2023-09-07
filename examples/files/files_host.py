#!/usr/bin/python3
"""
fs_host Host Implementation
===========================

This is a host (extends vgldk/tools/host.py) to handle commands
from fs_host.h in order to access files stored on a remote host.
The commands are 1:1 based on the previously used parabuddy and fs_parabuddy/fs_busbuddy.

"Host" is a more general concept than busbuddy/parabuddy. Instead of
having the filesystem pick which host protocol to use, host.h does all of that.
The actual hardware driver on the device, as well as the low-level protocol
can be chosen at compile time without toughing the actual program.

In contrast to "bdos_host.py" this is more "modern" and does not
require to send a FCB structure back and forth. So there is no
"current record" etc., just simple wrappers for fopen, fclose, ...

2023-09-07 Bernhard "HotKey" Slawik
"""

import time
import sys	# for sys.argv and sys.exit(status)
import getopt

# Import VGLDK tools
import sys
sys.path.append('../../tools')
from host import *

def put(t):
	print(t)

BASE_PATH = './testfiles'


FS_HOST_COMMAND_RETURN_OK = 0x06	#ACK
FS_HOST_COMMAND_RETURN_BYTE = 0x11
FS_HOST_COMMAND_RETURN_WORD = 0x12
FS_HOST_COMMAND_RETURN_ASCIIZ = 0x13
FS_HOST_COMMAND_RETURN_DATA = 0x14
FS_HOST_COMMAND_RETURN_NACK = 0x15	# NAK

# Functionality
FS_HOST_COMMAND_END_BOOTLOADER = 0x1A

FS_HOST_COMMAND_PING = 0xE0
FS_HOST_COMMAND_PING_HOST = 0xE1

#FS_HOST_COMMAND_SD_INIT = 0x20
#FS_HOST_COMMAND_SD_EXISTS = 0x21
#FS_HOST_COMMAND_SD_OPEN = 0x22
#FS_HOST_COMMAND_SD_CLOSE = 0x23
#FS_HOST_COMMAND_SD_REMOVE = 0x4
#FS_HOST_COMMAND_SD_MKDIR = 0x25
#FS_HOST_COMMAND_SD_RMDIR = 0x26

FS_HOST_COMMAND_FILE_OPENDIR = 0x30
FS_HOST_COMMAND_FILE_READDIR = 0x31
FS_HOST_COMMAND_FILE_CLOSEDIR = 0x32

FS_HOST_COMMAND_FILE_OPEN = 0x40
FS_HOST_COMMAND_FILE_CLOSE = 0x41
FS_HOST_COMMAND_FILE_EOF = 0x42
FS_HOST_COMMAND_FILE_READ = 0x43
FS_HOST_COMMAND_FILE_WRITE = 0x44
#FS_HOST_COMMAND_FILE_SEEK = 0x45
#FS_HOST_COMMAND_FILE_SIZE = 0x46
FS_HOST_COMMAND_FILE_AVAILABLE = 0x47

FS_HOST_ERROR_UNKNOWN = 0x01
FS_HOST_ERROR_LENGTH = 0x02

FS_HOST_FILE_READ = 0
FS_HOST_FILE_WRITE = 1

FS_HOST_NO_HANDLE = 0xff

MOUNTS = None

class Files_Host(Host):
	"""host.h talks to this counter part as a virtual disk and/or monitor"""
	
	def __init__(self, driver, protocol):	#, mounts=MOUNTS):
		Host.__init__(self, driver=driver, protocol=protocol)
		
		#self.mounts = mounts
		
		self.handles = []
		#self.dir_list = None
		#self.dir_i = 0
	
	def handle_frame(self, data):
		"Handle one complete FRAME of data"
		
		#put('frame: %s' % (str(data)))
		if SHOW_TRAFFIC: put('<< frame: (%d) [%s]' % (len(data), ' '.join([ '0x%02X'%b for b in data])))
		if (len(data) == 0): return False
		
		#realm = data[0]
		#self.reply_frame([0x00] + fcb)
		
		cmd = data[1]
		
		if (cmd == FS_HOST_COMMAND_PING):
			v = data[2] + 256*data[3]
			put('Ping: 0x%04' % v)
			
			# Pong!
			#self.send_word(FS_HOST_COMMAND_RETURN_WORD, v)
			self.reply_frame(bytes([data[2], data[3]]))
			
		elif (cmd == FS_HOST_COMMAND_END_BOOTLOADER):
			put('End bootloader')
			
		
		elif (cmd == FS_HOST_COMMAND_FILE_OPENDIR):
			path = ''.join(chr(b) for b in data[2:])
			
			put('opendir: "%s"' % path)
			
			try:
				# Actually open dir
				if (path == ''): path = '.'
				h = os.listdir(path)
				
				# Store local handle
				self.handles.append(h)
				
				# Generate remote handle
				hi = len(self.handles)-1
				
			except FileNotFoundError:
				hi = FS_HOST_NO_HANDLE
			
			#self.send_byte(FS_HOST_COMMAND_RETURN_BYTE, hi)
			self.reply_frame(bytes([hi]))
			
		elif (cmd == FS_HOST_COMMAND_FILE_CLOSEDIR):
			h = data[2]
			put('closedir: h=0x%02X' % h)
			
			# Close handle
			self.handles[h] = None
			
		
		elif (cmd == FS_HOST_COMMAND_FILE_READDIR):
			h = data[2]
			put('readdir: h=0x%02X' % h)
			
			name = ''
			try:
				name = self.handles[h].pop()	# Get next from listdir()
			except IndexError:
				# Empty / End
				name = ''	# '' = End
			#self.send_asciiz(FS_HOST_COMMAND_RETURN_ASCIIZ, name)
			self.reply_frame(bytes(name, 'ascii'))
			
		
		elif (cmd == FS_HOST_COMMAND_FILE_OPEN):
			path = ''.join(chr(b) for b in data[2:])
			put('open: "%s"' % path)
			
			path = self.translate_path(path)
			
			# Actually open dir
			mode = 'rb'
			h = open(path, mode)
			
			if not os.path.isfile(path):
				self.reply_frame(b'')
				return
			
			# Store local handle
			self.handles.append(h)
			
			# Generate remote handle
			hi = len(self.handles)-1
			#self.send_byte(FS_HOST_COMMAND_RETURN_BYTE, hi)
			self.reply_frame(bytes([hi]))
			
		elif (cmd == FS_HOST_COMMAND_FILE_CLOSE):
			h = data[2]
			put('close: h=0x%02X' % h)
			
			# Close handle
			self.handles[h].close()
			self.handles[h] = None
			
		
		elif (cmd == FS_HOST_COMMAND_FILE_EOF):
			h = data[2]
			if len(self.handles[h].peek()) == 0:
				self.reply_frame(bytes([1]))
			else:
				self.reply_frame(bytes([0]))
			
		elif (cmd == FS_HOST_COMMAND_FILE_READ):
			h = data[2]
			put('read: h=0x%02X' % h)
			
			max_buf = 32
			data = self.handles[h].read(max_buf)
			#self.send_frame(FS_HOST_COMMAND_RETURN_DATA, [ b for b in data ])
			self.reply_frame(data)
			
		else:
			put('Unknown command 0x%02X!' % cmd)
			return False
		
		#Host.handle_frame(data=data)
		
	def translate_path(self, path):
		"""Translate path to actual host file system path"""
		#path = os.path.join(BASE_PATH, path)
		path = '%s/%s' % (BASE_PATH, path)
		return path


if __name__ == '__main__':
	### Emulate using host.py (Start MAME and serve files to host.h)
	
	mame_sys = 'gl6000sl'
	cart_filename = 'out/files.cart.8kb.bin'
	
	#host.SHOW_TRAFFIC = True	# Debug traffic
	#host.SHOW_TRAFFIC_BYTES = True	# Debug traffic byte by byte
	
	# Start host in MAME mode...
	put('Starting host...')
	mame_roms_dir = None	# Use stock ROM
	driver = Driver_MAME(rompath=mame_roms_dir, emusys=mame_sys, cart_file=cart_filename)
	#driver = Driver_serial(baud=softuart_baud, stopbits=2)	# More stopbits = more time to process?
	#driver = Driver_serial(baud=9600, stopbits=1)
	
	# Chose a protocol
	protocol = Protocol_binary()	# For MAME
	#protocol = Protocol_binary_safe()	# For serial. Recommended.
	#protocol = Protocol_hex()	# Old text-only protocol
	#protocol = Protocol()
	
	# Start host
	put('Host driver=%s, protocol=%s' % (str(driver.__class__.__name__), str(protocol.__class__.__name__)))
	#h = Host(driver=driver, protocol=protocol
	h = Files_Host(driver=driver, protocol=protocol)	#, mounts=MOUNTS)
	h.open()
	
	if not h.is_open:
		put('Connection could not be opened. Aborting.')
		sys.exit(4)
	
	h.run()