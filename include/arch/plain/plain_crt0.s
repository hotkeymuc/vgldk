;; crt0 for a plain Z80 based app (loadable)
	.module crt0
	
	;.globl	_main
	.globl	_vgldk_init
	;.globl	VGLDK_SERIES
	
	.area	_HEADER (ABS)
	
	;; Zero page ROM start
	;.org 0x8000	; cartridges need "0x8000" here, internal ROMs need "0x0000" here
	
	;; Call main() function (C entry point)
	;call	_main
	jp	_vgldk_init
	;; End of main()
	
	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL
	
	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP
	
	.area   _CODE
	

