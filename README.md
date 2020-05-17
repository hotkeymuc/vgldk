# VGLDK
SDK for the Z80 based VTech "Genius LEADER" line of learning computers, also called "PreComputer" (us), "YENO MisterX" (de) or "Compusavant" (fr)

This is under construction. For actually working stuff, visit the [Hackaday project page](https://hackaday.io/project/166921-v-tech-genius-leader-precomputer-hacking) and get started using the zipped snapshots of "VGL CP/M" and other SDCC code examples.


## Target platforms
	pc1000
		VTech PreComputer 1000
		YENO Mister X
	gl2000
		(CompuSavant?)
		VTech PreComputer 2000
		VTech Genius LEADER 2000
		VTech Genius LEADER 2000 Compact
		VTech Genius LEADER 2000 PLUS
		YENO Mister X2
	gl4000
		(CompuSavant?)
		VTech Genius LEADER 4000 Quadro
		VTech Genius LEADER 4004 Quadro L
	gl6000sl
		VTech Genius LEADER 6000 SL
		VTech PreComputer Prestige

## Files
* examples: Some demonstrative use cases
* include: Contains all the headers

## History
* Started as a stand-alone reverse engineering project in late 2016
* Incorporated the code into the Z88DK repo Development branch (see z88dk.org, platform "vgl")
* Re-visited the whole project using just SDCC in 2019 (see hackaday.io, user "hotkey")
	* Several sub-projects emerged (CP/M, DOS, serial loader, bus buddy, ...)
* Now it is all getting re-organized into an SDK

## Further Reading
* [Hackaday project page](https://hackaday.io/project/166921-v-tech-genius-leader-precomputer-hacking)
* [Discussion on the Z88DK forum](https://www.z88dk.org/forum/viewtopic.php?id=10055)
* [VTech hardware development](https://www.thingiverse.com/thing:3108809)

2020-01-21 Bernhard "HotKey" Slawik