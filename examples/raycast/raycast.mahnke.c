/*
	Raycast
	=======
	
	Raycaster modeled after "MAHNKE QBasic", but optimized.
	
	Keys:
		* Cursor keys or WASD: Rotate/Move
	
	2020-10-06 Bernhard "HotKey" Slawik
*/

//#define TEXT_MODE
//#define GFX_MODE

#if VGLDK_SERIES == 6000
	// GL6000SL defaults to GFX mod e(although it can also do text mode)
	#define GFX_MODE
	
	#define GFX_BLOCKY	// Use simple/fast byte-wise mode (8th of the horiz. resolution, but patterns)
	
	#ifdef GFX_BLOCKY
		// We are drawing 8 bits at a time
		#define SCREEN_W (LCD_W/8)
	#else
		#define SCREEN_W LCD_W
	#endif
	#define SCREEN_H LCD_H
#else
	// Default is text mode
	#define TEXT_MODE
	
	#define SCREEN_W LCD_COLS
	#define SCREEN_H LCD_ROWS
#endif


#define lcd_MINIMAL	// Use minimal text mode (disable scrolling)
#include <vgldk.h>
#include <stdiomin.h>

// Helper to show some text
#define alert(s) { lcd_x = 0; lcd_y = 0; puts(s); getchar(); }
//#define alert(s) ;

#define FISHEYE_CORRECTION
#define OVER 1	// Oversample coordinates (increases overall spacial resolution)



// Auto-generated by sinetable.mahnke.py
#define SINTABLE_INDEX_TYPE unsigned char
#define SINTABLE_SIZE 256
#define SINTABLE_VALUE_TYPE signed char
#define SINTABLE_OVER 127
const SINTABLE_VALUE_TYPE SINTABLE[256] = {
	   0,    3,    6,    9,   12,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,   49,   51,   54,   57,   60,   63,   65,   68,   71,   73,   76,   78,   81,   83,   85,   88,
	  90,   92,   94,   96,   98,  100,  102,  104,  106,  107,  109,  111,  112,  113,  115,  116,  117,  118,  120,  121,  122,  122,  123,  124,  125,  125,  126,  126,  126,  127,  127,  127,
	 127,  127,  127,  127,  126,  126,  126,  125,  125,  124,  123,  122,  122,  121,  120,  118,  117,  116,  115,  113,  112,  111,  109,  107,  106,  104,  102,  100,   98,   96,   94,   92,
	  90,   88,   85,   83,   81,   78,   76,   73,   71,   68,   65,   63,   60,   57,   54,   51,   49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   12,    9,    6,    3,
	   0,   -3,   -6,   -9,  -12,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,  -49,  -51,  -54,  -57,  -60,  -63,  -65,  -68,  -71,  -73,  -76,  -78,  -81,  -83,  -85,  -88,
	 -90,  -92,  -94,  -96,  -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
	-127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100,  -98,  -96,  -94,  -92,
	 -90,  -88,  -85,  -83,  -81,  -78,  -76,  -73,  -71,  -68,  -65,  -63,  -60,  -57,  -54,  -51,  -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -12,   -9,   -6,   -3
};
// end of auto-generated sine table



SINTABLE_VALUE_TYPE _sin(SINTABLE_INDEX_TYPE a) {
	return SINTABLE[a % SINTABLE_SIZE];
}
SINTABLE_VALUE_TYPE _cos(SINTABLE_INDEX_TYPE a) {
	return SINTABLE[(a + (SINTABLE_SIZE/4)) % SINTABLE_SIZE];
}

// Player state
int player_x_over;
int player_y_over;
int player_a;

#define SPEED_MOVE (OVER*16)	// Speed for moving forward/back
#define SPEED_STRAFE (OVER*8)	// Speed for moving sideways
#define SPEED_TURN (SINTABLE_SIZE/32)	// Speed for turning


//const byte cols = 20;
//const byte rows = 4;
//const byte cols = SCREEN_W;
//const byte rows = SCREEN_H;
#define cols SCREEN_W
#define rows SCREEN_H

//const byte colstep = 1;	// Set to >1 to decrease horizontal resolution
#define colstep 1
//const byte angf = cols / 4;	// Resolution of sine table, at least 2-4 per column for good measure
//#define angf (cols / 4)
#define angf SINTABLE_SIZE
//const byte angtol = angf / 256;	// Resolution of edge finder, keep it around 1-2 degrees or 2-4 columns
#define angtol (angf / 64)

// Field of view
#define fovangf (angf / 4)	// Keep it around angf/6

#define DRAW_DIST 4	// Maximum drawing distance (inner loop). If too big there will be z-wrap-arounds.

//const byte tex_w = 64;	// Width of textures
//const byte tex_h = 64;	// Height of textures
//const byte tex_count = 12;	//7
#define tex_w 64	// Width of textures
#define tex_h 64	// Height of textures
#define tex_count 12	//7

//const int grid_x = 64;	// Width of level blocks
//const int grid_y = 64;	// Length of level blocks
//const int wall_height = 64;	// Height of level blocks
#define grid_x 64	// Width of level blocks
#define grid_y 64	// Length of level blocks
#define wall_height 64	// Height of level blocks


// Map
#define levelmap_w 16
#define levelmap_h 16
#define LEVEL_BLOCK_FREE ' '	// Which character means "can walk there"?
const char levelmap[levelmap_w][levelmap_h] = {
	{'#','#','#','#','#','#','%','?','?','#','#','#','#','#','#','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ', 32,' ',' ',' ',' ',' ',' ',' ','?'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'},
};


// Helpers
//const byte colsh = cols / 2;
//const byte rowsh = rows / 2;
#define colsh (cols / 2)
#define rowsh (rows / 2)

#define grid_x_over (grid_x * OVER)
#define grid_y_over (grid_y * OVER)
#define wall_height_over (wall_height * OVER)

//const byte fovangh = fovangf / 2;	// Half of FOV
//const byte angh = angf / 2;	// Half or "180 degrees"
//const byte angq = angf / 4;	// Quarter or "90 degrees"
//const byte angtq = 3 * angf / 4;	// Three quarters or "270 degrees"
//const byte angtol2 = angh - angtol;	// Other edge finder angle
#define fovangh (fovangf / 2)	// Half of FOV
#define angh (angf / 2)	// Half or "180 degrees"
#define angq (angf / 4)	// Quarter or "90 degrees"
#define angtq (3 * angf / 4)	// Three quarters or "270 degrees"
#define angtol2 (angh - angtol)	// Other edge finder angle


#ifdef TEXT_MODE
	/*
	void lcd_putchar_at(int x, int y, char c) {
		lcd_x = x;
		lcd_y = y;
		lcd_putchar(c);
	}
	*/
	
	#define CHAR_CEIL ' '
	#define CHAR_WALL 'X'
	#define CHAR_FLOOR ':'
	//#define CHAR_FLOOR ' '
	
	#define NUM_COL_PATTERNS 8
	// Column graphics
	const char COL_PATTERNS[NUM_COL_PATTERNS][SCREEN_H] = {
		{
			CHAR_WALL,
			CHAR_WALL,
			CHAR_WALL,
			CHAR_WALL,
		},
		{
			'-',
			CHAR_WALL,
			CHAR_WALL,
			CHAR_WALL,
		},
		
		{
			'_',
			CHAR_WALL,
			CHAR_WALL,
			'_',
		},
		{
			CHAR_CEIL,
			'-',
			CHAR_WALL,
			'-',
		},
		{
			CHAR_CEIL,
			'_',
			'_',
			CHAR_FLOOR,
		},
		{
			CHAR_CEIL,
			CHAR_CEIL,
			'=',
			CHAR_FLOOR,
		},
		{
			CHAR_CEIL,
			CHAR_CEIL,
			'-',
			CHAR_FLOOR,
		},
		/*
		{
			CHAR_CEIL,
			CHAR_CEIL,
			'_',
			CHAR_FLOOR,
		},
		*/
		{
			CHAR_CEIL,
			CHAR_CEIL,
			CHAR_CEIL,
			CHAR_FLOOR,
		},
	};
	
	
	void drawColumn(byte x, int z, char b) {
		// Draw one column of graphics
		byte y;
		char c;
		
		
		
		z /= grid_y_over;
		if ((z <= 0) || (z >= NUM_COL_PATTERNS)) z = NUM_COL_PATTERNS-1;	// z clip
		lcd_putchar_at(x, 1, '0'+z);
		
		
		for(y = 2; y < SCREEN_H; y++) {
			c = COL_PATTERNS[z][y];
			//if (c == CHAR_WALL) c = b;	// Replace wall character by block character
			lcd_putchar_at(x, y, c);
		}
		
	}
#endif

#ifdef GFX_MODE
	
	#define FB_INC (LCD_W/8)	// How far to increment on the frame buffer to be on next line
	
	void drawVLine_blocky(int x, int y1, int h, byte c) {
		byte *p;
		int y;
		if (h <= 0) return;
		if (y1 < 0) y1 = 0;
		if (y1+h >= SCREEN_H) h = SCREEN_H - y1 - 1;
		
		//c = 0xff = black
		p = (byte *)lcd_addr + y1*FB_INC + x;
		for(y = y1; y < y1+h; y++) {
			//p = (byte *)lcd_addr + (y * lcd_w + x) / 8;
			//p = (byte *)lcd_addr + (y * lcd_w/8 + x);
			*p = c;
			p += FB_INC;
		}
	}
	
	//void drawVLine_fine(int x, int y1, int h, byte c) {
	void drawVLine_fine(int x, int y1, int y2, byte c) {
		byte *p;
		int y;
		//byte c_andor;
		byte c_or;
		byte c_and;
		
		
		//if (h <= 0) return;
		if (y2 < y1) return;
		
		if (y1 < 0) y1 = 0;
		
		//if (y1+h >= SCREEN_H) h = SCREEN_H - y1 - 1;
		//y2 = y1 + h;
		if (y2 >= SCREEN_H) y2 = SCREEN_H - 1;
		
		// Start offset in frame buffer
		p = (byte *)lcd_addr + y1*FB_INC + (x >> 3);
		
		// Create the bitmask for that pixel column
		c_or = 1 << (7 - (x & 0x07));
		
		//if (c > 0) {
		if (c == 0xff) {
			// Color it (increase it / set bits via OR)
			//for(y = 0; y < h; y++) {
			for(y = y1; y <= y2; y++) {
				*p |= c_or;
				p += FB_INC;
			}
			return;
		}
		
		//else {
		if (c == 0x00) {
			// De-color it (decrease it / unset bits via AND)
			c_and = 0xff ^ c_or;
			//for(y = 0; y < h; y++) {
			for(y = y1; y <= y2; y++) {
				*p &= c_and;
				p += FB_INC;
			}
			return;
		}
		
		// Allow patterns
		c_and = 0xff ^ c_or;
		//for(y = 0; y < h; y++) {
		for(y = y1; y <= y2; y++) {
			if ((c & (1 << (y%8))) > 0)
				*p |= c_or;
			else
				*p &= c_and;
			
			p += FB_INC;
		}
		
	}
	
	
	void drawColumn(int x, int z, char b) {
		// Draw one column of graphics
		
		byte c;
		unsigned int h;
		byte y;
		byte yStart;
		
		#ifdef GFX_BLOCKY
		byte *p;
		#endif
		
		if (z < 1) {
			h = 0;
		} else {
			//if (z > OVERSAMPLE_RAY*MAX_DEPTH-1) z = OVERSAMPLE_RAY*MAX_DEPTH-1;	// z clip
			
			h = (wall_height_over * rows) / z;
			h = h / OVER;
			
			if (h > SCREEN_H) h = SCREEN_H;
			//else if (h < 0) h = 0;
		}
		// Draw column
		yStart = SCREEN_H/2 - h/2;
		
		// Wall pattern
		switch(b) {
			case LEVEL_BLOCK_FREE:	c = 0x00; break;
			case '?':	c = 1+8+64; break;
			case '%':	c = 0x55; break;
			case '#':	c = 0xff; break;
			default:
				c = 1+16;
		}
		
		#ifdef GFX_BLOCKY
			// 8-bit at once "blocky" mode
			// Ceiling
			y = 0;
			p = (byte *)(lcd_addr + x);	//(y * lcd_w/8 + x));
			while(y < yStart) {
				*p = 0x00;	// white
				//*p = ((y%2==0) ? 0x55 : 0xaa);	// 50% gray
				//*p = (y%2==0) ? (1+16) : (4+64);	// 25% gray
				p += FB_INC;
				y++;
			}
			
			// Wall
			drawVLine_blocky(x, yStart, h, c);
			
			// Floor
			y = yStart + h;
			p = (byte *)(lcd_addr + (y * lcd_w/8 + x));
			while(y < SCREEN_H) {
				//*(byte *)(lcd_addr + (y * lcd_w/8 + x)) = ((y%2==0) ? 0x55 : 0xaa);
				//*p = ((y%2==0) ? 0x55 : 0xaa);	// 50% gray
				*p = (y%2==0) ? (1+16) : (4+64);	// 25% gray
				//*p = ((y%2==0) ? 0x55 : 0xaa);	// stripes
				p += FB_INC;
				y++;
			}
		#else
			// Bit-wise "fine" mode
			
			// Ceiling
			drawVLine_fine(x, 0, yStart-1, 0x00);
			
			// Wall
			y = yStart + h;
			drawVLine_fine(x, yStart, y, c);
			
			// Floor
			//drawVLine_fine(x, y, rows, 0x00);	// white
			//drawVLine_fine(x, y, rows, ((y%2==0) ? 0x55 : 0xaa));	// 50% gray
			drawVLine_fine(x, y+1, rows, (x%2==0) ? (1+16) : (4+64));	// 25% gray
			//drawVLine_fine(x, y, rows, ((y%2==0) ? 0x55 : 0xaa));	// stripes
			
		#endif
		
	}
#endif




void drawScreen() {
	/*
	int last_y1;
	int last_y2;
	int last_nblockx;
	int last_nblocky;
	int last_dis;
	int last_vertical;
	*/
	
	signed int x;
	//SINTABLE_INDEX_TYPE an, ancheck;
	signed int an, ancheck;
	//SINTABLE_VALUE_TYPE ans, anc;
	signed int ans, anc;
	signed int count;
	signed int rest;
	byte vertical;
	
	signed int shortx, normalx;
	signed int shorty, normaly;
	
	signed char nblockx1, nblocky1;
	signed char nblockx2, nblocky2;
	byte fblock1;
	byte fblock2;
	signed int dir1, dis1, actdef1, texoff1;
	signed int dir2, dis2, actdef2, texoff2;
	
	
	//lcd_clear();
	for(x = 0; x < cols; x += colstep) {
		an = (player_a + angf + (x * fovangf) / cols - fovangh) % angf;
		
		ans = _sin(an);
		anc = _cos(an);
		
		// Horizontal wall detection
		nblockx1 = 0;
		nblocky1 = 0;
		fblock1 = LEVEL_BLOCK_FREE;
		dir1 = 0;
		dis1 = -1;
		actdef1 = 0;
		texoff1 = 0;
		ancheck = an % angh;
		if ((ancheck >= angtol) && (ancheck <= angtol2)) {
			dir1 = (an > angh) ? -1 : 1;
			
			//rest:int = abs((player.y_over % grid_y_over) - (dir1 * grid_y_over)) % grid_y_over
			rest = (int)(player_y_over % grid_y_over) - ((int)dir1 * (int)grid_y_over);
			if (rest < 0) rest = -rest;
			rest = rest % grid_y_over;
			if ((rest == 0) && (dir1 > 0)) rest = grid_y_over;	// Prevent flicker at player x=128, y=192
			
			shortx = ((int)rest * (int)dir1 * (int)anc) / (int)ans;
			actdef1 = shortx;
			nblockx1 = (player_x_over + shortx) / grid_x_over;
			nblocky1 = (player_y_over / grid_y_over) + dir1;
			if ((nblockx1 >= 0) && (nblockx1 < levelmap_w)) {
				fblock1 = levelmap[nblocky1][nblockx1];
				if (fblock1 != LEVEL_BLOCK_FREE) {
					// Standing in front of a block
					dis1 = rest;
				} else {
					//dis1 = -1;
					normalx = ((int)grid_y_over * (int)dir1 * (int)anc) / (int)ans;
					
					for(count = 1; count < DRAW_DIST; count++) {
						actdef1 += normalx;
						nblockx1 = (player_x_over + actdef1) / grid_x_over;	// trunc!
						nblocky1 += dir1;
						//if ((nblockx1 >= 0) && (nblockx1 < levelmap_w) && (nblocky1 >= 0) && (nblocky1 < levelmap_h)) {
						//if ((nblockx1 >= 0) && (nblockx1 < levelmap_w)) {
						if ((nblockx1 < 0) || (nblockx1 >= levelmap_w)) {
							fblock1 = LEVEL_BLOCK_FREE;	// resp. LEVEL_BLOCK_OOB
							break;
						}
						fblock1 = levelmap[nblocky1][nblockx1];
						if (fblock1 != LEVEL_BLOCK_FREE) {
							dis1 = (int)rest + (int)count * (int)grid_y_over;
							break;
						}
					}
				}
			}
			
			
			if (dis1 != -1) {
				#ifdef FISHEYE_CORRECTION
					// Fisheye correction
					//dis1 = abs(int(dis1 * costable[abs((x-colsh)*fovangf//cols)]/ans))	# // OVER
					ancheck = (x-colsh)*fovangf / cols;
					if (ancheck < 0) ancheck = -ancheck;
					dis1 = (dis1 * _cos(ancheck)) / ans;
				#else
					dis1 = ((int)SINTABLE_OVER * (int)dis1) / (int)ans;
				#endif
				if (dis1 < 0) dis1 = -dis1;
				
				//texoff1 = (abs(player.x_over + actdef1) / OVER) % tex_w;
				texoff1 = (player_x_over + actdef1) / OVER;
				if (texoff1 < 0) texoff1 = -texoff1;
				texoff1 = texoff1 % tex_w;
			}
			
		}
		
		// Vertical wall detection
		nblockx2 = 0;
		nblocky2 = 0;
		fblock2 = LEVEL_BLOCK_FREE;
		dir2 = 0;
		dis2 = -1;
		actdef2 = 0;
		texoff2 = 0;
		ancheck = (an + angq) % angh;
		if ((ancheck >= angtol) && (ancheck <= angtol2)) {
			dir2 = ((an % angtq) > angq) ? -1 : 1;
			
			//rest = abs((player.x_over % grid_x_over) - (dir2 * grid_x_over)) % grid_x_over
			rest = (int)(player_x_over % grid_x_over) - ((int)dir2 * grid_x_over);
			if (rest < 0) rest = -rest;
			rest = rest % grid_x_over;
			if ((rest == 0) && (dir2 > 0)) rest = grid_x_over;	// Prevent flicker at player x=128, y=192
			
			shorty = (int)rest * (int)dir2 * (int)ans / (int)anc;
			actdef2 = shorty;
			nblocky2 = (player_y_over + shorty) / grid_y_over;
			nblockx2 = (player_x_over / grid_x_over) + dir2;
			if ((nblocky2 >= 0) && (nblocky2 < levelmap_h)) {
				fblock2 = levelmap[nblocky2][nblockx2];
				if (fblock2 != LEVEL_BLOCK_FREE) {
					// Standing in front of a block
					dis2 = rest;
				} else {
					//dis2 = -1;
					normaly = (int)grid_x_over * (int)dir2 * (int)ans / (int)anc;
					
					for(count = 1; count < DRAW_DIST; count++) {
						actdef2 += normaly;
						nblocky2 = (player_y_over + actdef2) / grid_y_over;	// trunc!
						nblockx2 += dir2;
						//if ((nblockx2 >= 0) && (nblockx2 < levelmap_w) && (nblocky2 >= 0) && (nblocky2 < levelmap_h)) {
						//if ((nblocky2 >= 0) && (nblocky2 < levelmap_h)) {
						if ((nblocky2 < 0) || (nblocky2 >= levelmap_h)) {
							fblock2 = LEVEL_BLOCK_FREE;	// resp. LEVEL_BLOCK_OOB
							break;
						}
						fblock2 = levelmap[nblocky2][nblockx2];
						if (fblock2 != LEVEL_BLOCK_FREE) {
							dis2 = rest + (int)count * grid_x_over;
							break;
						}
					}
				}
			}
			
			
			if (dis2 != -1) {
				#ifdef FISHEYE_CORRECTION
					// Fisheye correction
					//dis2 = abs(int(dis2 * costable[abs((x-colsh)*fovangf//cols)]/anc))	# // OVER
					ancheck = (x-colsh)*fovangf / cols;
					if (ancheck < 0) ancheck = -ancheck;
					dis2 = (dis2 * _cos(ancheck)) / anc;
				#else
					dis2 = ((int)SINTABLE_OVER * (int)dis2) / (int)anc;
				#endif
				if (dis2 < 0) dis2 = -dis2;
				
				//texoff2 = (abs(player.x_over + actdef2) / OVER) % tex_w;
				texoff2 = (player_y_over + actdef2) / OVER;
				if (texoff2 < 0) texoff2 = -texoff2;
				texoff2 = texoff2 % tex_w;
			}
			
		}
		
		
		//dis1 = -1;	// For testing: Only show result of one intersection
		
		// Determine which of the ones (H/V) to use
		vertical = 0;
		if ((dis1 == -1) && (dis2 != -1)) {
			dis1 = dis2;
			vertical = 1;
		}
		if ((dis2 != -1) && (dis1 != -1) && (dis2 < dis1)) {
			dis1 = dis2;
			vertical = 1;
		}
		/*
		if (dis1 == -1)
			dis1 = 0;
		*/
		if (vertical == 1) {
			// Use values of vertical intersection
			texoff1 = texoff2;
			fblock1 = fblock2;
			
			nblockx1 = nblockx2;
			nblocky1 = nblocky2;
			actdef1 = actdef2;
			dir1 = dir2;
		}
		
		
		
		// Actually draw the column
		//if (fblock1 != LEVEL_BLOCK_FREE) {
		drawColumn(x, dis1, fblock1);
		//}
		
		/*
		// Debug display
		if ((x > 0) && (x % 4 == 0)) {
			lcd_x = x-2;	lcd_y = 2;	printf_d(ancheck);
			
			//lcd_x = x-2;	lcd_y = 4;	printf_d((int)dis1 / 256);
			//lcd_x = x-2;	lcd_y = 5;	printf_d(dis1 % 256);
		
			lcd_x = x-2;	lcd_y = 4;	printf_d((int)dis1 / grid_x_over);
			
		}
		*/
		
		//@TODO: Port my ASCII renderer
		/*
		# Draw ASCII
		if (USE_ASCII) and (x % ASCII_STEP == 0):
			ax:int = x // ASCII_STEP
			
			# Clear column
			#for ay in range(ASCII_ROWS): ascii_buffer[ay][ax] = ' '
			
			# Show corners
			if (dis1 == 0):
				alinehd = 0
			else:
				alinehd = round(wall_height_over * 2*ASCII_ROWS / dis1)
			
			ah1 = round(ASCII_ROWS - alinehd/2 - 1)	# "-1" to allow independent rounding of top/bottom
			ay1 = ah1 // 2
			if (ay1 >= 0) and (ay1 < ASCII_ROWS):
				ac = '-' if ah1%2 == 0 else '_'
				#if (vertical == last_vertical):
				#	if (last_y1 < ay1): ac = '\\'
				#	if (last_y1 > ay1): ac = '/'
				ascii_buffer[ay1][ax] = ac
			
			ah2 = round(ASCII_ROWS + alinehd/2)
			ay2 = ah2 // 2
			if (ay2 >= 0) and (ay2 < ASCII_ROWS):
				ac = '-' if ah2%2 == 0 else '_'
				#if (vertical == last_vertical):
				#	if (last_y2 < ay2): ac = '\\'
				#	if (last_y2 > ay2): ac = '/'
				ascii_buffer[ay2][ax] = ac
			
			# Sky
			if (ay1 > 0):
				for ay in range(0, ay1): ascii_buffer[ay][ax] = ' '
			
			# Wall
			if (ay1+1 < ay2):
				for ay in range(max(ay1+1, 0), min(ay2, ASCII_ROWS)):
					
					# Show block character
					#ascii_buffer[ay][ax] = chr(64 + fblock1)
					
					# Solid
					#ascii_buffer[ay][ax] = '#'
					ascii_buffer[ay][ax] = ' '
			
			# Floor
			if (ay2 < ASCII_ROWS-1):
				for ay in range(ay2+1, ASCII_ROWS): ascii_buffer[ay][ax] = ':'
			
			
			# Calc edge
			if (ax > 0) and ((last_vertical != vertical) or (last_nblockx != nblockx1) or (last_nblocky != nblocky1)):
				# Changed orientation: Show edge!
				# Edge should span the maximum of both heights
				ay1m = min(ay1, last_y1)
				ay2m = max(ay2, last_y2)
				ay = ay1m
				while (ay <= ay2m):
					if (ay >= 0) and (ay < ASCII_ROWS):
						c = ascii_buffer[ay][ax]
						if (c == '-') or (c == '_'): ascii_buffer[ay][ax] = '+'
						else:
							ascii_buffer[ay][ax] = '|'
					ay += 1
			
			last_y1 = ay1
			last_y2 = ay2
			last_nblockx = nblockx1
			last_nblocky = nblocky1
			last_vertical = vertical
			last_dis = dis1
		*/
		
	}
}



void main() {
	char c;
	int move_a;	// Rotation
	int move_x;	// Strafe
	int move_z;	// Move forward
	//SINTABLE_VALUE_TYPE sx;
	//SINTABLE_VALUE_TYPE sz;
	int ans, anc;
	int sx;
	int sz;
	
	// Setup text mode
	//lcd_scroll_cb = NULL;	// Disable auto-scroll
	lcd_x = 0; lcd_y = 0;
	
	//player_x_over = 29 * grid_x * OVER;
	//player_y_over = (57 - 3) * grid_y * OVER;
	player_x_over = levelmap_w/2 * grid_x_over;
	//player_y_over = levelmap_h/2 * grid_y_over;
	player_y_over = 3 * grid_y_over;
	//player_a = 3 * SINTABLE_SIZE / 4;	// Look upwards
	//player_a = 2 * SINTABLE_SIZE / 4;	// Look left
	player_a = 1 * SINTABLE_SIZE / 4;	// Look down
	//player_a = 0;	// Look right
	//player_a = 32;	// Look down/right
	
	while(1) {
		
		drawScreen();
		
		lcd_x = 0; lcd_y = 0;
		printf_d(player_x_over/grid_x_over);
		printf_d(player_y_over/grid_y_over);
		printf_d(player_a);
		
		/*
		lcd_x = 0; lcd_y = 0;
		puts("a="); printf_d(player_a);
		puts(" x="); printf_d(player_x);
		puts(" y="); printf_d(player_y);
		*/
		
		// Check keyboard
		move_a = 0;
		move_x = 0;
		move_z = 0;
		
		c = getchar();
		switch(c) {
			case 'a':
			case 'A':
			#ifdef KEY_CURSOR_LEFT
			case KEY_CURSOR_LEFT:
			#endif
			#ifdef KEY_LEFT
			case KEY_LEFT:
			#endif
				move_a = -1;
				break;
			case 'd':
			case 'D':
			#ifdef KEY_CURSOR_RIGHT
			case KEY_CURSOR_RIGHT:
			#endif
			#ifdef KEY_RIGHT
			case KEY_RIGHT:
			#endif
				move_a = 1;
				break;
			
			case 'w':
			case 'W':
			#ifdef KEY_UP
			case KEY_UP:
			#endif
				move_z = 1;
				break;
			case 's':
			case 'S':
			#ifdef KEY_DOWN
			case KEY_DOWN:
			#endif
				move_z = -1;
				break;
			
			case 'n':
			case 'N':
				move_x = -1;
				break;
			case 'm':
			case 'M':
				move_x = 1;
				break;
		}
		
		
		// Handle movement
		if ((move_a != 0) || (move_x != 0) || (move_z != 0)) {
			//player_a = (player_a + SINTABLE_SIZE - (move_a * SPEED_TURN)) % SINTABLE_SIZE;
			
			player_a += (move_a * SPEED_TURN);
			if (player_a < 0) player_a += SINTABLE_SIZE;
			else player_a = player_a % SINTABLE_SIZE;
			
			ans = _sin(player_a);
			anc = _cos(player_a);
			
			// Movement forward/back
			sx = move_z * anc * SPEED_MOVE;
			sz = move_z * ans * SPEED_MOVE;
			
			// Movement sideways
			sx += move_x * -ans * SPEED_STRAFE;
			sz += move_x * anc * SPEED_STRAFE;
			//sz = _sin(player_a + SINTABLE_SIZE/4);
			
			// Move player position
			player_x_over += sx / SINTABLE_OVER;
			player_y_over += sz / SINTABLE_OVER;
			
			/*
			// Clip to map boundaries
			if (player_x_over < 0) {
				alert("X < 0!");
				player_x_over = 0;
			} else
			if (player_x_over >= (levelmap_w * grid_x_over)) {
				alert("X > max!");
				player_x_over = (levelmap_w-1) * grid_x_over;
			}
			
			if (player_y_over < 0) {
				alert("Y < 0!");
				player_y_over = 0;
			} else
			if (player_y_over >= (levelmap_h * grid_y_over)) {
				alert("Y > max!");
				player_y_over = (levelmap_h-1) * grid_y_over;
			}
			
			
			
			// Into wall? Move back!
			//while
			if (levelmap[player_y_over/grid_y_over][player_x_over/grid_x_over] != LEVEL_BLOCK_FREE) {
				alert("Bump!");
				
				player_x_over -= sx / SINTABLE_OVER;
				player_y_over -= sz / SINTABLE_OVER;
			}
			*/
			
		}
		
	}
	
	
	//while(1) { }
}
