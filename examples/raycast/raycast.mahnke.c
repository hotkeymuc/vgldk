/*
	Raycast
	=======
	
	Raycaster after MAHNKE
	
	Keys:
		* Cursor keys or WASD: Rotate/Move
	
	2020-10-06 Bernhard "HotKey" Slawik
*/

//#define TEXT_MODE
//#define GFX_MODE

#if VGLDK_SERIES == 6000
	// GL6000SL defaults to GFX mod e(although it can also do text mode)
	#define GFX_MODE
	
	//#define SCREEN_W 240
	//#define SCREEN_W 30
	//#define SCREEN_H 100
	
	// We are drawing 8 bits at a time
	#define SCREEN_W (LCD_W/8)
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


//#define FISHEYE_CORRECTION

    


// Auto-generated by sintable.py
#define SINTABLE_INDEX_TYPE unsigned int
#define SINTABLE_SIZE 128
#define SINTABLE_VALUE_TYPE signed int
#define SINTABLE_OVER 128
const SINTABLE_VALUE_TYPE SINTABLE[128] = {
	   0,    6,   13,   19,   25,   31,   37,   43,   49,   55,   60,   66,   71,   76,   81,   86,   91,   95,   99,  103,  106,  110,  113,  116,  118,  121,  122,  124,  126,  127,  127,  128,
	 128,  128,  127,  127,  126,  124,  122,  121,  118,  116,  113,  110,  106,  103,   99,   95,   91,   86,   81,   76,   71,   66,   60,   55,   49,   43,   37,   31,   25,   19,   13,    6,
	   0,   -6,  -13,  -19,  -25,  -31,  -37,  -43,  -49,  -55,  -60,  -66,  -71,  -76,  -81,  -86,  -91,  -95,  -99, -103, -106, -110, -113, -116, -118, -121, -122, -124, -126, -127, -127, -128,
	-128, -128, -127, -127, -126, -124, -122, -121, -118, -116, -113, -110, -106, -103,  -99,  -95,  -91,  -86,  -81,  -76,  -71,  -66,  -60,  -55,  -49,  -43,  -37,  -31,  -25,  -19,  -13,   -6
};
// end of auto-generated sine table



SINTABLE_VALUE_TYPE _sin(SINTABLE_INDEX_TYPE a) {
	return SINTABLE[a];
}
SINTABLE_VALUE_TYPE _cos(SINTABLE_INDEX_TYPE a) {
	return SINTABLE[(a + (SINTABLE_SIZE/4)) % SINTABLE_SIZE];
}

// Player state
int player_x_over;
int player_y_over;
int player_a;

#define SPEED_MOVE (OVER*4)	// Speed for moving forward/back
#define SPEED_STRAFE (OVER*1)	// Speed for moving sideways
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
#define angtol 0	//(angf / 256)	// Resolution of edge finder, keep it around 1-2 degrees or 2-4 columns

// Field of view
#define fovangf (angf / 8)	// Keep it around angf/6

#define OVER 4	// Oversample coordinates
#define DRAW_DIST 16	// Maximum drawing distance (inner loop)

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
	{'#','#','#','#','#','#','#','#','#','#','#','#',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ','%',' ',' ','?',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','%',' ',' ',' ','#'},
	{'#',' ','%',' ',' ',' ',' ', 32, 32,' ',' ',' ',' ','?',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ','%',' ',' ',' ',' ',' ',' ',' ',' ','%',' ',' ',' ','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#',' ',' ',' ','#'},
	{'#',' ','%',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#'},
	{'#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ','%',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#'},
	{'#',' ','#','#','#','#','#','#','#','#','#','#','#','#','#','#'},
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
	#define OVERSAMPLE_RAY 1	// When pathtracing use enhanced z resolution
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
	
	#define MAX_DEPTH DRAW_DIST
	// Column graphics
	const char COL_PATTERNS[MAX_DEPTH][SCREEN_H] = {
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
	
	
	void drawCol(byte x, int z, char b) {
		// Draw one column of graphics
		byte y;
		char c;
		
		z /= OVERSAMPLE_RAY;
		if (z >= MAX_DEPTH) z = MAX_DEPTH-1;	// z clip
		
		for(y = 0; y < SCREEN_H; y++) {
			c = COL_PATTERNS[z][y];
			if (c == CHAR_WALL) c = b;	// Replace wall character by block character
			lcd_putchar_at(x, y, c);
		}
	}
#endif

#ifdef GFX_MODE
	#define OVERSAMPLE_RAY 1	// Enhanced ray cast resolution - This hugely increases calculation time!
	
	#define MAX_DEPTH (DRAW_DIST*OVER)
	
	#define FB_INC (LCD_W/8)	// How far to increment on the frame buffer to be on next line
	void drawVLine(int x, int y1, int h, byte c) {
		byte *p;
		int y;
		if (y1 < 0) y1 = 0;
		if (y1+h > SCREEN_H) h = SCREEN_H - y1;
		
		//c = 0xff = black
		p = (byte *)lcd_addr + y1*FB_INC + x;
		for(y = y1; y < y1+h; y++) {
			//p = (byte *)lcd_addr + (y * lcd_w + x) / 8;
			//p = (byte *)lcd_addr + (y * lcd_w/8 + x);
			*p = c;
			p += FB_INC;
		}
	}
	
	void drawCol(int x, int z, char b) {
		// Draw one column of graphics
		
		
		byte c;
		unsigned int h;
		//word zz;
		byte y;
		byte yStart;
		
		byte *p;
		
		if (z < 1) return;
		//if (z > OVERSAMPLE_RAY*MAX_DEPTH-1) z = OVERSAMPLE_RAY*MAX_DEPTH-1;	// z clip
		
		switch(b) {
			case LEVEL_BLOCK_FREE:	c = 0x00; break;
			case '?':	c = 1+8+64; break;
			case '%':	c = 0x55; break;
			case '#':	c = 0xff; break;
			default:
				c = 1+16;
		}
		
		
		// Calculate the height of the wall column
		//h = (MAX_DEPTH * SCREEN_H * OVERSAMPLE_RAY) / (z*4 + 1);
		
		// This is the tricky part (making it look nice)
		//h = (SCREEN_H * OVERSAMPLE_RAY*OVERSAMPLE_RAY) / (1 + z*z);
		//h = (SCREEN_H * OVERSAMPLE_RAY) / (1 + 3*z);
		//h = (SCREEN_H * OVERSAMPLE_RAY) / (1 + z);
		h = (wall_height_over * rows) / z;
		h = h / OVER;
		
		if (h > SCREEN_H) h = SCREEN_H;
		//else if (h < 0) h = 0;
		
		// Draw column
		yStart = SCREEN_H/2 - h/2;
		
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
		drawVLine(x, yStart, h, c);
		
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
		if ((ancheck > angtol) && (ancheck < angtol2)) {
			dir1 = (an > angh) ? -1 : 1;
			
			//rest:int = abs((player.y_over % grid_y_over) - (dir1 * grid_y_over)) % grid_y_over
			rest = (player_y_over % grid_y_over) - (dir1 * grid_y_over);
			if (rest < 0) rest = -rest;
			rest = rest % grid_y_over;
			if ((rest == 0) && (dir1 > 0)) rest = grid_y_over;	// Prevent flicker at player x=128, y=192
			
			shortx = dir1 * rest * anc / ans;
			actdef1 = shortx;
			nblockx1 = (player_x_over + shortx) / grid_x_over;
			nblocky1 = (player_y_over / grid_y_over) + dir1;
			if ((nblockx1 >= 0) && (nblockx1 < levelmap_w)) {
				dis1 = rest;
				fblock1 = levelmap[nblocky1][nblockx1];
				if (fblock1 == LEVEL_BLOCK_FREE) {
					dis1 = -1;
					normalx = dir1 * grid_y_over * anc / ans;
					for(count = 1; count < DRAW_DIST; count++) {
						actdef1 += normalx;
						nblockx1 = (player_x_over + actdef1) / grid_x_over;	// trunc!
						nblocky1 += dir1;
						//if (nblockx1 >= 0) and (nblockx1 < levelmap_w) and (nblocky1 >= 0) and (nblocky1 < levelmap_h):
						if ((nblockx1 >= 0) && (nblockx1 < levelmap_w)) {
							fblock1 = levelmap[nblocky1][nblockx1];
							if (fblock1 != LEVEL_BLOCK_FREE) {
								dis1 = rest + (count * grid_y_over);
								break;
							}
						}
					}
				}
			}
			
			
			if (dis1 != -1) {
				#ifdef FISHEYE_CORRECTION
					// Fisheye correction
					//@TODO: Numerical unstable
					//dis1 = abs(int(dis1 * costable[abs((x-colsh)*fovangf//cols)]/ans))	# // OVER
					ancheck = (x-colsh)*fovangf / cols;
					if (ancheck < 0) ancheck = -ancheck;
					dis1 = (dis1 * _cos(ancheck)) / ans;
				#else
					dis1 = (SINTABLE_OVER * dis1) / ans;
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
		if ((ancheck > angtol) && (ancheck < angtol2)) {
			dir2 = ((an % angtq) > angq) ? -1 : 1;
			
			//rest = abs((player.x_over % grid_x_over) - (dir2 * grid_x_over)) % grid_x_over
			rest = (player_x_over % grid_x_over) - (dir2 * grid_x_over);
			if (rest < 0) rest = -rest;
			rest = rest % grid_x_over;
			if ((rest == 0) && (dir2 > 0)) rest = grid_x_over;	// Prevent flicker at player x=128, y=192
			
			shorty = dir2 * rest * ans / anc;
			actdef2 = shorty;
			nblocky2 = (player_y_over + shorty) / grid_y_over;
			nblockx2 = (player_x_over / grid_x_over) + dir2;
			if ((nblocky2 >= 0) && (nblocky2 < levelmap_h)) {
				dis2 = rest;
				fblock2 = levelmap[nblocky2][nblockx2];
				if (fblock2 == LEVEL_BLOCK_FREE) {
					dis2 = -1;
					normaly = dir2 * grid_x_over * ans / anc;
					for(count = 1; count < DRAW_DIST; count++) {
						actdef2 += normaly;
						nblocky2 = (player_y_over + actdef2) / grid_y_over;	// trunc!
						nblockx2 += dir2;
						//if (nblockx2 >= 0) and (nblockx2 < levelmap_w) and (nblocky2 >= 0) and (nblocky2 < levelmap_h):
						if ((nblocky2 >= 0) && (nblocky2 < levelmap_h)) {
							fblock2 = levelmap[nblocky2][nblockx2];
							if (fblock2 != LEVEL_BLOCK_FREE) {
								dis2 = rest + (count * grid_x_over);
								break;
							}
						}
					}
				}
			}
			
			
			if (dis2 != -1) {
				#ifdef FISHEYE_CORRECTION
					// Fisheye correction
					//@TODO: Numerical unstable
					//dis2 = abs(int(dis2 * costable[abs((x-colsh)*fovangf//cols)]/anc))	# // OVER
					ancheck = (x-colsh)*fovangf / cols;
					if (ancheck < 0) ancheck = -ancheck;
					dis2 = (dis2 * _cos(ancheck)) / anc;
				#else
					dis2 = (SINTABLE_OVER * dis2) / anc;
				#endif
				if (dis2 < 0) dis2 = -dis2;
				
				//texoff2 = (abs(player.x_over + actdef2) / OVER) % tex_w;
				texoff2 = (player_y_over + actdef2) / OVER;
				if (texoff2 < 0) texoff2 = -texoff2;
				texoff2 = texoff2 % tex_w;
			}
			
		}
		
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
		if (dis1 == -1)
			dis1 = 0;
		
		if (vertical == 1) {
			// Use values of vertical intersection
			texoff1 = texoff2;
			fblock1 = fblock2;
			
			nblockx1 = nblockx2;
			nblocky1 = nblocky2;
			actdef1 = actdef2;
			dir1 = dir2;
		}
		
		/*
		//@FIXME: Debugging distance
		if (x % 4 == 0) {
			lcd_x = x;
			lcd_y = 2;
			printf_d(an);
			
			lcd_y = 4;
			printf_d(dis1 >> 8);
			lcd_y = 5;
			printf_d(dis1 & 0xff);
			
		}
		*/
		
		if (fblock1 != LEVEL_BLOCK_FREE) {
			drawCol(x, dis1, fblock1);
		}
		
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
	player_y_over = levelmap_h/2 * grid_y_over - 2;
	player_a = SINTABLE_SIZE / 2;
	
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
