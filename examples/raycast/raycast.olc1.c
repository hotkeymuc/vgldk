/*
	A crappy ass raycaster
	- no rotations
	- no perspective
	- no trigonometry
	- no perspective correction
	
	2020-07-07 Bernhard "HotKey" Slawik
*/


#define lcd_MINIMAL	// Use minimal text mode (no scrolling)
#include <vgldk.h>
#include <stdiomin.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

/*
void lcd_putchar_at(int x, int y, char c) {
	lcd_x = x;
	lcd_y = y;
	lcd_putchar(c);
}
*/

#define MAP_W 16
#define MAP_H 16
const char map[MAP_W*MAP_H] = {
	'#','#','#','#','#','#','#','#','#','.','.','.','.','.','.','.',
	'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'#','.','.','.','.','.','.','.','#','#','#','#','#','#','#','#',
	'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','#','#','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','#','#','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','#','#','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','#','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','#','#','#','#','.','.','#','#','#',
	'#','.','.','.','.','.','.','#','.','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','#','.','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','.','.','.','.','.','.','#','#','#','#','#','#','#','#','#',
	'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
	'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',
};



int nScreenWidth = 40;			// Console Screen Size X (columns)
int nScreenHeight = 10;			// Console Screen Size Y (rows)
int nMapWidth = 16;				// World Dimensions
int nMapHeight = 16;

float fPlayerX = 14.7f;			// Player Start Position
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;			// Player Start Rotation
float fFOV = 3.14159f / 4.0f;	// Field of View
float fDepth = 16.0f;			// Maximum rendering distance
float fSpeed = 5.0f;			// Walking Speed

float fElapsedTime;

void drawScreen() {
	int x;
	float fRayAngle;
	float fStepSize;	// Increment size for ray casting, decrease to increase resolution
	float fDistanceToWall;
	
	bool bHitWall;		// Set when ray hits wall block
	bool bBoundary;		// Set when ray hits boundary between two wall blocks
	
	float fEyeX;	// Unit vector for ray in player space
	float fEyeY;
	
	int nTestX;
	int nTestY;
	
	int nCeiling;
	int nFloor;
	char nShade;
	
	for (x = 0; x < nScreenWidth; x++) {
		// For each column, calculate the projected ray angle into world space
		fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
		
		// Find distance to wall
		fStepSize = 0.1f;		// Increment size for ray casting, decrease to increase
		fDistanceToWall = 0.0f;	// ...resolution
		
		bHitWall = false;		// Set when ray hits wall block
		bBoundary = false;		// Set when ray hits boundary between two wall blocks
		
		fEyeX = sinf(fRayAngle);
		fEyeY = cosf(fRayAngle);
		
		// Incrementally cast ray from player, along ray angle, testing for 
		// intersection with a block
		while (!bHitWall && fDistanceToWall < fDepth) {
			fDistanceToWall += fStepSize;
			nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
			nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
			
			// Test if ray is out of bounds
			if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
				bHitWall = true;			// Just set distance to maximum depth
				fDistanceToWall = fDepth;
			} else {
				// Ray is inbounds so test to see if the ray cell is a wall block
				if (map[nTestX * nMapWidth + nTestY] == '#') {
					// Ray has hit wall
					bHitWall = true;
					
					/*
					// To highlight tile boundaries, cast a ray from each corner
					// of the tile, to the player. The more coincident this ray
					// is to the rendering ray, the closer we are to a tile 
					// boundary, which we'll shade to add detail to the walls
					vector<pair<float, float>> p;
					
					// Test each corner of hit tile, storing the distance from
					// the player, and the calculated dot product of the two rays
					for (int tx = 0; tx < 2; tx++) {
						for (int ty = 0; ty < 2; ty++) {
							// Angle of corner to eye
							float vy = (float)nTestY + ty - fPlayerY;
							float vx = (float)nTestX + tx - fPlayerX;
							float d = sqrt(vx*vx + vy*vy); 
							float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
							p.push_back(make_pair(d, dot));
						}
					}
					// Sort Pairs from closest to farthest
					sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });
					
					// First two/three are closest (we will never see all four)
					float fBound = 0.01;
					if (acos(p.at(0).second) < fBound) bBoundary = true;
					if (acos(p.at(1).second) < fBound) bBoundary = true;
					if (acos(p.at(2).second) < fBound) bBoundary = true;
					*/
				}
			}
		}
		
		// Calculate distance to ceiling and floor
		nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
		nFloor = nScreenHeight - nCeiling;
		
		// Shader walls based on distance
		//nShade = ' ';
		
		//if (bBoundary)		nShade = ' '; else // Black it out
		if (fDistanceToWall <= fDepth / 4.0f)			nShade = 'O';	// Very close
		else if (fDistanceToWall < fDepth / 3.0f)		nShade = 'o';
		else if (fDistanceToWall < fDepth / 2.0f)		nShade = ',';
		else if (fDistanceToWall < fDepth)				nShade = '.';
		else											nShade = ' ';		// Too far away
		
		for (int y = 0; y < nScreenHeight; y++) {
			// Each Row
			if(y <= nCeiling)
				// Ceiling
				//screen[y*nScreenWidth + x] = ' ';
				nShade = ' ';
			else if(y > nCeiling && y <= nFloor) {
				//screen[y*nScreenWidth + x] = nShade;
			} else {
				// Floor
				// Shade floor based on distance
				float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
				if (b < 0.25)		nShade = '#';
				else if (b < 0.5)	nShade = 'x';
				else if (b < 0.75)	nShade = '.';
				else if (b < 0.9)	nShade = '-';
				else				nShade = ' ';
				//screen[y*nScreenWidth + x] = nShade;
			}
			lcd_putchar_at(x, y, nShade);
		}
	}
}


void main() __naked {
	char c;
	
	// Setup text mode
	//lcd_scroll_cb = NULL;	// Disable auto-scroll
	lcd_x = 0;
	lcd_y = 0;
	
	lcd_putchar_at(0,0,'H');
	puts("Hello");
	
	while(1) {
		
		fElapsedTime = 0.1f;
		
		lcd_putchar_at(0,0,'k');
		c = getchar();
		lcd_putchar_at(0,0,'A');
		switch(c) {
			case 'a':
			case 'A':
			#ifdef KEY_CURSOR_LEFT
			case KEY_CURSOR_LEFT:
			#endif
			#ifdef KEY_LEFT
			case KEY_LEFT:
			#endif
				lcd_putchar_at(0,0,'1');
				fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;
				lcd_putchar_at(0,0,'2');
				break;
			case 'd':
			case 'D':
			#ifdef KEY_CURSOR_RIGHT
			case KEY_CURSOR_RIGHT:
			#endif
			#ifdef KEY_RIGHT
			case KEY_RIGHT:
			#endif
				fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
				break;
			
			case 'w':
			case 'W':
			#ifdef KEY_UP
			case KEY_UP:
			#endif
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
				if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
					fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
					fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
				}
				break;
			case 's':
			case 'S':
			#ifdef KEY_DOWN
			case KEY_DOWN:
			#endif
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
				if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
					fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
					fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
				}
				break;
		}
		
		lcd_putchar_at(0,0,'D');
		drawScreen();
		
	}
	
}
