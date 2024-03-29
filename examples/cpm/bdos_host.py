#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
BDOS Host for VGL CP/M
======================

This script communicates with a VTech Genius Leader notebook and serves files to its CP/M system.
It can be a real computer connected via serial cable or a virtual computer (running in a modified version of MAME).

see tools/host.py for more info

2019-11-15 - 2023-09-04 Bernhard "HotKey" Slawik
"""


import time
import sys	# for sys.argv and sys.exit(status)
import getopt

sys.path.append('../../tools')
from host import *

# We need to wait a bit before sending a reply, because VGL does not have interrupt based serial and needs to be ready for data or it'll simply miss it
DMA_THROTTLE = None	#0.15	#0.1-0.2
DMA_MTU = 64	#32	# maximum transmission unit for DMA. e.g. 64 is fast, but checksums might be insufficient, data errors occur!


# Included LOAD
REPLY_THROTTLE = 0.05	# Only needed for LOAD
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

BDOS_MOUNTS = [
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


# BDOS definitions

# https:#www.seasip.info/Cpm/bdos.html
BDOS_FUNC_P_TERMCPM		= 0	# System Reset
BDOS_FUNC_C_READ		= 1	# Console input
BDOS_FUNC_C_WRITE		= 2	# Console output
BDOS_FUNC_A_READ		= 3	# Aux/Punch input
BDOS_FUNC_A_WRITE		= 4	# Aux/Punch output
BDOS_FUNC_L_WRITE		= 5	# Printer output
#BDOS_FUNC_DetectMemorySize	= 6	# Detect memory size; CP/M 1.3 only
BDOS_FUNC_C_RAWIO		= 6	# Direct console I/O; CP/M 1.4+
#BDOS_FUNC_A_STATIN	= 7	# Auxiliary Input status; Supported by: CP/M 3 and above. Not supported in MP/M.
BDOS_FUNC_GET_IOBYTE	= 7	# Returns I/O byte
#BDOS_FUNC_A_STATOUT	= 8	# Auxiliary Output status; Supported by: CP/M 3 and above. Not supported in MP/M.
BDOS_FUNC_SET_IOBYTE	= 8	# Set I/O byte
BDOS_FUNC_C_WRITESTR	= 9	# Output string (terminated by '$')
BDOS_FUNC_C_READSTR		= 10	# Buffered console input
BDOS_FUNC_C_STAT		= 11	# Console status
BDOS_FUNC_S_BDOSVER		= 12	# Return version number

BDOS_FUNC_DRV_ALLRESET	= 13	# Reset discs, go to A:
BDOS_FUNC_DRV_SET		= 14
BDOS_FUNC_F_OPEN		= 15
#BDOS_FUNC_D_OPEN	= 15	# Open directory; CP/M-86 v4
BDOS_FUNC_F_CLOSE		= 16
#BDOS_FUNC_D_CLOSE	= 16	# Close directory; CP/M-86 v4
BDOS_FUNC_F_SFIRST		= 17	# Search for first
BDOS_FUNC_F_SNEXT		= 18	# Search for next
BDOS_FUNC_F_DELETE		= 19	# Delete file
#BDOS_FUNC_D_DELETE	= 19	# Remove directory; CP/M-86 v4
BDOS_FUNC_F_READ		= 20	# Read next record
BDOS_FUNC_F_WRITE		= 21	# Write next record
BDOS_FUNC_F_MAKE		= 22	# Create file
#BDOS_FUNC_D_MAKE	= 22	# Create directory; CP/M-86 v4
BDOS_FUNC_F_RENAME		= 23	# Rename file

BDOS_FUNC_DRV_LOGINVEC	= 24	# Return bitmap of logged-in drives
BDOS_FUNC_DRV_GET		= 25	# Return current drive
BDOS_FUNC_F_DMAOFF		= 26	# Set DMA address
BDOS_FUNC_DRV_ALLOCVEC	= 27	# Return address of allocation map
BDOS_FUNC_DRV_SETRO		= 28	# Software write-protect current disc
BDOS_FUNC_DRV_ROVEC		= 29	# Return bitmap of read-only drives
#	30 - set echo mode for function 1; CP/M 1.3 only
BDOS_FUNC_F_ATTRIB		= 30	# set file attributes
BDOS_FUNC_DRV_DPB		= 31	# get DPB address
BDOS_FUNC_F_USERNUM		= 32	# get/set user number
BDOS_FUNC_F_READRAND	= 33	# Random access read record
BDOS_FUNC_F_WRITERAND	= 34	# Random access write record
BDOS_FUNC_F_SIZE		= 35	# Compute file size
BDOS_FUNC_F_RANDREC		= 36	# Update random access pointer
BDOS_FUNC_DRV_RESET		= 37	# Selectively reset disc drives
#	38 (DRV_ACCESS) - Access drives; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
#	39 (DRV_FREE) - Free drive; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
BDOS_FUNC_F_WRITEZF		= 40	# Write random with zero fill
#	41 - Test and write record; Supported by: MP/M, Concurrent CP/M.
#	42 (F_LOCK) - Lock record; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
#	43 (F_UNLOCK) - Unlock record; Supported by: MP/M, Concurrent CP/M, CP/Net redirector.
#	44 (F_MULTISEC) - Set number of records to read/write at once; Supported by: MP/M II and later.
#	45 (F_ERRMODE) - Set action on hardware error; Supported by: Personal CP/M, MP/M II and later, CP/Net redirector.
#	46 (DRV_SPACE) - Find free space on a drive; Supported by: MP/M II and later.
#	47 (P_CHAIN) - Chain to program; Supported by: MP/M II and later; CP/M-86 v1.1
#	48 (DRV_FLUSH) - Empty disc buffers; Supported by: Personal CP/M; MP/M II and later; CP/M-86 v1.1
#		49 - Access the System Control Block; Supported by: CP/M 3.
#	49 - Return address of system variables; Supported by: CP/M-86 v1.1
#	49 (S_SYSVAR) - Access the system variables; Supported by: CP/M-86 v4.
#		50 (S_BIOS) - Use the BIOS; Supported by: CP/M 3 and later; CP/M-86 v1.1
#	51 (F_DMASEG) - Set DMA segment; Supported by: CP/M-86 v1.1 and later.
#	52 (F_DMAGET) - Get DMA address; Supported by: CP/M-86 v3+, CCP/M-86.
#	53 (MC_MAX) - Allocate maximum memory; Supported by: CP/M-86 v1.1 and later.
#	54 (MC_ABSMAX) - Allocate absolute maximum memory; Supported by: CP/M-86 v1.1 and later.
#	54 - Get file time stamps; Supported by: Z80DOS, ZPM3
#	55 (MC_ALLOC) - Allocate memory; Supported by: CP/M-86 v1.1 and later.
#	55 - Use file time stamps; Supported by: Z80DOS, ZPM3
#	56 (MC_ABSALLOC) - Allocate absolute memory; Supported by: CP/M-86 v1.1 and later.
#	57 (MC_FREE) - Free memory; Supported by: CP/M-86 v1.1 and later.
#	58 (MC_ALLFREE) - Free all memory; Supported by: CP/M-86 v1.1 and later.
#		59 (P_LOAD) - Load overlay; Supported by: CP/M 3 and higher Loaders.
#		60 - Call to RSX; Supported by: CP/M 3 and later RSXs.
#	61 - Rename file; Supported by: DOS Plus v2.1
#	62 - Unknown; Supported by: DOS Plus v2.1
#	64 - Log in; Supported by: CP/Net.
#	65 - Log off; Supported by: CP/Net.
#	66 - Send message; Supported by: CP/Net.
#	67 - Receive message; Supported by: CP/Net.
#	68 - Get network status; Supported by: CP/Net.
#	69 - Get configuration table address; Supported by: CP/Net.
#	70 - Set compatibility attributes; Supported by: CP/Net-86, some 8-bit CP/Net versions.
#	71 - Get server configuration; Supported by: CP/Net-86, some 8-bit CP/Net versions.
#		98 - Clean up disc; Supported by: CP/M 3 (Internal?).
#		99 (F_TRUNCATE) - Truncate file; Supported by: CP/M 3 and later.
#	100 (DRV_SETLABEL) - Set directory label; Supported by: MP/M II and later.
#	101 (DRV_GETLABEL) - Get directory label byte; Supported by: MP/M II and later.
#	102 (F_TIMEDATE) - Get file date and time; Supported by: MP/M II and later.
#	103 (F_WRITEXFCB) - Set file password and protection; Supported by: MP/M II and later.
#	104 (T_SET) - Set date and time; Supported by: MP/M II and later; Z80DOS, DOS+.
#	105 (T_GET) - Get date and time; Supported by: MP/M II and later; Z80DOS, DOS+.
#	106 (F_PASSWD) - Set default password; Supported by: MP/M II and above, CP/Net redirector.
#	107 (S_SERIAL) - Get serial number; Supported by: MP/M II and above.
#		108 (P_CODE) - Get/put program return code; Supported by: CP/M 3 and above.
#		109 (C_MODE) - Set or get console mode; Supported by: Personal CP/M; CP/M 3 and above
#		110 (C_DELIMIT) - Get/set string delimiter; Supported by: Personal CP/M; CP/M 3 and above
#		111 (C_WRITEBLK) - Send block of text to console; Supported by: Personal CP/M; CP/M 3 and above
#		112 (L_WRITEBLK) - Send block of text to printer; Supported by: Personal CP/M; CP/M 3 and above
#	113 - Direct screen functions; Supported by: Personal CP/M.
#	115 - Reserved for GSX; Supported by: GSX (Graphics System Extension)
#	116 - Set file date & time; Supported by: CP/M-86 v4.
#	117 - BDOS v4.x internal; Supported by: CP/M-86 v4.
#	124 - Byte block copy; Supported by: Personal CP/M.
#	125 - Byte block alter; Supported by: Personal CP/M.
#	128 (M_ALLOC) - Absolute memory request; Supported by: MP/M, Concurrent CP/M
#	129 (M_ALLOC) - Relocatable memory request; Supported by: MP/M
#	130 (M_FREE) - Free memory; Supported by: MP/M, Concurrent CP/M
#	131 (DEV_POLL) - Poll I/O device; Supported by: MP/M, Concurrent CP/M
#	132 (DEV_WAITFLAG) - Wait on system flag; Supported by: MP/M, Concurrent CP/M
#	133 (DEV_SETFLAG) - Set system flag; Supported by: MP/M, Concurrent CP/M
#	134 (Q_MAKE) - Create message queue; Supported by: MP/M, Concurrent CP/M
#	135 (Q_OPEN) - Open message queue; Supported by: MP/M, Concurrent CP/M
#	136 (Q_DELETE) - Delete message queue; Supported by: MP/M, Concurrent CP/M
#	137 (Q_READ) - Read from message queue; Supported by: MP/M, Concurrent CP/M
#	138 (Q_CREAD) - Conditionally read from message queue; Supported by: MP/M, Concurrent CP/M
#	139 (Q_WRITE) - Write to message queue; Supported by: MP/M, Concurrent CP/M
#	140 (Q_CWRITE) - Conditionally write to message queue; Supported by: MP/M, Concurrent CP/M
#	141 (P_DELAY) - Delay; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
#	142 (P_DISPATCH) - Call dispatcher; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
#	143 (P_TERM) - Terminate process; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
#	144 (P_CREATE) - Create a subprocess; Supported by: MP/M, Concurrent CP/M
#	145 (P_PRIORITY) - Set process priority; Supported by: MP/M, Concurrent CP/M
#	146 (C_ATTACH) - Attach console; Supported by: MP/M, Concurrent CP/M
#	147 (C_DETACH) - Detach console; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
#	148 (C_SET) - Set console; Supported by: MP/M, Concurrent CP/M
#	149 (C_ASSIGN) - Assign console; Supported by: MP/M, Concurrent CP/M
#	150 (P_CLI) - Send CLI command; Supported by: MP/M, Concurrent CP/M
#	151 (P_RPL) - Call resident procedure library; Supported by: MP/M, Concurrent CP/M
#		152 (F_PARSE) - Parse filename; Supported by: MP/M, CP/M 3 and higher.
#	153 (C_GET) - Return console number; Supported by: MP/M, Concurrent CP/M
#	154 (S_SYSDAT) - System data address; Supported by: MP/M, Concurrent CP/M, CP/M-86 v4.
#	155 (T_SECONDS) - Get date and time; Supported by: MP/M, Concurrent CP/M
#	156 (P_PDADR) - Return address of process descriptor; Supported by: MP/M 2, Concurrent CP/M
#	157 (P_ABORT) - Abort a process; Supported by: MP/M 2, Concurrent CP/M
#	158 (L_ATTACH) - Attach printer; Supported by: MP/M 2, Concurrent CP/M
#	159 (L_DETACH) - Detach printer; Supported by: MP/M 2, Concurrent CP/M
#	160 (L_SET) - Select printer; Supported by: MP/M 2, Concurrent CP/M
#	161 (L_CATTACH) - Conditionally attach printer; Supported by: MP/M 2, Concurrent CP/M
#	162 (C_CATTACH)- Conditionally attach console; Supported by: MP/M 2, Concurrent CP/M
#	163 (S_OSVER) - Return version number; Supported by: DOSPLUS, MP/M 2, Concurrent CP/M
#	164 (L_GET) - Return default printer device number; Supported by: MP/M 2, Concurrent CP/M
#	175 - Return real drive ID; Supported by: DOSPLUS v2.1
#	P2DOS function 201 - set time; This has the same functionality under P2DOS as function 104 does under Z80DOS and DOS+.
#	DOS+ function 210 - Return system information; Supported by: DOS+
#	DOS+ function 211 - Print decimal number; Supported by: DOS+



# Status bytes for internal protocol (keep in-sync with bdos_host.h)
BDOS_HOST_CHECK_INIT = 0x55
BDOS_HOST_STATUS_ACK = 0xAA
BDOS_HOST_STATUS_NAK = 0x99
BDOS_HOST_STATUS_OK = 0x00
BDOS_HOST_STATUS_DMA_EOF = 0x1A
BDOS_HOST_STATUS_DMA_DATA = 0x20


def put(txt):
	print('bdos_host: %s' % txt)

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


class BDOS_Host(Host):
	"""The VGL CP/M BDOS talks to this counter part as a virtual disk and/or monitor"""
	
	def __init__(self, driver, protocol, mounts=BDOS_MOUNTS, chunk_throttle=DMA_THROTTLE):
		Host.__init__(self, driver=driver, protocol=protocol)
		
		self.mounts = mounts
		self.chunk_throttle = chunk_throttle
		
		# Open bin file
		with open(DEFAULT_BIN_FILENAME, 'rb') as h:
			self.bin_data = h.read()
		
		self.dat_data = None
		
		self.dir_list = None
		self.dir_i = 0
	
	def handle_frame(self, data):
		"Handle one complete FRAME of data"
		
		#put('frame: %s' % (str(data)))
		if SHOW_TRAFFIC: put('<< frame: (%d) [%s]' % (len(data), ' '.join([ '0x%02X'%b for b in data])))
		if (len(data) == 0): return False
		
		realm = data[0]
		if realm == ord('F'):
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
			put('FCB func #%d to FCB @ %04X: file drive=%d "%s.%s": ex=%02X, s1=%02X, s2=%02X, rc=%02X, cr=%02X' % (num, addr, fcb_dr, str_hex(fcb_name), str_hex(fcb_typ), fcb_ex, fcb_s1, fcb_s2, fcb_rc, fcb_cr))
			
			#@TODO: When using sfirst: dr = '?' (0x3f) means: Show metainfo / labels etc.
			if fcb_dr == 0x3f: fcb_dr = 0
			if fcb_dr > len(self.mounts):
				put('Unknown FCB DR=%d - using default drive' % fcb_dr)
				path = self.mounts[0]
			else:
				path = self.mounts[fcb_dr]
			
			if (num == BDOS_FUNC_F_OPEN): # 15 = Open file
				#full_filename = os.path.join(DATA_PATH, filename)
				full_filename = os.path.join(path, filename)
				put('F: Open file %s:%s ("%s")...' % (chr(ord('A') + fcb_dr), filename, full_filename))
				
				# Try lower case
				if not os.path.isfile(full_filename):
					full_filename = os.path.join(path, filename.lower())
				
				fcb_s2 = fcb[14] = fcb_s2 | 0x80	# Seems to be set, no matter if file found or not
				
				if not os.path.isfile(full_filename):
					# File was not found
					put('File "%s" was NOT found!' % full_filename)
					self.reply_frame([0xff])
					return
				
				# File was found, OK
				with open(full_filename, 'rb') as h:
					self.dat_data = h.read()
				
				file_size = len(self.dat_data)
				file_ofs = 0
				
				# Alter FCB
				#@FIXME: Fill out correct values!
				fcb_ex = fcb[12] = 0	#((file_size - file_ofs) % 524288) // 16384	# Current extent
				fcb_s1 = fcb[13] = 2	# 2? Number of extents?
				fcb_s2 = fcb[14] = ((file_size - file_ofs) // 524288) | 0x80	# Extent high byte; 0x80 is an "open" flag?
				fcb_rc = fcb[15] = min(0x80, ((file_size - file_ofs) // 128) )	# Record count in current extent, 0x80=continues...
				#fcb_d = ...	# Image of the second half of the directory entry, containing the file's allocation (which disc blocks it owns).
				fcb_d[:] = fcb[16:32] = [0x10, 0x00, 0x11, 0x00, 0x12, 0x00, 0x13, 0x00, 0x14, 0x00, 0x15, 0x00, 0x16, 0x00, 0x17, 0x00]
				fcb_cr = fcb[32] = 0	#(file_ofs % 16384) // 128	# Current record within extent
				fcb_r0 = fcb[33] = 0
				fcb_r1 = fcb[34] = 0
				fcb_r2 = fcb[35] = 0
				
				self.reply_frame([0x00] + fcb)
				return
			
			if (num == BDOS_FUNC_F_CLOSE): # 16 = Close file
				put('F: Close file "%s"' % filename)
				self.dat_data = None
				return
			
			if (num == BDOS_FUNC_F_SFIRST): # 17 = Search first
				# rewind!
				path_list = sorted(os.listdir(path))
				self.dir_list = [f for f in path_list if os.path.isfile(os.path.join(path, f))]
				put('Complete dir list of "%s": %s' % (path, str(self.dir_list)))
				self.dir_i = 0
				# continue...
			
			if (num == BDOS_FUNC_F_SFIRST) or (num == BDOS_FUNC_F_SNEXT): # Search (any)
				
				if (self.dir_list is None):
					put('! dir_list is empty!')
					self.reply_frame([0xff])
					return
				
				if (self.dir_i >= len(self.dir_list)):
					put('! dir_list has ended!')
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
				ex = (file_size % 524288) // 16384
				rc = min(0x80, (file_size % 16384) // 128)
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
				
				self.reply_frame([0x00] + fcb)	# Result = 0-3 = OK, 0xff = error
				return
			
			
			ofs = 0
			if (num == BDOS_FUNC_F_READ):	# 20 = Read sequencial
				#put('Read sequencial...')
				#ofs = (fcb_ex * 16384) + (fcb_cr * 128)
				
				rn = ((fcb_s2 & 0x0f) * 16384) + (fcb_ex * 128) + fcb_cr
				ofs = rn * 128
			
			if (num == BDOS_FUNC_F_READRAND):	# 33 = Read rand
				put('Read random...')
				# Convert from r0-r2 to cr,ex,s2
				#fcb->cr = fcb->r0 & 0x7f;	// bits 0...6 of r0
				#fcb->ex = ((fcb->r0 & 0x80) >> 7) | ((fcb->r1 & 0x0f) << 1);	// bit 7 of r0 + bits 0...3 of r1
				#fcb->s2 = ((fcb->r1 >> 4) & 0x0f);	// bits 4...7 of r1
				
				# Simply interpret r0-r2 as 24bit
				fcb_rr = ((fcb_r2 & 1) << 16) + (fcb_r1 << 8) + fcb_r0
				ofs = fcb_rr * 128	# Convert "record num" to file offset
				
			if (num == BDOS_FUNC_F_READ) or (num == BDOS_FUNC_F_READRAND):	# Read (any)
				if self.dat_data is None:
					put('!!! Read without data / closed file!')
					self.reply_frame([0xff])	# 0xFF = Hardware error
					return
				
				l_all = len(self.dat_data)
				
				put('Read data from file %s:"%s" at ofs=%d / %d' % (chr(ord('A') + fcb_dr), filename, ofs, l_all))
				
				if ofs >= l_all:
					put('Read at or beyond end of file. Returning EOF.')
					self.reply_frame([0x01])	# 0x01 = EOF
					return
				
				# Prepare DMA data to return
				data = []
				max_read = 128	# one DMA buffer = 128 bytes
				#l = min(max_read, l_all - ofs)	# Read at max 128 bytes
				
				is_eof = False
				for i in range(max_read):
					if (ofs + i < l_all):
						#b = ord(self.dat_data[ofs + i])	# Python 2
						b = self.dat_data[ofs + i]	# Python 3
						data.append(b)
					else:
						is_eof = True
						break
				
				# Send immediate status
				self.reply_frame([0x00])	# 0x00 = OK
				
				#put('Sending DMA data (%d bytes)...' % len(data))
				# Send sector in smaller chunks (if communication is flaky and frequent re-transmissions are expected)
				o = 0
				
				# MTU / Maximum transmission unit
				l2 = DMA_MTU
				#l2 = 16
				##l2 = 32	# 32 works quite OK
				#l2 = 48
				#l2 = 64	# 64 works kind-of... but 8-bit checksum might not be sufficient
				#l2 = 96	# Too big (NAK)
				while (l2 > 0):
					#put('Sending chunk o=%d / %d' % (o, len(data)))
					d = data[o:o+l2]
					l2 = len(d)
					if (l2 == 0): break	# Only send zero-length frame on true EOF (not only "end of current payload")
					
					self.reply_frame([BDOS_HOST_STATUS_DMA_DATA] + d)	# Prepend 0x20=DMA_DATA to indicate non-EOF, so we never send zero-length packages
					o += l2
					
					# Throttle
					if self.chunk_throttle is not None:
						time.sleep(self.chunk_throttle)
					
				#
				# Was this the very last sector of this file?
				if is_eof:
					put('Sending EOF')
					self.reply_frame([BDOS_HOST_STATUS_DMA_EOF])	# Send DMA_EOF
				#put('DMA sent.')
			#
			return True
		
		put('Unknown realm 0x%02X / "%s"' % (realm, chr(realm)))
		#Host.handle_frame(data=data)
		
	



### Main

def show_help():
	put(__doc__)
	put('bdos_host.py ...')

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
	
	
	# Chose a driver
	driver = Driver_serial(port=port, baud=baud, stopbits=2)	# More stopbits = more time to process?
	#driver = Driver_MAME(cart_file='./out/cpm_cart.bin')
	
	# Chose a protocol
	#protocol = Protocol_binary()	# For MAME
	protocol = Protocol_binary_safe()	# For serial. Recommended.
	#protocol = Protocol_hex()	# Old text-only protocol
	
	# Start host
	host = BDOS_Host(driver=driver, protocol=protocol, mounts=BDOS_MOUNTS)
	host.open()
	
	if not host.is_open:
		put('Connection could not be opened. Aborting.')
		sys.exit(4)
	
	#put('Loading binary file "%s"...' % (bin_filename))
	#host.upload(bin_filename)
	host.run()
	
	put('End.')
