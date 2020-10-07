import math

SINTABLE_INDEX_TYPE = 'unsigned char'
SINTABLE_SIZE = 256
SINTABLE_VALUE_TYPE = 'signed char'
SINTABLE_SCALE = 127

# Generate values
table = []
for i in range(SINTABLE_SIZE):
	a = i*(2.0*math.pi)/(SINTABLE_SIZE)
	v = math.sin(a) * SINTABLE_SCALE
	vi = int(round(v))
	table.append(vi)

# Output as .h
COLS = 32
r = '\n'
r += '// Auto-generated by %s\n' % __file__
r += '#define SINTABLE_INDEX_TYPE %s\n' % SINTABLE_INDEX_TYPE
r += '#define SINTABLE_SIZE %d\n' % SINTABLE_SIZE
r += '#define SINTABLE_VALUE_TYPE %s\n' % SINTABLE_VALUE_TYPE
r += '#define SINTABLE_SCALE %d\n' % SINTABLE_SCALE
r += 'const %s SINTABLE[%d] = {' % (SINTABLE_VALUE_TYPE, SINTABLE_SIZE)
for i,v in enumerate(table):
	if (i > 0): r += ','
	if (i % COLS == 0): r += '\n\t'
	else: r += ' '
	r += '% 4d' % v
r += '\n};\n'
r += '\n'
print(r)