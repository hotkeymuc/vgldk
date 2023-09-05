#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
MAME runner

Starts MAME and allows interaction via STDIO
(see mame_pc2000.cpp_patched.txt)

"""
import sys
PYTHON3 = (sys.version_info.major == 3)


# MAME
#MAME_COMMAND = './mame64'
#MAME_COMMAND = '../../../mame.git/mame64'
MAME_COMMAND = '/z/data/_code/_c/mame.git/mame64'
#MAME_ROMPATH = None
MAME_ROMPATH = '/z/apps/_emu/_roms'
MAME_EMUSYS = 'gl4000'
MAME_CART = 'out/monitor.cart.16kb.bin'


# General
SHOW_TRAFFIC = True

import time
import subprocess
from _thread import start_new_thread

def put(txt):
	print('mame: %s' % txt)


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

def str_binhex(b):
	return ', '.join(['%02X'%v for v in b])

class MAME:
	"Host implementation when using MAME"
	
	def __init__(self, command=MAME_COMMAND, rompath=MAME_ROMPATH, emusys=MAME_EMUSYS, cart=MAME_CART, speed=2.0, volume=24, buffer_size=3):
		self.command = command
		self.rompath = rompath
		self.emusys = emusys
		self.cart = cart
		
		self.buffer_size = buffer_size
		self.speed = speed
		self.volume = volume
		
		self.is_open = False
		self.running = False
		
		self.proc = None
		
		self.on_data = None
	
	def __del__(self):
		self.mame_close()
	
	def put(self, s):
		put('MAME	%s' % s)
	
	def open(self, run=True):
		self.put('Starting MAME...')
		start_new_thread(self.mame_open, (run,))
		while(not self.is_open):
			time.sleep(0.5)
	
	def close(self):
		self.mame_close()
		self.running = False
	
	def stop(self):
		#put(str(dir(self.proc)))
		#self.proc.terminate()
		#self.proc.kill()
		self.close()
	
	def mame_open(self, run=True):
		self.is_open = False
		
		cmd = self.command
		if (self.emusys): cmd += ' %s' % self.emusys
		#cmd += ' -nodebug'
		if (self.rompath): cmd += ' -rompath %s' % self.rompath
		#cmd += ' -cart out/cpm.cart.bin'
		if (self.cart): cmd += ' -cart %s' % self.cart
		cmd += ' -window'
		cmd += ' -nomax'
		cmd += ' -nofilter'
		cmd += ' -sleep'
		cmd += ' -volume -%d' % self.volume
		#cmd += ' -skip_disclaimer'
		cmd += ' -skip_gameinfo'
		cmd += ' -speed %.2f' % self.speed
		
		self.put(cmd)
		#self.proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		### https://stackoverflow.com/questions/1606795/catching-stdout-in-realtime-from-subprocess
		#self.proc = subprocess.Popen('stdbuf -o0 '+ cmd, stdout=subprocess.PIPE, shell=True, bufsize=0)
		self.proc = subprocess.Popen(
			'stdbuf -i%d -o%d %s' % (self.buffer_size, self.buffer_size, cmd),
			#cmd,
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			shell=True,
			#close_fds=False,	# Use close_fds=False on unix, close_fds=True on linux
			#bufsize=2
			#bufsize=self.buffer_size	# 0=unbuffered, 1=line buffered, n=buffer ~n bytes
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
		#self.put('Return code so far: %s' % str(self.proc.returncode))
		self.is_open = True
		
		if run:
			self.run()
		
	
	def mame_close(self):
		if not self.proc is None:
			self.proc.communicate()	# Close PIPE
	
	def mame_keep_alive(self):
		if self.proc is None: return
		
		if self.proc.poll() is not None:
			#self.is_open = False
			self.running = False
			self.mame_close()
			raise Exception('Process closed while polling.')
	
	def run(self):
		"Main loop"
		self.running = True
		
		#h = self.proc.stdout
		#put(str(dir(h)))
		
		while self.running:
			
			r = self.proc.poll()
			if (r is None):
				# Still running
				
				s = self.read()
				
				#self.handle_data(s)
				if self.on_data is not None:
					self.on_data(s)
				
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
	
	def read(self):
		#s = self.proc.stdout.read(0)
		#s = self.proc.stdout.read(1)
		#s = self.proc.stdout.read(64)
		s = self.proc.stdout.readline()
		if (SHOW_TRAFFIC): self.put('<<< %s' % str_binhex(s))
		return s
	
	def write(self, s):
		if (SHOW_TRAFFIC): self.put('>>> %s' % str_binhex(s))
		#self.proc.stdin.write(bytes('%2X\n' % b, 'ascii'))
		#self.proc.stdin.write(s)
		try:
			self.proc.stdin.write(s)	# s=bytes
			#self.proc.stdin.write(bytes(s, 'ascii'))	# s=string
			
		except BrokenPipeError:
			self.running = False
	

if __name__ == '__main__':
	#put(__doc__)
	
	"""
	argv = sys.argv[1:]
	try:
		opts, args = getopt.getopt(argv, 'pbh:', ['port=','baud='])
	except getopt.GetoptError:
		show_help()
		sys.exit(2)
	"""
	
	emu = MAME()
	
	emu.open()
	while emu.running:
		time.sleep(1)
	
	
	put('End.')