NAME = vagi

# To see the result on serial terminal and recover quickly
#SEND2MONITOR_KEEP_SIO = 1
#SEND2MONITOR_RESET = 1

# We need more than 8 KB...
#CART_SIZE_KB = 8
#CART_SIZE_KB = 32
CART_SIZE_KB = 1024

# For Emu:
# For targeting a specific model:
#VGLDK_SERIES = 1000
#VGLDK_SERIES = 2000
#VGLDK_SERIES = 4000
VGLDK_SERIES = 6000
LOC_DATA = 0xebb8

# For real hardware running MONITOR cart:
#	# For small loadable apps (without their own I/O):
#	VGLDK_SERIES = 0
#	#LOC_CODE = 0xc000
#	#LOC_DATA = 0xc800
#	LOC_CODE = 0xf000
#	LOC_DATA = 0xebc0
#	SEND2MONITOR_OPTIONS = --baud=9600

all: upload
#all: app
#all: cart
#all: emu

include ../../tools/Makefile.mk

