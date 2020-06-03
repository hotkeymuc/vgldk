#!/bin/python
"""
rel2app
=======

This script analyzes a SDCC .rel file and extracts relocation information.
Goal is to get a "loadable app" which can be loaded and re-located at run-time.

Made for my "V-Tech Genius Leader" projects using SDCC

2019-07-03 Bernhard "HotKey" Slawik

"""

WRITE_APP_BIN = True	# Write the relocatable binary to a .bin file
WRITE_APP_H = True	# Write out the APP_BIN as an includable .h header file
WRITE_RELOCATED = False	# Attempt a re-location here in python

# Which symbol names should be included in the .app file?
WRITE_ALL_SYMBOL_NAMES = False	# If True: all (!) symbol names will be written
WRITE_SYMBOL_NAMES = ['_main']	# Those symbols will always be written
WRITE_SYMBOLS_IN_AREA = ['ext']	# Symbols in this area are always written


SHOW_ALL_LINES = not True	# Display all text lines while parsing
SHOW_ADDS = False	# Log all additions of areas/symbols
SHOW_USAGES = not True	# Log all uses
DUMP_SYMBOLS = not True	# Show all symbols after parsing


LOC_CODE = 0x0000	# Should be same as compiler option --code-loc
LOC_DATA = 0xc000	# Should be same as compiler option --data-loc

AREA_NAME_EXT = 'ext'	# Name for symbols wihtout an explicit area
AREA_HEADER_BYTE_CODE = 0xC0
AREA_HEADER_BYTE_DATA = 0xDA
AREA_HEADER_BYTE_EXT = 0xE0

APP_SIGNATURE = 'HAPP'	# Maybe use some valid Z80 code?

import os	# For basename
import sys	# For failing

def put(txt):
	print(str(txt))

class Usage:
	"One usage of a symbol within a .rel file"
	def __init__(self, bin_ofs, sym_ofs=0):
		self.bin_ofs = bin_ofs	# Offset of usage within the binary chunk
		self.sym_ofs = sym_ofs	# Offset within the symbol (e.g. struct member access)
	
	def __repr__(self):
		return 'at %d / 0x%04X: sym_ofs=%d / 0x%04X' % (self.bin_ofs, self.bin_ofs, self.sym_ofs, self.sym_ofs)

class Symbol:
	"One symbol as defined at the top of a .rel file"
	def __init__(self, name, id, index=-1):
		self.name = name
		self.id = id
		self.index = index
		self.addr = 0x0000
		self.linked_addr = 0x0000
		self.size = 0
		self.usages = []
		
	
	def add_usage(self, addr):
		self.usages.append(addr)
	
	def __repr__(self):
		r = ''
		#r = 'Symbol "%s" (%s), area=%d, addr=%04X used at %s' % (self.name, self.id, self.area, self.addr, ', '.join([ '0x%04X' % u for u in self.uses]))
		r += 'Symbol "%s", id="%s", index=%d, addr=0x%04X, size=0x%04X, linked_addr=0x%04X' % (self.name, self.id, self.index, self.addr, self.size, self.linked_addr)
		if (len(self.usages) > 0):
			#r += ' usage at %s' % (', '.join([ '0x%04X' % u for u in self.usages]))
			r += ' usage at %s' % (', '.join([ str(u) for u in self.usages]))
		return r

class Area:
	"One area of symbols within a .rel file"
	def __init__(self, name=AREA_NAME_EXT, size=0):
		self.name = name
		self.size = size
		self.symbols = []
		self.index = 0 # For Ref's which we have to count manually...
		
		self.header_byte = 0x00
		if (name == '_CODE'): self.header_byte = AREA_HEADER_BYTE_CODE
		elif (name == '_DATA'): self.header_byte = AREA_HEADER_BYTE_DATA
		elif (name == AREA_NAME_EXT): self.header_byte = AREA_HEADER_BYTE_EXT
	
	def add_symbol(self, sym):
		
		# Guess addresses
		if (sym.id[:3] == 'Def'):	# Convert "Def1234" to addr=0x1234
			sym.addr = int(sym.id[3:], 16) 
			#put('(assuming address 0x%04X from name)' % (sym.addr))
		
		if (sym.id == 'Ref0000'):	# Keep track of unknown "Ref0000"
			#sym.addr = self.current_ofs
			#sym.size = 2	# Ref size: 16 bit
			#self.current_ofs += sym.size
			#self.current_ref += 1
			#self.size += sym.size	# _global area starts off as 0 size
			#put('(adding external reference "%s" to area "%s" at 0x%04X)' % (sym.name, self.name, sym.addr))
			pass
		
		sym.index = self.index
		
		self.symbols.append(sym)
		
		self.index += 1
		
		# Calculate sizes each time a new symbol is added
		# You could also do it once in the end
		self.calculate_symbol_sizes()
		
	def calculate_symbol_sizes(self):
		"Lays out all symbols in memory and calculates their respective sizes in between to guess the size"
		
		# Sort all symbols descending...
		sorted_symbols = sorted(self.symbols, key=lambda sym: sym.addr, reverse=True)
		#put('Sorted: ' + str(sorted_symbols))
		
		# Now calculate their sizes (against the previous highest address)
		upper_bound = self.size	# Start at the end of the area
		for sym in sorted_symbols:
			sym.size = upper_bound - sym.addr
			upper_bound = sym.addr
		
	def symbol_by_address(self, addr):
		"Returns the symbol (and offset within) which spans the given address"
		sorted_symbols = sorted(self.symbols, key=lambda sym: sym.addr, reverse=True)
		for sym in sorted_symbols:
			if sym.addr <= addr:
				sym_ofs = (addr - sym.addr)
				return sym, sym_ofs
		
		put('Unknown symbol address 0x%04X in %s' % (addr, str(self)))
		sys.exit(3)
		return None
	
	def symbol_by_index(self, i):
		for sym in self.symbols:
			if sym.index == i:
				return sym
		
		put('Unknown symbol index %d in %s' % (i, str(self)))
		sys.exit(4)
		return None
	
	def __repr__(self):
		return 'Area "%s", size=%d / 0x%04X' % (self.name, self.size, self.size)

def process_rel(filename_base):
	"Parse the given .rel file (without extension) and create a loadable app from it"
	
	filename_rel = filename_base + '.rel'
	filename_bin_linked = filename_base + '.bin'
	filename_bin_relocated = filename_base + '.relocated.bin'
	filename_app_bin = filename_base + '.app.bin'
	filename_app_h = filename_base + '.app.h'
	
	### Load the linked binary (to get some auto-generated calls)
	linked_bin_data = []
	with open(filename_bin_linked, 'rb') as h:
		linked_bin_d = h.read()
		for c in linked_bin_d:
			linked_bin_data.append(ord(c))
	
	rel_bin_data = []
	current_addr = 0x0000	# Offset of last text line (to keep "T" and "R" in sync)
	
	areas = dict()
	current_area = Area(name=AREA_NAME_EXT)	# Global/ext area
	areas[current_area.name] = current_area
	
	
	put('Parsing .rel file "%s"...' % filename_rel)
	
	for l in open(filename_rel, 'r'):
		l = l.strip()
		if len(l) < 1: continue
		if SHOW_ALL_LINES: put('\t"%s"' % l)
		
		parts = l.split(' ')
		if (l[0] == 'A'):
			# Area
			area_name = parts[1]
			area_size = int(parts[3], 16) if (parts[2] == 'size') else 0
			# flags=parts[5]
			# addr=parts[7]
			
			current_area = Area(name=area_name, size=area_size)
			if SHOW_ADDS:
				put('Adding %s' % str(current_area))
			
			#areas.append(current_area)
			areas[current_area.name] = current_area
			
		elif (l[0] == 'S'):
			# SYMBOL
			sym_name = parts[1]
			sym_id = parts[2]
			
			sym = Symbol(sym_name, sym_id)
			current_area.add_symbol(sym)
			
			if SHOW_ADDS:
				put('Adding %s' % str(sym))
		
		elif (l[0] == 'T'):
			# TEXT
			# T address16 [data...]
			
			# Extract address
			current_addr = int(parts[1], 16) + 0x100 * int(parts[2], 16)
			
			o = 3	# Start at text item #3
			addr = current_addr
			while (o < len(parts)):
				b = int(parts[o], 16)
				
				# Pad up to address
				while (addr >= len(rel_bin_data)): rel_bin_data.append(0xff)
				
				# Store data
				rel_bin_data[addr] = b
				
				# Inc
				addr += 1
				o += 1
			#
		elif (l[0] == 'R'):
			# Relocation stuff
			# Skip initial 4s
			if (l[:13] != 'R 00 00 00 00'):
				#put('Unexpected relocation start: "%s" (expected zeros)' % l)
				continue
			
			o = 5
			while (o < len(parts)):
				
				### Parse a relocation
				rel_type = int(parts[o], 16)	# 0x00 = memory, 0x02 = external?
				rel_ofs = int(parts[o+1], 16)
				rel_index = int(parts[o+2], 16) + 0x100 * int(parts[o+3], 16)	# 0000 = CODE, 0001 = DATA, ...
				
				#put('Relocation: type=%02X at buffer offset %d, area=%04X' % (rel_type, rel_ofs, rel_index))
				
				# R points to the actual hex tuple in previous line. We need to change that to absolute binary offset
				rel_ofs_abs = current_addr + rel_ofs - 2
				
				### Get the memory address (from binary data) to which the relocation points to
				# Caution! This address might point INTO a struct, so it is ABOVE the struct's known base address
				org_addr = rel_bin_data[rel_ofs_abs] + 0x100 * rel_bin_data[rel_ofs_abs+1]
				
				# Also get the address the linker put in the actual binary (interesting for auto-generated code that is not included in the .rel!)
				linked_addr = linked_bin_data[rel_ofs_abs] + 0x100 * linked_bin_data[rel_ofs_abs+1]
				
				#put('Relocation: rel_type=%02X, rel_index=%d at binary offset %d / 0x%04X: org_addr=%04X, linked_addr=%04X' % (rel_type, rel_index, rel_ofs_abs, rel_ofs_abs, org_addr, linked_addr))
				
				### Resolve to area, symbol and offset (by using the known R-data and "address")
				
				# Find symbol and create a "usage"
				a = None
				sym = None
				u = None
				if (rel_type == 0x00):
					
					if (rel_index == 0):	# Calling a function in program's scope (by address)
						a = areas['_CODE']
					elif (rel_index == 1):	# Accessing a variable (by address)
						a = areas['_DATA']
					else:
						put('Referencing unknown rel_index %d!' % rel_index)
						sys.exit(2)
					
					sym, sym_ofs = a.symbol_by_address(org_addr)
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)
					
					
				elif (rel_type == 0x02):
					# Calling an external function (e.g. auto-generated subs like "___sdcc_call_hl" or external undefined externals)
					a = areas[AREA_NAME_EXT]
					sym = a.symbol_by_index(rel_index)
					sym_ofs = org_addr
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)
				
				else:
					put('Unknown rel_type 0x%02X' % (rel_type))
					sys.exit(5)
				
				if SHOW_USAGES:
					put('Usage of symbol "%s" (%s, addr=0x%04X, linked_addr=0x%04X), sym_ofs=%d at binary position %d / 0x%04X' % (sym.name, a.name, sym.addr, sym.linked_addr, u.sym_ofs, rel_ofs_abs, rel_ofs_abs))
				sym.add_usage(u)
				
				
				# Store the sym_ofs inside the binary to safe space in the relocation table?
				#old = linked_bin_data[u.bin_ofs] + 0x100 * linked_bin_data[u.bin_ofs+1]
				#put('Altering binary: Putting sym_ofs=0x%04X for linked_addr=0x%04X at bin_ofs=0x%04X (old value=0x%04X)' % (u.sym_ofs, sym.linked_addr, u.bin_ofs, old))
				#linked_bin_data[u.bin_ofs] = u.sym_ofs % 0x100
				#linked_bin_data[u.bin_ofs + 1] = u.sym_ofs >> 8
				
				
				# Next R tuple
				o += 4
			#
		#
	#
	
	put('Parsing done.')
	
	put('Parsed .rel size=%d bytes' % (len(rel_bin_data)))
	put('Linked .bin size=%d bytes' % (len(linked_bin_data)))
	
	if DUMP_SYMBOLS:
		put('Symbols:')
		for area_name, a in areas.iteritems():
			put('\t* %s' % str(a))
			for sym in a.symbols:
				put('\t\t* %s' % str(sym))
				
			
		
	
	
	### Create .app structure
	app_data = []
	def write_byte(b):
		app_data.append(b)
	def write_word(w):
		app_data.append(w % 0x100)
		app_data.append(w >> 8)
	def write_str(s):
		write_byte(len(s))
		for c in s:
			write_byte(ord(c))
	
	### Write app_data header file
	#put('Binary size: %d / 0x%04X' % (len(rel_bin_data), len(rel_bin_data)))
	#write_word(len(rel_bin_data))	# Length of binary data
	
	# Write signature
	for c in APP_SIGNATURE:
		write_byte(ord(c))
	
	put('Binary size: %d / 0x%04X' % (len(linked_bin_data), len(linked_bin_data)))
	write_word(len(linked_bin_data))	# Length of binary data
	
	# Write binary data
	#for b in rel_bin_data:
	for b in linked_bin_data:
		write_byte(b)
	
	
	### Write areas/symbols to app_data
	put('Writing relocation information...')
	# Count actually used symbols
	areas_used = 0
	for area_name, a in areas.iteritems():
		# Count syms used in this area
		syms_used = 0
		for sym in a.symbols:
			if len(sym.usages) > 0: syms_used += 1
		if syms_used > 0:
			areas_used += 1
	
	write_word(areas_used)	# Number of (actual used) areas
	
	# Write out all used areas
	for area_name, a in areas.iteritems():
		
		# Count syms used in this area (again...)
		syms_used = 0
		for sym in a.symbols:
			if len(sym.usages) > 0: syms_used += 1
		if syms_used == 0: continue
		
		# Write area header
		put('  * Area "%s"' % (a.name))
		#write_str(a.name)	# Write full area name for better debugging
		write_byte(a.header_byte)	# Save space by just writing out an identification byte
		
		# Write areas's (used) symbols
		write_word(syms_used)
		for sym in a.symbols:
			if len(sym.usages) == 0: continue
			
			# Write symbol header
			put('    * Symbol "%s" (used %d times)' % (sym.name, len(sym.usages)))
			
			# To safe space: Only include important symbols
			if WRITE_ALL_SYMBOL_NAMES or (sym.name in WRITE_SYMBOL_NAMES) or (a.name in WRITE_SYMBOLS_IN_AREA):
				# Include full name of symbol (for symbolic linking)
				write_str(sym.name)
			else:
				# Unimportant symbol, do not include a name (safe space)
				write_str('')	# ...or just "0x00" for empty string
			
			#write_word(sym.addr)
			write_word(sym.linked_addr)
			
			# Write symbol's usages
			write_word(len(sym.usages))	# Number of usages of this symbol
			for u in sym.usages:
				write_word(u.bin_ofs)
				#write_word(u.sym_ofs)	# This can be looked up at bin_ofs! The value at bin_ofs = sym.linked_addr + u.sym_ofs
				
			
		
	
	put('Relocatable .app size=%d bytes' % (len(app_data)))
	### End of .app structure
	
	
	### Write files
	l = len(app_data)
	
	if WRITE_APP_BIN:
		put('Writing app .bin file "%s"...' % filename_app_bin)
		with open(filename_app_bin, 'wb') as h:
			h.write(bytearray(app_data))
	
	if WRITE_APP_H:
		
		data_name = os.path.basename(filename_base).upper()
		
		put('Writing app .h file "%s" with data named "%s[]"...' % (filename_app_h, data_name))
		with open(filename_app_h, 'w') as h:
			h.write('const unsigned char %s[%d] = {\n\t' % (data_name, l))
			col = 0
			i = 0
			for b in app_data:
				h.write('0x%02X' % b)
				if (i < l-1): h.write(', ')
				
				i += 1
				col += 1
				if (col >= 16):
					h.write('\n\t')
					col = 0
			
			if (col > 0): h.write('\n')
			h.write('};\n')
	
	
	"""
	#@FIXME: Not working atm.!
	if WRITE_RELOCATED:
		### Relocate binary (just for testing - will be done on runtime later on)
		put('Relocating binary...')
		
		new_offsets = {
			'_CODE': 0xc800,
			'_DATA': 0xc400,
			
			'_EXT': 0x8000
		}
		
		for id,sym in symbols.iteritems():
			if len(sym.uses) == 0:
				continue
			
			new_addr = sym.addr
			
			sym_area_name = areas[sym.area]
			new_addr = sym.addr + new_offsets[sym_area_name]
			put('Relocating symbol "%s" (%s) from 0x%04X to 0x%04X in %d places...' % (sym.name, sym_area_name, sym.addr, new_addr, len(sym.uses)))
			
			for u in sym.uses:
				# Replace all uses of this symbol by the new address
				#put('\t* Used at offset %d...' % (u))
				
				# Check if binary confirms that
				org_addr = data[u] + 0x100 * data[u+1]
				if (org_addr != sym.addr):
					put('!! Patch differs! Expected symbol address 0x%04X, but binary says 0x%04X !!' % (sym.addr, org_addr))
					return False
				
				# Apply patch
				data[u] = new_addr % 0x100
				data[u+1] = new_addr >> 8
				
			#
		#
		put('Relocation finished.')
		
		# Write out
		put('Writing relocated bin file "%s"...' % filename_bin_relocated)
		with open(filename_bin_relocated, 'wb') as h:
			h.write(bytearray(data))
	#
	"""
	
	put('Done processing "%s".' % (filename_base))


if __name__ == '__main__':
	#put(__doc__)
	import sys
	
	if (len(sys.argv) > 1):
		filename_base = sys.argv[1]
	else:
		filename_base = 'out/app_test'
	
	process_rel(filename_base)
	
	sys.exit(0)
