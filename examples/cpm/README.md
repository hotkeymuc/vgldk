# VGLDK CP/M

This is an experiment to let CP/M run on the VTech Genius Leader 4000.

## Running

Running CP/M on the VGL is not trivial, because CP/M requires RAM to be present at memory 0x0000. VGLs have their ROM there.
To achieve this, we can do a ROM-RAM-swaparoo while the machine is running:
 - You need a "bus doubler", so you can plug in a regular cartridge *and* a SRAM cartridge at the same time.
 - Insert a "monitor" cartridge (or any other self-contained serial loader).
 - Boot up the stock system into the monitor.
 - Now tie the ~CS line of the internal ROM high - this will disable it. Dumping lower memory should now show a lot of "floating" 0x7F bytes.
 - Now plug in (or enable) the SRAM cartridge and let it use the internal ROM's ~CS line. This will mount it at 0x0000.
 - Now upload the CP/M binaries to the newly mounted RAM at 0x0000
 - Boot into CP/M!


## Files

 - bint: Interrupt handler
 - bios: Basic I/O
 - bdos: File handling
 - cpm: Main compilation start
 - cpm_make.py: Python helper for compiling, linking and pushing the CP/M binary blobs to a serial monitor
 - ccp: Command processor (stand-alone CP/M binary!)
 - fcb: File control block definition
 - program: Includes for building an executable for CP/M
 - test: A Hello-World test file (compiles to TEST.COM)

