/*
	A cheap ass raytracer
	- no rotations
	- no perspective
	- no trigonometry
	- no perspective correction
	
	2020-01-22 Bernhard "HotKey" Slawik
*/

#define TEXT_MODE
//#define GFX_MODE


#define lcd_MINIMAL	// Use minimal text mode (no scrolling)
#include <vgldk.h>
#include <stdiomin.h>


#define LEVEL_W 16
#define LEVEL_H 16
const char LEVEL[LEVEL_W][LEVEL_H] = {
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','x'},
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
};


#ifdef TEXT_MODE
	#define SCREEN_W 20
	#define SCREEN_H 4
	#define MAX_DEPTH 8
	const char COL_PATTERNS[MAX_DEPTH][SCREEN_H] = {
		{
			'X',
			'X',
			'X',
			'X',
		},
		{
			'-',
			' ',
			' ',
			' ',
		},
		{
			'_',
			' ',
			' ',
			'_',
		},
		{
			' ',
			'-',
			' ',
			'-',
		},
		{
			' ',
			'_',
			'_',
			' ',
		},
		{
			' ',
			' ',
			'=',
			' ',
		},
		{
			' ',
			' ',
			'-',
			' ',
		},
		{
			' ',
			' ',
			' ',
			' ',
		},
	};


void drawCol(int x, int z) {
	int y;
	
	if (z > MAX_DEPTH-1) z = MAX_DEPTH-1;	// z clip
	
	for(y = 0; y < SCREEN_H; y++) {
		lcd_x = x;
		lcd_y = y;
		lcd_putchar(COL_PATTERNS[z][y]);
	}
	
}
#endif

#ifdef GFX_MODE
	//#define SCREEN_W 240
	#define SCREEN_W 30
	#define SCREEN_H 100
	#define MAX_DEPTH 8
void drawVLine(int x, int y1, int h) {
	byte *p;
	int y;
	
	for(y = y1; y < y1+h; y++) {
		//p = (byte *)lcd_addr + (y * lcd_w + x) / 8;
		p = (byte *)lcd_addr + (y * lcd_w/8 + x);
		*p = 0xff;
	}
}

void drawCol(int x, int z) {
	
	if (z > MAX_DEPTH-1) z = MAX_DEPTH-1;	// z clip
	
	//drawVLine(x, SCREEN_H/2 - z*2, z*4);
	drawVLine(x, z * 10, SCREEN_H - z*20);
	
}
#endif

int player_x = LEVEL_W/2;
int player_z = LEVEL_H/2;

const int fov = MAX_DEPTH;

void drawScreen() {
	int ix;
	int iz;
	int x;
	int z;
	int dx;
	int dz;
	
	lcd_clear();
	
	for(ix = 0; ix < SCREEN_W; ix++) {
		dx = -(fov / 2) + (fov * ix) / (SCREEN_W-1);
		dz = -MAX_DEPTH;
		for(iz = 0; iz < MAX_DEPTH; iz++) {
			x = player_x + (dx * iz) / (MAX_DEPTH-1);
			z = player_z + (dz * iz) / (MAX_DEPTH-1);
			if (x < 0) break;
			if (x >= LEVEL_W) break;
			if (z < 0) break;
			if (z >= LEVEL_H) break;
			
			if (LEVEL[z][x] != ' ') {
				drawCol(ix, iz);
				break;
			}
		}
	}
}

void main() __naked {
	char c;
	
	// Setup text mode
	//lcd_scroll_cb = NULL;	// Disable auto-scroll
	lcd_x = 0;
	lcd_y = 0;
	
	
	//printf("Hello World!");
	//printf("----____++++||||////");
	/*
	for(i = 0; i < 255; i++) {
		//printf((char)i);
		putchar(i);
		if (i % 40 == 0) getchar();
	}
	*/
	/*
	for(i = 0; i < SCREEN_W; i++) {
		drawCol(i, i);
	}
	*/
	
	player_x = LEVEL_W / 2;
	player_z = LEVEL_H / 2;
	//for(player_z = 1; player_z < LEVEL_H; player_z++) {
	while(1) {
		
		drawScreen();
		
		c = getchar();
		switch(c) {
			case 'w':
			case 'W':
			#ifdef KEY_UP
			case KEY_UP:
			#endif
				player_z --;
				break;
			case 's':
			case 'S':
			#ifdef KEY_DOWN
			case KEY_DOWN:
			#endif
				player_z ++;
				break;
			case 'a':
			case 'A':
			#ifdef KEY_CURSOR_LEFT
			case KEY_CURSOR_LEFT:
			#endif
			#ifdef KEY_LEFT
			case KEY_LEFT:
			#endif
				player_x --;
				break;
			case 'd':
			case 'D':
			#ifdef KEY_CURSOR_RIGHT
			case KEY_CURSOR_RIGHT:
			#endif
			#ifdef KEY_RIGHT
			case KEY_RIGHT:
			#endif
				player_x ++;
				break;
		}
	}
	
	
	//while(1) { }
}
