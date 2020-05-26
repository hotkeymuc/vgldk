/*
	A hardware monitor for the VGLDK
	
	2020-05-19 Bernhard "HotKey" Slawik
*/

#define VGLDK_VARIABLE_STDIO	// Allow changing putchar/getchar at runtime. Needed for serial I/O function
#include <vgldk.h>


const char VERSION[] = "Monitor 1.0";
const word tempAddr = 0xC400;

// Setup
#define MAX_ARGS 8
#define MAX_INPUT 64
char cmd_arg[MAX_INPUT];


// Features to include
//#define MONITOR_HELP	// Include help functionality (needs quite some space for strings...)
#define MONITOR_SOFTSERIAL	// Include the softserial include (serial using bit-banged parallel port)

//#define MONITOR_CMD_BEEP
#define MONITOR_CMD_CLS
#define MONITOR_CMD_DUMP
#define MONITOR_CMD_ECHO
//#define MONITOR_CMD_HELP
#define MONITOR_CMD_INTERRUPTS
#define MONITOR_CMD_LOOP
#define MONITOR_CMD_PORT
#define MONITOR_CMD_PAUSE
#ifdef MONITOR_SOFTSERIAL
	#define MONITOR_CMD_SOFTSERIAL
	#define MONITOR_SOFTSERIAL_AUTOSTART
#endif
#define MONITOR_CMD_VER

// Definitions
#define ERR_COMMAND_UNKNOWN -4
#define ERR_MISSING_ARGUMENT -5
#define ERR_NOT_IMPLEMENTED -6



#include <ports.h>

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

#define HEX_USE_DUMP
#include <hex.h>



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
	return 0;
}
#endif


#ifdef MONITOR_CMD_CLS
int cmd_cls(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	clear();
	return 0;
}
#endif

#ifdef MONITOR_CMD_DUMP
int cmd_dump(int argc, char *argv[]) {
	word a;
	char c;
	byte l;
	
	a = (argc < 2) ? tempAddr : hextow(argv[1]);
	
	l = 8;
	while(a < 0xffff) {
		dump(a, l);
		
		c = getchar();
		if ((c == 'q') || (c == 'Q')) break;
		else if ((c == 'r')) a = a;
		else if ((c == 'u') || (c == 'u')) a -= l;
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
	
	return 0;
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
	return 0;
}
#endif

int cmd_exit(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	running = false;
	return 0;
}

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
	
	return 0;
}
int cmd_ei(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	__asm
		ei
	__endasm;
	
	return 0;
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
	
	return 0;
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
	
	return 0;
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
	printf_x2(p); putchar(':'); printf("0x"); printf_x2(v); putchar(' ');
	
	for(p = 0; p < 8; p++) {
		if ((v & (1 << (7-p))) > 0) putchar('1'); else putchar('0');
	}
	//printf(" %d\n", v);
	putchar(' '); printf_d(v); printf("\n");
	
	return 0;
}

int cmd_out(int argc, char *argv[]) {
	byte p;
	byte v;
	
	if (argc < 3) return ERR_MISSING_ARGUMENT;
	
	p = hextob(argv[1]);
	v = hextob(argv[2]);
	
	port_out(p, v);
	
	return 0;
}
#endif


#ifdef MONITOR_CMD_PAUSE
int cmd_pause(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	//printf("Press any key");
	//beep();
	getchar();
	//clear();
	return 0;
}
#endif


#ifdef MONITOR_CMD_SOFTSERIAL
#include <softserial.h>

int cmd_serial_test(int argc, char *argv[]) {
	int c = 0;
	
	(void)argc;
	(void)argv;
	
	c = serial_isReady();
	printf("isReady=");
	printf_x2(c);
	printf("\n");
	
	return 0;
}

#ifdef VGLDK_VARIABLE_STDIO
int cmd_serial_io(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	// Use serial as stdio
	p_stdout_putchar = (t_putchar *)&serial_putchar;
	p_stdin_getchar = (t_getchar *)&serial_getchar;
	
	//p_stdin_gets = (t_gets *)&serial_gets;
	//p_stdin_inkey = (t_inkey *)&serial_inkey;
	
	stdio_echo = 0;	// Serial works better without gets echo
	
	/*
	while(1) {
		serial_gets(cmd_arg);
		//putchar('"'); printf(cmd_arg); putchar('"'); printf("\n");
		parse(cmd_arg);
	}
	*/
	return 0;
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
	
	return 0;
}
int cmd_serial_gets(int argc, char *argv[]) {
	char *buffer;
	
	(void)argc;
	(void)argv;
	
	buffer = &cmd_arg[0];
	serial_gets(buffer);
	printf(buffer);
	
	return 0;
}
int cmd_serial_put(int argc, char *argv[]) {
	int i;
	
	for(i = 1; i < argc; i++) {
		if (i > 1) serial_putchar(' ');
		serial_puts(argv[i]);
	}
	//printf("\n");
	return 0;
}
#endif

#ifdef MONITOR_CMD_VER
int cmd_ver(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	//printf("%s\n", VERSION);
	printf(VERSION);
	#ifdef VGLDK_SERIES
		#define xstr(s) str(s)
		#define str(s) #s
		printf(" for SERIES " xstr(VGLDK_SERIES) );
	#endif
	printf("\n");
	
	return 0;
}
#endif



// List of internal calls
const t_commandEntry COMMANDS[] = {
	#ifdef MONITOR_CMD_BEEP
	{"beep" , cmd_beep
		#ifdef MONITOR_HELP
		, "Make sound"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_CLS
	{"cls"  , cmd_cls
		#ifdef MONITOR_HELP
		, "Clear screen"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_DUMP
	{"dump" , cmd_dump
		#ifdef MONITOR_HELP
		, "Hex dump"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_ECHO
	{"echo" , cmd_echo
		#ifdef MONITOR_HELP
		, "Display text"
		#endif
	},
	#endif
	{"exit" , cmd_exit
		#ifdef MONITOR_HELP
		, "End shell"
		#endif
	},
	#ifdef MONITOR_CMD_HELP
	{"help" , cmd_help
		#ifdef MONITOR_HELP
		, "Show help"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_INTERRUPTS
	{"di" , cmd_di
		#ifdef MONITOR_HELP
		, "Disable interrupts"
		#endif
	},
	{"ei" , cmd_ei
		#ifdef MONITOR_HELP
		, "Enable interrupts"
		#endif
	},
	{"ints" , cmd_ints
		#ifdef MONITOR_HELP
		, "Let ints happen"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_LOOP
	{"loop" , cmd_loop
		#ifdef MONITOR_HELP
		, "Run command in loop"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_PORT
	{"in" , cmd_in
		#ifdef MONITOR_HELP
		, "Read port"
		#endif
	},
	{"out" , cmd_out
		#ifdef MONITOR_HELP
		, "Output to port"
		#endif
	},
	#endif
	
	#ifdef MONITOR_CMD_PAUSE
	{"pause", cmd_pause
		#ifdef MONITOR_HELP
		, "Wait for key"
		#endif
	},
	#endif
	#ifdef MONITOR_CMD_SOFTSERIAL
	{"stest", cmd_serial_test
		#ifdef MONITOR_HELP
		, "SoftSerial test"
		#endif
	},
	#ifdef VGLDK_VARIABLE_STDIO
	{"sio", cmd_serial_io
		#ifdef MONITOR_HELP
		, "Switch stdio to serial"
		#endif
	},
	#endif
	{"sget", cmd_serial_get
		#ifdef MONITOR_HELP
		, "SoftSerial get"
		#endif
	},
	{"sgets", cmd_serial_gets
		#ifdef MONITOR_HELP
		, "SoftSerial gets"
		#endif
	},
	{"sput", cmd_serial_put
		#ifdef MONITOR_HELP
		, "SoftSerial put"
		#endif
	},
	#endif
	
	#ifdef MONITOR_CMD_VER
	{"ver"  , cmd_ver
		#ifdef MONITOR_HELP
		, "Version"
		#endif
	},
	#endif
};

#ifdef MONITOR_CMD_HELP
// Actual implementation of "help", since it needs to know the COMMANDS variable
int cmd_help(int argc, char *argv[]) {
	
	word i;
	
	if (argc < 2) {
		// List all commands
		for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
			if (i > 0) printf(" ");
			printf("%s", COMMANDS[i].name);
		}
		printf("\n");
		return 0;
		
	}
	
	#ifdef MONITOR_HELP
	// Specific help
	for(i = 0; i < (sizeof(COMMANDS) / sizeof(t_commandEntry)); i++) {
		if (strcmp(argv[1], COMMANDS[i].name) == 0) {
			printf("%s: %s\n", COMMANDS[i].name, COMMANDS[i].help);
			return 0;
		}
	}
	printf("%s?\n", argv[1]);
	return ERR_COMMAND_UNKNOWN;
	#else
	(void)argv;
	return ERR_NOT_IMPLEMENTED;
	#endif
}
#endif

void prompt() {
	//printf("%s>", cwd);
	putchar('>');
}

// Command handler
int eval(int argc, char *argv[]) {
	int i;
	
	// No input? Continue.
	if (argc == 0) return 0;
	
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
		return 0;
	}
	if (strcmp(argv[0], "exit") == 0) {
		running = false;
		return 0;
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
		printf("Exit code "); printf_d(r); printf("\n");
	}
}



void main() __naked {
	
	#ifdef VGLDK_VARIABLE_STDIO
		// Variable STDIO must be initialized
		stdio_init();
	#endif
	
	clear();
	
	#ifdef MONITOR_SOFTSERIAL_AUTOSTART
	if (serial_isReady()) {
		// If serial cable is connected: Jump right into serial mode
		
		#ifdef VGLDK_VARIABLE_STDIO
		printf("Switch to serial I/O?");
		if (getchar() == 'y') {
			printf("Y\n");
			cmd_serial_io(0, NULL);
		} else {
			printf("N\n");
		}
		#endif
	}
	#endif
	
	// Banner
	cmd_ver(0, NULL);
	
	// Command line loop
	running = true;
	while(running) {
		
		// Prompt for input
		prompt();
		gets(cmd_arg);
		
		//putchar('"'); printf(cmd_arg); putchar('"'); printf("\n");
		parse(cmd_arg);
		
	}
	// Exited...
	
	printf("Bye!");
	
	// Off into the abyss...
	
	
}
