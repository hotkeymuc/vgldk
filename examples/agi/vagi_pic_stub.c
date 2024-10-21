#ifndef __VAGI_PIC_STUB_C__
#define __VAGI_PIC_STUB_C__
/*

VAGI PIC drawing (high-level)

STUB for code segmenting

*/

// Reserve RAM for other segment
byte vagi_drawing_step;	// = VAGI_STEP_VIS;	// Current rendering step (which kind of PIC data to process: 0=visual, 1=priority)
bool _dataOffsetNibble;	// = 0;
vagi_res_handle_t pic_res_h;	// Resource handle to read from (vagi_res.h)


// PictureMgr
uint8 _patCode;
uint8 _patNum;
uint8 _scrOn;	// = 0;
uint8 _priOn;	// = 0;
uint8 _scrColor;	// = 0;
uint8 _priColor;	// = 0;

uint8 _minCommand;	// = 0xf0;

//AgiPictureVersion _pictureVersion;
byte _pictureVersion;

int16 _width;	// = 160;
int16 _height;	// = 168;
//int16 _xOffset, _yOffset;

int _flags;
//int _currentStep;

// Provide stubs ("trampolines") to functions in the other bank

void vagi_pic_draw(byte pic_num) {
	// Call entry point of "vagi_pic_draw()" in other segment
	
	buffer_switch(BUFFER_BANK_PRI);
	buffer_clear(0x4);
	buffer_switch(BUFFER_BANK_VIS);
	buffer_clear(0xf);
	
	//printf("banking...");getchar();
	code_segment_call_b(2, code_segment_1__vagi_pic_draw_addr, pic_num);
	//printf("back.");getchar();
}

void vagi_pic_show() {
	//lcd_clear();
	draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0);	//, false);
	
	//code_segment_call(2, code_segment_1__vagi_pic_show_addr);
}

#endif