/*
	A hardware monitor for the VGLDK
	
	2020-05-19 Bernhard "HotKey" Slawik
*/

#define VGLDK_VARIABLE_STDIO	// Allow changing putchar/getchar at runtime. Needed for serial I/O function
#include <vgldk.h>


const char VERSION[] = "Monitor 1.2";
const word tempAddr = 0xC400;

// Setup
#define MAX_ARGS 8
#define MAX_INPUT 255	//128	//64
char cmd_arg[MAX_INPUT];


// Features to include (affects how big the binary gets)
//#define MONITOR_HELP	// Include "long" help functionality (needs quite some space for the strings...)

#define MONITOR_SERIAL	// Include serial functions
#define MONITOR_SERIAL_USE_SOFTSERIAL	// Use old ASM-based softserial
//#define MONITOR_SERIAL_USE_SOFTUART	// Use new C-based softuart
//#define MONITOR_SERIAL_AUTOSTART	// Make softserial take over STDIO at startup

#define MONITOR_FILES	// Include file system stuff


// Commands to include (affects how big the binary gets)
//#define MONITOR_CMD_BEEP
//#define MONITOR_CMD_CLS
//#define MONITOR_CMD_DUMP
//#define MONITOR_CMD_ECHO
//#define MONITOR_CMD_EXIT
#define MONITOR_CMD_HELP	// Even without MONITOR_HELP, the HELP command can list all commands
//#define MONITOR_CMD_INTERRUPTS
//#define MONITOR_CMD_LOOP
#define MONITOR_CMD_PEEKPOKE
//#define MONITOR_CMD_PAUSE
//#define MONITOR_CMD_PORT
//#define MONITOR_CMD_PAUSE
#define MONITOR_CMD_VER


// Definitions
#define ERR_OK 0
#define ERR_COMMAND_UNKNOWN -4
#define ERR_MISSING_ARGUMENT -5
#define ERR_NOT_IMPLEMENTED -6

#ifdef MONITOR_CMD_PORT
	#include <ports.h>	// For accessing ports dynamically
#endif

#include <stdiomin.h>	// for printf() putchar() gets() getchar()
//#include <stdlib.h>	// for atoi()
int atoi(const char *a) {
	int i;
	i = 0;
	while (*a >= '0') {
		i = i*10 + (word)((*a++) - '0');
	}
	return i;
}

//#include <string.h>	// For strcmp
byte strcmp(const char *cs, const char *ct) {
	while ((*cs != 0) && (*ct != 0)) {
		if (*cs++ != *ct++) return 1;
	}
	if (*cs != *ct) return 1;
	return 0;
}

byte stricmp1(char a, char b) {
	if ((a >= 'a') && (a <= 'z')) a -= ('a' - 'A');
	if ((b >= 'a') && (b <= 'z')) b -= ('a' - 'A');
	if (a != b) return 1;
	return 0;
}
byte stricmp(const char *cs, const char *ct) {
	while ((*cs != 0) && (*ct != 0)) {
		if (stricmp1(*cs++, *ct++)) return 1;
	}
	if (stricmp1(*cs, *ct)) return 1;
	return 0;
}



#ifdef MONITOR_CMD_DUMP
  #define HEX_USE_DUMP
  #define HEX_DUMP_WIDTH 8 //((DISPLAY_COLS-6)/3)
#endif
#include <hex.h>	// For hex stuff

void printf_bin(byte v) {
	byte p;
	
	for(p = 0; p < 8; p++) {
		if ((v & (1 << (7-p))) > 0) putchar('1'); else putchar('0');
	}
}

void printf_byte_pretty(byte v) {
	
	// Hex
	printf("0x"); printf_x2(v);
	
	// Binary
	putchar(' '); printf_bin(v);
	
	/*
	// Decimal
	//printf(" %d\n", v);
	putchar(' '); printf_d(v);
	*/
}


// Internal command call definition
typedef int (*t_commandCall)(int argc, char *argv[]);


// Struct for the internal commands
typedef struct {
	const char *name;
	t_commandCall call;
	
	#ifdef MONITOR_HELP
	const char *help;
	#endif
	
} t_commandEntry;

#ifdef MONITOR_HELP
	// Include help string
	#define T_COMMAND_ENTRY(n, c, h) {n, c, h}
#else
	// Discard help string
	#define T_COMMAND_ENTRY(n, c, h) {n, c}
#endif

// Shell state
byte running;

// Internal command implementations
void parse(char *arg);	// Forward declaration to input parser, needed for cmd_call()
int eval(int argc, char *argv[]);	// Forward declaration to input parser, needed for cmd_call()


#ifdef MONITOR_CMD_BEEP
int cmd_beep(int argc, char *argv[]) {
	
	if (argc <= 1) {
		beep();
	} else {
		
		vgl_sound_note(atoi(argv[1]), (argc > 2) ? atoi(argv[2]) : 500);
		
	}
	return ERR_OK;
}
#endif


#ifdef MONITOR_CMD_CLS
int cmd_cls(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	clear();
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_DUMP

int cmd_dump(int argc, char *argv[]) {
	word a;
	char c;
	byte l;
	
	a = (argc < 2) ? tempAddr : hextow(argv[1]);
	
	l = (argc < 3) ? (4 * 1) : hextob(argv[2]);	// 4 * LCD_ROWS
	while(a < 0xffff) {
		dump(a, l);
		
		c = getchar();
		if ((c == 'f') || (c == 'q') || (c == 'F') || (c == 'Q')) break;
		else if ((c == 'r')) a = a;
		else if ((c == 'b') || (c == 'u') || (c == 'B') || (c == 'U')) a -= l;
		#ifdef KEY_REPEAT
		else if (c == KEY_REPEAT) a = a;
		#endif
		#ifdef KEY_ESCAPE
		else if (c == KEY_ESCAPE) break;
		#endif
		#ifdef KEY_UP
		else if (c == KEY_UP) a -= l;
		#endif
		else a += l;
	}
	
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_ECHO
int cmd_echo(int argc, char *argv[]) {
	int i;
	
	for(i = 1; i < argc; i++) {
		if (i > 1) printf(" ");
		printf(argv[i]);
	}
	printf("\n");
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_EXIT
int cmd_exit(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	running = false;
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_HELP
int cmd_help(int argc, char *argv[]);	// Forward declaration, since "help" needs to know all the commands and the commands need to know this function...
// implemented after declaration of COMMANDS[]
#endif

#ifdef MONITOR_CMD_INTERRUPTS
int cmd_di(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	__asm
		di
	__endasm;
	
	return ERR_OK;
}
int cmd_ei(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	__asm
		ei
	__endasm;
	
	return ERR_OK;
}

int cmd_ints(int argc, char *argv[]) {
	int i;
	(void)argc;
	(void)argv;
	
	cmd_ei(0, NULL);
	for(i = 0; i < 0x80; i++) {
		__asm
			nop
		__endasm;
	}
	cmd_di(0, NULL);
	
	return ERR_OK;
}

#endif

#ifdef MONITOR_CMD_LOOP
int cmd_loop(int argc, char *argv[]) {
	char c;
	
	while(1) {
		
		eval(argc-1, &argv[1]);
		
		
		//c = inkey();
		c = getchar();
		
		if (c == 'q') break;
		#ifdef KEY_ESCAPE
		if (c == KEY_ESCAPE) break;
		#endif
		
		
	}
	
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_PAUSE
int cmd_pause(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	//printf("Press any key");
	//beep();
	getchar();
	//clear();
	return ERR_OK;
}
#endif

#ifdef MONITOR_CMD_PEEKPOKE
int cmd_peek(int argc, char *argv[]) {
	
	word a;
	byte v;
	byte l;
	
	if (argc < 2) {
		return ERR_MISSING_ARGUMENT;
	}
	
	a = hextow(argv[1]);
	
	printf_x4(a);
	putchar(':');
	
	if (argc > 2) {
		// Peek multiple
		l = hextob(argv[2]);
		while (l > 0) {
			v = *((byte *)a);
			printf_x2(v);
			l--;
			a++;
		}
		
	} else {
		// Peek one (pretty)
		
		v = *((byte *)a);
		printf_byte_pretty(v);
	}
	
	putchar('\n');
	
	return ERR_OK;
}
int cmd_poke(int argc, char *argv[]) {
	word a;
	byte v;
	byte *b;
	
	if (argc < 3) {
		return ERR_MISSING_ARGUMENT;
	}
	
	a = hextow(argv[1]);
	//v = hextow(argv[2]);
	//*((byte *)a) = v;
	
	// Allow multiple
	b = argv[2];
	do {
		v = hextob(b);
		*((byte *)a) = v;
		b+=2;
		a++;
	} while (*b != 0);
	
	return ERR_OK;
}

word temp;
typedef int (t_plain_vgldkinit)(t_putchar *, t_getchar *);
int cmd_call(int argc, char *argv[]) {
	word a;
	
	if (argc < 2) {
		return ERR_MISSING_ARGUMENT;
	}
	
	a = hextow(argv[1]);
	temp = a;
	
	//@TODO: Push residual arguments on stack (argc-2, argv[2:])
	
	#ifdef VGLDK_VARIABLE_STDIO
		// Just call it and let the compiler/linker take care of the stack
		return (*(t_plain_vgldkinit *)temp)(p_stdout_putchar, p_stdin_getchar);
	
		/*
		// Push STDIO pointers to stack
		// Apps with plain architecture will get those as parameters.
		// Others can just ignore them (but will mess up the stack on return)
		__asm
			ld	hl, (_p_stdout_putchar)
			push	hl
			ld	hl, (_p_stdin_getchar)
			push	hl
		__endasm;
		*/
	#else
	__asm
		ld	hl, (_temp)
		
		; Trickery: Use "call" to call a label that does not return. That way the "jp" magically becomes a "call"!
		call	_call_encap	; Call but do not ret there, so PC+3 gets on stack!
		jp	_call_end		; The "jp" below should return to this instruction
		_call_encap:
			jp	(hl)	; This actually becomes a fake "call"
			; Do not ret! This is intentionally left blank
		_call_end:
	__endasm;
	return ERR_OK;
	#endif
	
}
#endif

#ifdef MONITOR_CMD_PORT
int cmd_in(int argc, char *argv[]) {
	byte p;
	byte v;
	
	if (argc < 2) return ERR_MISSING_ARGUMENT;
	
	p = hextob(argv[1]);
	v = port_in(p);
	//printf("0x%02X = 0x%02X / %d\n", p, v, v);
	
	//printf("%02X:0x%02X ", p, v);
	printf_x2(p);
	putchar(':');
	printf_byte_pretty(v);
	putchar('\n');
	
	return ERR_OK;
}

int cmd_out(int argc, char *argv[]) {
	byte p;
	byte v;
	
	if (argc < 3) return ERR_MISSING_ARGUMENT;
	
	p = hextob(argv[1]);
	v = hextob(argv[2]);
	
	port_out(p, v);
	
	return ERR_OK;
}
#endif


#ifdef MONITOR_CMD_VER
int cmd_ver(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	//printf("%s\n", VERSION);
	printf(VERSION);
	
	// Print the VGLDK_SERIES value
	#ifdef VGLDK_SERIES
		// Super-hack to get the value of a DEFINE as a string
		#define xstr(s) str(s)
		#define str(s) #s
		printf(" / " xstr(VGLDK_SERIES) );
	#endif
	putchar('\n');
	
	return ERR_OK;
}
#endif




// Commands for additional functions
#ifdef MONITOR_FILES

// File systems
//#include <fs_null.h>
//#include <fs_internal.h>

#define PB_USE_MAME	// For testing in MAME
//#define PB_USE_SOFTSERIAL	// For running on real hardware
//#define PB_DEBUG_FRAMES
//#define PB_DEBUG_PROTOCOL_ERRORS
#include <parabuddy.h>
#include <fs_parabuddy.h>

// Define roots for the root filesystem
#define FS_ROOT_MOUNTS {\
	/*{"nul",	&fs_null},*/		\
	/*{"int",	&fs_internal},*/	\
	{"pb",	&fs_parabuddy},	\
}

#include <fs_root.h>

//#define FILEIO_ROOT_FS fs_internal
#define FILEIO_ROOT_FS fs_root
#include <fileio.h>


int cmd_files_cd(int argc, char *argv[]) {
	
	if (argc < 2) {
		// Show CWD
		puts(cwd);
		putchar('\n');
		return ERR_OK;
	}
	
	// Change directory
	absPath(argv[1], cwd);	// Overwrite in-place (risky!)
	return ERR_OK;
}

int cmd_files_ls(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	file_DIR *dir;
	dirent *de;
	
	dir = opendir(cwd);
	if (dir == NULL) return errno;
	
	while (de = readdir(dir)) {
		printf((char *)de->name);
		putchar(' ');
		//printf_d(de->size);
		//putchar('\n');
	}
	putchar('\n');
	closedir(dir);
	
	return ERR_OK;
}

#define FILES_BUF_SIZE 32
int cmd_files_cat(int argc, char *argv[]) {
	file_FILE *f;
	size_t l;
	char buf[FILES_BUF_SIZE];
	
	if (argc < 2) return ERR_MISSING_ARGUMENT;
	
	// Open file
	f = fopen(argv[1], "r");
	if (f == NULL) return errno;
	
	// Read file buffer-by-buffer
	//while(feof(f) == 0) {
	do {
		l = fread(&buf[0], 1, FILES_BUF_SIZE-1, f);
		buf[l] = '\0';	// Terminate string
		if (l > 0)
			printf(buf);
	} while (l > 0);
	fclose(f);
	//putchar('\n');	// final LF?
	
	return ERR_OK;
}


//#include "driver/mame.h"
/*
int cmd_files_iotest(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	if (argc < 2) {
		#ifdef __MAME_H
		printf_x2(mame_getchar());
		#endif
	} else {
		#ifdef __MAME_H
		mame_put(argv[1], strlen(argv[1]));
		#endif
	}
	
	return ERR_OK;
}
*/
#endif


#ifdef MONITOR_SERIAL

#ifdef MONITOR_SERIAL_USE_SOFTSERIAL
	// Use old (but trusted) arch specific ASM based softserial
	#include <softserial.h>
#endif

#ifdef MONITOR_SERIAL_USE_SOFTUART
	// Use C softuart
	#include "driver/softuart.h"

	// Make softuart compatible with softserial
	#define serial_getchar_nonblocking softuart_receiveByte
	#define serial_putchar softuart_sendByte
	char serial_getchar() {
		int c;
		c = -1;
		while (c < 0) {
			c = serial_getchar_nonblocking();
		}
		return c;
	}
	word serial_gets(char *buffer) {
		char *pb;
		int c;
		
		pb = buffer;
		do {
			c = serial_getchar();
			if (c == 10) break;
			*pb++ = c;
		} while(c != 10);
		
		*pb++ = 0;	// Terminate
		return (word)pb - (word)buffer;	// Return L
	}
	void serial_puts(char *buffer) {
		char *pb;
		pb = buffer;
		while (*pb != 0) {
			serial_putchar(*pb++);
		}
	}
#endif

/*
int cmd_serial_test(int argc, char *argv[]) {
	int c = 0;
	
	(void)argc;
	(void)argv;
	
	c = serial_isReady();
	printf("isReady=");
	printf_x2(c);
	printf("\n");
	
	return ERR_OK;
}
*/

#ifdef VGLDK_VARIABLE_STDIO
// Allow switching STDIO to serial (onl available when built with VARIABLE_STDIO)
int cmd_serial_io(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	if (p_stdout_putchar != (t_putchar *)&serial_putchar) {
		// Use serial as stdio
		p_stdout_putchar = (t_putchar *)&serial_putchar;
		p_stdin_getchar = (t_getchar *)&serial_getchar;
		
		//p_stdin_gets = (t_gets *)&serial_gets;
		//p_stdin_inkey = (t_inkey *)&serial_inkey;
		
		stdio_echo = 0;	// Serial works better without gets echo
	} else {
		// Back to normal
		stdio_init();
	}
	
	/*
	while(1) {
		serial_gets(cmd_arg);
		parse(cmd_arg);
	}
	*/
	return ERR_OK;
}
#endif

int cmd_serial_get(int argc, char *argv[]) {
	int c;
	
	(void)argc;
	(void)argv;
	
	//c = serial_getchar();
	c = -1;
	while (c < 0) {
		c = serial_getchar_nonblocking();
		//printf_d(c);
	}
	
	printf_x2(c);
	
	return ERR_OK;
}
int cmd_serial_gets(int argc, char *argv[]) {
	char *buffer;
	
	(void)argc;
	(void)argv;
	
	buffer = &cmd_arg[0];
	serial_gets(buffer);
	printf(buffer);
	
	return ERR_OK;
}
int cmd_serial_put(int argc, char *argv[]) {
	int i;
	
	for(i = 1; i < argc; i++) {
		if (i > 1) serial_putchar(' ');
		serial_puts(argv[i]);
	}
	//printf("\n");
	return ERR_OK;
}
#endif


// List of internal calls
const t_commandEntry COMMANDS[] = {
	#ifdef MONITOR_CMD_BEEP
	T_COMMAND_ENTRY("beep", cmd_beep, "Make sound"),
	#endif
	#ifdef MONITOR_CMD_CLS
	T_COMMAND_ENTRY("cls"  , cmd_cls, "Clear screen"),
	#endif
	#ifdef MONITOR_CMD_DUMP
	T_COMMAND_ENTRY("dump" , cmd_dump, "Hex dump"),
	#endif
	#ifdef MONITOR_CMD_ECHO
	T_COMMAND_ENTRY("echo" , cmd_echo, "Display text"),
	#endif
	#ifdef MONITOR_CMD_EXIT
	T_COMMAND_ENTRY("exit" , cmd_exit, "End session"),
	#endif
	#ifdef MONITOR_CMD_HELP
	T_COMMAND_ENTRY("help" , cmd_help, "List cmds, explain"),
	#endif
	#ifdef MONITOR_CMD_INTERRUPTS
	T_COMMAND_ENTRY("di" , cmd_di, "Disable interrupts"),
	T_COMMAND_ENTRY("ei" , cmd_ei, "Enable interrupts"),
	T_COMMAND_ENTRY("ints" , cmd_ints, "Let ints happen"),
	#endif
	#ifdef MONITOR_CMD_LOOP
	T_COMMAND_ENTRY("loop" , cmd_loop, "Run command in loop"),
	#endif
	#ifdef MONITOR_CMD_PORT
	T_COMMAND_ENTRY("in" , cmd_in, "Read port"),
	T_COMMAND_ENTRY("out" , cmd_out, "Output to port"),
	#endif
	
	#ifdef MONITOR_CMD_PAUSE
	T_COMMAND_ENTRY("pause", cmd_pause, "Wait for key"),
	#endif
	#ifdef MONITOR_CMD_PEEKPOKE
	T_COMMAND_ENTRY("peek", cmd_peek, "View mem"),
	T_COMMAND_ENTRY("poke", cmd_poke, "Modfiy mem"),
	T_COMMAND_ENTRY("call", cmd_call, "Call mem"),
	#endif
	#ifdef MONITOR_CMD_VER
	T_COMMAND_ENTRY("ver"  , cmd_ver, "Version"),
	#endif
	
	
	// Additional features
	#ifdef MONITOR_FILES
	T_COMMAND_ENTRY("cd", cmd_files_cd, "Change dir"),
	T_COMMAND_ENTRY("ls", cmd_files_ls, "Show entries"),
	T_COMMAND_ENTRY("cat", cmd_files_cat, "List file"),
	//T_COMMAND_ENTRY("iotest", cmd_files_iotest, "IO test"),
	#endif
	
	
	#ifdef MONITOR_SERIAL
	//T_COMMAND_ENTRY("stest", cmd_serial_test, "Serial test"),
	#ifdef VGLDK_VARIABLE_STDIO
	T_COMMAND_ENTRY("sio", cmd_serial_io, "Switch STDIO to serial"),
	#endif
	T_COMMAND_ENTRY("sget", cmd_serial_get, "Serial get"),
	T_COMMAND_ENTRY("sgets", cmd_serial_gets, "Serial gets"),
	T_COMMAND_ENTRY("sput", cmd_serial_put, "Serial put"),
	#endif

};

#ifdef MONITOR_CMD_HELP
// Actual implementation of "help", since it needs to know the COMMANDS variable
int cmd_help(int argc, char *argv[]) {
	
	word i;
	
	if (argc < 2) {
		// List all commands
		for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
			if (i > 0) putchar(' ');	//printf(" ");
			//printf("%s", COMMANDS[i].name);
			puts(COMMANDS[i].name);
			
			#ifdef MONITOR_HELP
			//printf(": ");
			//printf(COMMANDS[i].help);
			//putchar('\n');
			#endif
		}
		printf("\n");
		return ERR_OK;
		
	}
	
	#ifdef MONITOR_HELP
	// Specific help
	for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
		if (stricmp(argv[1], COMMANDS[i].name) == 0) {
			//printf("%s: %s\n", COMMANDS[i].name, COMMANDS[i].help);
			printf(COMMANDS[i].name);
			printf(": ");
			printf(COMMANDS[i].help);
			putchar('\n');
			return ERR_OK;
		}
	}
	//printf("%s?\n", argv[1]);
	//printf(argv[1]); printf("?\n");
	return ERR_COMMAND_UNKNOWN;
	#else
	(void)argv;
	return ERR_NOT_IMPLEMENTED;
	#endif
}
#endif

void prompt() {
	#ifdef MONITOR_FILES
	// Show current directory, like in DOS
	printf(cwd);
	#endif
	putchar('>');
}

// Command handler
int eval(int argc, char *argv[]) {
	int i;
	
	// No input? Continue.
	if (argc == 0) return ERR_OK;
	
	// Parse/run
	//printf("argc=%d\n", argc);
	
	/*
	printf("argc="); printf_d(argc); printf("\n");
	for(i = 0; i < argc; i++) {
		//printf("args[%d]=\"%s\"\n", i, argv[i]);
		printf("args["); printf_d(i); printf("]=\""); printf(argv[i]); printf("\"\n");
	}
	*/
	
	/*
	// Hard-coded commands
	if (strcmp(argv[0], "list") == 0) {
		// List all commands
		for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
			if (i > 0) printf(", ");
			printf("%s", i);
		}
		printf("\n");
		return ERR_OK;
	}
	if (strcmp(argv[0], "exit") == 0) {
		running = false;
		return ERR_OK;
	}
	*/
	
	
	// Internal commands
	for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
		if (stricmp(argv[0], COMMANDS[i].name) == 0) {
			return COMMANDS[i].call(argc, argv);
		}
	}
	
	//printf("\"%s\"?\n", argv[0]);
	putchar('"'); printf(argv[0]); printf("\"?\n");
	return ERR_COMMAND_UNKNOWN;
}


void parse(char *s) {
	// Parse a string into argv[], including quotes, escape sequences and variables
	int r;
	char arg[MAX_INPUT];	// Destination string (slightly modified from original string)
	char *argv[MAX_ARGS];	// Pointers withing arg
	int argc;
	char *sc;	// Source pointer
	char *ac;	// arg pointer
	char *varNameP;	// For parsing variable names
	char *varValP;	// For parsing variable names
	char c;
	byte isEscape;
	byte isQuote;
	byte isVar;
	
	// Scan args
	//@TODO: Parse variables using strtok(): https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
	
	argc = 0;
	sc = &s[0];
	
	// Skip empty space lead-in
	while(*sc == ' ') {
		sc++;
	}
	
	// Ignore comments - ignore as a whole
	if (*sc == '#') return;
	
	ac = &arg[0];
	
	argv[0] = ac;
	c = *sc;
	if (c != 0x00) argc++;	// There seems to be at least ONE arg
	
	isEscape = 0;
	isQuote = 0;
	isVar = 0;
	varNameP = sc;
	varValP = ac;
	
	while (c != 0x00) {
		c = *sc;
		
		if (isEscape) {
			switch(c) {
				case 'r': c = '\r'; break;
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case '0': c = 0; break;
			}
			*ac++ = c;
			isEscape = 0;
		} else
		if (c == '\\') {
			isEscape = 1;
		} else
		if (c == '"') {
			if (isQuote == 1) isQuote = 0;
			else isQuote = 1;
		} else
		if (c == ';') {
			// Execute and start over
			*ac++ = 0x00;	// Terminate string
			
			// Execute
			r = eval(argc, &argv[0]);
			
			// Recurse
			sc++;
			parse(sc);
			
			// And stop
			return;
		}
		if (c == '%') {
			if (isVar == 0) {
				isVar = 1;
				varNameP = ac;
				varValP = ac;
			} else {
				// Var name ended!
				
				//@FIXME: Implement looking up vars
				// Temporarily terminate arg/varNameP (to be used as varName)
				*ac = 0;
				//ac = varValP + sprintf(varValP, "%s=%d", varNameP, 1234);	// Just output its name
				
				isVar = 0;
			}
		} else
		if (c == 0x0a) {
			*ac++ = 0x00;	// Terminate string at first new line
			break;
		} else
		if ((c == 0x20) && (isQuote == 0)) {
			*ac++ = 0x00;	// Terminate arg at space and continue with next argv
			argv[argc] = ac;
			if (argc >= MAX_ARGS) break;
			argc++;
		} else {
			// Add character to arg
			*ac++ = c;
		}
		
		sc++;
	}
	
	// Handle input
	r = eval(argc, &argv[0]);
	
	// Handle return value
	if (r != 0) {
		//printf("Exit code %d\n", r);
		//printf("Exit code "); printf_d(r); printf("\n");
		printf("Exit code ");
		printf("0x"); printf_x2(r);
		printf("\n");
	}
}



void main() __naked {
	#ifdef MONITOR_SERIAL_AUTOSTART
	int c;
	#endif
	
	#ifdef VGLDK_VARIABLE_STDIO
		// Variable STDIO must be initialized
		stdio_init();
	#endif
	
	clear();
	
	#ifdef MONITOR_FILES
	// Mount file systems
	//drives[0] = &fs_internal;
	//drives[1] = &fs_parabuddy;
	
	// CWD
	//cwd[0] = 'A';
	//cwd[1] = FILEIO_DRIVE_DELIMITER;
	//cwd[2] = 0;
	cwd[0] = FILE_PATH_DELIMITER;
	cwd[1] = 0;
	#endif
	
	#ifdef MONITOR_SERIAL
	#ifdef MONITOR_SERIAL_AUTOSTART
	if (serial_isReady()) {
		// If serial cable is connected: Ask for which I/O to use
		
		#ifdef VGLDK_VARIABLE_STDIO
		printf("CR to activate\n");
		serial_puts("CR to activate\n");
		
		while(1) {
			c = serial_getchar_nonblocking();
			if ((c == 10) || (c == 13)) {
				printf("Serial mode\n");
				cmd_serial_io(0, NULL);
				break;
			}
			c = keyboard_inkey();
			if ((c == 10) || (c == 13)) {
				printf("OK\n");
				break;
			}
		}
		
		/*
		printf("Switch to serial I/O?");
		if (getchar() == 'y') {
			printf("Y\n");
			cmd_serial_io(0, NULL);
		} else {
			printf("N\n");
		}
		*/
		#endif
		
	}
	#endif
	#endif
	
	#ifdef MONITOR_CMD_VER
	// Banner
	cmd_ver(0, NULL);
	#endif
	
	// Command line loop
	running = true;
	while(running) {
		
		// Prompt for input
		prompt();
		gets(cmd_arg);
		putchar('\n');
		
		//putchar('"'); printf(cmd_arg); putchar('"'); printf("\n");
		parse(cmd_arg);
		
	}
	// Exited...
	
	printf("Bye!");
	
	// Off into the abyss...
	
	
}
