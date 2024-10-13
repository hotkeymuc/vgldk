#ifndef __AGI_PIC_C__
#define __AGI_PIC_C__
/*
AGI PIC drawing
based on scummvm's agi implementation 2024-09-14
* ScummVM: https://github.com/scummvm/scummvm/tree/master/engines/agi

2024-09-16 Bernhard "HotKey" Slawik
*/

//#define AGI_PIC_FILL_CASES	// Optimize fill by pre-selecting a "draw_FillCheck_case" function to use throughout the fill
//#define AGI_PIC_NIBBLE_MODE	// nibbleMode=false for SQ2, it uses separate bytes for color value of 0xf0 / 0xf2

#include "agi_pic.h"

// Connections to the outer world:
byte vagi_drawing_step;
bool _dataOffsetNibble; // = 0;
vagi_res_handle_t pic_res_h;	// Resource handle to read from (vagi_res.h)


// PictureMgr
uint8 _patCode;
uint8 _patNum;
uint8 _scrOn; // = 0;
uint8 _priOn; // = 0;
uint8 _scrColor; // = 0;
uint8 _priColor; // = 0;

uint8 _minCommand; // = 0xf0;

//AgiPictureVersion _pictureVersion;
byte _pictureVersion;

int16 _width; // = 160;
int16 _height; // = 168;
//int16 _xOffset, _yOffset;

int _flags;	// Contains flags for "special treatment" (e.g. "exit on command 0xf3")
//int _currentStep;


//@TODO: Inline it?
void putVirtPixel(int x, int y) {
	if (x < 0) return;
	if (y < 0) return;
	if (x >= AGI_FRAME_WIDTH) return;
	if (y >= AGI_FRAME_HEIGHT) return;
	
	//@TODO: Do the _step checking on a higher level (e.g. line or flood fill)! This would save a lot of superfluous compare operations
	if ((vagi_drawing_step == VAGI_STEP_VIS) && (_scrOn))
		frame_set_pixel_4bit(x, y, _scrColor);
	
	if ((vagi_drawing_step == VAGI_STEP_PRI) && (_priOn))
		frame_set_pixel_4bit(x, y, _priColor);
}


byte getNextByte() {
	if (!_dataOffsetNibble) {
		return vagi_res_read(pic_res_h);
	} else {
		byte curByte = vagi_res_read(pic_res_h) << 4;
		return (vagi_res_peek(pic_res_h) >> 4) | curByte;
	}
}

//bool getNextParamByte(byte &b) {
bool getNextParamByte(byte *b) {
	//byte value = getNextByte();
	//if (value >= _minCommand) {
	//	_dataOffset--;
	//	return false;
	//}
	if (vagi_res_eof(pic_res_h)) return false;
	
	byte value = vagi_res_peek(pic_res_h);
	if (value >= _minCommand) return false;
	value = vagi_res_read(pic_res_h);	// Increase the file offset
	
	*b = value;
	return true;
}

byte getNextNibble() {
	if (!_dataOffsetNibble) {
		_dataOffsetNibble = true;
		return vagi_res_peek(pic_res_h) >> 4;
	} else {
		_dataOffsetNibble = false;
		//return _data[_dataOffset++] & 0x0F;
		return vagi_res_read(pic_res_h) & 0x0F;
	}
}



/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
void plotPattern(int x, int y) {
	static const uint16 binary_list[] = {
		0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
		0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
	};
	
	static const uint8 circle_list[] = {
		0, 1, 4, 9, 16, 25, 37, 50
	};
	
	static uint16 circle_data[] = {	// Not const! It will be patched depending on game
		0x8000,
		0xE000, 0xE000, 0xE000,
		0x7000, 0xF800, 0x0F800, 0x0F800, 0x7000,
		0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x7C00, 0x3800,
		0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x7F00, 0x1C00,
		0x0E00, 0x3F80, 0x7FC0, 0x7FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00,
		0x0F80, 0x3FE0, 0x7FF0, 0x7FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x7FF0, 0x7FF0, 0x3FE0, 0x0F80,
		0x07C0, 0x1FF0, 0x3FF8, 0x7FFC, 0x7FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0
	};
	
	uint16 circle_word;
	const uint16 *circle_ptr;
	uint16 counter;
	uint16 pen_width = 0;
	int pen_final_x = 0;
	int pen_final_y = 0;
	
	uint8 t = 0;
	uint8 temp8;
	uint16 temp16;
	
	int pen_x = x;
	int pen_y = y;
	uint16 pen_size = (_patCode & 0x07);
	
	circle_ptr = &circle_data[circle_list[pen_size]];
	
	// SGEORGE : Fix v3 picture data for drawing circles. Manifests in goldrush
	if (_pictureVersion == AGIPIC_V2) {
		circle_data[1] = 0;
		circle_data[3] = 0;
	}
	
	// setup the X position
	// = pen_x - pen.size/2
	
	pen_x = (pen_x * 2) - pen_size;
	if (pen_x < 0) pen_x = 0;
	
	temp16 = (_width * 2) - (2 * pen_size);
	if (pen_x >= temp16)
		pen_x = temp16;
	
	pen_x /= 2;
	pen_final_x = pen_x;    // original starting point?? -> used in plotrelated
	
	// Setup the Y Position
	// = pen_y - pen.size
	pen_y = pen_y - pen_size;
	if (pen_y < 0) pen_y = 0;
	
	temp16 = 167 - (2 * pen_size);
	if (pen_y >= temp16)
		pen_y = temp16;
	
	pen_final_y = pen_y;    // used in plotrelated
	
	t = (uint8)(_patNum | 0x01);     // even
	
	// new purpose for temp16
	
	temp16 = (pen_size << 1) + 1;   // pen size
	pen_final_y += temp16;                  // the last row of this shape
	temp16 = temp16 << 1;
	pen_width = temp16;                 // width of shape?
	
	bool circleCond;
	int counterStep;
	int ditherCond;
	
	if (_flags & kPicFCircle)
		_patCode |= 0x10;
	
	/*
	if (_vm->getGameType() == GType_PreAGI) {
		circleCond = ((_patCode & 0x10) == 0);
		counterStep = 3;
		ditherCond = 0x03;
	} else {
	*/
		circleCond = ((_patCode & 0x10) != 0);
		counterStep = 4;
		ditherCond = 0x02;
	//}
	
	for (; pen_y < pen_final_y; pen_y++) {
		circle_word = *circle_ptr++;
		
		for (counter = 0; counter <= pen_width; counter += counterStep) {
			if (circleCond || ((binary_list[counter >> 1] & circle_word) != 0)) {
				if ((_patCode & 0x20) != 0) {
					temp8 = t % 2;
					t = t >> 1;
					if (temp8 != 0)
						t = t ^ 0xB8;
				}
				
				// == box plot, != circle plot
				if ((_patCode & 0x20) == 0 || (t & 0x03) == ditherCond)
					putVirtPixel(pen_x, pen_y);
			}
			pen_x++;
		}
		
		pen_x = pen_final_x;
	}
	
	return;
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void plotBrush() {
	for (;;) {
		if (_patCode & 0x20) {
			if (!getNextParamByte(&_patNum))
				break;
		}

		byte x1, y1;
		if (!(getNextParamByte(&x1) && getNextParamByte(&y1)))
			break;

		plotPattern(x1, y1);
	}
}

void draw_SetColor() {
	if (!getNextParamByte(&_scrColor))
		return;
	/*
	// For CGA, replace the color with its mixture color
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
		break;
	default:
		break;
	}
	*/
}

void draw_SetPriority() {
	getNextParamByte(&_priColor);
}

// this gets a nibble instead of a full byte
// used by some V3 games, special resource flag RES_PICTURE_V3_NIBBLE_PARM is set
void draw_SetNibbleColor() {
	_scrColor = getNextNibble();
	/*
	// For CGA, replace the color with its mixture color
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
		break;
	default:
		break;
	}
	*/
}

void draw_SetNibblePriority() {
	_priColor = getNextNibble();
}

/**
 * Draw an AGI line.
 * A line drawing routine sent by Joshua Neal, modified by Stuart George
 * (fixed >>2 to >>1 and some other bugs like x1 instead of y1, etc.)
 * @param x1  x coordinate of start point
 * @param y1  y coordinate of start point
 * @param x2  x coordinate of end point
 * @param y2  y coordinate of end point
 */

//void PictureMgr::draw_Line(int16 x1, int16 y1, int16 x2, int16 y2)
void draw_Line(int16 x1, int16 y1, int16 x2, int16 y2) {
	//int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;
	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta, tmp;
	
	//printf("draw_Line x="); printf_d(x1); printf(" y="); printf_d(y1);
	//printf("to x="); printf_d(x2); printf(" y="); printf_d(y2);
	//printf(" scrOn="); printf_d(_scrOn); printf(" priOn="); printf_d(_priOn);
	//putchar('\n');
	
	// Quick reject
	if ((vagi_drawing_step == VAGI_STEP_VIS) && (!_scrOn)) return;
	if ((vagi_drawing_step == VAGI_STEP_PRI) && (!_priOn)) return;
	
	x1 = CLIP(x1, 0, _width - 1);
	x2 = CLIP(x2, 0, _width - 1);
	y1 = CLIP(y1, 0, _height - 1);
	y2 = CLIP(y2, 0, _height - 1);
	
	
	// Vertical line
	if (x1 == x2) {
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		
		for (; y1 <= y2; y1++)
			putVirtPixel(x1, y1);
		
		return;
	}
	
	// Horizontal line
	if (y1 == y2) {
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		for (; x1 <= x2; x1++)
			putVirtPixel(x1, y1);
		return;
	}
	
	y = y1;
	x = x1;
	
	stepY = 1;
	deltaY = y2 - y1;
	if (deltaY < 0) {
		stepY = -1;
		deltaY = -deltaY;
	}
	
	stepX = 1;
	deltaX = x2 - x1;
	if (deltaX < 0) {
		stepX = -1;
		deltaX = -deltaX;
	}
	
	if (deltaY > deltaX) {
		i = deltaY;
		detdelta = deltaY;
		errorX = deltaY / 2;
		errorY = 0;
	} else {
		i = deltaX;
		detdelta = deltaX;
		errorX = 0;
		errorY = deltaX / 2;
	}
	
	putVirtPixel(x, y);
	do {
		errorY += deltaY;
		if (errorY >= detdelta) {
			errorY -= detdelta;
			y += stepY;
		}
		
		errorX += deltaX;
		if (errorX >= detdelta) {
			errorX -= detdelta;
			x += stepX;
		}
		
		putVirtPixel(x, y);
		i--;
	} while (i > 0);
}

/**
 * Draw a relative AGI line.
 * Draws short lines relative to last position. (drawing action 0xF7)
 */
void draw_LineShort() {
	byte x1, y1, disp;
	
	if (!(getNextParamByte(&x1) && getNextParamByte(&y1)))
		return;
	
	putVirtPixel(x1, y1);
	
	for (;;) {
		if (!getNextParamByte(&disp))
			break;
		
		int dx = ((disp & 0xf0) >> 4) & 0x0f;
		int dy = (disp & 0x0f);
		
		if (dx & 0x08)
			dx = -(dx & 0x07);
		if (dy & 0x08)
			dy = -(dy & 0x07);
		
		draw_Line(x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}
}

/**************************************************************************
** absoluteLine
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void draw_LineAbsolute() {
	byte x1, y1, x2, y2;
	
	if (!(getNextParamByte(&x1) && getNextParamByte(&y1)))
		return;
	
	putVirtPixel(x1, y1);
	
	for (;;) {
		if (!(getNextParamByte(&x2) && getNextParamByte(&y2)))
			break;
		
		draw_Line(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
}

/*
// flood fill
typedef struct {
	byte x;
	byte y;
} fill_stack_t;
#define FILL_STACK_MAX 128
*/

#ifdef AGI_PIC_FILL_CASES
	// Speed up be pre-selecting the right case (that won't change throuout the fill!)
	typedef bool (draw_FillCheck_t)(int16, int16);
	
	bool draw_FillCheck_case1(int16 x, int16 y) {
		if (x < 0 || x >= _width || y < 0 || y >= _height) return false;
		//if (!_priOn && _scrOn && _scrColor != 15)
		return (frame_get_pixel_4bit(x, y) == 15);
	}
	bool draw_FillCheck_case2(int16 x, int16 y) {
		if (x < 0 || x >= _width || y < 0 || y >= _height) return false;
		//if (_priOn && !_scrOn && _priColor != 4)
		return (frame_get_pixel_4bit(x, y) == 4);
	}
	bool draw_FillCheck_case3(int16 x, int16 y) {
		if (x < 0 || x >= _width || y < 0 || y >= _height) return false;
		//return (_scrOn && frame_get_pixel_4bit(x, y) == 15 && _scrColor != 15);
		return (frame_get_pixel_4bit(x, y) == 15);
	}
	#define draw_FillCheck(x,y) ((*p_draw_FillCheck)(x, y))
#else
	
	// Working but slow (because it checks things that don't change throughout the fill!)
	bool inline draw_FillCheck(int16 x, int16 y) {
		// Return TRUE if the current pixel should be flood-filled
		//	Original:
		//		byte screenColor;
		//		byte screenPriority;
		//		
		//		if (x < 0 || x >= _width || y < 0 || y >= _height)
		//			return false;
		//		
		//		//x += _xOffset;
		//		//y += _yOffset;
		//		
		//		screenColor = _gfx->getColor(x, y);
		//		screenPriority = _gfx->getPriority(x, y);
		//		
		//		if (_flags & kPicFTrollMode)
		//			return ((screenColor != 11) && (screenColor != _scrColor));
		//		
		//		if (!_priOn && _scrOn && _scrColor != 15)
		//			return (screenColor == 15);
		//		
		//		if (_priOn && !_scrOn && _priColor != 4)
		//			return screenPriority == 4;
		//		
		//		return (_scrOn && screenColor == 15 && _scrColor != 15);
		
		byte c;
		
		// Check "outside" to be faster!
		//if (x < 0 || x >= _width || y < 0 || y >= _height) return false;
		
		c = frame_get_pixel_4bit(x, y);
		
		
		// Naive case: Looks like ScummVM (fills too less)
		//if (_scrOn) return (c == 15);
		//return (c == 4);
		
		// VAGI:
		//if (_priOn) return (c == 4);
		//return (c == 15);
		
		// ScummVM:
		if ((!_priOn) && (_scrOn) && (_scrColor != 15))
			return (c == 15);
		
		if ((_priOn) && (!_scrOn) && (_priColor != 4))
			return (c == 4);
		
		// htk addition:
		if (vagi_drawing_step == VAGI_STEP_PRI)
			//return (c != _priColor);
			return (c == 4);
		
		return (_scrOn && c == 15 && _scrColor != 15);
	}
#endif


//fill_stack_t stack[FILL_STACK_MAX];	// extern

// can be inline as long as it is only used once
bool inline _draw_Fill(int16 x, int16 y) {
	fill_stack_t stack[FILL_STACK_MAX];	// on stack
	
	byte stack_pos = 0;
	//fill_stack_t p;
	int16 px, py;
	
	if (!_scrOn && !_priOn) return false;
	
	// Quick reject
	if ((vagi_drawing_step == VAGI_STEP_VIS) && (!_scrOn)) return false;
	if ((vagi_drawing_step == VAGI_STEP_PRI) && (!_priOn)) return false;
	
	#ifdef AGI_PIC_FILL_CASES
	// Pre-select the appropriate checking function (we can decide these things IN ADVANCE before entering the flood fill)
	draw_FillCheck_t *p_draw_FillCheck;
	if (!_priOn && _scrOn && _scrColor != 15)
		p_draw_FillCheck = (draw_FillCheck_t *)&draw_FillCheck_case1;
	else if (_priOn && !_scrOn && _priColor != 4)
		p_draw_FillCheck = (draw_FillCheck_t *)&draw_FillCheck_case2;
	else {
		if (!_scrOn) return false;
		if (_scrColor == 15) return false;
		p_draw_FillCheck = (draw_FillCheck_t *)&draw_FillCheck_case3;
	}
	#endif
	
	// For testing: Draw a cross
	//const byte b = 4;
	//draw_Line(x-b, y-b, x+b, y+b);
	//draw_Line(x-b, y+b, x+b, y-b);
	
	
	// Push initial pixel on the stack
	stack[0].x = x;
	stack[0].y = y;
	stack_pos++;
	
	// Exit if stack is empty
	while (stack_pos > 0) {
		
		stack_pos--;
		px = stack[stack_pos].x;
		py = stack[stack_pos].y;
		
		byte c;	//unsigned int c;
		bool newspanUp, newspanDown;
		
		if (!draw_FillCheck(px, py))
			continue;
		
		// Scan for left border
		if (px == 0) {
			c = 0;
		} else {
			for (c = px - 1; (c > 0) && draw_FillCheck(c, py); c--);
			c++;
		}
		
		newspanUp = newspanDown = true;
		for (; (c < _width) && draw_FillCheck(c, py); c++) {
			putVirtPixel(c, py);
			
			if ((py > 0) && (draw_FillCheck(c, py - 1))) {
				if (newspanUp) {
					stack[stack_pos].x = c;
					stack[stack_pos].y = py-1;
					stack_pos++;
					newspanUp = false;
					
					if (stack_pos >= FILL_STACK_MAX) {
						//printf("FILL_STACK_MAX!");
						return false;
					}
				}
			} else {
				newspanUp = true;
			}
			
			if ((py+1 < _height) && (draw_FillCheck(c, py + 1))) {
				if (newspanDown) {
					stack[stack_pos].x = c;
					stack[stack_pos].y = py+1;
					stack_pos++;
					newspanDown = false;
					
					if (stack_pos >= FILL_STACK_MAX) {
						//printf("FILL_STACK_MAX!");
						return false;
					}
				}
			} else {
				newspanDown = true;
			}
		}
	}
	return true;
}

void draw_Fill() {
	byte x1, y1;
	bool cont = true;
	
	while (getNextParamByte(&x1) && getNextParamByte(&y1)) {
		//printf("x="); printf_x2(x1); printf(", y="); printf_x2(y1); printf("\n");
		
		if (cont) {	// If an error occurs: Do not call drawFill again, something went wrong.
			cont = _draw_Fill(x1, y1);
			if (!cont) {
				#ifdef VAGI_PIC_IGNORE_FILL_STACK
				// Ignore!
				#else
				printf("F!");
				#endif
			}
		}
	}
}



/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
void draw_xCorner(bool skipOtherCoords) {
	byte x1, x2, y1, y2, dummy;
	
	if (!(getNextParamByte(&x1) && getNextParamByte(&y1)))
		return;
	
	putVirtPixel(x1, y1);
	
	for (;;) {
		if (!getNextParamByte(&x2))
			break;
		
		if (skipOtherCoords)
			if (!getNextParamByte(&dummy))
				break;
		
		draw_Line(x1, y1, x2, y1);
		x1 = x2;
		
		if (skipOtherCoords)
			if (!getNextParamByte(&dummy))
				break;
		
		if (!getNextParamByte(&y2))
			break;
		
		draw_Line(x1, y1, x1, y2);
		y1 = y2;
	}
}

/**************************************************************************
** draw_yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void draw_yCorner(bool skipOtherCoords) {
	byte x1, x2, y1, y2, dummy;
	
	if (!(getNextParamByte(&x1) && getNextParamByte(&y1)))
		return;
	
	putVirtPixel(x1, y1);
	
	for (;;) {
		if (skipOtherCoords)
			if (!getNextParamByte(&dummy))
				break;
		
		if (!getNextParamByte(&y2))
			break;
		
		draw_Line(x1, y1, x1, y2);
		y1 = y2;
		if (!getNextParamByte(&x2))
			break;
		
		if (skipOtherCoords)
			if (!getNextParamByte(&dummy))
				break;
		
		draw_Line(x1, y1, x2, y1);
		x1 = x2;
	}
}



/*
void drawPictureC64() {
	byte curByte;
	
	//debugC(8, kDebugLevelMain, "Drawing C64 picture");
	
	_scrColor = 0x0;
	
	while (_dataOffset < _dataSize) {
		curByte = getNextByte();
		
		if ((curByte >= 0xF0) && (curByte <= 0xFE)) {
			_scrColor = curByte & 0x0F;
			continue;
		}
		
		switch (curByte) {
		case 0xe0:  // x-corner
			draw_xCorner();
			break;
		case 0xe1:  // y-corner
			draw_yCorner();
			break;
		case 0xe2:  // dynamic draw lines
			draw_LineShort();
			break;
		case 0xe3:  // absolute draw lines
			draw_LineAbsolute();
			break;
		case 0xe4:  // fill
			draw_SetColor();
			draw_Fill();
			break;
		case 0xe5:  // enable screen drawing
			_scrOn = true;
			break;
		case 0xe6:  // plot brush
			// TODO: should this be getNextParamByte()?
			_patCode = getNextByte();
			plotBrush();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}
*/

void drawPictureV1() {
	byte curByte;
	
	//debugC(8, kDebugLevelMain, "Drawing V1 picture");
	
	while (!vagi_res_eof(pic_res_h)) {
		curByte = getNextByte();
		
		switch (curByte) {
			case 0xf1:
				draw_SetColor();
				_scrOn = true;
				_priOn = false;
				break;
			case 0xf3:
				draw_SetColor();
				_scrOn = true;
				draw_SetPriority();
				_priOn = true;
				break;
			case 0xfa:
				_scrOn = false;
				_priOn = true;
				draw_LineAbsolute();
				_scrOn = true;
				_priOn = false;
				break;
			case 0xfb:
				draw_LineShort();
				break;
			case 0xff: // end of data
				return;
			default:
				//warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
				break;
		}
	}
}

void drawPictureV15() {
	byte curByte;
	
	//debugC(8, kDebugLevelMain, "Drawing V1.5 picture");
	
	//while (_dataOffset < _dataSize) {
	while (!vagi_res_eof(pic_res_h)) {
		curByte = getNextByte();
		
		switch (curByte) {
		case 0xf0:
			// happens in all Troll's Tale pictures
			// TODO: figure out what it was meant for
			break;
		case 0xf1:
			draw_SetColor();
			_scrOn = true;
			break;
		case 0xf3:
			if (_flags & kPicFf3Stop)
				return;
			break;
		case 0xf8:
			draw_yCorner(true);
			break;
		case 0xf9:
			draw_xCorner(true);
			break;
		case 0xfa:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfb:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfe:
			draw_SetColor();
			_scrOn = true;
			draw_Fill();
			break;
		case 0xff: // end of data
			return;
		default:
			//warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

//void drawPictureV2() {
void drawPictureV2(word pic_num) {
	byte curByte;
	
	//bool nibbleMode = false;	// false for SQ2, it uses separate bytes for color value of 0xf0 / 0xf2
	//bool mickeyCrystalAnimation = false;
	//int  mickeyIteration = 0;
	#ifdef AGI_PIC_SHOW_PROGRESS
	byte status = 0;	// Status countdown
	#endif
	
	
	// Open resource
	// Use abstract vagi_res API
	pic_res_h = vagi_res_open(AGI_RES_KIND_PIC, pic_num);
	if (pic_res_h < 0) {
		printf("PIC err=-"); printf_d(-pic_res_h); printf("!\n");
		getchar();
		return;
	}
	
	// Prepare:
	//_dataSize = agi_res_size;
	//_dataOffset = 0;
	_dataOffsetNibble = 0;
	
	_width = 160;	//PIC_WIDTH;	//160;
	_height = 168;	//PIC_HEIGHT;	//168;
	
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_scrColor = 15;
	_priColor = 4;
	
	//_pictureVersion = AGIPIC_V1;
	//_pictureVersion = AGIPIC_V15;
	_pictureVersion = AGIPIC_V2;
	_minCommand = 0xf0;
	
	
	//debugC(8, kDebugLevelMain, "Drawing V2/V3 picture");
	//@FIXME: Ignored:
	/*
	if (_vm->_game.dirPic[_resourceNr].flags & RES_PICTURE_V3_NIBBLE_PARM) {
		// check, if this resource uses nibble mode (0xF0 + 0xF2 commands take nibbles instead of bytes)
		nibbleMode = true;
	}
	
	if ((_flags & kPicFStep) && _vm->getGameType() == GType_PreAGI) {
		mickeyCrystalAnimation = true;
	}
	*/
	
	//while (_dataOffset < _dataSize) {
	while (!vagi_res_eof(pic_res_h)) {
		// Draw status
		//lcd_text_col = 0; lcd_text_row = 0; printf("draw: "); printf_d(vagi_res_tell(pic_res_h)); printf(" / "); printf_d(vagi_res_size(pic_res_h));
		
		curByte = getNextByte();	// Get next byte
		
		//printf(" = "); printf_d(curByte); putchar('\n'); getchar();
		//printf_x2(curByte); //putchar('\n'); getchar();
		
		if (
			(curByte < 0xf0)	// aka. _minCommand
			|| (curByte > 0xfa)	// GBAGI only recognizes 0xf0-0xfa
		) {
			//printf("PIC err");
			break;
		}
		
		switch (curByte & 0x0f) {
			case 0x0:	// 0xf0 = VisEnable
				/*
				if (!nibbleMode) {
					draw_SetColor();
				} else {
					draw_SetNibbleColor();
				}
				*/
				#ifdef AGI_PIC_NIBBLE_MODE
					draw_SetNibbleColor();
				#else
					draw_SetColor();
				#endif
				_scrOn = true;
				break;
			case 0x1:	// 0xf1 = VisDisable
				_scrOn = false;
				break;
			case 0x2:	// 0xf2 = PriEnable
				/*
				if (!nibbleMode) {
					draw_SetPriority();
				} else {
					draw_SetNibblePriority();
				}
				*/
				#ifdef AGI_PIC_NIBBLE_MODE
					draw_SetNibblePriority();
				#else
					draw_SetPriority();
				#endif
				_priOn = true;
				break;
			case 0x3:	// 0xf3 = PriDisable
				_priOn = false;
				break;
			case 0x4:	// 0xf4 = YCorner
				draw_yCorner(false);	//@FIXME: true or false?
				break;
			case 0x5:	// 0xf5 = XCorner
				draw_xCorner(false);	//@FIXME: true or false?
				break;
			case 0x6:	// 0xf6 = AbsLine
				draw_LineAbsolute();
				break;
			case 0x7:	// 0xf7 = RelLine
				draw_LineShort();
				break;
			case 0x8:	// 0xf8 = Fill
				draw_Fill();
				#ifdef AGI_PIC_SHOW_PROGRESS
				status = 0;	// Force status update
				#endif
				break;
			case 0x9:	// 0xf9 = ChangePen
				// TODO: should this be getNextParamByte()?
				_patCode = getNextByte();
				/*
				if (_vm->getGameType() == GType_PreAGI)
					plotBrush();
				*/
				break;
			case 0xa:
				plotBrush();	// 0xfa = PlotPen
				break;
			case 0xc:	// 0xfc = ?? fill2?
				draw_SetColor();
				draw_SetPriority();
				draw_Fill();
				#ifdef AGI_PIC_SHOW_PROGRESS
				status = 0;	// Force status update
				#endif
				break;
			case 0xf: // 0xff = end of data
				vagi_res_close(pic_res_h);
				return;
			default:
				//warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
				break;
		}
		/*
		// This is used by Mickey for the crystal animation
		// One frame of the crystal animation is shown on each iteration, based on _currentStep
		if (mickeyCrystalAnimation) {
			if (_currentStep == mickeyIteration) {
				int storedXOffset = _xOffset;
				int storedYOffset = _yOffset;
				// Note that picture coordinates are correct for Mickey only
				showPic(10, 0, _width, _height);
				_xOffset = storedXOffset;
				_yOffset = storedYOffset;
				_currentStep++;
				if (_currentStep > 14)  // crystal animation is 15 frames
					_currentStep = 0;
				// reset the picture step flag - it will be set when the next frame of the crystal animation is drawn
				_flags &= ~kPicFStep;
				return;     // return back to the game loop
			}
			mickeyIteration++;
		}
		*/
		
		/*
		//@FIXME: For debugging!
		// Intermediate draw after each step
		buffer_switch(AGI_FRAME_BANK_LO);
		draw_buffer(AGI_FRAME_BANK_LO, 0,0, false);
		*/
		#ifdef AGI_PIC_SHOW_PROGRESS
		if (status == 0) {	// Only when status variable overflows
			// Status bar
			// 240 pixels with 8 pixel chunks = 30 chunks
			const byte bar_height = 4;
			//word a = LCD_ADDR;	// At top
			word a = LCD_ADDR + (LCD_HEIGHT*LCD_WIDTH/8) - bar_height*(LCD_WIDTH/8);	// At bottom
			
			//byte progress = ((word)_dataOffset * 30) / _dataSize;	// Leads to overflow while multiplying
			//byte progress = ((word)agi_res_ofs * 3) / (agi_res_size / 10);	// Must work with smaller numbers
			byte progress;
			if (vagi_drawing_step == VAGI_STEP_VIS)
				//progress = ((word)agi_res_ofs * 3) / (agi_res_size / 5);	// Must work with smaller numbers
				//progress = ((word)vagi_res_states[pic_res_h].offset * 3) / (vagi_res_states[pic_res_h].res_size / 5);	// Must work with smaller numbers
				progress = ((word)vagi_res_tell(pic_res_h) * 3) / (vagi_res_size(pic_res_h) / 5);	// Must work with smaller numbers
			else
				//progress = 15 + ((word)agi_res_ofs * 3) / (agi_res_size / 5);	// Must work with smaller numbers
				//progress = 15 + ((word)vagi_res_states[pic_res_h].offset * 3) / (vagi_res_states[pic_res_h].res_size / 5);	// Must work with smaller numbers
				progress = 15 + ((word)vagi_res_tell(pic_res_h) * 3) / (vagi_res_size(pic_res_h) / 5);	// Must work with smaller numbers
			
			// Draw bar
			memset((byte *)(a + progress), 0x00, 30);
			a += 30;
			for(byte i = 1; i < bar_height; i++) {
				if (progress > 0) memset((byte *)a, 0xff, progress);
				if (progress < 30) memset((byte *)(a + progress), (i&1) ? 0x55 : 0xAA, 30-progress);
				a += 30;
			}
		}
		status++;
		#endif
	}
	
	
	// Close the resource handle
	vagi_res_close(pic_res_h);
}

/*
void drawPictureAGI256() {
	const uint32 maxFlen = _width * _height;
	int16 x = 0;
	int16 y = 0;
	byte *dataPtr = _data;
	byte *dataEndPtr = _data + _dataSize;
	byte color = 0;
	
	//debugC(8, kDebugLevelMain, "Drawing AGI256 picture");
	
	while (dataPtr < dataEndPtr) {
		color = *dataPtr++;
		_gfx->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, color, 0);
		
		x++;
		if (x >= _width) {
			x = 0;
			y++;
			if (y >= _height) {
				break;
			}
		}
	}
	
	if (_dataSize < maxFlen) {
		warning("Undersized AGI256 picture resource %d, using it anyway. Filling rest with white", _resourceNr);
		while (_dataSize < maxFlen) {
			x++;
			if (x >= _width) {
				x = 0;
				y++;
				if (y >= _height)
					break;
			}
			_gfx->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, 15, 0);
		}
	} else if (_dataSize > maxFlen)
		warning("Oversized AGI256 picture resource %d, decoding only %ux%u part of it", _resourceNr, _width, _height);
}
*/

#endif
