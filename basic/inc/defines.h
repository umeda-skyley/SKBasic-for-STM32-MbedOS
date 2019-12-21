/*
 *  defines.h      miscellaneous include file for the KLBasic interpreter
 */

#ifndef  DEFINES_H
#define  DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  History
 *
 *  Version 0.1
 *  Baseline release.  (Hooray!)
 *
 *  Version 0.2  (01 Sep 2008  KEL)
 *  Fixed a slew of bugs, including changes to rgoto() to allow jumping into
 *  the program from run mode or immediate mode.  Fixed break (ctrl-c) support.
 *  Cleaned up processing of function calls within a PRINT statement (didn't
 *  handle a space after the function invocation).  Added SGN() support.
 *
 *  Version 0.3   (28 Sep 2008  KEL)
 *  Fixed bug in outhexbyte() that printed the top nybble as 0.
 *
 *  Version 0.4   (30 Apr 2011  KEL)
 *  Added peek(), peek16(), and peek32().  Fixed MOD defines so KLB no longer
 *  recognizes % as mod operator; only sees MOD.  Added support for indirection;
 *  @ gives 32-bit indirection, @16 gives 16-bit indirection, @8 gives
 *  8-bit indirection.  Indirection works on either side of = (assignment).
 *  Spaces are permitted after the indirection operators.
 *
 *  Version 0.5   (18 Jun 2011  KEL)
 *  Renamed the indirection operators; they are now @ (8-bit), @16,
 *  and @32.  This is in keeping with the notion that peek() is an
 *  8-bit function.  Fixed bug that started the first RUN command in
 *  immediate mode, which caused the first GOSUB-RETURN to end the
 *  program.  Recoded the STACK structure to now be STACK8 and STACK32.
 *  This change let me optimize the opstack, which only uses 8-bit values.
 *  Cleaned up the code in pshop() to use the stack operators and to NOT
 *  try and execute op tokens when there is only one op token on the
 *  stack.  Added code to initialize the op and number stacks at the
 *  start of each line in the main loop.  Changed length of input buffer to 128
 *  chars.  Changed length of token buffer to 80 bytes.
 *
 *  Version 0.6    (2 Jul 2011  KEL)
 *  Fixed bug in findvar() that selected the wrong variable in the variable
 *  table if the name of the variable was a superset of the target variable
 *  (c in the table would match c, ca, or cat as targets).
 *  Numerous fixes as part of porting KLBasic to the LPC1768 (mbed).  Fixed
 *  bug that caused a lock-up if you autostarted a program that used arrays.
 *  Removed the #ifdef WINDOWS clauses; it is now either AVR or everything else.
 *
 *  Version 0.7    (16 Jan 2012   KEL)
 *  Added CALL statement, to support jumps to C/assembly langauge code stored
 *  in upper flash (addon or external code module).
 *  Older vesion of CALL, which was a function, has been commented out.
 *  Added function time$(), only available from within a print statement.
 *  Changed time command to systime.  Added sysdate command.
 *
 *  Version 0.8    (14 Apr 2012   KEL)
 *  This version of the core has had all mbed functionality removed and
 *  recoded, where appropriate, for AVR.  The corresponding header files
 *  for the mbed (LPC1768) version have been moved into the mbed development
 *  directory.  Added code to read the first line of each file stored in
 *  flash (not EEPROM) and display that following a LIST FILES command.
 *  Fixed bug in rdim() AVR code that clobbered a following variable in
 *  varram when a DIM statement was executed.  Fixed bug in function table
 *  that caused functions later in the table to show the wrong text when
 *  LISTed.
 *
 *  Version 0.9    (30 Apr 2012   KEL)
 *  Fixed bug in runline() that caused a crash if a command contained a
 *  mid-EOL (':') followed by one or more spaces.
 */
#include "compiler_options.h"



/*
 *  My version of the Microsoft VC++ compiler did not support stdint.h,
 *  so I'm defining them here.
 */
#if  defined(WINDOWS)
typedef signed char 		int8_t;
typedef unsigned char 		uint8_t;
typedef signed short 		int16_t;
typedef unsigned short 		uint16_t;
typedef signed int 			int32_t;
typedef unsigned int 		uint32_t;
typedef signed long int 	int64_t;
typedef unsigned long int 	uint64_t;
#define FARPTR
#define	CONST
#elif defined(CPU78K0R)
typedef signed char 		int8_t;
typedef unsigned char 		uint8_t;
typedef signed short 		int16_t;
typedef unsigned short 		uint16_t;
typedef signed long			int32_t;
typedef unsigned long 		uint32_t;
typedef signed long int 	int64_t;
typedef unsigned long int 	uint64_t;
#define FARPTR	__far
#define	CONST	const
#elif defined(CPUSTM32)
#include  <stdint.h>		// this will be target-specific
#define FARPTR
#define	CONST const
#else
#include  <stdint.h>		// this will be target-specific
#define FARPTR
#define	CONST
#endif


/*
 *  Now create a set of shorthand notations for the above standard
 *  integer types.
 */

#define  U8			uint8_t
#define  I8			int8_t
#define  U16		uint16_t
#define  I16		int16_t
#define  U32		uint32_t
#define  I32		int32_t
#define  U64		uint64_t
#define  I64		int64_t

 
 
/*
 *  Define the strings for the core info and core version.  The version
 *  is of the form "major.minor".  The strings for the target version
 *  must be defined in targetxxx.h and will be displayed in the
 *  outheader() function.
 */
#define  CORE_VERSION	"0.9"
#define  CORE_INFO      "KLBasic core"


typedef  struct  tm     TM;

#ifdef   OWNER
#ifndef EXTERN
#define  EXTERN
#endif
#else
#define  EXTERN  extern
#endif

#if  defined(AVR)
#include <avr/pgmspace.h>
#elif defined(CPU78K0R) || defined(CPUML7416)
#define  PSTR(x)  (FARPTR char *)(x)
#elif defined(CPUSTM32)
#define  PSTR(x)  (char *)(x)
#else
#define  PSTR(x)  x
#define  strcpy_P  strcpy
#endif


#include  "stacks.h"
#include  "funcs.h"




#ifndef  FALSE
#define  FALSE	  0
#define  TRUE     !FALSE
#endif



/*
 *  Define some system-wide macros for handling tasks otherwise done with sscanf()
 *  and sprintf();  This helps reduce the amount of space taken up by unused parts
 *  of these large library routines.
 */

/*
 *  SPRINTF_U2DEC      convert value v to two decimal digits, insert in string s
 */
#define  SPRINTF_U2DEC(v, s)   *s++=(((U8)v/10)+'0');    \
							   *s++=(((U8)v%10)+'0');


/*
 *  SSCANF_CHR2DEC      convert two decimal chars at s into value v
 */
#define  SSCANF_CHR2DEC(s, v)	v=(*s)-'0';		\
								v=(v*10)+(*(s+1)-'0');




/*
 *         ;$title    DEFINES
 *
 *         $IFDEF  HC11
 *
 ***** HC11EVB defines *****
 *
 *ROMBEG:   EQU    $E000
 *ROMSIZE:  EQU    $2000
 *ACIAST:   EQU    $9800
 *ACIADT:   EQU    ACIAST+1
 *DFLOP:    EQU    $4000
 *SWPRE:    EQU    02           ; SOFTWARE PRESCALER VALUE.
 *
 *
 *
 *SBASBEG:  equ    0
 *SBASEND:  equ    2
 *SVARBEG:  equ    4
 *SVAREND:  equ    6
 *SHILINE:  equ    8
 *AUTOSTF:  equ    10
 *SSTART:   equ    11
 *
 *         $ELSE
 *
 ***** 6809/FLEX development defines ****
 *
 *ROMBEG:   EQU    $8000
 *ROMSIZE:  EQU    $2000
 *RAMBEG:   EQU    $4000
 *RAMSIZE:  EQU    $2000
 *STACKP:   EQU    RAMBEG+RAMSIZE-1
 *SWSTACK:  EQU    RAMBEG+RAMSIZE-512
 *ACIAST:   EQU    $E010
 *ACIADT:   EQU    ACIAST+1
 *
 *        ORG    $6000
 *
 *SEEPROM:  EQU    *
 *SBASBEG:  RMB    2
 *SBASEND:  RMB    2
 *SVARBEG:  RMB    2
 *SVAREND:  RMB    2
 *AUTOSTF:  RMB    1
 *SHILINE:  RMB    2
 *SSTART:   EQU    *
 *
 *
 *        $ENDIF
 */



/*
 **** hc11 (device dependent) defines ****
 * 
 *EEPBASAD: EQU    $B600			; // EEPROM base address
 *MAXEESUB: EQU    255			; // maximum EEP subscript
 *
 *         I/O Register Offsets From The Base Address
 *
 *PPROG:    EQU    $3B			; // EEPROM programing control register
 *ADCTL:    EQU    $30			; // A-TO-D control/status register
 *ADR1:     EQU    $31			; // A/D result register 1
 *ADR2:     EQU    $32           ; // A/D result register 2
 *ADR3:     EQU    $33           ; // A/D result register 3
 *ADR4:     EQU    $34           ; // A/D result register 4
 *PORTAIO:  EQU    $00           ; // PORTA I/O register
 *PORTBIO:  EQU    $04           ; // PORTB I/O register
 *PORTCIO:  EQU    $03           ; // PORTC I/O register
 *PORTDIO:  EQU    $08           ; // PORTD I/O register
 *PORTEIO:  EQU    $0A           ; // PORTE I/O register
 *TCNT:     EQU    $0E           ; // TIMER/COUNTER register
 *TOC1REG:  EQU    $16           ; // TIMER Output Compare 1 register
 *TFLAG1:   EQU    $23           ; // TIMER Flag #1 register
 *TMSK1:    EQU    $22           ; // TIMER Mask #1 register
 *TMSK2:    EQU    $24           ; // TIMER Mask #2 register
 *OPTION:   EQU    $39           ; // OPTION select register
 *BAUD:     EQU    $2B           ; // SCI baud rate select register
 *SCCR1:    EQU    $2C           ; // SCI control register #1
 *SCCR2:    EQU    $2D           ; // SCI control register #2
 *SCSR:     EQU    $2E           ; // SCI status register
 *SCDR:     EQU    $2F           ; // SCI transmit/recieve data register
 *PACNT:    EQU    $27           ; // PACC count register
 *PACTL:    EQU    $26           ; // PACC control register
 *TFLG2:    EQU    $25           ; // TIMER Flag #2 register
 *INIT:     EQU    $3D           ; // INIT (Base address of RAM & I/O Regs) Register
 */




/*
 *  Define a function table structure.  This structure contains a
 *  name/pointer pair for various types of functions.
 */

typedef  struct
{
	void				(*funcptr)(void);
	char				name[10];
}  RTFUNC;





/**** misc. defines ****/
 
#define  EOL		13		/* end of line marker */
#define  CR			13      /* same as EOL */
#define  LF			10      /* linefeed character */
#define  BS			'\b'    /* backspace character */
#define  SPC		' '     /* space character */
#define  MIDEOL		':'     /* mid EOL character */
#define  COMMA		','     /* comma */
#define  SEMI		';'     /* semicolin */
#define  NUM		1       /* getvar return flag */
#define  STRING		2       /* getvar return flag */
#define  BREAK_CHAR 3       /* control-c (break character) */
//#define  NULL		0       /* null value */			// defined in stdio.h

#define  AUTOST_EE	0x55	/* this value in AUTOST location in EEPROM means OK to run EEPROM */
#define  AUTOST_FL0	0x5a	/* this value in AUTOST location in EEPROM means OF to run flash0 */
#define  AUTOST_FL1	0x5b	/* this value in AUTOST location in EEPROM means OF to run flash1 */
#define  AUTOST_FL2	0x5c	/* this value in AUTOST location in EEPROM means OF to run flash2 */
#define  AUTOST_OFF	0xff	/* this value in AUTOST location in EEPROM means no autostart */


#define  PROMPT_CHAR	'>'	/* char to display for input prompt */


#define  IBUFLEN	128     /* input buffer max length */
#define  TBUFLEN	128      /* token buffer max length */
//#define  SWSTKSize	592

#define  OPSLEN		30      /* operator stack length */
#define  NUMSLEN	60      /* operand stack length */
#define  FORSLEN	10      /* nuumber of FOR_ENTRY items in forstack */
#define  WHSLEN		16      /* WHILE..ENDWH stack length */
#define  GOSLEN		16      /* GOSUB stack length */

#define  MAX_VAR_NAME_LEN   14	/* max chars in variable or port name */

#define  RANDOM_SEED_VAL   12345678L		/* used in rrnd() to seed random number gen */

#define  INITIAL_STRASTG	2		/* initial dynamic memory index (bytes), must not be 0! */

#define  MAX_CALL_ARGS		4		/* maximum number of arguments in CALL statement */


/*
 *  For those targets that support flash files (files stored in on-chip flash),
 *  define the number of files available.
 *
 *  This needs to be globally defined here, rather than in the targetxxx.h file,
 *  so the LIST command code knows how many flash files to process when listing
 *  files.
 */
#define  NUM_FLASH_FILES			3
#define  MAX_FILE_NAME_LEN			30


//       ***** define error codes *****
 
#define  LINRANG		1	/* line number range error */
#define  SYTXERR		2	/* syntax error */
#define  IVEXPERR		3	/* invalid expression error */
#define  UPARNERR		4	/* unbalanced parentheses error */
#define  DTMISERR		5	/* data type mismatch error */
#define  OPRTRERR		6	/* illegal operator error */
#define  ILVARERR		7	/* illegal variable error */
#define  ILTOKERR		8	/* illegal token error */
#define  OMEMERR		9	/* out of memory error */
#define  INTOVERR		10	/* integer overflow error */
#define  IVHEXERR		11	/* invalid hex digit error */
#define  HEXOVERR		12	/* hex number overflow */
#define  MISQUERR		13	/* missing quote error */
#define  MPARNERR		14	/* missing open or closing parenthisis */
#define  IONSYERR		15	/* "ON" syntax error */
#define  MTHENERR		16	/* missing "THEN" in "IF" statement */
#define  MTOERR			17	/* missing "TO" in "FOR" statement */
#define  LINENERR		18	/* line number error */
#define  IDTYERR		19	/* illegal data type error */
#define  EXPCXERR		20	/* expression too complex (xlator token buff ovf.) */
#define  MCOMAERR		21	/* missing comma */
#define  MCMSMERR		22	/* missing comma or semicolin */
#define  MSTKOERR		23	/* math stack overflow error */
#define  UNDIMERR		24	/* undimensioned array error */
#define  SUBORERR		25	/* subscript out of range error */
#define  ZDIVERR		26	/* divide by zero error */
#define  LNFERR			27	/* line not found error */
#define  GOSOVERR		28	/* too many nested GOSUB's */
#define  RWOGERR		29	/* RETURN w/o GOSUB error */
#define  WHSOVERR		30	/* too many active WHILE's */
#define  ENDWHERR		31	/* ENDWH statement w/o WHILE */
#define  ONARGERR		32	/* ON argument is negative, zero, or too large */
#define  NOSUBERR		33	/* non-subscriptable variable found in DIM statem. */
#define  REDIMERR		34	/* variable has already been DIMensioned */
#define  FORNXERR		35	/* too many active FOR -- NEXT loops */
#define  MFRNXERR		36	/* mismatched FOR -- NEXT statements. */
#define  CNTCNERR		37	/* can't continue */
#define  ODRDERR		38	/* out of data in read or restore statement */
#define  NEGSUBER		39	/* negative subscripts not allowed */
#define  EESUBERR		40	/* EEP() subscript negative or > 200 */
#define  PRFUNERR		41	/* function only allowed in print statement */
#define  TABARGER		42	/* argument <0 or >255 in TAB() function */
#define  CALLARGSER		43	/* too many arguments in CALL statement */
//#define  CHRARGER		43	/* argument <0 or >255 in CHR$() function */
#define  BADTIMEFMTERR	44	/* argument to time$() function is unknown */
//#define  INVCHERR		45	/* invalid channel number in ADC() function */
//#define  PRTASERR		46	/* tried to assign a value <0 or >255 to PORT(X) */
//#define  ILPRTERR		47	/* illegal port error */
//#define  ILLIOERR		48	/* illegal I/O vector number <0 or >7 */
//#define  UNINIERR		49	/* uninitalized I/O vector */
//#define  HEX2AERR		50	/* argument <0 or >255 in HEX2 function */
#define  NOTALERR		51	/* statement not allowed in direct mode */
//#define  NOTINTER		52	/* an RETI statement executed when not in interrupt */
//#define  PACCARGE		53	/* tried to assign a value of <0 or >255 to PACC */
//#define  INTMODER		54	/* interrupt or count mode error in ONPACC */
#define  EETOSMAL		55	/* program storage EEPROM is Too Small */

#define  STKUNDER		56  /* stack underflow */
#define  BRKDETECT		57	/* user entered break char on console */

#define  CONADDRERR		58	/* cannot use ADDR() with constant */

#define  FLTOSMAL		59	/* program storage flash is too small */

#define	INVARGERR		60	/* Illegal argument */
#define	OUTOFRANGEERR	61
#define	STROVERFLOWERR	62
#define	MUST16ERR		63

#if defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
#define	EXIT_BASIC		255
#endif

/*
 *  Define error message strings.
 *
 *  The order of these error strings must match EXACTLY with the error
 *  code number above; the error code number is used as the index into
 *  this array.  Note that the array starts with error message #1, not
 *  0.  When indexing into this array, be sure to use (errcode-1) to
 *  adjust for this.
 */

EXTERN  FARPTR CONST char      ErrStrs[][40]

#ifdef  AVR
PROGMEM
#endif

#ifdef  OWNER
=  {
	"Line number out of range",			// 1
	"Syntax error",						// 2
	"Invalid expression",				// 3
	"Unbalanced parens",				// 4
	"Data type mismatch",				// 5
	"Illegal operator",					// 6
	"Illegal variable",					// 7
	"Illegal token (internal error)",	// 8
	"Out of memory",					// 9
	"Integer overflow",					// 10
	"Invalid hex digit",				// 11
	"Hex number overflow",				// 12
	"Missing quote",					// 13
	"Expected open or close paren",		// 14
	"'ON' syntax error",				// 15
	"Missing THEN in IF statement",		// 16
	"Missing TO in FOR statement",		// 17
	"Line number error",				// 18
	"Illegal data type",				// 19
	"Expression too complex",			// 20
	"Missing comma",					// 21
	"Missing comma or semicolon",		// 22
	"Math stack overflow",				// 23
	"Undimensioned array",				// 24
	"Subscript out of range",			// 25
	"Divide by zero",					// 26
	"Line not found",					// 27
	"Too many nested GOSUBs",			// 28
	"RETURN without GOSUB",				// 29
	"Too many active WHILEs",			// 30
	"ENDWH without WHILE",				// 31
	"ON argument must be > zero",		// 32
	"Nonsubscript variable in DIM",		// 33
	"Variable already DIMensioned",		// 34
	"Too many active FOR-NEXT loops",	// 35
	"NEXT does not match FOR",			// 36
	"Can't CONTinue",					// 37
	"Out of data in READ or RESTORE",	// 38
	"Negative subscript not allowed",	// 39
	"Illegal EEP() argument",			// 40
	"Only allowed in PRINT statement",	// 41
	"Argument out of range in TAB",		// 42
	"Too many arguments in CALL",		// 43
	"Arugment to time$() is unknown",	// 44
	"-- 45 --",							// 45
	"-- 46 --",							// 46
	"-- 47 --",							// 47
	"-- 48 --",							// 48
	"-- 49 --",							// 49
	"-- 50 --",							// 50
	"-- 51 --",							// 51
	"-- 52 --",							// 52
	"-- 53 --",							// 52
	"-- 54 --",							// 54
	"Not enough EEPROM for save",		// 55
	"Stack underflow",					// 56
	"User entered BREAK",				// 57
	"Can't use ADDR with constant",		// 58
	"Not enough flash for save",		// 59
	"Illegal argument",					// 60
	"Argument out of range",			// 61
	"Strings overflow",					// 62
	"Secure key must be 16bytes"		// 63
	" "									// end of table
}
#endif
;







/*
 *                          Token definitions
 *
 *  The mathematical operator tokens and the keyword tokens share the same
 *  numerical sequence.  This explains the gaps in the token sequence in
 *  both tables.
 *
 *  If you want to assign new tokens, you must choose values not used by
 *  EITHER table.
 */

/*
 *  Define the mathematical operator tokens. 
 *  These defines contain heirarchy information in bits 4-7.  See the
 *  pushop() and doop() functions for how these bits are used.
 */
#define  OP_PRECED_MASK	0xf0	/* top four bits hold operator precedence */
#define  OPARNTOK		0x10	/* '(' */
#define  CPARNTOK		0x11	/* ')' */
#define  ANDTOK			0x20	/* 'AND' */
#define  ORTOK			0x21	/* 'OR' */
#define  EORTOK			0x22	/* 'EOR' */
#define  LTTOK			0x30	/* '<' */
#define  GTTOK			0x31	/* '> */
#define  LTEQTOK		0x32	/* '<=' */
#define  GTEQTOK		0x33	/* '>=' */
#define  EQTOK			0x34	/* '=' */
#define  NOTEQTOK		0x35	/* '<>' */
#define  PLUSTOK		0x40	/* '+' */
#define  MINUSTOK		0x41	/* '-' */
#define  SPLUSTOK		0x42	/* '+' */
#define  MULTTOK		0x50	/* '*' */
#define  DIVTOK			0x51	/* '/' */
#define  MODTOK			0x52	/* '%' */
#define  PWRTOK			0x60	 /* '^' */
#define  INDIRTOK		0x70	/* '@' */
#define  NOTTOK			0x71	/* 'NOT' */
#define  NEGTOK			0x72	/* '-' (unary minus) */
#define  INDIR32TOK		0x73	/* '@32' */
#define  INDIR16TOK		0x74	/* '@16' */


 
/*
 *  Define the keyword tokens used internally by KLBasic.
 */
 
#define  LETTOK			1		/* LET */
#define  IMLETTOK		2		/* implied LET */
#define  PRINTTOK		3		/* PRINT */
#define  FORTOK			4		/* FOR */
#define  NEXTTOK		5		/* NEXT */
#define  TRONTOK		6		/* TRON */
#define  TROFFTOK		7		/* TROFF */
#define  POKETOK		8		/* POKE */
#define  DIMTOK			9		/* DIM */
#define  REMTOK			0x0A	/* REM */
#define  CALLTOK		0x0B	/* CALL */
#define  DATATOK		0x0C	/* DATA */
#define  READTOK		0x0D	/* READ */
#define  RESTRTOK		0x0E	/* RESTORE */
#define  GOSUBTOK		0x0F	/* GOSUB */
#define  GOTOTOK		0x12	/* GOTO */
#define  ONTOK			0x13	/* ON */
#define  RETNTOK		0x14	/* RETURN */
#define  IFTOK			0x15	/* IF */
#define  INPUTTOK		0x16	/* INPUT */
#define  STOPTOK		0x17	/* STOP */
#define  ENDTOK			0x18	/* END */
#define  WHILETOK		0x19	/* WHILE */
#define  ENDWHTOK		0x1A	/* ENDWH */
#define  EEPTOK			0x1B	/* EEP */
#define  EEP16TOK		0x1C	/* EEP16 */
#define  EEP32TOK		0x1D	/* EEP32 */
//#define  TIMESTRTOK		0x1E	/* TIMESTR */	// this should be a function
//#define  DATESTRTOK		0x1F	/* DATESTR */	// this should be a function
#define  INBYTTOK		0x23	/* INBYTE */
//#define  TIMETOK		0x24	/* TIME */			// this is now a command, token not needed
#define  ONTIMTOK		0x25	/* ONTIME */
#define  ONIRQTOK		0x26	/* ONIRQ */
#define  RETITOK		0x27	/* RETI */
#define  ONPACTOK		0x28	/* ONPACC */
#define  SLEEPTOK		0x29	/* SLEEP */
//#define  RTIMETOK		0x2A	/* RTIME */			// this is not used by KLBasic
#define  FUNCTFLG		0x36	/* function flag byte */
#define  TOTOK			0x37	/* TO */
#define  THENTOK		0x38	/* THEN */
#define  ELSETOK		0x39	/* ELSE */
#define  STEPTOK		0x3A	/* STEP */
#define  QPRINTTOK		0x43	/* Quick print (?) */
#define  QREMTOK		0x44	/* Quick REM (') */
#define  LABELTOK		0x61	/* LABEL */

#define	STRCATTOK		0x2b
#define	STRINDTOK		0x2c

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
#define	SKSREGTOK		0x45
#define	SKSENDTOK		0x46
#define	SKFLASHTOK		0x47
#define	SKBCTOK			0x48
#define	SKSYNCTOK		0x49
#define	SKINQTOK		0x4a
#define	SKPAIRTOK		0x4b
#define	SKUNPAIRTOK		0x4c
#define	SKNOWTOK		0x4d
#define	SKSLEEPTOK		0x4e
#define	SKSETPSTOK		0x4f
#define	SKCLRTBLTOK		0x53
#define	SKCLRCACHETOK	0x54
#define	SKLKUPTOK		0x55
#define	SKREVLKUPTOK	0x56
#define	SKSETNAMETOK	0x57
#define	SKSETKEYTOK		0x58
#define	SKARGTOK		0x59
#define	RXDATATOK		0x5a
#define	ACKTOK			0x5b
#define	EVENTTOK		0x5c
#define	WAITSETTOK		0x5d
#define	WAITJNETOK		0x5e
#elif	defined(SKBASIC_EMBEDDED)
#define	SKARGTOK		0x59
#define	TIM1OK			0x5a
#define	TIM2OK			0x5b
#define	WAITSETTOK		0x5d
#define	WAITJNETOK		0x5e
#endif

/*
 *  Define the keyword strings recognized by KLBasic.  Keywords are
 *  commands or non-function verbs used either inside a program
 *  or at the command prompt.
 */

#define  LET_STR		"let"
#define  PRINT_STR		"print"
#define  FOR_STR		"for"
#define  NEXT_STR		"next"
#define  TRON_STR		"tron"
#define  TROFF_STR		"troff"
#define  POKE_STR		"poke"
#define  DIM_STR		"dim"
#define  REM_STR		"rem"
#define  QREM_STR		"'"				/* quick REM is a single-quote */
#define  CALL_STR		"call"
#define  DATA_STR		"data"
#define  READ_STR		"read"
#define  RESTORE_STR	"restore"
#define  GOSUB_STR		"gosub"
#define  QPRINT_STR		"?"
#define  GOTO_STR		"goto"
#define  ON_STR			"on"
#define  RETURN_STR		"return"
#define  IF_STR			"if"
#define  INPUT_STR		"input"
#define  STOP_STR		"stop"
#define  END_STR		"end"
#define  WHILE_STR		"while"
#define  ENDWHILE_STR	"endwh"
#define  EEP_STR		"eep"
#define  EEP16_STR		"eep16"
#define  EEP32_STR		"eep32"
#define  INBYTE_STR		"inbyte"
#define  ONTIME_STR		"ontime"
#define  ONIRQ_STR		"onirq"
#define  RETI_STR		"reti"
#define  ONPACC_STR		"onpacc"
#define  SLEEP_STR		"sleep"
#define  RTIME_STR		"rtime"
#define  AND_STR		"and"
#define  OR_STR			"or"
#define  EOR_STR		"eor"
#define  NOT_STR		"not"
#define  NEG_STR		"-"
#define  PLUS_STR		"+"
#define  MINUS_STR		"-"
#define  STRPLUS_STR	"+"
#define  MULT_STR		"*"
#define  DIV_STR		"/"
#define  MOD_STR		"mod"
#define  TO_STR			"to"
#define  STEP_STR		"step"
#define  THEN_STR		"then"
#define  ELSE_STR		"else"
#define  LT_STR			"<"
#define  GT_STR			">"
#define  LTEQ_STR		"<="
#define  GTEQ_STR		">="
#define  EQ_STR			"="
#define  NOTEQ_STR		"<>"
#define  INDIR32_STR	"@32"
#define  INDIR_STR		"@"
#define  INDIR16_STR	"@16"
#define  PWR_STR		"^"
#define  LABEL_STR		"label"

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
#define  WAIT_STR		"wait"
#define  EVENT_STR		"EVENT"
#define  RXDATA_STR		"RXDATA"
#define  ACK_STR		"ACK"
#define  SYNC_STR		"SYNC"
#define  PAIR_STR		"PAIR"
#define  DETECT_STR		"DETECT"
#define  FOUNDN_STR		"FOUNDN"
#define  FOUNDC_STR		"FOUNDC"
#define  PONG_STR		"PONG"
#define  NEIGHBOR_STR	"NEIGHBOR"
#define  SKSREG_STR		"SKSREG"
#define  SKSEND_STR		"SKSEND"
#define  SKFLASH_STR	"SKFLASH"
#define  SKBC_STR		"SKBC"
#define  SKSYNC_STR		"SKSYNC"
#define  SKINQ_STR		"SKINQ"
#define  SKPAIR_STR		"SKPAIR"
#define  SKUNPAIR_STR	"SKUNPAIR"
#define  SKNOW_STR		"SKNOW"
#define  SKSLEEP_STR	"SKSLEEP"
#define  SKSETPS_STR	"SKSETPS"
#define  SKCLRTBL_STR	"SKCLRTBL"
#define  SKCLRCACHE_STR	"SKCLRCACHE"
#define  SKLKUP_STR		"SKLKUP"
#define  SKREVLKUP_STR	"SKREVLKUP"
#define  SKSETNAME_STR	"SKSETNAME"
#define  SKSETKEY_STR	"SKSETKEY"
#endif
#if defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
#define  EXIT_STR		"exit"
#endif
#if defined(SKBASIC_EMBEDDED)
#define  TIM1_STR		"TIM1"
#define  TIM2_STR		"TIM2"
#define  WAIT_STR		"wait"
#endif

#define  PNUM_STR		"#"
#define  EQUAL_STR		"="
#define  MIDEOL_STR		":"
#define  SEMI_STR		";"
#define  COMMA_STR		","




 
/*
 *  Define the function tokens used by KLBasic.  Funtions return a value
 *  when invoked.
 */
 
//#define  FDIVTOK		0x01	/* FDIV */
#define  CHRTOK			0x02	/* CHR$ */
#define  SQRTTOK		0x03	/* SQRT */
#define  ABSTOK			0x04	/* ABS */
#define  RNDTOK			0x05	/* RND */
#define  SGNTOK			0x06	/* SGN */
#define  TABTOK			0x07	/* TAB */
//#define  CALLTOK		0x08	/* CALL */
#define  PEEKTOK		0x09	/* PEEK */
#define  FEEPTOK		0x0A	/* EEP */
#define  HEXTOK			0x0B	/* HEX */
//#define  FTIMETOK		0x0c	/* TIME */		// not used in KLBasic
#define  TIMESTRTOK		0x0c	/* TIME$ */
#define  HEX2TOK		0x0d	/* HEX2 */
#define  HEX4TOK		0x0e	/* HEX4, was PACC */
#define  PEEK16TOK		0x0f	/* PEEK16 */
#define	 PEEK32TOK		0x10	/* PEEK32 */
#define  ADDRTOK		0x11	/* ADDR */
#define  FEEP16TOK		0x12	/* EEP16 */
#define  FEEP32TOK		0x13	/* EEP32 */
#define  FSTRCMPTOK		0x14	/* strcmp */
#define  FSTRINDTOK		0x15	/* strind */
#define  FSTRLENTOK		0x16	/* strlen */
//#define  FPORTE_STR		0x10	/* PORTE */




/*
 *  Define the function names as strings.
 */

//#define	 FDIV_STR		"fdiv"
#define  CHRS_STR		"chr$"
#define  SQRT_STR		"sqrt"
#define  ABS_STR		"abs"
#define  RND_STR		"rnd"
#define  SGN_STR		"sgn"
#define  TAB_STR		"tab"

#define  PEEK_STR		"peek"
#define  PEEK16_STR		"peek16"
#define  PEEK32_STR		"peek32"

#define  FEEP_STR		"eep"
#define  HEX_STR		"hex"
#define  FEEP16_STR		"eep16"
#define  FEEP32_STR		"eep32"
#define  TIMESTR_STR	"time$"
#define  DATESTR_STR	"date$"
#define  FPORTE_STR		"PORTE"
//#define  FTIME_STR		"time"
#define  HEX2_STR		"hex2"
//#define  FPACC_STR		"pacc"
#define  HEX4_STR		"hex4"
#define  ADDR_STR		"addr"
#define	STRCAT_STR		"strcat"
#define	STRCMP_STR		"strcmp"
#define	STRIND_STR		"strind"
#define	STRLEN_STR		"strlen"



/*
 *  FunctionTable holds the keyword strings and matching function pointers for
 *  the supported KLBasic functions.  The placement of a function name/pointer
 *  entry in the table must match the token number assigned to that function.
 *  For example, the ABS function has token 0x04, so the ABS name/pointer entry
 *  must be the fourth (1-based) entry in this table.
 */
EXTERN FARPTR CONST RTFUNC  FunctionTable[]

#ifdef  AVR
PROGMEM
#endif

#ifdef  OWNER
=  {
	{0,					" "},			// 0x01  FDIV removed, not needed
	{rchrs,				CHRS_STR},		// 0x02
	{rsqrt,				SQRT_STR},		// 0x03
	{rabs,				ABS_STR},		// 0x04
	{rrnd,				RND_STR},		// 0x05
	{rsgn,				SGN_STR},		// 0x06
	{rtab,				TAB_STR},		// 0x07
	{0,					" "},			// 0x08
	{rpeek,				PEEK_STR},		// 0x09
	{rfeep,				FEEP_STR},		// 0x0a
	{rhex,				HEX_STR},		// 0x0b
//	{rtimestr,			TIMESTR_STR},	// 0x0c
	{0,					" "},			// 0x0c  placeholder for timestr, remove when you restore the rtimestr entry
	{rhex2,				HEX2_STR},		// 0x0d
	{rhex4,				HEX4_STR},		// 0x0e
	{rpeek16,			PEEK16_STR},	// 0x0f
	{rpeek32,			PEEK32_STR},	// 0x10
	{raddr,				ADDR_STR},		// 0x11
	{rfeep16,			FEEP16_STR},	// 0x12
	{rfeep32,			FEEP32_STR},	// 0x13
	{rfstrcmp,			STRCMP_STR},	// 0x14
	{rfstrind,			STRIND_STR},	// 0x15
	{rfstrlen,			STRLEN_STR},	// 0x16
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	{0,					" "},			// 0x17
	{0,					" "},			// 0x18
	{0,					" "},			// 0x19
	{0,					" "},			// 0x1a
	{0,					" "},			// 0x1b
	{0,					" "},			// 0x1c
	{0,					" "},			// 0x1d
	{0,					" "},			// 0x1e
	{0,					" "},			// 0x1f
	{0,					" "},			// 0x20
	{0,					" "},			// 0x21
	{0,					" "},			// 0x22
	{0,					" "},			// 0x23
	{0,					" "},			// 0x24
	{0,					" "},			// 0x25
	{0,					" "},			// 0x26
	{0,					" "},			// 0x27
	{0,					" "},			// 0x28
	{0,					" "},			// 0x29
	{0,					" "},			// 0x2a
	{0,					" "},			// 0x2b
	{0,					" "},			// 0x2c
	{0,					" "},			// 0x2d
	{0,					" "},			// 0x2e
	{0,					" "},			// 0x2f
	{0,					" "},			// 0x30
	{0,					" "},			// 0x31
	{0,					" "},			// 0x32
	{0,					" "},			// 0x33
	{0,					" "},			// 0x34
	{0,					" "},			// 0x35
	{0,					" "},			// 0x36
	{0,					" "},			// 0x37
	{0,					" "},			// 0x38
	{0,					" "},			// 0x39
	{0,					" "},			// 0x3a
	{0,					" "},			// 0x3b
	{0,					" "},			// 0x3c
	{0,					" "},			// 0x3d
	{0,					" "},			// 0x3e
	{0,					" "},			// 0x3f
	{0,					" "},			// 0x40
	{0,					" "},			// 0x41
	{0,					" "},			// 0x42
	{0,					" "},			// 0x43
	{0,					" "},			// 0x44
	{0,					" "},			// 0x45
	{rsksend,			SKSEND_STR},	// 0x46
	{rskflash,			SKFLASH_STR},	// 0x47
	{rskbc,				SKBC_STR},		// 0x48
	{rsksync,			SKSYNC_STR},	// 0x49
	{rskinq,			SKINQ_STR},		// 0x4a
	{rskpair,			SKPAIR_STR},	// 0x4b
	{rskunpair,			SKUNPAIR_STR},	// 0x4c
	{0,					" "},			// 0x4d
	{0,					" "},			// 0x4e
	{0,					" "},			// 0x4f
	{0,					" "},			// 0x50
	{0,					" "},			// 0x51
	{0,					" "},			// 0x52
	{0,					" "},			// 0x53
	{0,					" "},			// 0x54
	{rsklkup,			SKLKUP_STR},	// 0x55
	{rskrevlkup,		SKREVLKUP_STR},	// 0x56
#endif
	{0,					" "}
}
#endif						// #ifdef  OWNER
;





/*
 *  Define the commands known to KLBasic.  These commands must be entered at
 *  the command line; they cannot appear inside a program.
 */

#define  LIST_STR		"list"
#define  RUN_STR		"run"
#define  NEW_STR		"new"
#define  CLEAR_STR		"clear"
#define  CONT_STR		"cont"
#define  FREE_STR		"free"
#define  LOAD_STR		"load"
#define  SAVE_STR		"save"
#define  AUTO_STR		"autost"
#define  SYSTIME_STR	"systime"
#define  SYSDATE_STR	"sysdate"
//#define  NOAUTO_STR		"noauto"
//#define  FLSAVE_STR		"flsave"
//#define  FLLOAD_STR		"flload"

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
#define  SKINFO_STR		"SKINFO"
#define  SKRESET_STR	"SKRESET"
#define  SKSAVE_STR		"SKSAVE"
#define  SKLOAD_STR		"SKLOAD"
#define  SKRFCTRL_STR	"SKRFCTRL"
#define  SKRFREG_STR	"SKRFREG"
#define  SKVER_STR		"SKVER"
#define  SKPOW_STR		"SKPOW"
#define  SKTABLE_STR	"SKTABLE"

#define  FORMAT_STR		"format"
#define  FILES_STR		"files"
#define  DELETE_STR		"delete"
#endif

// autostart options

#define  AUTOST_OFF_STR		"off"
#define  AUTOST_EE_STR		"ee"
#define  AUTOST_FL0_STR		"fl0"
#define  AUTOST_FL1_STR		"fl1"
#define  AUTOST_FL2_STR		"fl2"



// numerical/variable tokens 

#define  ARRAY_MASK		0x10
#define  CONST_MASK		0x20
 
#define  FVARTOK		0x81					/* floating point variable address */
#define  SVARTOK		0x82					/* string variable address */
#define  PVARTOK		0x83					/* port address (treat as a variable) */
#define  IVARTOK		0x84					/* integer variable address */
 
#define  FAVARTOK		(FVARTOK | ARRAY_MASK)	/* floating point array (0x91) */
#define  SAVARTOK		(SVARTOK | ARRAY_MASK)	/* string array (0x92) */
#define  IAVARTOK		(IVARTOK | ARRAY_MASK)	/* integer array (0x94) */
 
#define  FCONTOK		(FVARTOK | CONST_MASK)	/* floating point constant (0xa1) */
#define  SCONTOK		(SVARTOK | CONST_MASK)	/* string constant (0xa2) */
#define  ICONTOK		(IVARTOK | CONST_MASK)	/* integer constant (0xa4) */
#define  LCONTOK		(0x88 | CONST_MASK)		/* line # constant (0xa8) */
 
#define  ISIZ			(sizeof(U32))			/* number of bytes in integer variable */
#define  SSIZ			256						/* number of bytes in string variable */
#define  PSIZ			(sizeof(U16))			/* number of bytes in port address */
#define  FSIZ			(sizeof(U32))			/* number of bytes in f.p. variable */
#define  ASIZ			(sizeof(U32))			/* number of bytes for array variable in dictionary */


#define  BREAK_CNT		10						/* used in crun() to time check for breaks */

// misc. tokens 
 
#define  MSCNTOK		0x7F	/* multiple space count token */
#define  SSCNTOK		0x7E	/* single space token */
#define  EOLTOK			0x7D	/* end of line token */
#define  COMMATOK		0x7C	/* , */
#define  SEMITOK		0x7B	/* ; */
#define  MEOLTOK		0x7A	/* : */
#define  EQUALTOK		0x79	/* '=' */
#define  PNUMTOK		0x78	/* '#' */


#define  JMPOP			0x7E	/* OP-CODE FOR "JMP" (USED TO INITALIZE INTERRUPT TABLE) */



/*
 *  define structs
 */
typedef struct
{
	U16					voff;		// index into var table for control variable
	I32					step;		// step value (could be negative!)
	I32					termval;	// terminating loop value
	unsigned char		*tbuf;		// points to end of FOR statement
	U16					cur;		// holds curline for this FOR statement
}  FOR_ENTRY;


typedef struct
{
	unsigned char		*thenptr;	// points to start of TRUE (THEN) code
	unsigned char		*elseptr;	// points to start of FALSE (ELSE) code
	int					curline;	// holds curline for this IF-THEN statement
}  IF_ENTRY;



 
//       *********** define variables *********

EXTERN  unsigned char		*ibufptr;		// pointer into input buffer
EXTERN  unsigned char		inbuff[IBUFLEN];	// input buffer
EXTERN  unsigned char		*tbufptr;		// pointer into token buffer
EXTERN  unsigned char		tknbuf[TBUFLEN];	// token buffer

EXTERN  unsigned char		*basbeg;		// start of basic program area
EXTERN  unsigned char		*basend;		// end of basic program
EXTERN  unsigned char		*basptr;		// pointer into basic program area

EXTERN  U8					*varbegin;		// start of variable storage area
EXTERN  U8					*varend;		// end of variable storage area
EXTERN  U16					hiline;			// highest line number in program buffer

EXTERN  unsigned char		*basmend;		// physical end of basic program memory
EXTERN  unsigned char		*varmend;		// physical end of variable memory



EXTERN  unsigned int		firstlin;		// first line to list
EXTERN  unsigned int		lastlin;		// last line to list
EXTERN  unsigned char		*intptr;		// integer pointer


EXTERN  unsigned char		errcode;		// error status byte
EXTERN  unsigned char		immid;			// immediate mode flag
EXTERN  unsigned char		breakcnt;		// break check count
EXTERN  volatile unsigned char		breakflag;		// TRUE if user entered break char from console
EXTERN  unsigned char		count;			// count used in ESAVE and ELOAD routines (need??)
EXTERN  unsigned char		ifwhflag;		// translating IF flag
EXTERN  unsigned char		trflag;			// trace mode flag
EXTERN  unsigned char		contflag;		// continue flag
EXTERN  unsigned char		runflag;		// indicates we are in run mode
EXTERN  unsigned char		printpos;		// current print position

EXTERN  STACK32				numstack;		// numeric operand stack
EXTERN  STACK8				opstack;		// operator stack
// EXTERN  STACK				forstack;		// FOR stack
EXTERN  unsigned char		*whstack[WHSLEN];	// WHILE stack
//EXTERN  STACK				gostack;		// GOSUB stack

EXTERN  U32					numarray[NUMSLEN];	// array for numstack
EXTERN  U8					oparray[OPSLEN];	// array for opstack

EXTERN  FOR_ENTRY			forstack[FORSLEN];	// stack for holding FOR statements
EXTERN  unsigned char		forindex;			// index for forstack
EXTERN  char				forflag;			// TRUE if parsing a FOR statement

EXTERN  unsigned char		whindex;			// index for while stack

EXTERN  unsigned char		*gostack[GOSLEN];	// stack for holding GOSUB statements
EXTERN  unsigned char		goindex;			// index for gostack

//EXTERN  IF_ENTRY			ifstack[IFSLEN];	// stack for holding IF statements
//EXTERN  unsigned char		ifindex;			// index for ifstack


//EXTERN  char				*numstack;		// numeric operand stack pointer
//EXTERN  char				*opstack;		// operator stack pointer
//EXTERN  char				*forstack;		// FOR stack pointer
//EXTERN  char				*whstack;		// WHILE stack pointer
//EXTERN  char				*gostack;		// GOSUB stack pointer

EXTERN  unsigned int		curline;		// line number that we are currently interpreting
EXTERN  unsigned char		*adrnxlin;		// address of the next line

EXTERN  U8					*strastg;		// dynamic string/array pool pointer
EXTERN  U8					*dynmemend;		// ending addr of dynamic memory pool

EXTERN  unsigned char		*fence;			// variable end fence in case of error in translation
EXTERN  unsigned char		*ipsave;		// interpretive pointer save for "BREAK"
EXTERN  unsigned char		*dataptr;		// pointer to data for READ statement
EXTERN  unsigned int		randomseed;		// random number seed (was random, conflicts with C's random() function
EXTERN  unsigned char		devnum;			// I/O device number

EXTERN  unsigned int		timereg;		// TIME register
EXTERN  unsigned int		timecmp;		// TIME compare register
EXTERN  unsigned char		timepre;		// TIME prescaler

EXTERN  unsigned int		ontimlin;		// ONTIME line number to goto
EXTERN  unsigned int		onirqlin;		// ONIRQ line number to goto
EXTERN  unsigned int		onpaclin;		// ONPACC line number to goto
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
EXTERN  unsigned int		onrxdatalin;	// ON RXDATA line number to goto
EXTERN  unsigned int		onacklin;		// ON ACK line number to goto
EXTERN  unsigned int		oneventlin;		// ON EVENT line number to goto

EXTERN  U16					erxorigin;		// ERXDATA
EXTERN  U16					erxdst;			// ERXDATA
EXTERN  U16					erxmsgid;		// ERXDATA
EXTERN  U16					erxselector;	// ERXDATA
EXTERN  U8					erxrssi;		// ERXDATA
EXTERN  U16					erxlen;			// ERXDATA
EXTERN  U8					eackstatus;		// EACK
EXTERN  U16					eackdst;		// EACK
EXTERN  U16					eackmsgid;		// EACK
#endif

#if defined(SKBASIC_EMBEDDED)
EXTERN  U16					ontim1lin;		//on TIM1 line number to goto
EXTERN  U16					ontim2lin;		//on TIM2 line number to goto
#endif

EXTERN  char				xonch;			// XON character for printer
EXTERN  char				xoffch;			// XOFF character for printer
EXTERN  unsigned int		scurline;		// saves CURLINE during interrupt processing
EXTERN  unsigned int		sadrnxln;		// saves ADRNXLIN during interrupt processing

EXTERN  U8					xdimflag;		// TRUE if executing xdim() function now

EXTERN  unsigned char		*inbuffs;		// pointer to start of input buffer
EXTERN  unsigned char		*tknbufs;		// pointer to start of token buffer

EXTERN	char				*eopstk;		// end of operator stack
EXTERN	char				*stops;			// start of operator stack
EXTERN	char				*enumstk;		// end of operand stack
EXTERN	char				*stnums;		// start of operand stack
EXTERN	char				*eforstk;		// end of FOR-NEXT stack
EXTERN	char				*stforstk;		// start of FOR-NEXT stack
EXTERN	char				*ewhstk;		// end of WHILE stack
EXTERN	char				*stwhstk;		// start of WHILE stack
EXTERN	char				*egostk;		// end of GOSUB stack
EXTERN	char				*stgostk;		// start of GOSUB stack;
EXTERN	char				*IOBaseV;		// address vector for I/O registers
EXTERN	char				*dname;			// place to put the variable name when doing a dump command

EXTERN  unsigned int		submax;			// ??
EXTERN  unsigned int		subcnt;			// ??
EXTERN  unsigned int		VarSize;		// used by the line editor; size of the variable table

											
/*
 *  Define the function pointer table of run-time routines.
 *
 *  Note that the order of pointers in this table is important.  It must track
 *  exactly the values assigned to the corresponding run-time tokens that will
 *  be pulled from the token buffer or read from the stored-program area.
 */
EXTERN FARPTR CONST RTFUNC		RTFuncs[]

#ifdef  AVR
PROGMEM
#endif

#ifdef  OWNER
=  {
	{rlet,		LET_STR},		// 0x01
	{rlet,		""},			// 0x02  placeholder for implied LET
	{rprint,	PRINT_STR},		// 0x03
	{rfor,		FOR_STR},		// 0x04
	{rnext,		NEXT_STR},		// 0x05
	{rtron,		TRON_STR},		// 0x06
	{rtroff,	TROFF_STR},		// 0x07
	{rpoke,		POKE_STR},		// 0x08
	{rdim,		DIM_STR},		// 0x09
	{rrem,		REM_STR},		// 0x0a
//	{rcall,		CALL_STR},		// 0x0b  was pacc
	{0,			" "},			// 0x0b
	{rdata,		DATA_STR},		// 0x0c
	{rread,		READ_STR},		// 0x0d
	{rrestor,	RESTORE_STR},	// 0x0e
	{0,			GOSUB_STR},		// 0x0f  special case, uses rpreparegosub().  See crun().
	{0,			"("},			// 0x10  OPARNTOK
	{0,			")"},			// 0x11  CPARNTOK
	{rgoto,		GOTO_STR},		// 0x12
	{ron,		ON_STR},		// 0x13
	{rreturn,	RETURN_STR},	// 0x14
	{rif,		IF_STR},		// 0x15
	{rinput,	INPUT_STR},		// 0x16
	{rstop,		STOP_STR},		// 0x17
	{rend,		END_STR},		// 0x18
	{rwhile,	WHILE_STR},		// 0x19
	{rendwh,	ENDWHILE_STR},	// 0x1a
	{reep,		EEP_STR},		// 0x1b
	{reep16,	EEP16_STR},		// 0x1c  was porta
	{reep32,	EEP32_STR},		// 0x1d  was portb
//	{rtimestr,	TIMESTR_STR},	// 0x1e	 moved to functions, not a statement any longer
//	{rdatestr,	DATESTR_STR},	// 0x1f  mvoed to functions, not a statement any longer
	{0,			" "},			// 0x1e  removed timestr statement, calls to localtime() lock up
	{0,			" "},			// 0x1f  removed datestr statement, calls to localtime() lock up
	{r_and,		AND_STR},		// 0x20		ANDTOK
	{rorv,		OR_STR},		// 0x21		ORTOK
	{reor,		EOR_STR},		// 0x22		EORTOK
	{rinbyte,	INBYTE_STR},	// 0x23
//	{rtime,		TIME_STR},		// 0x24  moved to commands, not a statement any longer
	{0,			" "},			// 0x24  removed time statement, calls to localtime() lock up
	{rontime,	ONTIME_STR},	// 0x25
	{ronirq,	ONIRQ_STR},		// 0x26
	{rreti,		RETI_STR},		// 0x27
	{ronpacc,	ONPACC_STR},	// 0x28
	{rsleep,	SLEEP_STR},		// 0x29
	{0,			" "},			// 0x2a  was rrtime, not used by KLBasic
	{rstrcat,	STRCAT_STR},	// 0x2b
	{rstrind,	STRIND_STR},	// 0x2c
	{0,			" "},			// 0x2d
	{0,			" "},			// 0x2e
	{0,			" "},			// 0x2f
	{rlt,		LT_STR},		// 0x30		LTTOK
	{rgt,		GT_STR},		// 0x31		GTTOK
	{rlteq,		LTEQ_STR},		// 0x32		LTEQTOK
	{rgteq,		GTEQ_STR},		// 0x33		GTEQTOK
	{req,		EQ_STR},		// 0x34		EQTOK
	{rnoteq,	NOTEQ_STR},		// 0x35		NOTEQTOK
	{0,			" "},			// 0x36		FUNCTFLG
	{0,			TO_STR},		// 0x37		TOTOK
	{0,			THEN_STR},		// 0x38		THENTOK
	{0,			ELSE_STR},		// 0x39		ELSETOK
	{0,			STEP_STR},		// 0x3a		STEPTOK
	{0,			" "},			// 0x3b
	{0,			" "},			// 0x3c
	{0,			" "},			// 0x3d
	{0,			" "},			// 0x3e
	{0,			" "},			// 0x3f
	{rplus,		PLUS_STR},		// 0x40		PLUSTOK
	{rminus,	MINUS_STR},		// 0x41		MINUSTOK
	{0,			STRPLUS_STR},	// 0x42		SPLUSTOK
	{rprint,	QPRINT_STR},	// 0x43		quick print
	{rrem,		QREM_STR},		// 0x44		quick REM (single-char remark)
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	{rsksreg,	SKSREG_STR},	// 0x45
	{rsksend,	SKSEND_STR},	// 0x46
	{rskflash,	SKFLASH_STR},	// 0x47
	{rskbc,		SKBC_STR},		// 0x48
	{rsksync,	SKSYNC_STR},	// 0x49
	{rskinq,	SKINQ_STR},		// 0x4a
	{rskpair,	SKPAIR_STR},	// 0x4b
	{rskunpair,	SKUNPAIR_STR},	// 0x4c
	{rsknow,	SKNOW_STR},		// 0x4d
	{rsksleep,	SKSLEEP_STR},	// 0x4e
	{rsksetps,	SKSETPS_STR},	// 0x4f
#elif defined(SKBASIC_EMBEDDED)
	{0,			" "},			// 0x45
	{0,			" "},			// 0x46
	{0,			" "},			// 0x47
	{0,			" "},			// 0x48
	{0,			" "},			// 0x49
	{0,			" "},			// 0x4a
	{0,			" "},			// 0x4b
	{0,			" "},			// 0x4c
	{0,			" "},			// 0x4d
	{0,			" "},			// 0x4e
	{0,			" "},			// 0x4f
#else
	{0,			" "},			// 0x45
	{0,			" "},			// 0x46
	{0,			" "},			// 0x47
	{0,			" "},			// 0x48
	{0,			" "},			// 0x49
	{0,			" "},			// 0x4a
	{0,			" "},			// 0x4b
	{0,			" "},			// 0x4c
	{0,			" "},			// 0x4d
	{0,			" "},			// 0x4e
	{0,			" "},			// 0x4f
#endif
	{rmult,		MULT_STR},		// 0x50		MULTTOK
	{rdiv,		DIV_STR},		// 0x51		DIVTOK
	{rmod,		MOD_STR},		// 0x52		MODTOK
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	{rskclrtbl,	SKCLRTBL_STR},	// 0x53
	{rskclrcache,	SKCLRCACHE_STR},// 0x54
	{rsklkup,	SKLKUP_STR},	// 0x55
	{rskrevlkup,SKREVLKUP_STR},	// 0x56
	{rsksetname,SKSETNAME_STR},	// 0x57
	{rsksetkey,	SKSETKEY_STR},	// 0x58
	{0,			""},			// 0x59
	{0,			RXDATA_STR},	// 0x5a
	{0,			ACK_STR},		// 0x5b
	{0,			EVENT_STR},		// 0x5c
	{rwait_set,	WAIT_STR},		// 0x5d
	{rwait_jne,	""},			// 0x5e
#elif defined(SKBASIC_EMBEDDED)
	{0,			" "},			// 0x53
	{0,			" "},			// 0x54
	{0,			" "},			// 0x55
	{0,			" "},			// 0x56
	{0,			" "},			// 0x57
	{0,			" "},			// 0x58
	{0,			" "},			// 0x59
	{0,			TIM1_STR},		// 0x5a
	{0,			TIM2_STR},		// 0x5b
	{0,			" "},			// 0x5c
	{rwait_set,	WAIT_STR},		// 0x5d
	{rwait_jne,	""},			// 0x5e
#else
	{0,			" "},			// 0x53
	{0,			" "},			// 0x54
	{0,			" "},			// 0x55
	{0,			" "},			// 0x56
	{0,			" "},			// 0x57
	{0,			" "},			// 0x58
	{0,			" "},			// 0x59
	{0,			" "},			// 0x5a
	{0,			" "},			// 0x5b
	{0,			" "},			// 0x5c
	{0,			" "},			// 0x5d
	{0,			" "},			// 0x5e
#endif
	{0,			" "},			// 0x5f
	{rpwr,		PWR_STR},		// 0x60		PWRTOK
	{rlabel,	LABEL_STR},		// 0x61		LABELTOK
	{0,			" "},			// 0x62
	{0,			" "},			// 0x63
	{0,			" "},			// 0x64
	{0,			" "},			// 0x65
	{0,			" "},			// 0x66
	{0,			" "},			// 0x67
	{0,			" "},			// 0x68
	{0,			" "},			// 0x69
	{0,			" "},			// 0x6a
	{0,			" "},			// 0x6b
	{0,			" "},			// 0x6c
	{0,			" "},			// 0x6d
	{0,			" "},			// 0x6e
	{0,			" "},			// 0x6f
	{rindir,	INDIR_STR},		// 0x70		INDIRTOK
	{rnot,		NOT_STR},		// 0x71		NOTTOK
	{rneg,		NEG_STR},		// 0x72		NEGTOK
	{rindir32,	INDIR32_STR},	// 0x73		INDIR32TOK
	{rindir16,	INDIR16_STR},	// 0x74		INDIR16TOK
	{0,			" "},			// 0x75
	{0,			" "},			// 0x76
	{0,			" "},			// 0x77
	{0,			PNUM_STR},		// 0x78		PNUMTOK
	{0,			EQUAL_STR},		// 0x79		EQUALTOK
	{rnop,		MIDEOL_STR},	// 0x7a		MEOLTOK
	{0,			SEMI_STR},		// 0x7b		SEMITOK
	{0,			COMMA_STR},		// 0x7c		COMMATOK
	{0,			" "},			// 0x7d		EOLTOK
	{0,			" "},			// 0x7e		SSCNTOK
	{0,			" "}			// 0x7f		MSCNTOK
}
#endif						// #ifdef  OWNER
;





											
/*
 *
           $if      * > $9E
           $fatal   "Ran out of Page 0 RAM"
          $endif
 *
 *
 */







/*
         ORG    $009E
*
CONSTAT:  RMB    3        ; GET CONSOLE STATUS FOR BREAK ROUTINE.
INCONNE:  RMB    3        ; GET BYTE DIRECTLY FROM CONSOLE FOR BREAK ROUTINE.
*
         ORG    $00A4
*
INTABLE:  RMB    16       ; RESERVE SPACE FOR 8 DIFFERENT INPUT ROUTINES.
OUTABLE:  RMB    16       ; RESERVE SPACE FOR 8 DIFFERENT OUTPUT ROUTINES.
*
*
*
*/

/*

         ORG    $00C4    ; START OF RAM INTERRUPT VECTORS.
*
RAMVECTS: EQU    *
SCISS:    RMB    3        ; SCI SERIAL SYSTEM.
SPITC:    RMB    3        ; SPI TRANSFER COMPLETE.
PACCIE:   RMB    3        ; PULSE ACCUMULATOR INPUT EDGE.
PACCOVF:  RMB    3        ; PULSE ACCUMULATOR OVERFLOW.
TIMEROVF: RMB    3        ; TIMER OVERFLOW.
TOC5:     RMB    3        ; TIMER OUTPUT COMPARE 5.
TOC4:     RMB    3        ; TIMER OUTPUT COMPARE 4.
TOC3:     RMB    3        ; TIMER OUTPUT COMPARE 3.
TOC2:     RMB    3        ; TIMER OUTPUT COMPARE 2.
TOC1:     RMB    3        ; TIMER OUTPUT COMPARE 1.
TIC3:     RMB    3        ; TIMER INPUT CAPTURE 3.
TIC2:     RMB    3        ; TIMER INPUT CAPTURE 2.
TIC1:     RMB    3        ; TIMER INPUT CAPTURE 1.
REALTIMI: RMB    3        ; REAL TIME INTERRUPT.
IRQI:     RMB    3        ; IRQ INTERRUPT.
XIRQ:     RMB    3        ; XIRQ INTERRUPT.
SWII:     RMB    3        ; SOFTWARE INTERRUPT.
ILLOP:    RMB    3        ; ILLEGAL OPCODE TRAP.
COP:      RMB    3        ; WATCH DOG TIMER FAIL.
CMF:      RMB    3        ; CLOCK MONITOR FAIL.
*
*
*/

#ifdef __cplusplus
}
#endif

#endif

