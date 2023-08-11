#!/usr/bin/python3
"""
REL File loader
===============

This script analyzes a SDCC .rel file and extracts relocation information.
It is made without documentation, a lot of trial and error.

TODO:
	Check .noi file for symbol offsets

2019-2023-08-11 Bernhard "HotKey" Slawik
"""

SHOW_ALL_LINES = not True	# Display all text lines while parsing
SHOW_ADDS = False	# Log all additions of areas/symbols
SHOW_USAGES = not True	# Log all uses
DUMP_SYMBOLS = not True	# Show all symbols after parsing


def put(txt):
	print(str(txt))


AREA_NAME_CODE = '_CODE'
AREA_NAME_DATA = '_DATA'
AREA_NAME_EXT = 'global'	# Name for symbols wihtout an explicit area

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
	def __init__(self, name=AREA_NAME_EXT, size=0, area_num=0):
		self.name = name
		self.size = size
		self.symbols = []
		self.area_num = area_num
		self.index = 0 # For Ref's which we have to count manually...
		
	
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
		
		#put('Unknown symbol address 0x%04X in %s' % (addr, str(self)))
		#sys.exit(3)
		return None, None
	
	def symbol_by_index(self, i):
		for sym in self.symbols:
			if sym.index == i:
				return sym
		
		#put('Unknown symbol index %d in %s' % (i, str(self)))
		#sys.exit(4)
		return None
	
	def __repr__(self):
		return 'Area "%s", size=%d / 0x%04X' % (self.name, self.size, self.size)

def process_rel_file(filename_rel, filename_bin_linked=None):
	"Reads and parses the given .rel file (without extension). If linked binary is availble, it is loaded for reference."
	
	### Load the linked binary (to get some auto-generated calls)
	if filename_bin_linked is not None:
		put('Loading linked binary for reference: "%s"...' % filename_bin_linked)
		linked_bin_data = []
		with open(filename_bin_linked, 'rb') as h:
			linked_bin_d = h.read()
			for c in linked_bin_d:
				#linked_bin_data.append(ord(c))	# Python2
				linked_bin_data.append(c)	# Python3
	else:
		# Ignore linked binary
		linked_bin_data = None
	
	rel_bin_data = []
	current_addr = 0x0000	# Offset of last text line (to keep "T" and "R" in sync)
	
	put('Parsing .rel file "%s"...' % filename_rel)
	
	line_num = 0
	area_num = 0
	areas = dict()
	areas_by_num = dict()
	current_area = None	# See "H" header
	
	# Read line-wise
	for l in open(filename_rel, 'r'):
		line_num += 1
		l = l.strip()
		if len(l) < 1: continue
		if SHOW_ALL_LINES: put('\t"%s"' % l)
		
		parts = l.split(' ')
		if (l[0] == 'H'):
			
			# Header
			area_count = int(parts[1], 16)
			global_sym_count = int(parts[3], 16)
			
			# Prepare EXT area for global symbols outside any given area
			put('Adding EXT area as last area #%d' % area_count)
			current_area = Area(name=AREA_NAME_EXT, area_num=area_count)	# Global/ext area
			areas[current_area.name] = current_area
			areas_by_num[current_area.area_num] = current_area	#@FIXME: Is it always "9" or always the LAST one? (see "H" header)
			area_num = 0
			
		elif (l[0] == 'A'):
			# Area
			area_name = parts[1]
			area_size = int(parts[3], 16) if (parts[2] == 'size') else 0
			area_flags = parts[5]	# Unhandled, yet
			area_addr = parts[7]	# Unhandled, yet
			
			put('Adding area #%d: "%s"' % (area_num, area_name))
			
			current_area = Area(name=area_name, size=area_size, area_num=area_num)
			if SHOW_ADDS:
				put('Adding area %s' % str(current_area))
			
			#areas.append(current_area)
			areas[current_area.name] = current_area
			areas_by_num[current_area.area_num] = current_area
			area_num += 1
			
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
			
			o = 3	# Start at part[3] (first parts were "T" <ALo> <AHi>)
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
			
			#put('Line %d: Relocation header: "%s"' % (line_num, l))
			
			# @FIXME: Skip initial 4 zeros, because I don't know their meaning
			if (l[:13] != 'R 00 00 00 00'):
				put('Line %d: Ignoring unknown relocation header: "%s" (expected all zeros)' % (line_num, l))
				#continue
			rel_unknown_1 = int(parts[1], 16)
			rel_unknown_2 = int(parts[2], 16)
			rel_unknown_3_4 = int(parts[3], 16) + 0x100 * int(parts[4], 16)	#@TODO: Default area?
			
			o = 5
			while (o < len(parts)):
				
				### Parse a relocation
				rel_type = int(parts[o], 16)	# 0x00 = memory, 0x02 = external?, 0x09=??, 0x89=???
				rel_ofs = int(parts[o+1], 16)
				rel_index = int(parts[o+2], 16) + 0x100 * int(parts[o+3], 16)	# 0000 = CODE, 0001 = DATA, ...
				#put('Relocation: type=%02X at offset=%d, area=%04X' % (rel_type, rel_ofs, rel_index))
				
				# R points to the actual hex tuple in previous line. We need to change that to absolute binary offset
				rel_ofs_abs = current_addr + rel_ofs - 2
				
				### Get the memory address (from binary data) to which the relocation points to
				# Caution! This address might point INTO a struct, so it is ABOVE the struct's known base address
				org_addr = rel_bin_data[rel_ofs_abs] + 0x100 * rel_bin_data[rel_ofs_abs+1]
				
				if linked_bin_data is None:
					# Linked binary is not available
					linked_addr = 0xffff
				else:
					# Also get the address the linker put in the actual binary (interesting for auto-generated code that is not included in the .rel!)
					linked_addr = linked_bin_data[rel_ofs_abs] + 0x100 * linked_bin_data[rel_ofs_abs+1]
				
				### Resolve to area, symbol and offset (by using the known R-data and "address")
				
				# Find area with given rel_index
				a = None
				for area_name, a2 in areas.items():
					if a2.area_num == rel_index:
						a = a2
						break
				else:
					put('Line %d: Unhandled relocation due to unknown area rel_index: rel_type=%02X, rel_index=%d at offset=%d / 0x%04X: org_addr=%04X, linked_addr=%04X' % (line_num, rel_ofs_abs, rel_ofs_abs, rel_type, rel_index, org_addr, linked_addr))
					put('Skipping!')
					break
				
				# Find symbol and create a "usage"
				u = None
				sym = None
				if (rel_type == 0x00):
					
					sym, sym_ofs = a.symbol_by_address(org_addr)
					
					put('Line %d / offset %d / 0x%04X: Normal Relocation (rel_type=%02X), rel_index=%d: org_addr=%04X, linked_addr=%04X (Area %s: Symbol "%s")' % (line_num, rel_ofs_abs, rel_ofs_abs, rel_type, rel_index, org_addr, linked_addr, a.name, sym.name if sym is not None else '???'))
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)
				
				elif (rel_type == 0x02):
					# Calling an external/global symbol by its INDEX (e.g. auto-generated subs like "___sdcc_call_hl" or external undefined externals)
					a = areas[AREA_NAME_EXT]
					sym = a.symbol_by_index(rel_index) # Symbol by INDEX! (Because its address is unknown/0x0000)
					sym_ofs = org_addr
					
					put('Line %d / offset %d / 0x%04X: Global Relocation (rel_type=%02X), rel_index=%d: org_addr=%04X, linked_addr=%04X (Area %s: Symbol "%s")' % (line_num, rel_ofs_abs, rel_ofs_abs, rel_type, rel_index, org_addr, linked_addr, a.name, sym.name if sym is not None else '???'))
					
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)
				
				elif (rel_type == 0x09):
					# Loading the LOW BYTE of the pointer to an array, e.g. "ld	a,#<(_lcd_map_4rows)"
					sym, sym_ofs = a.symbol_by_address(org_addr)
					
					sym, sym_ofs = a.symbol_by_address(org_addr)
					put('Line %d / offset %d / 0x%04X: LO addr (rel_type=%02X), rel_index=%d: org_addr=%04X, linked_addr=%04X (Area %s: Symbol "%s")' % (line_num, rel_ofs_abs, rel_ofs_abs, rel_type, rel_index, org_addr, linked_addr, a.name, sym.name if sym is not None else '???'))
					
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)	#@TODO: Usage of the LO BYTE of the ADDRESS of this symbol
				elif (rel_type == 0x89):
					# Loading the HIGH BYTE of the pointer to an array, e.g. "ld	a,#>(_lcd_map_4rows)"
					sym, sym_ofs = a.symbol_by_address(org_addr)
					
					sym, sym_ofs = a.symbol_by_address(org_addr)
					put('Line %d / offset %d / 0x%04X: HI addr (rel_type=%02X), rel_index=%d: org_addr=%04X, linked_addr=%04X (Area %s: Symbol "%s")' % (line_num, rel_ofs_abs, rel_ofs_abs, rel_type, rel_index, org_addr, linked_addr, a.name, sym.name if sym is not None else '???'))
					
					sym.linked_addr = linked_addr - sym_ofs	# Update linked address
					u = Usage(bin_ofs=rel_ofs_abs, sym_ofs=sym_ofs)	#@TODO: Usage of the HI BYTE of the ADDRESS of this symbol
					
				else:
					put('Line %d: Unknown rel_type: rel_type=%02X, rel_index=%d at offset=%d / 0x%04X: org_addr=%04X, linked_addr=%04X' % (line_num, rel_type, rel_index, rel_ofs_abs, rel_ofs_abs, org_addr, linked_addr))
					#sys.exit(5)
					put('Skipping!')
					break
				
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
	if linked_bin_data is not None:
		put('Linked .bin size=%d bytes' % (len(linked_bin_data)))
	
	if DUMP_SYMBOLS:
		put('Symbols:')
		#for area_name, a in areas.iteritems():	# Python2
		for area_name, a in areas.items():	# Python3
			put('\t* %s' % str(a))
			for sym in a.symbols:
				put('\t\t* %s' % str(sym))
				
			
	
	put('Done processing "%s".' % (filename_rel))
	return areas, rel_bin_data, linked_bin_data

if __name__ == '__main__':
	#name = '../examples/hello/out/hello'
	#process_rel_file(name+'.rel', name+'.cart.8kb.bin')
	name = '../examples/test/out/test'
	process_rel_file(name+'.rel', name+'.bin')
