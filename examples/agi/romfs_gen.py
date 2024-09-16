#!/bin/python3
"""
Helper for "ROM FS".
This tools helps to bundle external data files into a ROM image.

@TODO:
	* "De-fragment": Try fitting as much into a bank without crossing the borders

2024-09-16 Bernhard "HotKey" Slawik
"""

import sys
import os
import glob	# for wildcard file specs
from optparse import OptionParser


def put(t):
	print(str(t))

#ID_PREFIX = 'RFS_'
ID_PREFIX = 'R_'

class ROMFS_File:
	def __init__(self, filename, data=b'', id=None, org_filename=None, align_size=1):
		self.filename = filename
		self.data = data
		self.id = ID_PREFIX + self.filename.replace('.', '_').upper()
		self.size = len(self.data)
		self.org_filename = org_filename if org_filename is not None else self.filename
		
		self.align_size = align_size
		
		self.data_offset = 0	# Absolute data offset (always starts at 0), unaligned
		self.aligned_offset = 0	# Absolute data offset (alsways starts at 0), aligned
		self.file_offset = 0	# Where in the ROM binary file the data starts (usually same as chip_offset)
		self.chip_offset = 0	# Where inside the physical ROM chip the data starts (usually at the same offset as inside the ROM file)
		self.mem_offset = 0	# Where in memory (inside the bank) the data starts (depends where it is bank switched to...)
		self.mem_bank = 0	# The bank where the data starts

class ROMFS:
	def __init__(self, file_offset=0, chip_offset=0, mem_offset=0, mem_bank_start=0, mem_bank_size=0x1000):
		self.files = []
		self.data_offset = 0	# Absolute data offset (always starts at 0), unaligned
		self.aligned_offset = 0	# Absolute data offset (alsways starts at 0), aligned
		self.file_offset = file_offset	# Where in the ROM binary file the data starts (usually same as chip_offset)
		self.chip_offset = chip_offset	# Where inside the physical ROM chip the data starts (usually at the same offset as inside the ROM file)
		self.mem_offset = mem_offset	# Where in memory the data starts (depends where it is bank switched to...)
		self.mem_bank_start = mem_bank_start	# At which bank to start
		self.mem_bank_size = mem_bank_size	# How big one bank is
		
		self.final_data_offset = 0
		self.final_aligned_offset = 0
		self.final_file_offset = 0
		self.final_chip_offset = 0
		self.final_mem_offset = 0
		self.final_mem_bank = 0
	
	def add_file(self, file):
		self.files.append(file)
	
	def layout(self, fix_crossing=False, verbose=False):
		"""Calculate file offsets"""
		o = self.data_offset	# Should be 0!
		o_aligned = self.aligned_offset	# Should be 0!
		o_file = self.file_offset
		o_chip = self.chip_offset
		o_mem = self.mem_offset
		o_mem_bank = self.mem_bank_start
		
		for f in self.files:
			# Align
			align = 0
			
			# Check for fine alignment
			if (f.align_size > 1) and (o_mem % f.align_size > 0):
				# Non-aligned!
				o_align = ((o_mem // f.align_size) + 1) * f.align_size
				align = o_align - o_mem
				if verbose: put(f'	* Aligning "{f.filename}" on 0x{f.align_size:04X} byte segments: From 0x{o_mem:04X} to 0x{o_align:04X} ({align} bytes wasted)')
			
			# Check for segment crossing
			if ((o_mem+align) // self.mem_bank_size) != (o_mem + align + f.size) // self.mem_bank_size:
				# Allow pushing the file across boundaries
				if fix_crossing:
					if (f.size > self.mem_bank_size):
						put(f'	* File "{f.filename}" is larger than a segment and MUST cross boundaries!')
					else:
						o_align = ((o_mem // self.mem_bank_size) + 1) * self.mem_bank_size
						align = o_align - o_mem
						put(f'	* File "{f.filename}" is pushed to next segment: From 0x{o_mem:04X} to 0x{o_align:04X} ({align} bytes wasted)')
				else:
					put(f'	* File "{f.filename}" crosses bank boundaries!')
			
			# Add the alignment padding
			#o += align	# Do not align absolute data offset!
			o_aligned += align
			o_file += align
			o_chip += align
			o_mem += align
			o_mem_bank = (o_mem // self.mem_bank_size)	# Re-calculate
			
			# Update file stats
			f.data_offset = o
			f.aligned_offset = o_aligned
			f.file_offset = o_file
			f.chip_offset = o_chip
			f.mem_offset = self.mem_offset + (o_mem % self.mem_bank_size)
			f.mem_bank = o_mem_bank
			
			# Step over the data
			o += f.size
			o_aligned += f.size
			o_file += f.size
			o_chip += f.size
			o_mem += f.size
			o_mem_bank = (o_mem // self.mem_bank_size)	# Re-calculate
		
		self.final_data_offset = o
		self.final_aligned_offset = o_aligned
		self.final_file_offset = o_file
		self.final_chip_offset = o_chip
		self.final_mem_offset = o_mem	#self.mem_offset + (o_mem % self.mem_bank_size)
		self.final_mem_bank = o_mem_bank
	
	def dump_stats(self):
		"""Return a file list with stats"""
		
		r = ''
		
		# Header
		id_size = len('id')
		filename_size = len('filename')
		for f in self.files:
			id_size = max(id_size, len(f.id))
			filename_size = max(filename_size, len(f.filename))
		
		#r += f"{'num'} {'id':<{id_size}} {'filename':<{filename_size}} {'offset':>8} {'size':>8}\n"
		#r += f"{'':-<3} {'':-<{id_size}} {'':-<{filename_size}} {'':->8} {'':->8}\n"
		r += f"{'num'} {'filename':<{filename_size}} {'id':<{id_size}} {'offset':>8} {'aligned':>8} {'file ofs':>8} {'chip ofs':>8} {'bank':>4} {'addr':>6} {'size':>8}\n"
		r += f"{'':-<3} {'':-<{filename_size}} {'':-<{id_size}} {'':->8} {'':->8} {'':->8} {'':->8} {'':->4} {'':->6} {'':->8}\n"
		
		# Each file...
		total_data_size = 0
		aligned_max = 0
		aligned_waste = 0
		o_aligned_previous = self.aligned_offset
		for i,f in enumerate(self.files):
			#r += f"{i:>3} {f.id:<{id_size}} {f.filename:<{filename_size}} 0x{f.data_offset:06X} {f.size:>8,}\n"
			r += f"{i:>3} {f.filename:<{filename_size}} {f.id:<{id_size}} 0x{f.data_offset:06X} 0x{f.aligned_offset:06X} 0x{f.file_offset:06X} 0x{f.chip_offset:06X} 0x{f.mem_bank:02X} 0x{f.mem_offset:04X} {f.size:>8,}\n"
			
			total_data_size += f.size
			aligned_max = max(aligned_max, f.aligned_offset + f.size)
			aligned_waste += (f.aligned_offset - o_aligned_previous)
			o_aligned_previous = f.aligned_offset + f.size
			
		r += f"{'':=<3} {'':=<{filename_size}} {'':=<{id_size}} {'':=>8} {'':=>8} {'':=>8} {'':=>8} {'':=>4} {'':=>6} {'':=>8}\n"
		r += f"{len(self.files):>3} {'final':<{filename_size}} {'':<{id_size}} 0x{self.final_data_offset:06X} 0x{self.final_aligned_offset:06X} 0x{self.final_file_offset:06X} 0x{self.final_chip_offset:06X} 0x{self.final_mem_bank:02X} 0x{self.final_mem_offset % self.mem_bank_size:04X} {total_data_size:>8,}\n"
		
		r += '\n'
		#r += f'Data offset  : 0x{self.data_offset:06X} / {self.data_offset:>8,} bytes / {self.data_offset/1024:6.1f} KB\n'
		r += f'Data final   : 0x{self.final_data_offset:06X} / {self.final_data_offset:>8,} bytes / {self.final_data_offset/1024:6.1f} KB\n'
		#r += f'Data size    : 0x{total_data_size:06X} / {total_data_size:>8,} bytes / {total_data_size/1024:6.1f} KB\n'
		
		#r += f'Aligned ofs. : 0x{self.aligned_offset:06X} / {self.aligned_offset:>8,} bytes / {self.aligned_offset/1024:6.1f} KB\n'
		r += f'Aligned final: 0x{self.final_aligned_offset:06X} / {self.final_aligned_offset:>8,} bytes / {self.final_aligned_offset/1024:6.1f} KB\n'
		#r += f'Aligned max  : 0x{aligned_max:06X} / {aligned_max:>8,} bytes / {aligned_max/1024:6.1f} KB\n'
		r += f'Padding waste: 0x{aligned_waste:06X} / {aligned_waste:>8,} bytes / {aligned_waste/1024:6.1f} KB\n'
		
		r += f'File offset  : 0x{self.file_offset:06X} / {self.file_offset:>8,} bytes / {self.file_offset/1024:6.1f} KB\n'
		r += f'File final   : 0x{self.final_file_offset:06X} / {self.final_file_offset:>8,} bytes / {self.final_file_offset/1024:6.1f} KB\n'
		
		r += f'Chip offset  : 0x{self.chip_offset:06X} / {self.chip_offset:>8,} bytes / {self.chip_offset/1024:6.1f} KB\n'
		r += f'Chip final   : 0x{self.final_chip_offset:06X} / {self.final_chip_offset:>8,} bytes / {self.final_chip_offset/1024:6.1f} KB\n'
		
		r += f'Memory offset: 0x{self.mem_offset:06X} / {self.mem_offset:>8,} bytes / {self.mem_offset/1024:6.1f} KB\n'
		r += f'Memory final : 0x{self.final_mem_offset:06X} / {self.final_mem_offset:>8,} bytes / {self.final_mem_offset/1024:6.1f} KB\n'
		r += f'Bank size    : 0x{self.mem_bank_size:06X} / {self.mem_bank_size:>8,} bytes / {self.mem_bank_size/1024:6.1f} KB: banks 0x{self.mem_bank_start:02X}...0x{self.final_mem_offset // self.mem_bank_size:02X}\n'
		return r
	
	def dump_h_file(self):
		"""Return a h file to access the files"""
		r = ''
		r += '#ifndef __ROMFS_GEN_H__\n'
		r += '#define __ROMFS_GEN_H__\n'
		r += '/*\n'
		r += f'*	Auto-generated by "{__file__}"\n'
		args = ' '.join(sys.argv)
		r += f'*	Command line arguments: "{str(args)}"\n'
		r += '*	\n'
		r += '\n'.join([ f'*	{l}' for l in self.dump_stats().split('\n') ]) + '\n'
		r += '*/\n'
		
		r += '\n'
		r += f'#define R_BANK_SIZE 0x{self.mem_bank_size:04X}\n'
		r += '\n'
		r += '// File look-up table\n'
		r += f'#define R_MEM_OFFSET 0x{self.mem_offset:04X}\n'
		r += f'#define R_NUM_FILES {len(self.files)}\n'
		files_var_name = 'R_FILES'
		r += f'static const romfs_entry_t {files_var_name}[{len(self.files)}] = ' + '{\n'
		#r += '	//' + f"{'bank':>3} 	{'addr':>6} 	{'size':>6}" + '\n'
		r += '	//' + f"{'bank':>3} 	{'addr':>6} 	{'banks':>4}	{'+size':>6}" + '\n'
		for f in self.files:
			#r += f'	// {f.id} ({f.filename})'+'\n'
			#r += '	{' + f'{f.mem_bank:>3},	0x{f.mem_offset:04X},	{f.size:>6}' + '},' + f'	// {f.filename}, {f.size} bytes' + '\n'
			#r += '	{' + f'{f.mem_bank:>3},	0x{f.mem_offset:04X},	{f.size // self.mem_bank_size:>2}, {f.size % self.mem_bank_size:>6}' + '},' + f'	// {f.id}: "{f.filename}", {f.size:,} bytes' + '\n'
			r += '	{' + f'{f.mem_bank:>3},	0x{f.aligned_offset % self.mem_bank_size:04X},	{f.size // self.mem_bank_size:>2}, {f.size % self.mem_bank_size:>6}' + '},' + f'	// {f.id}: "{f.filename}", {f.size:,} bytes' + '\n'
		r += '};\n'
		
		r += '\n'
		r += '// Use these to address a file entry using e.g. romfs_fopen()\n'
		r += 'enum R_File {\n'
		for f in self.files:
			r += f'	{f.id.lower()}'+', \n'
		r += '};\n'
		
		r += '\n'
		for i,f in enumerate(self.files):
			r += f'#define {f.id} {i}'+'\n'
		r += '\n'
		
		r += '#endif\n'
		return r


if __name__ == '__main__':
	
	parser = OptionParser(
		description='Bundle binary files into your ROM',
		usage='Usage: %prog [options] file_spec...'
	)
	
	align_size = 1	#0x100	#0x1000	# 0x2000	# 0x4000
	file_offset = 0x4000
	chip_offset = file_offset
	mem_offset = 0x4000	# 0x0000 or 0x4000
	mem_bank_size = 0x4000
	mem_bank_start = 0
	pad = 0xff
	fix_crossing = False
	verbose = False
	
	parser.add_option('-f', '--file-offset', dest='file_offset', default=file_offset, action='store', type='int', help=f'Offset in final file (default: 0x{file_offset:04X})')
	parser.add_option('-c', '--chip-offset', dest='chip_offset', default=chip_offset, action='store', type='int', help=f'Offset in ROM chip (default: 0x{chip_offset:04X})')
	parser.add_option('-m', '--mem-offset', dest='mem_offset', default=mem_offset, action='store', type='int', help=f'Offset in memory (default: 0x{mem_offset:04X})')
	parser.add_option('-b', '--mem-bank-start', dest='mem_bank_start', default=mem_bank_start, action='store', type='int', help=f'Number of first memory bank to use (default: {mem_bank_start})')
	parser.add_option('-s', '--mem-bank-size', dest='mem_bank_size', default=mem_bank_size, action='store', type='int', help=f'Size of each memory bank (default: 0x{mem_bank_size:04X})')
	parser.add_option('-a', '--align-size', dest='align_size', default=align_size, action='store', type='int', help=f'Alignment/padding (default: 0x{align_size:04X})')
	
	#parser.add_option('-s', '--src-ofs', dest='src_ofs', default=0, action='store', type='int', help='Source file offset (default: 0)')
	#parser.add_option('-n', '--src-num', dest='src_num', default=-1, action='store', type='int', help='Source data count (default: all)')
	
	parser.add_option('-p', '--pad', dest='pad', default=pad, action='store', type='int', help='Padding (default: %d / 0x%02X)' % (pad,pad))
	parser.add_option('-x', '--fix-crossing', dest='fix_crossing', default=fix_crossing, action='store_true', help='Try keeping files from crossing bank bounds')
	
	#parser.add_option('-t', '--keep-trailing', dest='keep_trailing', default=not skip_trailing, action='store_true', help='Keep trailing 0xFF and 0x00')
	#parser.add_option('-z', '--skip-zeros', dest='skip_zeros', default=skip_zeros, action='store_true', help='Skip zero bytes')
	
	parser.add_option('-r', '--file-src', action='store', type='string', dest='filename_src', help='Source ROM file to read')
	parser.add_option('-w', '--file-dst', action='store', type='string', dest='filename_dst', help='Destination ROM file to write')
	parser.add_option('-g', '--file-h', action='store', type='string', dest='filename_h', help='C-style .h file to generate')
	
	parser.add_option('-v', '--verbose', dest='verbose', default=verbose, action='store_true', help='Show verbose traffic')
	
	opt, args = parser.parse_args()
	
	file_offset = opt.file_offset	# 0x8000
	chip_offset = opt.chip_offset	# 0x8000
	mem_offset = opt.mem_offset	# 0x8000
	mem_bank_size = opt.mem_bank_size	# 0x2000
	mem_bank_start = opt.mem_bank_start	# 0
	align_size = max(1, opt.align_size)	#0x2000
	verbose = opt.verbose
	fix_crossing = opt.fix_crossing
	
	filename_src = opt.filename_src
	filename_dst = opt.filename_dst
	filename_h = opt.filename_h
	
	# Positional
	if len(args) == 0:
		parser.print_help()
		sys.exit(1)
	
	romfs = ROMFS(file_offset=file_offset, chip_offset=chip_offset, mem_offset=mem_offset, mem_bank_size=mem_bank_size, mem_bank_start=mem_bank_start)
	
	# Process file specs
	if verbose: put('Gathering...')
	for arg in args:
		filespec = arg
		if verbose: put(f'	* Processing spec "{filespec}"...')
		for filename in glob.glob(filespec):
			filename_abs = os.path.abspath(filename)	# Resolve relative path elements
			filename_base = os.path.basename(filename)	# Extract filename without path
			if verbose: put(f'		* filename="{filename}", filename_abs="{filename_abs}", filename_base="{filename_base}"')
			
			# Ingest!
			with open(filename, 'rb') as h:
				data = h.read()
			
			f = ROMFS_File(filename=filename_base, data=data, org_filename=filename, align_size=align_size)
			romfs.add_file(f)
		#
	#
	
	if len(romfs.files) == 0:
		put('No files matched the given spec(s).')
		sys.exit(1)
	
	
	# Lay out!
	if verbose: put('Laying out...')
	romfs.layout(fix_crossing=fix_crossing, verbose=verbose)
	
	# Dump output header file
	if filename_h is None:
		put(romfs.dump_stats())
	else:
		put(f'Writing .h file "{filename_h}"...')
		with open(filename_h, 'w') as h:
			h.write(romfs.dump_h_file())
	
	# Create output ROM file
	if filename_dst is None:
		if verbose: put('No destination ROM file specified. End.')
		sys.exit(2)
		
	
	data = b''
	if filename_src is None:
		#put(f'No source ROM file specified!')
		#sys.exit(3)
		put(f'No source ROM file specified. Using null.')
	else:
		if verbose: put(f'Reading source file "{filename_src}"...')
		with open(filename_src, 'rb') as h:
			data = h.read()
		if verbose: put(f'	* Read 0x{len(data):06X} / {len(data)} source bytes')
	
	# Process each file...
	if verbose: put('Processing...')
	for f in romfs.files:
		if verbose: put(f'	* Processing file "{f.filename}"...')
		
		# Pad output file to file offset
		if (f.file_offset > len(data)):
			d = f.file_offset - len(data)
			if verbose: put(f'		* Padding data from 0x{len(data):06X} to 0x{f.file_offset:06X}... (0x{d:04X} / {d:,} bytes)')
			data += bytes([ pad ] * d)
		if len(data) < f.file_offset:
			put(f'!!! Something went wrong: Current data size ({len(data):,}) mismatches expected file offset ({f.file_offset:,}).')
			sys.exit(9)
		
		if (len(data) > f.file_offset):
			# Overwrite
			if verbose: put(f'		* Pasting data at offset 0x{f.file_offset:06X} ({f.size:,} bytes)...')
			data = data[:f.file_offset] + f.data + data[f.file_offset+f.size:]
		else:
			# Append
			if verbose: put(f'		* Appending data at offset 0x{f.file_offset:06X} ({f.size:,} bytes)...')
			data += f.data
	
	put(f'Writing output file "{filename_dst}": {len(data):,} bytes / {len(data)/1024:6.1f} KB...')
	with open(filename_dst, 'wb') as h:
		h.write(data)
	
	if verbose: put('Done.')
	

