#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
	Wrapper around SDCC compiler
	
	e.g. used in "CP/M" example to do some compiler magic.

"""

import os

def put(t):
	print(f'sdcc: {t}')

def compile(
		crt_s_files = ['./crt0.s'],
		crt_rel_file = 'out/crt0.rel',
		source_files = ['./main.c'],	# CRT0 rel file is prepended automatically
		output_hex_file = 'out/main.hex',
		output_bin_file = 'out/main.bin',
		lib_paths = [],	# ['../../lib']
		include_paths = ['../../include'],
		loc_code = 0x8000,	# 0x8000 for cartridge
		loc_data = 0xc000,	# 0xc000 for internal RAM
		defines = { 'VGLDK_SERIES': 4000 }	#VGLDK_SERIES
	):
	"""Wrapper around the SDCC compiler"""
	
	"""
	### Alternative: Compile source file(s) using Z88DK, generate .bin file
	my_env = os.environ.copy()
	my_env['PATH'] = os.environ['PATH'] + ';' + os.path.join(z88dk_path, 'bin')
	my_env['OZFILES'] = z88dk_lib_path	# Normally these files reside inside the z88dk, but we can hijack them and use our minimal local version
	my_env['ZCCCFG'] = os.path.abspath(z88dk_lib_path + '/config')
	
	#cmd = os.path.join(z88dk_path, 'bin', 'zcc')
	#cmd = 'zcc +vgl -vn -clib=sdcc_iy -SO3 --max-allocs-per-node200000 %PROGNAME%.c -o %PROGNAME% -create-app'
	#cmd = 'zcc +vgl -v -clib=sdcc_iy -SO3 --max-allocs-per-node200000 %PROGNAME%.c -o %PROGNAME% -create-app'
	cmd = 'zcc'
	cmd += ' +' + z88dk_target
	cmd += ' -vn'
	cmd += ' -clib=new'	# "new" or "sdcc_iy" or ...
	cmd += ' -subtype=' + z88dk_subtype
	#cmd += ' -I' + os.path.abspath(libs_path)
	#cmd += ' -I' + os.path.abspath(os.path.join(z88dk_path, 'include'))
	#cmd += ' -lm'
	#cmd += ' -l' + 'gen_math'
	#cmd += ' -l' + 'zx80_clib'
	#cmd += ' -lndos'
	#cmd += ' -l' + 'z88_clib'
	#cmd += ' -l' + 'z88_math'
	cmd += ' %s' % ' '.join(source_files)	# may only be a single .c file
	cmd += ' -o ' + output_bin_file
	cmd += ' -create-app'
	"""
	
	# Compile .s file(s) to .rel file
	cmd = 'sdasz80'
	cmd += ' -o %s' % crt_rel_file
	cmd += ' %s' % ' '.join(crt_s_files)
	
	put('>> %s' % cmd)
	r = os.system(cmd)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Compile source file(s) using SDCC, generate .hex file
	cmd = 'sdcc -mz80'
	
	#cmd += ' --model-small'	# model-small = default, but not supported on Z80
	cmd += ' --no-std-crt0'	# Provide our own crt0 .rel
	#cmd += ' --nostdlib'
	
	# Add lib and include path(s)
	if lib_paths is not None:
		for lib_path in lib_paths:
			cmd += ' --lib-path %s' % lib_path
	
	if include_paths is not None:
		for include_path in include_paths:
			cmd += ' -I %s' % include_path
	
	cmd += ' --code-loc 0x%04X' % loc_code
	#if loc_stack is not None: cmd += ' --stack-loc 0x%04X' % loc_stack
	if loc_data is not None: cmd += ' --data-loc 0x%04X' % loc_data
	#if loc_idata is not None: cmd += ' --idata-loc 0x%04X' % loc_idata
	#if loc_xram is not None: cmd += ' --xram-loc 0x%04X' % loc_xram
	
	# Add "#define"s
	for k,v in defines.items():
		cmd += ' -D %s=%s' % (k, v)
	
	#cmd += ' --verbose'	# Show stages of compilation
	#cmd += ' --vc'	# vc = messages are compatible with Micro$oft visual studio
	#cmd += ' -V'	# V = Show actual commands that are run by the compiler
	
	cmd += ' -o %s' % output_hex_file
	cmd += ' %s %s' % (crt_rel_file, ' '.join(source_files))	# .rel, .c, .c ...
	
	put('>> %s' % cmd)
	r = os.system(cmd)
	if r != 0:
		put('Result: %s' % r)
		sys.exit(1)
	#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
	
	# Analyze the intermediate .rel file (without binary references)
	#process_rel_file(crt_rel_file)	#, output_bin_file)
	
	
	### Convert .hex file to .bin file
	if output_bin_file is not None:
		cmd = 'objcopy -Iihex -Obinary %s %s' % (output_hex_file, output_bin_file)
		put('>> %s' % cmd)
		r = os.system(cmd)
		if r != 0:
			put('Result: %s' % r)
			sys.exit(1)
		#put(subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, env=env).stdout.read())
		
		#@FIXME: The .bin file also contains some data at the RAM area... (e.g. file offset 0xc000)
		
		## Analyze final .rel file (with binary references)
		#process_rel_file(crt_rel_file, output_bin_file)
		
		## Show cartridge size statistics
		#calcsize.analyze(output_bin_file)
		
		### Return pure binary data
		with open(output_bin_file, 'rb') as h:
			data = h.read()
		return data
	

if __name__ == '__main__':
	#compile()
	pass
	