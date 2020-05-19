/*
	A hardware monitor for the VGLDK
	
	2020-05-19 Bernhard "HotKey" Slawik
*/

#include <vgldk.h>

#include <stdio.h>	// for printf() putchar() gets() getchar()
#include <ports.h>

#define HEX_USE_DUMP
#include <hex.h>

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

const char VERSION[] = "Monitor 1.0";
const word tempAddr = 0xC400;

// Setup
#define MAX_ARGS 8
#define MAX_INPUT 64



// Features to include
#define MONITOR_HELP	// Include help functionality

//#define MONITOR_CMD_BEEP
#define MONITOR_CMD_CLS
#define MONITOR_CMD_DUMP
#define MONITOR_CMD_ECHO
#define MONITOR_CMD_HELP
#define MONITOR_CMD_LOOP
#define MONITOR_CMD_PORT
#define MONITOR_CMD_PAUSE
#define MONITOR_CMD_VER

// Definitions
#define ERR_COMMAND_UNKNOWN -4
#define ERR_MISSING_ARGUMENT -5
#define ERR_NOT_IMPLEMENTED -6



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
		printf("%s", argv[i]);
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

#ifdef MONITOR_CMD_LOOP
int cmd_loop(int argc, char *argv[]) {
	char c;
	
	while(1) {
		
		eval(argc-1, &argv[1]);
		
		c = inkey();
		#ifdef KEY_ESCAPE
		if (c == KEY_ESCAPE) break;
		#else
		if (c == 'q') break;
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
	
	printf("%02X:0x%02X ", p, v);
	for(p = 0; p < 8; p++) {
		if ((v & (1 << (7-p))) > 0) putchar('1'); else putchar('0');
	}
	printf(" %d\n", v);
	
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

#ifdef MONITOR_CMD_VER
int cmd_ver(int argc, char *argv[]) {
	(void) argc; (void) argv;
	
	//printf("%s\n", VERSION);
	printf("%s", VERSION);
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
	//for(i = 0; i < argc; i++) printf("args[%d]=\"%s\"\n", i, argv[i]);
	
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
	
	printf("\"%s\"?\n", argv[0]);
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
				ac = varValP + sprintf(varValP, "%s=%d", varNameP, 1234);	// Just output its name
				
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
			argc++;
		} else {
			// Add character to arg
			*ac++ = c;
		}
		
		sc++;
	}
	
	// Handle input
	r = eval(argc, argv);
	
	// Handle return value
	if (r != 0) {
		printf("Exit code %d\n", r);
	}
}


void main() __naked {
	char arg[MAX_INPUT];
	
	// Banner
	cmd_ver(0, NULL);
	
	
	// Command line loop
	running = true;
	while(running) {
		
		// Prompt for input
		prompt();
		gets(arg);
		parse(arg);
		
	}
	// Exited...
	
	printf("Bye!");
	
	// Off into the abyss...
	
	
}
