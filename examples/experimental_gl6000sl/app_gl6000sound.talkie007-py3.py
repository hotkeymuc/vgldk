#!/usr/bin/python3

"""
Talkie

Starting from version 004 this is not the original implementation any more.
I am widening up the types to allow better real-time modifications

Based on https://github.com/going-digital/Talkie
Converted from Arduino to Python
by Bernhard Slawik on 2015-08-19 while watching CCC-Camp 2015 videos

Encode using "QBOX Pro"

"""

import time
import random
#import pyaudio

DUMP_FRAME_INFO = True

TIME_STRETCH = 1
ATTACK_RATIO = 0.05	#0.02 is good	# 1.0 = instant frames, 0.01=soft 0.001=too-soft

PREGAIN = 1.0	# For the voicing function
SAMPLES_PER_FRAME = 25*8	# 200 for 8000 Hz (40 fps)
NUM_K = 10
SAMPLE_RATE = 8000

def put(t):
	print(str(t))

# The ROMs used with the TI speech were serial, not byte wide.
# Here's a handy routine to flip ROM data which is usually reversed.
def rev(a):
	
	# 76543210
	a = (a >> 4) | (a << 4) #Swap in groups of 4
	# 32107654
	a = ((a & 0xcc) >>2) | ((a & 0x33)<<2)	# Swap in groups of 2
	# 10325476
	a = ((a & 0xaa)>>1) | ((a & 0x55)<<1)	# Swap bit pairs
	# 01234567
	return a
#put(rev(0b00000001))
#put(rev(0b00000100))

def int16_t(v):
	if (v > 0x7fff): v -= 0x1000
	elif (v < -0x8000): v += 0x1000
	return v

def uint8_t_array(a):
	r = []
	for v in a:
		if (v > 0xff): raise Exception('Value error creating uint8_t_array!')
		r.append(v)
	return r

def int8_t_array(a):
	r = []
	for v in a:
		if (v > 0x7f): v -= 0x100
		elif (v < -0x80): v += 0x100
		r.append(v)
	return r

def int16_t_array(a):
	r = []
	for v in a:
		if (v > 0x7fff): v -= 0x10000
		elif (v < -0x8000): v += 0x10000
		r.append(v)
	return r



# Parameter envelopes (mapping of bitstream value to usable value)
TMS_ENERGY = uint8_t_array([0x00,0x02,0x03,0x04,0x05,0x07,0x0a,0x0f,0x14,0x20,0x29,0x39,0x51,0x72,0xa1,0xff])
TMS_PERIOD = uint8_t_array([0x00,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2D,0x2F,0x31,0x33,0x35,0x36,0x39,0x3B,0x3D,0x3F,0x42,0x45,0x47,0x49,0x4D,0x4F,0x51,0x55,0x57,0x5C,0x5F,0x63,0x66,0x6A,0x6E,0x73,0x77,0x7B,0x80,0x85,0x8A,0x8F,0x95,0x9A,0xA0])

#int16_t
TMS_K = [0]*10
TMS_K[0]	= int16_t_array([0x82C0,0x8380,0x83C0,0x8440,0x84C0,0x8540,0x8600,0x8780,0x8880,0x8980,0x8AC0,0x8C00,0x8D40,0x8F00,0x90C0,0x92C0,0x9900,0xA140,0xAB80,0xB840,0xC740,0xD8C0,0xEBC0,0x0000,0x1440,0x2740,0x38C0,0x47C0,0x5480,0x5EC0,0x6700,0x6D40])
TMS_K[1]	= int16_t_array([0xAE00,0xB480,0xBB80,0xC340,0xCB80,0xD440,0xDDC0,0xE780,0xF180,0xFBC0,0x0600,0x1040,0x1A40,0x2400,0x2D40,0x3600,0x3E40,0x45C0,0x4CC0,0x5300,0x5880,0x5DC0,0x6240,0x6640,0x69C0,0x6CC0,0x6F80,0x71C0,0x73C0,0x7580,0x7700,0x7E80])

#int8_t
TMS_K[2]	= int8_t_array([0x92,0x9F,0xAD,0xBA,0xC8,0xD5,0xE3,0xF0,0xFE,0x0B,0x19,0x26,0x34,0x41,0x4F,0x5C])
TMS_K[3]	= int8_t_array([0xAE,0xBC,0xCA,0xD8,0xE6,0xF4,0x01,0x0F,0x1D,0x2B,0x39,0x47,0x55,0x63,0x71,0x7E])

TMS_K[4]	= int8_t_array([0xAE,0xBA,0xC5,0xD1,0xDD,0xE8,0xF4,0xFF,0x0B,0x17,0x22,0x2E,0x39,0x45,0x51,0x5C])
TMS_K[5]	= int8_t_array([0xC0,0xCB,0xD6,0xE1,0xEC,0xF7,0x03,0x0E,0x19,0x24,0x2F,0x3A,0x45,0x50,0x5B,0x66])
TMS_K[6]	= int8_t_array([0xB3,0xBF,0xCB,0xD7,0xE3,0xEF,0xFB,0x07,0x13,0x1F,0x2B,0x37,0x43,0x4F,0x5A,0x66])
TMS_K[7]	= int8_t_array([0xC0,0xD8,0xF0,0x07,0x1F,0x37,0x4F,0x66])
TMS_K[8]	= int8_t_array([0xC0,0xD4,0xE8,0xFC,0x10,0x25,0x39,0x4D])
TMS_K[9]	= int8_t_array([0xCD,0xDF,0xF1,0x04,0x16,0x20,0x3B,0x4D])

### Chirp wave form
# Original talkie chirp
#CHIRP_SIZE = 41
#CHIRP = int8_t_array([0x00,0x2a,0xd4,0x32,0xb2,0x12,0x25,0x14,0x02,0xe1,0xc5,0x02,0x5f,0x5a,0x05,0x0f,0x26,0xfc,0xa5,0xa5,0xd6,0xdd,0xdc,0xfc,0x25,0x2b,0x22,0x21,0x0f,0xff,0xf8,0xee,0xed,0xef,0xf7,0xf6,0xfa,0x00,0x03,0x02,0x01])

# Saw chirp
CHIRP_SIZE = 41*2
CHIRP = int8_t_array([((i * 4) % 0x100) for i in range(CHIRP_SIZE)])

"""
r = ''
for c in CHIRP:
	if (r != ''): r += ', '
	r += '0x%02X' % ((c + 0x1000) % 0x100)
put('int8_t chirp[CHIRP_SIZE] = {' + r + '};')
#exit
"""

#CHIRP_SIZE = 255
#CHIRP = int8_t_array([int(random.random()*0xff) for i in range(CHIRP_SIZE)])

class TalkieFrame:
	"""Represents a state of the synth (i.e. one frame of speech)"""
	def __init__(self, energy=0, period=0, k=None, ofs=None):
		self.energy = energy
		self.period = period
		self.k = k if k is not None else [ 0 for i in range(NUM_K) ]
		self.ofs = ofs	# Data offset, for debugging
	
	@staticmethod
	def clone(org):
		f = TalkieFrame()
		f.energy = org.energy
		f.period = org.period
		for i in range(NUM_K):
			f.k[i] = org.k[i]
		return f

class TalkieParser:
	"""Parsed Speak-and-Spell type speech data"""
	def __init__(self, filename=None, data=None, ofs=0, l=None):
		
		self.data = None
		self.ptrAddr = 0
		self.ptrBit = 0
		self.ptrLen = 0
		
		if filename is not None:
			self.load_file(filename, ofs=ofs, l=l)
		if data is not None:
			#self.set_data(data)
			self.set_data(data[:l] if l is not None else data)
	
	def set_data(self, data):
		self.data = data
		self.ptrLen = len(self.data)
		self.rewind()
	
	def rewind(self):
		self.ptrAddr = 0
		self.ptrBit = 0
	
	def get_byte(self, addr):
		if addr < self.ptrLen:
			return rev(self.data[addr])
		else:
			return rev(0xff)
	
	def get_bits(self, bits):
		#put('Get ' + str(bits) + ' bits from ' + 'data' + ' at ' + str(self.ptrAddr) + ' = [' + str(self.data[self.ptrAddr]) + ']...')
		#uint8_t value;
		#uint16_t data;
		#data = rev(pgm_read_byte(ptrAddr))<<8
		data = self.get_byte(self.ptrAddr)<<8
		if (self.ptrBit+bits > 8):
			#data |= rev(pgm_read_byte(ptrAddr+1))
			data |= self.get_byte(self.ptrAddr+1)
		
		#data <<= self.ptrBit
		data = (data << self.ptrBit) & 0xffff
		
		value = data >> (16-bits)
		self.ptrBit += bits
		if (self.ptrBit >= 8):
			self.ptrBit -= 8
			self.ptrAddr += 1
		
		#put('value=' + str(value))
		return value
	
	def eof(self):
		if (self.ptrAddr >= self.ptrLen): return True
		return False
	
	def parse_frame(self, f=None):
		"Read speech data, processing the variable size frames."
		
		if f == None: f = TalkieFrame(ofs=self.ptrAddr)
		
		energy = self.get_bits(4)
		
		if (energy == 0):
			# Energy = 0: rest frame
			f.energy = 0
			#f.period = 0
			
		elif (energy == 0xf):
			# Energy = 15: stop frame. Silence the synthesiser.
			#self.synthEnergy = 0
			f.energy = 0
			#f.period = 0
			for i in range(10):
				f.k[i] = 0
			
		else:
			f.energy = TMS_ENERGY[energy]
			
			repeat = self.get_bits(1)
			f.period = TMS_PERIOD[self.get_bits(6)]
			
			# A repeat frame uses the last coefficients
			if (repeat == 0):
				# All frames use the first 4 coefficients
				f.k[0] = TMS_K[0][self.get_bits(5)] / 32768.0
				f.k[1] = TMS_K[1][self.get_bits(5)] / 32768.0
				f.k[2] = TMS_K[2][self.get_bits(4)] / 128.0
				f.k[3] = TMS_K[3][self.get_bits(4)] / 128.0
				
				if (f.period > 0):
					# Voiced frames use 6 extra coefficients.
					f.k[4] = TMS_K[4][self.get_bits(4)] / 128.0
					f.k[5] = TMS_K[5][self.get_bits(4)] / 128.0
					f.k[6] = TMS_K[6][self.get_bits(4)] / 128.0
					f.k[7] = TMS_K[7][self.get_bits(3)] / 128.0
					f.k[8] = TMS_K[8][self.get_bits(3)] / 128.0
					f.k[9] = TMS_K[9][self.get_bits(3)] / 128.0
				#else:
				#	f.energy = self.synthEnergy2
				#	f.energy = self.synthPeriod2
			#
		#
		
		#put('frame:	energy=%s	period=%d	k=%s' % (energy, f.period, str(f.k)))
		
		return f
	
	def load_file(self, filename, ofs=0, l=None):
		data = []
		f = open(filename, 'rb')
		
		#check = f.read(1)
		#oc = ord(check[0])
		#if (oc == 9) or (oc == 0x2a) or ((oc >= 64) and (oc < 128)):
		ext = filename[-4:].lower()
		if ext in ['.hex', '.sfm']:
			put('Text format')
			for line in f:
				line = line.decode('ascii').strip()
				#put('"' + line + '"')
				if (line[0:4] == 'BYTE'):
					values = line[5:].replace('>','').split(',')
					for v in values:
						data.append(int(v, 16))
		else:
			put('Binary file')
			# Binary file
			data = bytearray(f.read())
			put('len=%d' % (len(data)))
			
			"""
			# Search for sense
			o = ofs	#0x7c74c	#0 + 0xcf18
			l = 256
			
			put('Search mode (playing %d bytes, then shifting onwards)' % (l))
			while True:
				put('Trying o=%04X' % (o))
				data_test = data[o:o+l]
				for i in range(8): data_test.append(0xff)	# Make it stop
				
				self.output = []
				self.data = ''
				self.say(uint8_t_array(data_test))
				put('o=%d / 0x%04X: Talking...' % (o, o))
				self.play_pyaudio()
				
				o += l//4-1	# l
			"""
			
		f.close()
		
		
		#self.set_data(data)
		# Allow specifying length
		self.set_data(data[:ofs+l] if l is not None else data)
		
		self.ptrAddr = ofs
		
		return data




class TalkieSynth:
	"""Texas Instruments like speech synthesizer"""
	def __init__(self):
		
		self.frame_current = TalkieFrame()
		self.frame_next = TalkieFrame()
		
		# Statics
		self.period_counter = 0
		self.random_seed = 1
		
		self.x = [0 for i in range(NUM_K) ]
		
	def set_frame(self, f):
		self.frame_next = f
	
	def render_sample(self):
		
		#u = [0,0,0,0,0,0,0,0,0,0,0]	# [ 0 ] * NUM_K+1
		u = [ 0 for i in range(NUM_K+1) ]
		
		f2 = self.frame_next
		if ATTACK_RATIO < 1:
			# Soft changes
			f1 = self.frame_current
			r = ATTACK_RATIO / TIME_STRETCH
			
			#if (f1.energy < f2.energy): f1.energy += 0.3
			#elif (f1.energy > f2.energy): f1.energy -= 0.3
			f1.energy = (f1.energy * (1.0-r)) + (f2.energy * r)
			
			if (f2.period > 0):
				#if (f1.period < f2.period): f1.period += 0.1 / TIME_STRETCH
				#elif (f1.period > f2.period): f1.period -= 0.1 / TIME_STRETCH
				f1.period = (f1.period * (1.0-r)) + (f2.period * r)
			else:
				#f1.period = f2.period
				# Keep old frequency
				pass
			
			for i in range(NUM_K):
				f1.k[i] = (f1.k[i] * (1.0-r)) + (f2.k[i] * r)
			
			f = f1
		else:
			f = self.frame_next
		
		
		if (f2.period > 0):
			# Voiced source
			
			#self.period_counter = (self.period_counter + 1) % int(f.period)
			if (self.period_counter < f.period): self.period_counter += 1
			else: self.period_counter = 0
			
			if (self.period_counter < CHIRP_SIZE):
				u[NUM_K] = (CHIRP[self.period_counter] * f.energy) / 256.0
			else:
				u[NUM_K] = 0
		else:
			# Unvoiced source
			
			# Quick random:
			self.random_seed = ((self.random_seed >> 1) ^ (0xB800 if (self.random_seed & 1 > 0) else 0)) & 0xffff
			u[NUM_K] = (f.energy if (self.random_seed & 1 > 0) else -f.energy)
			
			# Real random:
			#u[NUM_K] = (random.random()*2.0-1.0) * f.energy * 4.0
		#
		
		u[NUM_K] *= PREGAIN
		
		# Lattice filter forward path
		i = NUM_K
		while i > 0:
			u[i-1] = u[i] - (f.k[i-1] * self.x[i-1])
			i -= 1
		
		
		# Lattice filter reverse path
		i = NUM_K-1
		while i > 0:
			self.x[i] = self.x[i-1] + (f.k[i-1]*u[i-1])
			i -= 1
		self.x[0] = u[0]
		
		
		# Output clamp
		if (u[0] > 511): u[0] = 511
		elif (u[0] < -512): u[0] = -512
		
		# Return a 8 bit signed value (-127..127)
		return int(u[0] // 4)+0x80
		
	#
	
#


stream = None
pa = None
def play_pyaudio(data):
	"""Helper function to play audio data via pyaudio"""
	global stream, pa
	import pyaudio
	
	#SAMPLE_RATE = 8000	#44100
	CHANNELS = 1
	# open stream
	pa = pyaudio.PyAudio()
	stream = pa.open(
		format = pa.get_format_from_width(1),
		channels = CHANNELS,
		rate = SAMPLE_RATE,
		output = True
	)
	
	if data is None:
		put('No data given, stream started! Don\'t forget to buffer_pyaudio() and stop_pyaudio()!')
		return
	
	tim = len(data) / SAMPLE_RATE
	put('Length: ' + str(tim) + ' seconds')
	
	put('Playing...')
	#stream.write(self.output)
	#for v in self.output:
	#	stream.write(chr(v))
	stream.write(bytes(data))
	
	time.sleep(tim)
	time.sleep(0.2)	# Some more to be safe
	
	stream.close()
	pa.terminate()

def buffer_pyaudio(data):
	global stream, pa
	
	#stream.write(self.output)
	#for v in self.output:
	#	stream.write(chr(v))
	stream.write(bytes(data))

def stop_pyaudio():
	global stream, pa
	
	stream.close()
	pa.terminate()


def say_file(filename, ofs=None, l=None):
	say(filename=filename, ofs=ofs, l=l)
def say_data(data, ofs=None, l=None):
	say(data=data, ofs=ofs, l=l)
def say(filename=None, data=None, ofs=None, l=None):
	voice = TalkieSynth()
	parser = TalkieParser(filename=filename, data=data, ofs=ofs, l=l)
	
	
	play_pyaudio(None)	 #Start stream (no data given)
	#data = []
	frame = 0
	while not parser.eof():
		f = parser.parse_frame()
		
		if DUMP_FRAME_INFO:
			put('ptr=%d (0x%06X / 0x%06X)		energy=%d,	period=%d, 	k=[ %s ]' % (f.ofs,f.ofs, parser.ptrLen, f.energy, f.period,	',	'.join([ '%.5f'%v for v in f.k ])))
		
		voice.set_frame(f)
		
		# Do the rendering
		data = []	# Only frame-wise
		for i in range(SAMPLES_PER_FRAME * TIME_STRETCH):
			v = voice.render_sample()
			data.append(v)
		
		buffer_pyaudio(data)
		if (frame > 4):	# Pre-buffer without delay
			time.sleep(len(data) / SAMPLE_RATE)
		frame += 1
	
	time.sleep(1)	# Post-delay
	
	#Play all at once, once it is finished:
	#play_pyaudio(data)
	
	stop_pyaudio()



if __name__ == '__main__':
	
	#import example_Vocab_UK_Acorn
	#say_data(example_Vocab_UK_Acorn.spA)
	#say_data(example_Vocab_UK_Acorn.spE)
	#say_data(example_Vocab_UK_Acorn.spNO)
	#say_data(example_Vocab_UK_Acorn.spNOT)
	#say_data(example_Vocab_UK_Acorn.spANOTHER)
	#say_data(example_Vocab_UK_Acorn.spHUNDRED)
	
	#import example_Vocab_US_Large
	#say_data(example_Vocab_US_Large.spALPHA)
	#say_data(example_Vocab_US_Large.spBRAVO)
	#say_data(example_Vocab_US_Large.spCHARLIE)
	#say_data(example_Vocab_US_Large.spDELTA)
	
	#say_data(example_Vocab_US_Large.spEMERGENCY)
	
	#TIME_STRETCH = 2;	say_data(example_Vocab_US_Large.spEMERGENCY)
	#TIME_STRETCH = 4;	say_data(example_Vocab_US_Large.spEMERGENCY)
	
	#say_data(example_Vocab_US_Large.spIMMEDIATELY)
	#say_data(example_Vocab_US_Large.spDOWNWIND)
	#say_data(example_Vocab_US_Large.spCONVERGING)
	#say_data(example_Vocab_US_Large.spAIR_BRAKES)
	#say_data(example_Vocab_US_Large.spEVACUATION)
	#say_data(example_Vocab_US_Large.spLANDING_GEAR)
	#say_data(example_Vocab_US_Large.spSEQUENCE)
	#say_data(example_Vocab_US_Large.spCAUTION)
	
	#import example_Vocab_US_Male
	#say_data(example_Vocab_US_Male.spDIRECTION)
	#say_data(example_Vocab_US_Male.spNORTH)
	#say_data(example_Vocab_US_Male.spINSPECTOR)
	#say_data(example_Vocab_US_Male.spGREEN)
	#say_data(example_Vocab_US_Male.spTEMPERATURE)
	#say_data(example_Vocab_US_Male.spCIRCUIT)
	#say_data(example_Vocab_US_Male.spDEVICE)
	
	#import example_Vocab_US_TI99
	#say_data(example_Vocab_US_TI99.spWHAT_WAS_THAT)
	#say_data(example_Vocab_US_TI99.spTRY_AGAIN)
	#say_data(example_Vocab_US_TI99.spTEXAS_INSTRUMENTS)
	#say_data(example_Vocab_US_TI99.spREADY_TO_START)
	#say_data(example_Vocab_US_TI99.spNICE_TRY)
	#say_data(example_Vocab_US_TI99.spGOOD_WORK)
	#say_data(example_Vocab_US_TI99.spCONNECTED)
	
	#import example_Vocab_Soundbites
	#say_data(example_Vocab_Soundbites.spHASTA_LA_VISTA)
	#say_data(example_Vocab_Soundbites.spONE_SMALL_STEP)
	#say_data(example_Vocab_Soundbites.spHMMM_BEER)
	
	
	#voice.say_file('files/DIPDOK.SFM')
	#voice.say_file('files/DIPDOK.BIN')
	#voice.say_file('files/SCHWEICH.SFM')
	
	#voice.say_file('files/BEARDY.SFM')
	#voice.say_file('files/TOMSDINR.hex')
	#say_file('files/BRITNEY_1.hex')
	#say_file('files/BRITNEY_2.hex')
	#say_file('files/BRITNEY_3.hex')
	#say_file('files/CHECK2.SFM')
	#voice.say_file('files/VTech_GeniusLeaderNotebook_guitar_etc.bin')
	
	#TIME_STRETCH = 2
	#say_file('files/VTech_GeniusLeaderNotebook_guitar_etc.bin')
	#say_file('files/VTech_GeniusLeaderNotebook.bin', 0x7c74e)
	
	TIME_STRETCH = 2
	# Findings: 0x6D000-0x6E000	!!!!!!!!!!!!
	# Hint: The last frame is usually "0x00" or "0x0F"
	#filename = 'files/VTech_GeniusLeader_ROM_GL6000SL_27-5894-01.bin'
	filename = '/z/data/_devices/VTech_Genius_Lerncomputer/Disassembly/ROM_GL6000SL_27-5894-01'
	#say_file(filename, ofs=0x6D000, l=0x400)
	
	#say_file(filename, ofs=0x6D124, l=0x08 + 60)
	
	#say_file(filename, ofs=0x6D131, l=0x0a + 0)	# Bing
	
	#say_file(filename, ofs=0x6D13b-1, l=0x60 + 60)	# 6d13c=Jingle
	
	#say_file(filename, ofs=0x6D141, l=0x20 + 80)	# 0x6D141 = Boing
	
	#say_file(filename, ofs=0x6D157-2, l=0x120 + 80)	# Instrument-Sound?
	
	#say_file(filename, ofs=0x6D2BC, l=0x20 + 20)
	
	#say_file(filename, ofs=0x6D274, l=0x20 + 20)	# Hello?
	say_file(filename, ofs=0x6D2f4, l=0x30)	# ~6d2f4+- = Reverb / delete
	#say_file(filename, ofs=0x6D5e6, l=0x30 + 40)	# 	! 0x6D5e6 = mewewew
	
	
	#voice.say_file('encode/OUT/MASGER/AIN0.sfm')
	#voice.say_file('files/MASGER/E1.sfm')
	#voice.say_file('files/MASGER/N0.sfm')
	#voice.say_file('files/MASGER/T.sfm')
	#voice.say_file('files/MASGER/_.sfm')
	#voice.say_file('files/MASGER/GAI.sfm')
	#voice.say_file('files/MASGER/L.sfm')
	#voice.say_file('files/MASGER/EU.sfm')
	#voice.say_file('files/MASGER/AIN0.sfm')
	
	#voice.say_file('files/MASGER/AIN0.sfm')
	#voice.say_file('files/JINGLE.sfm')
	#voice.say_file('files/JINGLED.sfm')
	
	#voice.save_output('output.raw')
	
	#voice.say_file('files/CHRIZ.SFM')
	#voice.play_pyaudio()
