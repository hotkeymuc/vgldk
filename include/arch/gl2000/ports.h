#ifndef __PORTS2000_H
#define __PORTS2000_H
/*
VTech Genius Leader Ports

MEM:
	Bank  	CPU Address	  	Physical       	 	 876 5432 1098 7654 3210
	======	===========	==	===============	=	 === ==== ==== ==== ====
	Bank 0	0000 - 3fff	=>	ROM 0000 - 3fff	=	 BBB BBxx xxxx xxxx xxxx
	Bank 1	4000 - 7fff	=>	ROM 0000 - 3fff	=	 BBB BBxx xxxx xxxx xxxx
	Bank 2	8000 - bfff	=>	Ctg 0000 - 3fff	=	 BBB BBxx xxxx xxxx xxxx
	RAM   	C000 - ffff	=>	RAM 0000 - 3fff	=	       ?x xxxx xxxx xxxx
	
	Bank switchers mask by 0x1f, meaning 5 bits in theory


	Some info (from MAME's pc2000.cpp)
	
	1000:
		void pc1000_state::pc1000_mem(address_map &map) {
			map.unmap_value_high();
			map(0x0000, 0x3fff).rom().region("bios", 0x00000);
			map(0x4000, 0x47ff).ram();
			map(0x8000, 0xbfff).r(m_cart, FUNC(generic_slot_device::read_rom));    //0x8000 - 0xbfff tests a cartridge, header is 0x55 0xaa 0x33, if it succeeds a jump at 0x8010 occurs
			map(0xc000, 0xffff).bankr("bank1");
		}
		void pc1000_state::pc1000_io(address_map &map) {
			map(0x0000, 0x01ff).r(FUNC(pc1000_state::kb_r));
			map(0x4000, 0x4000).mirror(0xfe).rw(FUNC(pc1000_state::lcdc_control_r), FUNC(pc1000_state::lcdc_control_w));
			map(0x4100, 0x4100).mirror(0xfe).rw(FUNC(pc1000_state::lcdc_data_r), FUNC(pc1000_state::lcdc_data_w));
		}
	
	2000:
		void pc2000_state::pc2000_mem(address_map &map) {
			map.unmap_value_high();
			map(0x0000, 0x3fff).bankr("bank0");
			map(0x4000, 0x7fff).bankr("bank1");
			map(0x8000, 0xbfff).bankr("bank2");    //0x8000 - 0xbfff tests a cartridge, header is 0x55 0xaa 0x59 0x45, if it succeeds a jump at 0x8004 occurs
			map(0xc000, 0xdfff).ram();
		}
		void pc2000_state::pc2000_io(address_map &map) {
			map.unmap_value_high();
			map.global_mask(0xff);
			
			map(0x00, 0x00).w(FUNC(pc2000_state::rombank0_w));
				optional_memory_bank m_bank0;
				m_bank0->set_entry(data & 0x1f);
			map(0x01, 0x01).w(FUNC(pc2000_state::rombank1_w));
				required_memory_bank m_bank1;
				m_bank1->set_entry(data & 0x1f);
			map(0x03, 0x03).w(FUNC(pc2000_state::rombank2_w));
				optional_memory_bank m_bank2;
					if (data & 0x80)
						m_bank2->set_entry(data & 0x8f);   //cartridge
					else
						m_bank2->set_entry(data & 0x1f);
			
			map(0x0a, 0x0b).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
			
			map(0x10, 0x11).rw(FUNC(pc2000_state::key_matrix_r), FUNC(pc2000_state::key_matrix_w));
				// 0x10 also used for printer data out
				// 0x11 also used for printer status IN (connected / transmit)
			map(0x12, 0x12).rw(FUNC(pc2000_state::beep_r), FUNC(pc2000_state::beep_w));
			// 0x12 also used for power off (0x01)
			// 0x12 also used for printer port Pin 1 (0x04)
			// 0x12 also used for beeper (0x08)
			// 0x12 also used for caps lock (0x20)
		}


2019-11-03 Bernhard "HotKey" Slawik
*/

//@FIXME: Make all those port_* function "__naked"/"__inline" or create a macro

/*
byte port_in(byte p) {
	(void)p;
	
	__asm
		;push af
		
		; Get parameter from stack into a
		ld hl,#0x0002
		add hl,sp
		ld c,(hl)
		
		in	a, (c)
		ld	l, a
		
		;pop af
		ret
	__endasm;
	return 0;
}

void port_out(byte p, byte v)  {
	(void)p;	// suppress warning "unreferenced function argument"
	(void)v;	// suppress warning "unreferenced function argument"
	__asm
		;push hl
		;push af
		
		; Get parameter v from stack into a
		ld hl,#0x0002
		add hl,sp
		ld a,(hl)
		
		; Get parameter p from stack into c
		ld hl,#0x0004
		add hl,sp
		ld c,(hl)
		
		out	(c), a
		
		;pop af
		;pop hl
		;ret
	__endasm;
}
*/


byte port_0x10_in() {
	__asm
		;push af
		
		in	a, (0x10)
		ld	l, a
		
		;pop af
		ret
	__endasm;
	return 0;
}

void port_0x10_out(byte a)  {
	(void)a;	// suppress warning "unreferenced function argument"
	__asm
		;push hl
		;push af
		
		; Get parameter from stack into a
		ld hl,#0x0002
		add hl,sp
		ld a,(hl)
		
		out	(0x10), a
		
		;pop af
		;pop hl
		;ret
	__endasm;
}

byte port_0x11_in() {
	__asm
		;push af
		
		in	a, (0x11)
		ld	l, a
		
		;pop af
		ret
	__endasm;
	return 0;
}

void port_0x11_out(byte a) {
	(void)a;	// suppress warning "unreferenced function argument"
	__asm
		;push af
		;push hl
		
		; Get parameter from stack into a
		ld hl,#0x0002
		add hl,sp
		ld a,(hl)
		
		out	(0x11), a
		
		;pop hl
		;pop af
		;ret
	__endasm;
}

byte port_0x12_in() {
	__asm
		;push af
		
		in	a, (0x12)
		ld	l, a
		
		;pop af
		ret
	__endasm;
	return 0;
}

void port_0x12_out(byte a) {
	(void)a;	// suppress warning "unreferenced function argument"
	__asm
		;push af
		;push hl
		
		; Get parameter from stack into a
		ld hl,#0x0002
		add hl,sp
		ld a,(hl)
		
		out	(0x12), a
		
		;pop hl
		;pop af
		;ret
	__endasm;
}

void vgl_shutdown() {
	port_0x12_out(0x01);	// BIOS4000 0181
	__asm
		halt
	__endasm;

}

#endif // __PORTS2000_H