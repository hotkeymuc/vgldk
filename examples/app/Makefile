
#NAME = app_hello
#NAME = app_tests
#NAME = app_cserial
#NAME = app_test_softuart
NAME = app_test_keyboard

# To see the result on serial terminal and recover quickly
#SEND2MONITOR_KEEP_SIO = 1
#SEND2MONITOR_RESET = 1

# For targeting a specific model:
#VGLDK_SERIES = 1000
#VGLDK_SERIES = 2000
#VGLDK_SERIES = 4000
#VGLDK_SERIES = 6000

# For small loadable apps (without their own I/O):
VGLDK_SERIES = 0
LOC_CODE = 0xc400
LOC_DATA = 0xc080


all: upload
#all: app
#all: cart
#all: emu

include ../../tools/Makefile.mk

