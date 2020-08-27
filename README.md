# VGLDK
A development kit (SDK) for the Z80 based VTech "Genius LEADER" line of learning computers (also called "PreComputer", "YENO MisterX" or "Compusavant")

This is not a usable development environment, yet. For some working proof-of-concept stuff, visit the [Hackaday project page](https://hackaday.io/project/166921-v-tech-genius-leader-precomputer-hacking) and get started using the zipped snapshots of "VGL CP/M" and other SDCC code examples.


## Target platforms
	pc1000
		VTech PreComputer 1000 [US/UK]
		YENO Mister X [DE]
	gl2000
		VTech PreComputer 2000 [US/UK]
		VTech Genius LEADER 2000 [DE]
		VTech Genius LEADER 2000 PLUS [DE]
		VTech Genius LEADER 2000 Compact [DE]
		VTech PreComputer THINK BOOK [UK]
		YENO Mister X2
	gl4000
		VTech PreComputer POWER PAD [UK]
		VTech Genius LEADER 4000 Quadro [DE]
		VTech Genius LEADER 4000 Quadro [DE]
		VTech Genius LEADER 4004 Quadro L [DE]
	gl6000sl
		VTech Genius LEADER 6000 SL [DE]
		VTech PreComputer Prestige
	
	planned:
	gl7000
		VTech PC ENDEAVOUR [UK]
		VTech Genius LEADER 7007 SL [DE]
		VTech Genius LEADER 8008 CX [DE]
		(CompuSavant? [FR])

## Files
* examples: Some demonstrative use cases and experiments
* include: Hardware drivers and a rudamentary libc environment
* tools: Scripts that help development

## Getting started
* Clone this repo to some nice place
* Make sure you have SDCC (Small Devices C Compiler) installed
* Make sure you have MAME and "Genius Leader" ROMs installed if you wish to use emulation
* Enter an example directory (e.g. "monitor")
* Look at the `Makefile` to configure the desired system and to set some important flags
* Run `make cart` to compile a cartridge ROM file that can be burned or loaded into MAME
* Run `make emu` to start MAME with the compiled cartridge
* Run `make burn` to burn the ROM image to an EEPROM using MiniPro

## History
* Started as a stand-alone reverse engineering project in late 2016
* Incorporated the code into the Z88DK repo Development branch (see z88dk.org, platform "vgl")
* Re-visited the whole project using just SDCC in 2019 (see hackaday.io, user "hotkey")
* Several sub-projects emerged (CP/M, DOS, serial loader, bus buddy, ...) which I now try to develop under a common development environment
* As of 2020, the VGLDK hardware drivers have become much cleaner "pure C" implementations than the previous Z88DK assembly versions

## Further Reading
* [Hackaday project page](https://hackaday.io/project/166921-v-tech-genius-leader-precomputer-hacking)
* [Discussion on the Z88DK forum](https://www.z88dk.org/forum/viewtopic.php?id=10055)
* [VTech hardware development](https://www.thingiverse.com/thing:3108809)

2020-08-28 Bernhard "HotKey" Slawik