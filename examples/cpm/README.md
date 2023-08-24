# VGLDK CP/M

This is an experiment to let CP/M run on the VTech Genius Leader 4000.

## Running

Running CP/M on the VGL is not trivial, because CP/M requires RAM to be present at memory 0x0000. But stock systems have their ROM there.
To achieve this, we can do a "ROM-RAM-swaparoo" while the machine is running:
 - You need a DIY "bus doubler" in order to plug in two cartridges at the same time.
 - Enable the internal ROM (this is the stock configuration)
 - Insert the (E)EPROM cartridge containing the CP/M code
 - Remove the RAM cartridge (or leave it connected and tie its ~CS HIGH to +5V in order to temporarily disable it)
 - Boot the stock firmware, start into the cartridge...
 - The CP/M BIOS should boot up. BDOS should pause while displaying "RAM check..."
 - While the system is running: Disable internal ROM (tie its ~CS HIGH to +5V)
 - While the system is running: Enable RAM on internal ROM space (bind its ~CS to internal ROM ~CS)
 - BDOS should detect the RAM and start up into the CCP prompt
 - You can connect the bdos_host.py or an Arduino to the serial port and have it serve the files. (See BDOS_HOST configuration variables in cpm_make.py)


## Memory Layout

We are using a stock "SUPERSPEICHER" 32KB SRAM cartridge as system ROM (0x0000...0x7FFF).
The CP/M routines (BIOS, BDOS, CCP) can be located there, but this reduces the overal usable memory for programs.
Some CP/M programs (e.g. ZORK) look at the BDOS vector at address 0x0005 to determine the RAM size. So, the lower BDOS is in memory, the less memory is available.

That's why cpm_make.py is configured to put as much code into the read-only cartridge ROM space as possible (0x8000...0xBFFF) and away from precious SRAM (0x0000...0x7FFF).
We can place a "fake" BDOS vector at 0x0005, which points to the top of SRAM. It's just another jump to the "real" BDOS in cart ROM. (See BDOS_PATCHED_ENTRY_ADDRESS variable)

```

Stock memory layout of GL4000:
	Memory Bank	Usage
	0000 - 3FFF	Internal ROM (bank switched via port 0x00)
	4000 - 7FFF	Internal ROM (bank switched via port 0x01, even cartridge!)
	8000 - 9FFF	Cartridge ROM 0000 - 1FFF (bank switched via port 0x03?)
	A000 - BFFF	Cartridge ROM 2000 - 3FFF (bank switched via port 0x02)
	C000 - DFFF	Internal RAM 0000 - 1FFF
	E000 - FFFF	Internal RAM 0000 - 1FFF copy / unused

In order to get CP/M running, 32KB of additional RAM have to be
"mounted" to the base of address space, "overshadowing" internal system ROM.
This can be achieved by connecting the internal ROM's ~CS HIGH while
connecting its signal to the external SRAM ~CS.

Modified memory layout for CP/M:
	Memory Bank	Usage
	0000 - 3FFF	Cartridge RAM 0000 - 3FFF
	4000 - 7FFF	Cartridge RAM 4000 - 7FFF
	8000 - 9FFF	Cartridge ROM 0000 - 1FFF (bank switched via port 0x03?)
	A000 - BFFF	Cartridge ROM 2000 - 3FFF (bank switched via port 0x02)
	C000 - DFFF	Internal RAM 0000 - 1FFF
	E000 - FFFF	Internal RAM 0000 - 1FFF copy / unused

```


## Files

 - bint.c/h: Interrupt handler
 - bios.c/h: Basic I/O
 - bdos.c/h: File handling
 - cpm.c/h: Main compilation target
 - cpm_make.py: **Python helper for building the whole project**
 - ccp.c/h: Command processor (stand-alone CP/M binary!)
 - fcb.h: File control block definition
 - program.c/h: Includes for building a stand-alone executable for CP/M
 - test.c/h: A Hello-World test file (compiles to TEST.COM)

