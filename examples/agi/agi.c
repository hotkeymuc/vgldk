
#include "agi.h"

#include "agi_vars.h"	// To show status of variables

VERTYPE AGIVER;

// from agimain.c:
BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED, QUIT_FLAG;
BOOL SOUND_ON, PIC_VISIBLE, PRI_VISIBLE, STATUS_VISIBLE, VOBJ_BLOCKING, WALK_HOLD;
U8 oldScore;
U8 horizon; 
U8 picNum;
U8 minRow,inputPos,statusRow;
U8 textColour,textAttr,textRow,textCol;
int minRowY,ticks;
RECT8 objBlock;
char cursorChar;
char szGameID[MAX_ID_LEN+1];
int pushedScriptCount, scriptCount;
//...


// invobj.c:
U8 invObjRooms[MAX_IOBJ];


// parse.c:
U16 input[MAX_INPUT],inpos;
int wordCount;
BOOL MORE_MODE;
char *wordStrings[MAX_INPUT];
char szInput[64];


// more:

void dump_vars() {
	//for(int i = 0; i < MAX_VARS; i++) {
	printf("v:");
	for(int i = 0; i < 20; i++) {
		printf_x2(vars[i]);
	}
	putchar('\n');
}

void WriteStatusLine() {
	if (!STATUS_VISIBLE) return;
	lcd_text_col = 0;
	lcd_text_row = statusRow;	//0;
	//printf("STATS:");
	printf(&szGameID[0]);
	dump_vars();
}



char *parse_int(char *p, int *r) {
	char c;
	int i;
	
	i = 0;
	while(c = *p) {
		if ((c < '0') || (c > '9')) break;
		i = (i*10) + (c - '0');
		p++;
	}
	
	*r = i;
	return p;
}

void vagi_printf(char *p) {
	// Print while handling AGI special constructs, like "%s4" to print string 4
	char c;
	int i;
	
	//printf(s);
	while(c = *p++) {
		if (c != '%') {
			putchar(c);
			continue;
		}
		c = *p++;
		p = parse_int(p, &i);
		if      (c == 's') vagi_printf(strings[i]);	//printf(strings[i]);
		else if (c == 'v') printf_d(vars[i]);
		else if (c == 'w') printf(wordStrings[i-1]);	// nth typed in word (1-based!)
		else if (c == 'm') vagi_printf(GetMessage(curLog, i));
		else if (c == 'g') vagi_printf(GetMessage(log0, i));
		//else if (c == 'o') objNames[i];
		else {
			putchar('<'); putchar(c); printf_d(i); putchar('>');
		}
	}
	
}



bool MessageBox(char *t) {
	bool r;
	
	//printf("MessageBox: [");
	putchar('[');
	//printf(t);
	vagi_printf(t);	// Takes care of "%s4" etc.
	putchar(']');
	//putchar('\n');
	
	r = true;
	if(TestFlag(fPRINTMODE)) {
		ResetFlag(fPRINTMODE);
	} else {
		
		if (vars[vPRINTDURATION]) {
			byte delay = vars[vPRINTDURATION];
			
			//@TODO: Delay (while checking for user reply)...
		//	r = (getchar() == 10);	//@FIXME: Must be non-blocking with TIMEOUT!
			
			vars[vPRINTDURATION] = 0;
			
		} else {
			r = (getchar() == 10);
		}
	}
	return r;
}
bool MessageBoxXY(char *t, byte x, byte y, byte w) {
	lcd_text_col = x;
	lcd_text_row = y;
	(void)w;
	return MessageBox(t);
}
/*
see agi.h:
	ERR_BAD_CMD,
	ERR_TEST_CMD,
	ERR_PIC_CODE,
	ERR_VOBJ_NUM,
	ERR_VOBJ_VIEW,
	ERR_VOBJ_LOOP,
	ERR_VOBJ_CEL,
	ERR_NO_VIEW,
	ERR_VOBJ_NO_CEL,
	ERR_PRINT,
	ERR_NOMENUSET,
	ERR_ADDMENUITEM,
	ERR_FINDMENUITEM,
*/
#define ErrorMessage(msg, param) ErrorMessage3(msg, param, 0,0)
#define ErrorMessage2(msg, param1, param2) ErrorMessage3(msg, param1, param2, 0)
void ErrorMessage3(int msg, int param1, int param2, int param3) {
	printf("Error #");
	printf_d(msg); printf(": "); printf_d(param1); putchar('|'); printf_d(param2); putchar('|'); printf_d(param3);
	getchar();
}

#define STATE_BYTES 7
#define MULT 0x13B /* for STATE_BYTES==6 only */
#define MULT_LO (MULT & 255)
#define MULT_HI (MULT & 256)

U8 rand() {
	static U8 state[STATE_BYTES] = { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
	static U16 c = 0x42;
	static int i = 0;
	U16 t;
	U8 x;
	
	x = state[i];
	t = (U16)x * MULT_LO + c;
	c = t >> 8;
#if MULT_HI
	c += x;
#endif
	x = t & 255;
	state[i] = x;
	if (++i >= sizeof(state))
		i = 0;
	return x;
}


// from GBAGI:parse.c:
word get_word_id(char *szWord, byte *word_len) {
	romfs_handle_t tok_h;
	byte b;
	byte c;
	word word_id;
	
	char word[MAX_STRINGS_LEN];	// Holding currently generated word (SQ2: Longest word = 24)
	char *w;	// Current pointer in generated word
	char *p;	// Current pointer in szWord
	
	//byte word_len;
	byte matches;
	bool matching;
	
	byte full_matches;	// to keeep on searching for longer matches
	U16 full_word_id;
	
	tok_h = romfs_fopen(r_words_tok);	// Open WORDS.TOK
	romfs_fskip(tok_h, 52);	// Skip header
	
	p = szWord;
	matching = true;
	matches = 0;
	full_matches = 0;
	full_word_id = -1;
	
	// Stream in all words
	word[0] = 0;
	//word_len = 0;
	w = &word[0];
	while(!romfs_feof(tok_h)) {
		
		// Read prefix length
		b = romfs_fread(tok_h);
		
		// Take prefix bytes from previous word
		w = &word[b];
		//word_len = b;
		
		if (b < full_matches) break;	// Continue to find a longer match until the prefix is smaller than the best match
		
		if (b <= matches) {
			matches = b;	// Reduce number of matching chars if prefix is smaller
			matching = true;
		}
		p = &szWord[b];
		
		// Get rest of characters
		b = 0;
		while (b <= 127) {
			b = romfs_fread(tok_h);
			
			if (b < 32) {
				c = (b ^ 127);
			} else if (b > 127) {
				c = ((b - 128) ^ 127);
				//break!
			} else if (b == 95) {
				c = ' ';
			} else continue;	//?
			
			*w++ = c;
			//word_len ++;
			
			if (matching) {
				//if (*p == c) matches++;
				if ((*p | 0x20) == c) matches++;	// Convert input to lower case
				else matching = false;
				p++;
			}
		}
		*w = 0;	// Terminate
		
		// Get word number (This time, it is HI, LO for a change!)
		word_id = (U16)romfs_fread(tok_h) << 8;	// HI
		word_id |= romfs_fread(tok_h);	// LO
		
		if (matching) {
			
			//put(f'#{word_id}: {str(word)}')
			//printf("word #"); printf_x2(num_words >> 8); printf_x2(num_words & 0xff);
			//printf("id="); printf_x2(word_id >> 8); printf_x2(word_id & 0xff);
			//printf(" \""); printf(&word[0]); printf("\"\n");
			
			*word_len = w - &word[0];
			
			// Check if whole input word was covered
			//if (*p == 0) {
			if ((*p & ~0x20) == 0) {	// handles 0x00 (end-of-word) and 0x20 (space)
				// Full match!
				
				//printf("Found id="); printf_x2(word_id >> 8); printf_x2(word_id & 0xff);
				//printf(" \""); printf(&word[0]); printf("\"\n");
				
				//return word_id;
				//break;
				// Remember that and keep searching for longer match
				if (matches > full_matches) {
					full_matches = matches;
					full_word_id = word_id;
				}
			}
			
		}
		
	}
	
	romfs_fclose(tok_h);
	
	return full_word_id;
}


/*
// Test: Show all words in WORDS.TOK
void show_all_words() {
	romfs_handle_t tok_h;
	byte b;
	word num_words;
	word word_id;
	
	char word[MAX_STRINGS_LEN];	// Holding currently generated word (SQ2: Longest word = 24)
	char *w;	// Current pointer in generated word
	
	num_words = 0;
	
	tok_h = romfs_fopen(r_words_tok);	// Open WORDS.TOK
	romfs_fskip(tok_h, 52);	// Skip header
	
	word[0] = 0;
	w = &word[0];
	while(!romfs_feof(tok_h)) {
		
		// Read prefix length
		b = romfs_fread(tok_h);
		//if not b: break
		
		// Take prefix bytes from previous word
		w = &word[b];
		
		// Get rest of characters
		while(1) {
			b = romfs_fread(tok_h);
			
			if (b < 32) {
				*w++ = (b ^ 127);
			} else if (b > 127) {
				*w++ = ((b - 128) ^ 127);
				break;
			} else if (b == 95) {
				*w++ = ' ';
			}
		}
		*w = 0;	// Terminate
		
		// Get word number (This time, it is HI, LO for a change!)
		word_id = (U16)romfs_fread(tok_h) << 8;	// HI
		word_id |= romfs_fread(tok_h);	// LO
		
		//put(f'#{word_id}: {str(word)}')
		printf("word #"); printf_x2(num_words >> 8); printf_x2(num_words & 0xff);
		printf(": id="); printf_x2(word_id >> 8); printf_x2(word_id & 0xff);
		printf(" \""); printf(&word[0]); printf("\"\n");
		
		num_words ++;
	}
	romfs_fclose(tok_h);
}
*/

void InitParseSystem() {
	//memset(input,0,sizeof(input));
	//inpos = wordCount = 0;
	//memset(wordStrings,0,sizeof(wordStrings));
	
	wordCount = 0;
	inpos = 0;
	szInput[0]='\0';
	
	//show_all_words();
	
	// Test word look-up:
	//printf("Enter word:"); gets(&szInput[0]);
	//word word_id = get_word_id(&szInput[0]);
	//getchar();
	
	// Test sentence parser:
	//printf("Enter input:"); gets(&szInput[0]);
	//ParseInput(&szInput[0]);
	//getchar();
	
}

char *ParseInput(char *sStart) {
	char *s;
	word word_id;
	byte word_len;
	
	//@TODO: ignore case
	
	//printf("ParseInput: \""); printf(sStart); printf("\"...\n");
	
	s = sStart;
	inpos = 0;
	wordCount = 0;
	
	while(*s) {
		// Skip white space
		if (*s == ' ') {
			s++;
			continue;
		}
		
		// Check word(s)
		//printf("Parsing \""); printf(s); printf("\"...\n");
		
		word_id = get_word_id(s, &word_len);
		if (word_id == -1) {
			printf("word: \""); printf(s); printf("\"?");
			vars[vUNKWORD] = ++wordCount;
			break;
		}
		
		// Add word_id, but skip group 0 (which contains common "filler" words, like "a")
		// Group 0 = filler
		// Group 1 = "any word"
		// Group 9999 = "rol" (Rest of Line)
		if (word_id > 0) {
			wordStrings[wordCount] = s;
			input[wordCount++] = word_id;
		}
		
		// Skip over found word
		s += word_len;
		if (*s == 0) break;
		
		// Terminate the found word in input string...
		*s++ = '\0';
		// ...and continue at next word (skip the space/zero)
	}
	
	// Set flag for AGI!
	if (wordCount) SetFlag(fPLAYERCOMMAND);
	
	inpos = wordCount;
	return wordStrings[0];
}