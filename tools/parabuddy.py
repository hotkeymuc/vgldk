#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
ParallelBuddy implementation in Python

This is not a stand-alone program, but a module.

2020-08-29 Bernhard "HotKey" Slawik
"""
import time
import os	# for listdir

def put(txt):
	print('parabuddy: %s' % txt)


PB_MAX_FRAME_SIZE = 64
PB_MAX_FILENAME = 32


PB_PREFIX = 0xfe	# STX 0x02 or ENQ 0x05
#PB_TERMINATOR = 0x03	# ETX 0x03 or EOT 0x04

PB_COMMAND_RETURN_OK = 0x06	#ACK
PB_COMMAND_RETURN_BYTE = 0x11
PB_COMMAND_RETURN_WORD = 0x12
PB_COMMAND_RETURN_ASCIIZ = 0x13
PB_COMMAND_RETURN_DATA = 0x14
PB_COMMAND_RETURN_NACK = 0x15	# NAK

# Functionality
PB_COMMAND_END_BOOTLOADER = 0x1A

PB_COMMAND_PING = 0xE0
PB_COMMAND_PING_HOST = 0xE1

#PB_COMMAND_SD_INIT = 0x20
#PB_COMMAND_SD_EXISTS = 0x21
#PB_COMMAND_SD_OPEN = 0x22
#PB_COMMAND_SD_CLOSE = 0x23
#PB_COMMAND_SD_REMOVE = 0x4
#PB_COMMAND_SD_MKDIR = 0x25
#PB_COMMAND_SD_RMDIR = 0x26

PB_COMMAND_FILE_OPENDIR = 30
PB_COMMAND_FILE_READDIR = 31
PB_COMMAND_FILE_CLOSEDIR = 32

PB_COMMAND_FILE_OPEN = 40
PB_COMMAND_FILE_CLOSE = 0x41
PB_COMMAND_FILE_EOF = 0x42
PB_COMMAND_FILE_READ = 0x43
PB_COMMAND_FILE_WRITE = 0x44
#PB_COMMAND_FILE_SEEK = 0x45
#PB_COMMAND_FILE_SIZE = 0x46
PB_COMMAND_FILE_AVAILABLE = 0x47

PB_ERROR_UNKNOWN = 0x01
PB_ERROR_LENGTH = 0x02

PB_FILE_READ = 0
PB_FILE_WRITE = 1

PB_NO_HANDLE = 0xff


SHOW_TRAFFIC = True

class ParaBuddy:
	def __init__(self):
		self.on_send = None
		
		self.handles = []
		
		# For buffered input
		self.state = 0
		self.data_len = 0
		self.data_buf = []
	
	def put(self, t):
		put(t)
	
	def send_raw(self, data):
		"Actually transmit data (array of bytes)"
		if SHOW_TRAFFIC:
			self.put('>>> %s' % (', '.join(['0x%02X'%b for b in data])))
		if self.on_send is not None:
			self.on_send(data)
	
	def send_frame(self, cmd, data):
		"Send header and data"
		prefix = [ PB_PREFIX, PB_PREFIX, PB_PREFIX ]
		header = [
			1+len(data),
			cmd
		]
		#self.send_raw(prefix+header+data)
		self.send_raw(prefix+header+data+prefix+prefix)	# Just to keep the buffer flowing
	
	def send_byte(self, cmd, v):
		self.send_frame(cmd, [v])
	
	def send_word(self, cmd, v):
		self.send_frame(cmd, [v % 256, v // 256])
	
	def send_asciiz(self, cmd, v):
		self.send_frame(cmd, [ ord(b) for b in v ])
	
	def handle_data(self, data):
		for b in data:
			self.handle_byte(b)
	
	def handle_byte(self, b):
		
		if self.state == 0:
			self.data_len = b
			self.data_buf = []
			self.state = 1
		
		self.data_buf.append(b)
		
		if (1+self.data_len == len(self.data_buf)):
			self.handle_frame(self.data_buf)
			self.state = 0
			self.data_buf = []
			
	
	def handle_frame(self, data):
		"Callback to handle one data frame"
		put('<<< %s' % (', '.join([ '0x%02X'%b for b in data])))
		
		l = data[0]
		if (l != len(data)-1):
			put('Lengths differ! Given=%d, actual=%d' % (l, len(data)-1))
			return False
		
		cmd = data[1]
		
		if (cmd == PB_COMMAND_PING):
			v = data[2] + 256*data[3]
			put('Ping: 0x%04' % v)
			
			# Pong!
			self.send_word(PB_COMMAND_RETURN_WORD, v)
			
		elif (cmd == PB_COMMAND_END_BOOTLOADER):
			put('End bootloader')
			
		
		elif (cmd == PB_COMMAND_FILE_OPENDIR):
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
				hi = PB_NO_HANDLE
			
			self.send_byte(PB_COMMAND_RETURN_BYTE, hi)
			
		elif (cmd == PB_COMMAND_FILE_CLOSEDIR):
			h = data[2]
			put('closedir: h=0x%02X' % h)
			
			# Close handle
			self.handles[h] = None
			
		
		elif (cmd == PB_COMMAND_FILE_READDIR):
			h = data[2]
			put('readdir: h=0x%02X' % h)
			
			name = ''
			try:
				name = self.handles[h].pop()	# Get next from listdir()
			except IndexError:
				# Empty / End
				name = ''	# '' = End
			self.send_asciiz(PB_COMMAND_RETURN_ASCIIZ, name)
			
		
		elif (cmd == PB_COMMAND_FILE_OPEN):
			path = ''.join(chr(b) for b in data[2:])
			put('open: "%s"' % path)
			
			# Actually open dir
			mode = 'rb'
			h = open(path, mode)
			
			# Store local handle
			self.handles.append(h)
			
			# Generate remote handle
			hi = len(self.handles)-1
			self.send_byte(PB_COMMAND_RETURN_BYTE, hi)
			
		elif (cmd == PB_COMMAND_FILE_CLOSE):
			h = data[2]
			put('close: h=0x%02X' % h)
			
			# Close handle
			self.handles[h].close()
			self.handles[h] = None
			
		
		elif (cmd == PB_COMMAND_FILE_READ):
			h = data[2]
			put('read: h=0x%02X' % h)
			
			max_buf = 32
			data = self.handles[h].read(max_buf)
			self.send_frame(PB_COMMAND_RETURN_DATA, [ b for b in data ])
			
		else:
			put('Unknown command 0x%02X!' % cmd)
			return False
		


if __name__ == '__main__':
	
	put('End.')