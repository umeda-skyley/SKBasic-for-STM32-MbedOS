/*
 *  command1      part of Basic11 compiler
 */

#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#ifdef	AVR
#include  <avr/pgmspace.h>
#endif


#include  "basictime.h"		// needed for localtime
#include  "defines.h"
#include  "funcs.h"

/*
 *  Local functions
 */
static void				prtvarname(unsigned int  vaddr);
static void				prtconname(unsigned char  size);
static void				prttoeol(void);
static void				liarray(void);
static U8				chgstrtotime(char  *s, TM  *pt);
static U8				chgstrtodate(char  *s, TM  *pt);

static void				showtmvalues(struct tm  *pt);

/*
typedef  struct
{
	void				(*funcptr)(void);
	char				name[10];
}  RTFUNC;
*/


/*
 *  Local variables
 */
static  unsigned char			*tokptr;		// token pointer (used by LIST command)




/*
 *  Define the function pointer table of run-time routines.
 *
 *  Note that the order of pointers in this table is important.  It must track
 *  exactly the values assigned to the corresponding run-time tokens that will
 *  be pulled from the token buffer or read from the stored-program area.
 */
/*
RTFUNC		RTFuncs[] =
{
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
	{rpacc,		PACC_STR},		// 0x0b
	{rdata,		DATA_STR},		// 0x0c
	{rread,		READ_STR},		// 0x0d
	{rrestor,	RESTORE_STR},	// 0x0e
	{rgosub,	GOSUB_STR},		// 0x0f
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
	{rporta,	PORTA_STR},		// 0x1c
	{rportb,	PORTB_STR},		// 0x1d
	{rportc,	PORTC_STR},		// 0x1e
	{rportd,	PORTD_STR},		// 0x1f
	{0,			AND_STR},		// 0x20		ANDTOK
	{0,			OR_STR},		// 0x21		ORTOK
	{0,			EOR_STR},		// 0x22		EORTOK
	{rinbyte,	INBYTE_STR},	// 0x23
	{rtime,		TIME_STR},		// 0x24
	{rontime,	ONTIME_STR},	// 0x25
	{ronirq,	ONIRQ_STR},		// 0x26
	{rreti,		RETI_STR},		// 0x27
	{ronpacc,	ONPACC_STR},	// 0x28
	{rsleep,	SLEEP_STR},		// 0x29
	{rrtime,	RTIME_STR},		// 0x2a
	{0,			" "},			// 0x2b
	{0,			" "},			// 0x2c
	{0,			" "},			// 0x2d
	{0,			" "},			// 0x2e
	{0,			" "},			// 0x2f
	{0,			LT_STR},		// 0x30		LTTOK
	{0,			GT_STR},		// 0x31		GTTOK
	{0,			LTEQ_STR},		// 0x32		LTEQTOK
	{0,			GTEQ_STR},		// 0x33		GTEQTOK
	{0,			EQ_STR},		// 0x34		EQTOK
	{0,			NOTEQ_STR},		// 0x35		NOTEQTOK
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
	{0,			PLUS_STR},		// 0x40		PLUSTOK
	{0,			MINUS_STR},		// 0x41		MINUSTOK
	{0,			STRPLUS_STR},	// 0x42		SPLUSTOK
	{rprint,	QPRINT_STR},	// 0x43		quick print
	{0,			" "},			// 0x44
	{0,			" "},			// 0x45
	{0,			" "},			// 0x45
	{0,			" "},			// 0x47
	{0,			" "},			// 0x48
	{0,			" "},			// 0x49
	{0,			" "},			// 0x4a
	{0,			" "},			// 0x4b
	{0,			" "},			// 0x4c
	{0,			" "},			// 0x4d
	{0,			" "},			// 0x4e
	{0,			" "},			// 0x4f
	{0,			MULT_STR},		// 0x50		MULTTOK
	{0,			DIV_STR},		// 0x51		DIVTOK
	{0,			MOD_STR},		// 0x52		MODTOK
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
	{0,			" "},			// 0x5f
	{0,			PWR_STR},		// 0x60		PWRTOK
	{0,			" "},			// 0x61
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
	{0,			INDIR_STR},		// 0x70		INDIRTOK
	{0,			NOT_STR},		// 0x71		NOTTOK
	{0,			NEG_STR},		// 0x72		NEGTOK
	{0,			" "},			// 0x73
	{0,			" "},			// 0x74
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
};

*/





/*
 *  chckcmds      process a command token
 */
int  chckcmds(void)
{
	if      (match(LIST_STR))  clist();		// list all or part of the program
	else if (match(RUN_STR))   crun();		// run the program
	else if (match(NEW_STR))   cnew();		// erase the program
	else if (match(CONT_STR))  ccont();		// continue the program
	else if (match(CLEAR_STR)) cclear();	// clear variables
	else if (match(FREE_STR))  bsc_cmd_free();	// show amount of free memory
	else if (match(LOAD_STR))  cload();		// load RAM from non-volatile memory
	else if (match(SAVE_STR))  csave();		// save RAM to non-volatile memory
#if !defined(SKBASIC_IMPL) && !defined(SKBASIC_CMD) && !defined(SKBASIC_EMBEDDED)
	else if (match(AUTO_STR))  cautost();	// mark program in EEPROM for autostart
#endif
//	else if (match(TIME_STR))  cmd_time();	// show/set system time
//	else if (match(DATE_STR))  cdate();		// show/set system date
//	else if (match(NOAUTO_STR))  cnoauto();	// remove autostart mark in EEPROM
//#ifdef  AVR
//	else if (match(FLSAVE_STR))  cflsave();	// save to flash memory
//	else if (match(FLLOAD_STR))  cflload();	// read from flash memory
//#endif
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	else if (match(FORMAT_STR))   cformat();
	else if (match(FILES_STR))    cfiles();
	else if (match(DELETE_STR))   cdelete();
	else if (match(SKINFO_STR))   cskinfo();
	else if (match(SKINFO_STR))   cskinfo();
	else if (match(SKRESET_STR))  cskreset();
	else if (match(SKSAVE_STR))   csksave();
	else if (match(SKLOAD_STR))   cskload();
	else if (match(SKRFCTRL_STR)) cskrfctrl();
	else if (match(SKRFREG_STR))  cskrfreg();
	else if (match(SKVER_STR))    cskver();
	else if (match(SKPOW_STR))    cskpow();
	else if (match(SKTABLE_STR))  csktable();
#endif
#if	defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
	else if (match(EXIT_STR))     cexit();
#endif

	else    return(0);
	return (1);
}
/* 
 * 
 * CHCKCMDS: EQU    *
 *           JSR    GETCHR        ; GET FIRST CHAR FROM THE INPUT BUFFER.
 *           CMPA   #EOL          ; IS IT AN EOL?
 *           BNE    CHKCMDS1      ; NO. GO CHECK FOR COMMANDS.
 * CHKCMDS5: LDD    #0            ; YES. JUST RETURN.
 *           RTS
 * CHKCMDS1: LDX    #CMDTBL       ; POINT TO COMMAND TABLE.
 * CHKCMDS2: JSR    STREQ         ; GO CHECK FOR A COMMAND.
 *           BCS    CHKCMDS3      ; IF WE FOUND ONE GO EXECUTE IT.
 * CHKCMDS4: INX                  ; ADVANCE POINTER TO NEXT CHAR IN TABLE ENTRY.
 *           LDAA   0,X           ; GET THE CHAR. ARE WE AT THE END OF THIS ENTRY?
 *           BNE    CHKCMDS4      ; NO. KEEP GOING TILL WE ARE PAST IT.
 *           INX                  ; BYPASS END OF COMMAND MARKER & EXECUTION ADDR.
 *           INX
 *           INX
 *           TST    0,X           ; ARE WE AT THE END OF THE TABLE?
 *           BNE    CHKCMDS2      ; NO. GO CHECK THE NEXT TABLE ENTRY.
 *           BRA    CHKCMDS5      ; YES. RETURN W/ ENTRY NOT FOUND INDICATION.
 * CHKCMDS3: LDX    1,X           ; GET ADDRESS OF COMMAND.
 *           JSR    0,X           ; GO DO IT.
 *           LDD    #1            ; SHOW WE EXECUTED A COMMAND.
 *           RTS                  ; RETURN.
 * *
 * *
 * CMDTBL:   EQU    *
 *           FCC    "LIST"
 *           FCB    0
 *           FDB    CLIST
 *           FCC    "RUN"
 *           FCB    0
 *           FDB    CRUN
 *           FCC    "NEW"
 *           FCB    0
 *           FDB    CNEW
 *           FCC    "CONT"
 *           FCB    0
 *           FDB    CCONT
 *           FCC    "CLEAR"
 *           FCB    0
 *           FDB    CCLEAR
 *           FCC    "ESAVE"
 *           FCB    0
 *           FDB    CESAVE
 *           FCC    "ELOAD"
 *           FCB    0
 *           FDB    CELOAD
 *           FCC    "LLIST"
 *           FCB    0
 *           FDB    CLLIST
 *           FCC    "AUTOST"
 *           FCB    0
 *           FDB    CAUTOST
 *           FCC    "NOAUTO"
 *           FCB    0
 *           FDB    CNOAUTO
 *           FCC       "FREE"
 *           FCB       0
 *           FDB       CFREE
 *           FCB    0             ;END OF TABLE MARKER.
 */          


/*
 *  cmd_time      process the TIME command
 */
/*
void  cmd_time(void)
{
	time_t				_time;
	struct tm			*pt;
	char				systime[50];
	U8					res;
		
	targetgetsystime(&_time);			// get current calendar time (seconds)
	pt = localtime(&_time);				// convert to broken time starting at 1900 in tm struct
	tmtotimestr(pt, systime);			// convert to time string
	pl_P(PSTR("\n\rTime is "));
	pl(systime);
	pl_P(PSTR("\n\rEnter new time: "));
	getline();
	if (*ibufptr == EOL)  return;
	res = chgstrtotime(ibufptr, pt);		// translate, if possible
	if (res == 0)
	{
		pl_P(PSTR("\n\rBad time format ignored; time not changed!"));
	}
	else
	{
		_time = mktime(pt);
		targetsetsystime(_time);
	}
	return;
}
*/

/*
 *  cdate      process the DATE command
 */
/*
void  cdate(void)
{
	time_t				_time;
	struct tm			*pt;
	char				sysdate[50];
	U8					res;
		
	targetgetsysdate(&_time);			// get current calendar time (in seconds)
	pt = localtime(&_time);				// convert to broken time in tm struct
	tmtodatestr(pt, sysdate);			// convert to date string
	pl_P(PSTR("\n\rDate is "));
	pl(sysdate);
	pl_P(PSTR("\n\rEnter new date: "));
	getline();
	if (*ibufptr == EOL)  return;
	res = chgstrtodate(ibufptr, pt);	// translate, if possible
	if (res == 0)
	{
		pl_P(PSTR("\n\rBad date format ignored; date not changed!"));
	}
	else
	{
		_time = mktime(pt);
		tergetsetsysdate(&_time);
	}
	return;
}
*/



void  tmtotimestr(TM  *pt, char  *s)
{
//	sprintf(s, "%02d:%02d:%02d", pt->tm_hour, pt->tm_min, pt->tm_sec);
	SPRINTF_U2DEC(pt->tm_hour, s);
	*s++ = ':';
	SPRINTF_U2DEC(pt->tm_min, s);
	*s++ = ':';
	SPRINTF_U2DEC(pt->tm_sec, s);
	*s = 0;
}



void  tmtodatestr(TM  *pt, char  *s)
{
//	sprintf(s, "%02d %3s %02d", pt->tm_mday, "mon", pt->tm_year % 100);
	SPRINTF_U2DEC(pt->tm_mday, s);
	*s++ = ' ';
	strcpy(s, "mon");
	s = s + 3;
	*s++ = ' ';
	SPRINTF_U2DEC(pt->tm_year%100, s);
	*s = 0;
}



static U8  chgstrtotime(char  *s, TM  *pt)
{
	unsigned char		hour;
	unsigned char		min;
	unsigned char		sec;

	if ((inbuff[2] != ':') || (inbuff[5] != ':') || (inbuff[8] != EOL))  return  0;		// isn't even close, bomb out
	SSCANF_CHR2DEC(&s[0], hour);
	SSCANF_CHR2DEC(&s[3], min);
	SSCANF_CHR2DEC(&s[6], sec);
//	sscanf(s, "%hu:%hu:%hu", &hour, &min, &sec);
	if ((hour>23) || (min>59) || (sec>59))  return  0;
	pt->tm_hour = hour;
	pt->tm_min = min;
	pt->tm_sec = sec;
	return  1;
}
 


static U8  chgstrtodate(char  *s, TM  *pt)
{
	return 0;
}



static void  showtmvalues(TM  *pt)
{
	nl();
	outdeci(pt->tm_year);
	outbyte(' ');
	outdeci(pt->tm_mon);
	outbyte(' ');
	outdeci(pt->tm_mday);
	outbyte(' ');
	outdeci(pt->tm_hour);
	outbyte(' ');
	outdeci(pt->tm_min);
	outbyte(' ');
	outdeci(pt->tm_sec);
	nl();
}


/*
 *  clist      process the LIST command
 */

void  clistprog(int firstlin, int lastlin)
{
	unsigned char			*intptr;
	unsigned char			token;
	unsigned int			ln;
	if (firstlin > lastlin) return;			// if range is inverted, forget it
	intptr = findline(firstlin);
	tokptr = intptr;

/*
 *           JSR    GETCHR
 *           JSR    NUMERIC
 *           BCC    CLIST2
 *           JSR    GETLINUM
 *           STD    FIRSTLIN
 *           JSR    GETCHR
 *           CMPA   #'-'
 *           BEQ    CLIST3
 *           LDD    FIRSTLIN
 *           STD    LASTLIN
 *           CPD    HILINE
 *           BLS    CLIST4
 *           RTS
 * CLIST3:   JSR    INCIBP
 *           JSR    GETLINUM
 *           CPD    HILINE
 *           BLS    CLIST13
 *           LDD    HILINE
 * CLIST13:  STD    LASTLIN
 *           BRA    CLIST4
 * CLIST2:   CMPA   #EOL
 *           BEQ    CLIST14
 *           RTS
 * CLIST14:  LDX    BASBEG
 *           LDD    0,X
 *           STD    FIRSTLIN
 *           LDD    HILINE
 *           STD    LASTLIN
 * CLIST4:   LDD    FIRSTLIN
 *           CPD    LASTLIN
 *           BLS    CLIST5
 *           RTS
 * CLIST5:   LDD    FIRSTLIN
 *           JSR    FINDLINE
 *           STX    TOKPTR
 *           LDD    LASTLIN
 *           JSR    FINDLINE
 *           LDD    0,X
 *           CPD    LASTLIN
 *           BNE    CLIST12
 *           LDAB   2,X
 *           ABX
 * CLIST12:  STX    LASTLIN
 * *
 */

	while (1)
	{
		if (tokptr >= basend)  return;	// if ran out of program, leave now

		ln = getU16((U16 *)tokptr);			// get line number
		tokptr = tokptr + sizeof(U16);	// move past 16-bit line number
		if (ln > lastlin)  return;		// if reached the end of the range, leave now

		outdeci(ln);					// show the line number
		tokptr++;						// step past length byte
		while (*tokptr != EOLTOK)
		{
			token = *tokptr;			// check the next token
			if (token >= 0x80)			// if variable token (magic number!)
			{
				lvarcon();
			}
			else
			{
				lkeyword();
			}
		}
		nl();
		++tokptr;
	}
}

void  clist(void)
{
	unsigned char			*intptr;
	unsigned char			token;
	unsigned int			ln;
	unsigned int			n;
	char					portname[MAX_VAR_NAME_LEN+1];
	char					*varptr;
	unsigned char			progbuff[TBUFLEN];
	//unsigned char			j;
	unsigned char			*tmpptr;
	U32						t32;

	skipspcs();								// skip any spaces after "LIST"

/*
 * CLIST:    EQU    *
 *           JSR    NL2
 *           LDD    BASBEG
 *           CPD    BASEND
 *           BNE    CLIST1
 *           RTS
 * CLIST1:   JSR    SKIPSPCS
 */

	if  (isalpha(*ibufptr))				// if listing system elements of some kind...
	{
		if (match("ports"))				// if listing ports...
		{
			nl();
			pl("List of ports --");
			for (n=0; ; n++)			// walk through the port table
			{
				if ((n % (79/MAX_VAR_NAME_LEN)) == 0)  nl();		// magic number!  guess at length of line on console device
				targetgetportname(n, portname);
				if (strlen(portname) == 0) break;
				if (strlen(portname) >= MAX_VAR_NAME_LEN)  portname[MAX_VAR_NAME_LEN] = 0;
				else					     while (strlen(portname) < MAX_VAR_NAME_LEN)  strcat(portname, " ");
				pl(portname);
			}
			nl();
		}
		else if (match("vars"))
		{
			n = 0;						// counter for keeping it pretty
			nl();
			varptr = (char *)varbegin;			// start at beginning of variable table
			pl("List of variables --");
			while (*varptr)				// for all variables in list...
			{
				if ((n % (79/(MAX_VAR_NAME_LEN+2))) == 0)  nl();		// magic number!  guess at length of line on console device
				if ((unsigned char)(*varptr)==IVARTOK)	// if simple integer variable...
				{
					varptr++;			// move to start of name
					pl(varptr);			// show the name
					ln = MAX_VAR_NAME_LEN + 2 - strlen(varptr);		// need to pretty up the display (+2 to allow for parens in array name)
					while (ln--)  pl(" ");
					pl("    ");
					varptr = varptr + ISIZ + MAX_VAR_NAME_LEN;	// magic number!
				}
				else if ((unsigned char)(*varptr)==IAVARTOK)	// if array...
				{
					varptr++;			// move to start of name
					pl(varptr);			// show the name
					pl("(");			// show this is an array
					pl(")");
					ln = MAX_VAR_NAME_LEN + 2 - strlen(varptr);		// need to pretty up the display (+2 to allow for parens in array name)
					while (ln--)  pl(" ");
					pl("  ");
					varptr = varptr + ISIZ + MAX_VAR_NAME_LEN;	// magic number!
				}
				else if ((unsigned char)(*varptr)==SVARTOK)	// if string...
				{
					varptr++;			// move to start of name
					pl(varptr);			// show the name
					pl("$");			// show this is an array
					ln = MAX_VAR_NAME_LEN + 2 - strlen(varptr);		// need to pretty up the display (+2 to allow for parens in array name)
					while (ln--)  pl(" ");
					pl("  ");
					varptr = varptr + SSIZ + MAX_VAR_NAME_LEN;	// magic number!
				}
				else					// unknown token...
				{
					break;
				}
				n++;
			}
		}
#if	!defined(SKBASIC_IMPL) && !defined(SKBASIC_CMD) && !defined(SKBASIC_EMBEDDED)
		else if (match("files"))
		{
			tmpptr = tokptr;					// save global pointer
			for (n=0; n<NUM_FLASH_FILES; n++)					// need to use NUM_FLASH_FILES
			{
				pl_P(PSTR("fl"));
				outdeci(n);
				pl_P(PSTR(": "));
				t32 = targetgetflashprogramaddr((U8)n);	// try to find program in flash file
				if (t32)						// if found it...
				{
					targetcopyfromflash(progbuff, t32, TBUFLEN);	// read first line from flash
					tokptr = progbuff;			// switch token pointer to the RAM copy of flash
					ln = getU16((U16 *)tokptr);		// get line number, if any
					tokptr = tokptr + sizeof(U16);	// move past 16-bit line number
					if (ln == 0xffff)			// if no program...
					{
						pl_P(PSTR(" No program."));
					}
					else
					{
						outbyte(' ');			// make it pretty
						outdeci(ln);			// show the line number
						tokptr++;				// step past length byte
						while (*tokptr != EOLTOK)
						{
							token = *tokptr;	// check the next token
							if (token >= 0x80)	// if variable token (magic number!)
							{
								lvarcon();
							}
							else
							{
								lkeyword();
							}
						}
					}
				}

				else							// sorry, no such flash file
				{
					pl_P(PSTR("No such flash file."));
				}
				nl();
			}
			tokptr = tmpptr;					// restore global pointer
		}
#endif
		else							// got me scratching...
		{
			pl_P(PSTR("Unknown LIST topic.  Try 'list ports' or 'list vars' or 'list files'."));
		}
		nl();
		return;							// done listing elements, need to leave now
	}
	else if (isdigit(*ibufptr))			// if there is a line number specified...
	{
		if (basbeg == basend) return;	// if basic buffer empty, return
		firstlin = getlinum();			// we are listing lines in the program
		skipspcs();						// skip any spaces
		if (*ibufptr == '-')			// if of form "line# - line#...
		{
			ibufptr++;					// same as incibp()
			skipspcs();					// skip any spaces
			if (isdigit(*ibufptr))		// if second line# specified...
			{
				lastlin = getlinum();	// get ending line number
			}
			else						// no second line#...
			{
				lastlin = hiline;		// print from first line to end of program
			}
		}
		else							// no second number...
		{
			lastlin = firstlin;			// use firstlin as lastlin
		}
	}
	else									// not a numeric argument and not a system element, assume list all program
	{
		if (basbeg == basend) return;		// if basic buffer empty, return
		intptr = basbeg;
		lastlin = hiline;
		firstlin = getU16((U16 *)intptr);			// get first line number
		intptr = intptr + sizeof(U16)-1;	// stop at last cell in line number
	}
	nl();								// make it pretty
	clistprog(firstlin, lastlin);
	return;
}






/*
 * CLIST6:   LDD    TOKPTR
 *           CPD    LASTLIN
 *           BNE    CLIST7
 *           RTS
 * CLIST7:   LDX    TOKPTR
 *           LDD    0,X
 *           INX
 *           INX
 *           INX
 *           STX    TOKPTR
 *           JSR    OUTDECI
 * CLIST8:   LDX    TOKPTR
 *           LDAA   0,X
 *           CMPA   #EOLTOK
 *           BEQ    CLIST9
 *           TSTA
 *           BMI    CLIST10
 *           JSR    LKEYWORD
 *           BRA    CLIST8
 * CLIST10:  JSR    LVARCON
 *           BRA    CLIST8 
 * CLIST9:   JSR    NL
 *           LDX    TOKPTR
 *           INX
 *           STX    TOKPTR
 *           BRA    CLIST6
 */

/***** lvarcon() *****/
void  lvarcon(void)
{
	unsigned char				tok;

	tok = *tokptr;						// check this token
	if (tok <= 0x88)					// magic number!  (this doesn't look right)
	{
		if      (tok == FVARTOK) lfvar();
		else if (tok == SVARTOK) lsvar();
		else if (tok == IVARTOK) livar();
		else if (tok == PVARTOK) lpvar();	// list a port variable name
		else 
		{
			errcode = ILTOKERR;
			return;
		}
	}
	else if (tok <= 0xA8)				// magic number!
	{
		if      (tok == FCONTOK) lfcon();
		else if (tok == SCONTOK) lscon();
		else if (tok == LCONTOK) llcon();
		else if (tok == ICONTOK) licon();
		else if (tok == IAVARTOK)  liarray();
		else 
		{
			errcode = ILTOKERR;
			return;
		}
	}
	else 
	{ 
		errcode = ILTOKERR;
		return;
	}
}

/*
 * LVARCON:  EQU    *
 *           LDX    TOKPTR
 *           LDAA   0,X
 *           ANDA   #$EF          ; MASK OFF ARRAY INDICATOR IF PRESENT.
 *           LDX    #VCTOKTBL
 * LVARCON1: CMPA   0,X
 *           BEQ    LVARCON2
 *           INX
 *           INX
 *           INX
 *           TST    0,X
 *           BNE    LVARCON1
 *           LDAA   #ILTOKERR
 *           JMP    RPTERR
 * LVARCON2: LDX    1,X
 *           JSR    0,X
 *           RTS
 * *
 * *
 * VCTOKTBL: EQU    *
 *           FCB    IVARTOK
 *           FDB    LIVAR
 *           FCB    SCONTOK
 *           FDB    LSCON
 *           FCB    LCONTOK
 *           FDB    LLCON
 *           FCB    ICONTOK
 *           FDB    LICON
 *           FCB    0                     ; END OF TABLE MARKER.
 */



/*
 *  lpvar      display name of port based on token
 */
void  lpvar(void)
{
	char				namebuf[MAX_VAR_NAME_LEN+1];
	U16					index;

	tokptr++;						// step past port var token
	index = getU16((U16 *)tokptr);			// get index into port table
	tokptr = tokptr + sizeof(U16);	// step past index
	targetgetportname(index, namebuf);	// have routine in target code copy name to buffer
	pl(namebuf);					// display name
}



/*
 *  lfvar      display name of FP variable based on token
 */
void  lfvar(void)
{
	unsigned int				offset;

	tokptr++;						// step past FP var token
	offset = getU16((U16 *)tokptr);		// get offset from start of variables
	tokptr = tokptr + sizeof(U16);	// step past offset
	offset = offset + (unsigned int)varbegin;		// calc address of variable name
	prtvarname(offset);				// print the variable name
}



/*
 *  lsvar      display name of a string variable
 */
void  lsvar(void)
{
	unsigned int				offset;

	tokptr++;						// step past FP var token
	offset = getU16((U16 *)tokptr);		// get offset from start of variables
	tokptr = tokptr + sizeof(U16);	// step past offset
	offset = offset + (unsigned int)varbegin;		// calc address of variable name
	prtvarname(offset);				// print the variable name
	outbyte('$');					// append string qualifier
}



/*
 *  livar      list integer variable name
 */
void  livar(void)
{
	lfvar();						// print the basic variable name
//	outbyte('%');					// append integer qualifier
}



/*
 *  liarray      list integer array name and subscript
 */
static void  liarray(void)
{
	U8						token;
	U8						pcount;

	lfvar();						// print the basic variable name
	pcount = 0;
	while (*tokptr != EOLTOK)
	{
		if (pcount == OPARNTOK)  pcount++;	// count open parens
		token = *tokptr;			// check the next token
		if (token >= 0x80)			// if variable token (magic number!)
		{
			lvarcon();
		}
		else
		{
			lkeyword();
		}
		if (pcount == CPARNTOK)		// if just did a close paren
		{
			pcount--;				// count this one
			if (pcount == 0)  return;	// if matched, all done
		}
	}
}


/*
 * LIVAR:    EQU    *
 *           LDX    TOKPTR
 *           INX
 *           LDD    0,X
 *           ADDD   VARBEGIN
 *           INX
 *           INX
 *           STX    TOKPTR
 *           XGDX
 * LIVAR2:   LDAA   1,X
 *           JSR    OUTBYTE
 *           LDAA   2,X
 *           BEQ    LIVAR1
 *           JSR    OUTBYTE
 * LIVAR1:   RTS
 */

/*
 *  prtvarname      print variable name (used only by CLIST)
 */
static void  prtvarname(unsigned int  vaddr)
{
	unsigned char				*ptr;
	char						n;

	ptr = (unsigned char *)vaddr;	// pretend this is a pointer
	ptr++;							// step past varible type byte
	for (n=0; n<MAX_VAR_NAME_LEN; n++)
	{
		if (*ptr)  outbyte(*ptr);
		ptr++;
	}
//	outbyte(*ptr);					// output letter in variable name
//	ptr++;							// move to number, if any
//	if (*ptr)						// if number is not zero...
//	{
//		outbyte(*ptr);				// print that too
//	}
}




/*
 *  lfcon    list FP constant
 */
void  lfcon(void)
{
	prtconname(FSIZ);
}

/*
 * LFCON:    EQU    *
 *           LDD    TOKPTR
 *           ADDD   #FSIZ+1
 * LFCON2:   XGDX
 *           LDAB   0,X
 *           INX
 * LFCON1:   LDAA   0,X
 *           JSR    OUTBYTE
 *           INX
 *           DECB
 *           BNE    LFCON1
 *           STX    TOKPTR
 *           RTS
 *
 */

/***** licon() *****/
void  licon(void)
{
	prtconname(ISIZ);				// print integer constant name
}

/*
 * LICON:    EQU    *
 *           LDD    TOKPTR
 *           ADDD   #ISIZ+1
 *           BRA    LFCON2
 */


/***** lscon() *****/
void  lscon(void)
{
	unsigned char	count = tokptr[1];
	int				i;
	int				binary = 0;

	for(i = 0; i < count; i++){
		if(tokptr[2 + i] < ' '){
			binary = 1;
			break;
		}
	}
	if(binary){
		lbcon();
	}else{
		outbyte('"');
		prtconname(0);
		outbyte('"');
	}
}

/*
 * LSCON:    EQU    *
 *           LDD    TOKPTR
 *           ADDD   #1
 *           BRA    LFCON2
 * *
 */

void  lbcon(void)
{
	unsigned char			count;

	tokptr = tokptr + 1;
	count = *tokptr++;
	outbyte('$');
	outbyte('"');
	while (count--)
	{
		outhexbyte(*tokptr++);
	}
	outbyte('"');
	return;
}

/*
 *  llcon      display a line-number constant
 */
void  llcon(void)
{
	int						con;

	tokptr++;
	con = getU16((U16 *)tokptr);				// get line number from buffer
	tokptr = tokptr + sizeof(U16);		// step past line number
	outdeci(con);
	return;
}

/*
 * LLCON:    EQU    *
 *           LDX    TOKPTR
 *           INX
 *           LDD    0,X
 *           INX
 *           INX
 *           STX    TOKPTR
 *           JSR    OUTDECI
 *           RTS
 */



/*
 *  prtconname      print constant name from token buffer
 *
 *  Used only by clist().
 */
static void  prtconname(unsigned char  size)
{
	unsigned char			count;

	tokptr = tokptr + size + 1;
	count = *tokptr++;
	while (count--)
	{
		outbyte(*tokptr++);
	}
	return;
}




/***** lkeyword *****/
void  lkeyword(void)
{
	char					token;
	int						n;

	token = *tokptr++;
	switch (token)					// based on the token...
	{
		case  MSCNTOK:				// multiple spaces
		token = *tokptr++;			// get number of spaces
		for (n=0; n<token; n++)		// print required number of spaces
		{
			outbyte(' ');
		}
		break;

//		case  COMMATOK:				// comma
//		outbyte(',');
//		break;
//
//		case  SEMITOK:				// semicolon
//		outbyte(';');
//		break;

		case  SSCNTOK:				// single space
		outbyte(' ');
		break;

		case  REMTOK:				// remark
		pl_P(PSTR(REM_STR));		// print the remark keyword
		tokptr++;					// step over length byte in REM statement
		prttoeol();					// print all chars until EOL token
		break;

		case  QREMTOK:				// quick remark
		pl_P(PSTR(QREM_STR));		// print the quick remark keyword
		tokptr++;					// step over length byte in QREM statement
		prttoeol();					// print all chars until EOL token
		break;

		case  DATATOK:				// data
		pl_P(PSTR(DATA_STR));		// print the DATA keyword
		tokptr++;					// step over length byte in DATA statement
		prttoeol();					// print the rest of the line
		break;

		case  FUNCTFLG:				// function, next byte defines the function
		token = *tokptr++;			// get function identifier
		pl_P((char*)FunctionTable[token-1].name);	// allow for 0-based indexing
		break;

		default:
		pl_P((char*)RTFuncs[token-1].name);	// allow for 0-based indexing
//		if ((token == THENTOK) || (token == ELSETOK))  tokptr++;	// step past offset that follows these tokens
		break;
	}
}



static  void  prttoeol(void)
{
	while (*tokptr != EOLTOK)	// for all chars in the comment...
	{
		if(*tokptr == 0x0d){
			tokptr += 1;
		}else{
			outbyte(*tokptr++);		// print the comment
		}
	}
}





/*
 * LKEYWORD: EQU    *
 *           LDX    TOKPTR
 *           LDAA   0,X
 *           INX
 *           STX    TOKPTR
 *           CMPA   #MSCNTOK
 *           BNE    LKEYWRD3
 *           JMP    LMSPCS
 * LKEYWRD3: CMPA   #REMTOK
 *           BNE    LKEYWRD4
 *           JMP    LREMLINE
 * LKEYWRD4: CMPA   #DATATOK
 *           BNE    LKEYWRD5
 *           JMP    LDATALIN
 * LKEYWRD5: CMPA   #FUNCTFLG
 *           BNE    LKEYWRD6
 *           LDX    TOKPTR
 *           LDAA   0,X
 *           INX
 *           STX    TOKPTR
 *           LDX    #LFUNCTBL
 *           BRA    LKEYWRD1
 * LKEYWRD6: LDX    #TOKTBL
 * LKEYWRD1: CMPA   0,X
 *           BEQ    LKEYWRD2
 *           INX
 *           INX
 *           INX
 *           TST    0,X
 *           BNE    LKEYWRD1
 *           LDAA   #ILTOKERR
 *           JMP    RPTERR
 * LKEYWRD2: LDX    1,X
 *           JMP    PL
 * *
 * *
 * LMSPCS:   EQU    *
 *           LDX    TOKPTR
 *           LDAB   0,X
 *           INX
 *           STX    TOKPTR
 *           LDAA   #$20
 * LMSPCS1:  JSR    OUTBYTE
 *           DECB
 *           BNE    LMSPCS1
 *           RTS
 * *
 * *
 * LDATALIN: EQU    *
 *           LDX    #DATA
 *           JSR    PL
 *           BRA    LREM3
 * *
 * *
 * LREMLINE: EQU    *
 *           LDX    #REM
 *           JSR    PL
 * LREM3:    LDX    TOKPTR
 *           INX                 ; PUT POINTER PAST LENGTH BYTE.
 * LREM1:    LDAA   0,X
 *           CMPA   #EOL   
 *           BNE    LREM2
 *           INX
 *           STX    TOKPTR
 *           RTS
 * LREM2:    JSR    OUTBYTE
 *           INX
 *           BRA    LREM1
 * *
 * *
 * TOKTBL:   EQU     *
 *           FCB     LETTOK
 *           FDB     LET
 *           FCB     READTOK
 *           FDB     READ
 *           FCB     RESTRTOK
 *           FDB     RESTORE
 *           FCB     GOSUBTOK
 *           FDB     GOSUB
 *           FCB     GOTOTOK
 *           FDB     GOTO
 *           FCB     ONTOK
 *           FDB     ON
 *           FCB     RETNTOK
 *           FDB     RETURN
 *           FCB     IFTOK
 *           FDB     IIF
 *           FCB     THENTOK
 *           FDB     THENS
 *           FCB     ELSETOK
 *           FDB     ELSES
 *           FCB     INPUTTOK
 *           FDB     INPUT
 *           FCB     PRINTTOK
 *           FDB     PRINT
 *           FCB     FORTOK
 *           FDB     FOR
 *           FCB     NEXTTOK
 *           FDB     NEXT
 *           FCB     STOPTOK
 *           FDB     STOPSS
 *           FCB     ENDTOK
 *           FDB     ENDS
 *           FCB     TRONTOK
 *           FDB     TRON
 *           FCB     TROFFTOK
 *           FDB     TROFF
 *           FCB     WHILETOK
 *           FDB     WHILE
 *           FCB     ENDWHTOK
 *           FDB     ENDWH
 *           FCB     STEPTOK
 *           FDB     STEP
 *           FCB     TOTOK
 *           FDB     TO
 *           FCB     COMMATOK
 *           FDB     COMMAC
 *           FCB     SEMITOK
 *           FDB     SEMIC
 *           FCB     MEOLTOK
 *           FDB     COLLINC
 *           FCB     IMLETTOK
 *           FDB     IMLET
 *           FCB     POKETOK
 *           FDB     POKE
 *           FCB     EQUALTOK
 *           FDB     EQ
 *           FCB     OPARNTOK
 *           FDB     OPARN
 *           FCB     CPARNTOK
 *           FDB     CPARN
 *           FCB     ANDTOK
 *           FDB     ANDS
 *           FCB     ORTOK
 *           FDB     ORS
 *           FCB     EORTOK
 *           FDB     EORS
 *           FCB     LTTOK
 *           FDB     LT
 *           FCB     GTTOK
 *           FDB     GT
 *           FCB     LTEQTOK
 *           FDB     LTEQ
 *           FCB     GTEQTOK
 *           FDB     GTEQ
 *           FCB     EQTOK
 *           FDB     EQ
 *           FCB     NOTEQTOK
 *           FDB     NOTEQ
 *           FCB     PLUSTOK
 *           FDB     PLUS
 *           FCB     MINUSTOK
 *           FDB     MINUS
 *           FCB     MULTTOK
 *           FDB     MULT
 *           FCB     DIVTOK
 *           FDB     DIV
 *           FCB     MODTOK
 *           FDB     MODS
 *           FCB     NOTTOK
 *           FDB     NOTS
 *           FCB     RTIMETOK
 *           FDB     RTIMES
 *           FCB     NEGTOK
 *           FDB     NEGS
 *           FCB     SSCNTOK
 *           FDB     SPACE
 *           FCB     DIMTOK
 *           FDB     DIM
 *           FCB     EEPTOK
 *           FDB     EEP
 *           FCB     PORTATOK
 *           FDB     PORTA
 *           FCB     PORTBTOK
 *           FDB     PORTB
 *           FCB     PORTCTOK
 *           FDB     PORTC
 *           FCB     PORTDTOK
 *           FDB     PORTD
 *           FCB     PNUMTOK
 *           FDB     POUNDSGN
 *           FCB     INBYTTOK
 *           FDB     INBYTES
 *           FCB     TIMETOK
 *           FDB     TIME
 *           FCB     ONTIMTOK
 *           FDB     ONTIME
 *           FCB     ONIRQTOK
 *           FDB     ONIRQ
 *           FCB     RETITOK
 *           FDB     RETI
 *           FCB     PACCTOK
 *           FDB     PACC
 *           FCB     ONPACTOK
 *           FDB     ONPACC
 *           FCB     SLEEPTOK
 *           FDB     SLEEP
 *           FCB     0            ; END OF TABLE MARKER.
 * *
 * *
 * LFUNCTBL: EQU    *
 *           FCB    FDIVTOK
 *           FDB    FDIVS
 *           FCB    CHRTOK
 *           FDB    CHRS
 *           FCB    ADCTOK
 *           FDB    ADCS
 *           FCB    ABSTOK
 *           FDB    ABS
 *           FCB    RNDTOK
 *           FDB    RND
 *           FCB    SGNTOK
 *           FDB    SGN
 *           FCB    TABTOK
 *           FDB    TABS
 *           FCB    CALLTOK
 *           FDB    CALL
 *           FCB    PEEKTOK
 *           FDB    PEEK
 *           FCB    FEEPTOK
 *           FDB    EEP
 *           FCB    HEXTOK
 *           FDB    HEX
 *           FCB    FPRTATOK
 *           FDB    PORTA
 *           FCB    FPRTBTOK
 *           FDB    PORTB
 *           FCB    FPRTCTOK
 *           FDB    PORTC
 *           FCB    FPRTDTOK
 *           FDB    PORTD
 *           FCB    FPRTETOK
 *           FDB    PORTE
 *           FCB    FTIMETOK
 *           FDB    TIME
 *           FCB    HEX2TOK
 *           FDB    HEX2
 *           FCB    FPACCTOK
 *           FDB    PACC
 * IMLET:    FCB    0            ;  NO KETWORD TO PRINT FOR AN IMPLIED LET.
 * COLLINC:  FCC    ":"
 *           FCB    0
 * SEMIC:    FCC    ";"
 *           FCB    0
 * COMMAC:   FCC    ","
 *           FCB    0
 * OPARN:    FCC    "("
 *           FCB    0
 * CPARN:    FCC    ")"
 *           FCB    0
 * SPACE:    FCC    " "
 *           FCB    0
 * PORTE:    FCC    "PORTE"
 *           FCB    0
 * POUNDSGN: FCC    "#"
 *           FCB    0
 */


/*
 *  crun      the RUN command
 */
void  crun(void)
{
	nl();
	runinit();
	runflag = 1;
	if (basbeg == basend)					// if nothing in memory...
	{
		pl_P(PSTR("No program loaded in memory.\n\r"));
		return;
	}
	tbufptr = basbeg;						// start at beginning of storage area
	adrnxlin = basbeg;						// for now, next line is also beginning
	_crun();								// all set, now start running
	if (errcode)							// anything wrong?
	{
		rpterr();							// if problems, tell the user
	}
	nl();
	runflag = 0;
}





/*
 *  _crun      run-time execution loop
 */
void  _crun(void)
{
	unsigned char		tok;
#ifdef  AVR
	void				(*fptr)(void);
#endif
	while (errcode == 0)						// as long as no error...
	{
		StartNextLine();						// prepare to start next line
		tbufptr++;								// step over length byte
		while (*tbufptr != EOLTOK)				// for all commands in this line...
		{
			rskipspc();							// skip past any spaces
			tok = *tbufptr++;					// get the execution token, advance pointer
			switch  (tok)						// based on the token value...
			{
				case  GOSUBTOK:					// gosub
				rpreparegosub();				// prepare for the GOSUB
				if (errcode)  return;			// if we hit an error, bail now
				tok = GOTOTOK;					// all set up, pretend this is a GOTO
				break;

				default:						// for all other cases...
				break;							// no setup required
			}
#ifdef  WINDOWS		
			RTFuncs[tok-1].funcptr();			// invoke the selected function (adjust for indexing)
#endif
#ifdef  AVR
			fptr = (void *)pgm_read_word(&RTFuncs[tok-1].funcptr);	// read addr of function from pointer table
			fptr();								// run it
#endif
#if  defined(CPU78K0R) || defined(CPUML7416) || defined(CPUSTM32)
			RTFuncs[tok-1].funcptr();			// invoke the selected function (adjust for indexing)
#endif
			if (errcode)  return;				// if we hit an error, bail now
			targetruntask();
			if (breakcnt)  breakcnt--;			// count this break check
			if (breakcnt == 0)					// if time to check...
			{
				if (chkbrk())					// if user wants out...
				{
					return;
				}
			}
		}
		if (tbufptr+1 == basend)  return;		// if we hit the end of the program, bail now
		tbufptr++;								// no, keep on going
	}
}

/*
 * CRUN:     EQU    *
 *           JSR    NL2          ; DO 2 CR/LF SEQUENCES.
 *           JSR    RUNINIT      ; INITALIZE RUNTIME VARIABLES.
 *           LDAA   #1           ; SET THE RUN MODE FLAG.
 *           STAA   RUNFLAG
 * *
 * *        END OF POINTER INITIALIZATIONS
 * *
 *           LDY    BASBEG       ; POINT TO THE START OF THE PROGRAM.
 *           CPY    BASEND       ; IS THERE A PROGRAM IN MEMORY?
 *           BNE    CRUN5        ; YES. GO RUN IT.
 *           RTS                 ; NO. RETURN.
 * *
 * CRUN5:    LDD    0,Y          ; GET NUMBER OF FIRST/NEXT LINE OF BASIC PROGRAM.
 *           STD    CURLINE      ; MAKE IT THE CURRENT LINE.
 *           TST    TRFLAG       ; IS THE TRACE MODE TURNED ON?
 *           BEQ    CRUN6        ; NO. CONTINUE.
 *           LDAA   #'['         ; YES. PRINT THE CURRENT LINE.
 *           JSR    OUTBYTE
 *           LDD    CURLINE
 *           JSR    OUTDECI
 *           LDAA   #']'
 *           JSR    OUTBYTE
 *           JSR    NL
 * CRUN6:    PSHY                ; SAVE POINTER TO START OF NEW LINE.
 *           LDAB   2,Y          ; GET LENGTH OF LINE.
 *           ABY                 ; POINT TO START OF NEXT LINE.
 *           STY    ADRNXLIN     ; SAVE THE ADDRESS OF THE NEXT LINE.
 *           PULY
 *           LDAB   #3           ; BYTE COUNT OF LINE NUMBER & LENGTH.
 *           ABY                 ; POINT TO THE FIRST TOKEN.
 * CRUN4:    BSR    RSKIPSPC     ; SKIP SPACES IF PRESENT.
 *           LDAB   0,Y          ; GET KEYWORD TOKEN.
 *           INY                 ; POINT PAST THE KEYWORD.
 *           BSR    RSKIPSPC     ; SKIP SPACES AFTER KEYWORD.
 *           DECB                ; SUBTRACT ONE FOR INDEXING.
 *           LSLB                ; MULTIPLY BY THE # OF BYTES PER ADDRESS.
 *           LDX    #RKEYWORD    ; POINT TO RUN TIME ADDRESS TABLE.
 *           ABX                 ; POINT TO ADDRESS
 *           LDX    0,X          ; POINT TO RUNTIME ROUTINE.
 *           JSR    0,X          ; GO DO IT.
 * *
 * *
 * CRUN2:    DEC    BREAKCNT     ; SHOULD WE CHECK FOR A BREAK YET?
 *           BNE    CRUN7        ; NO. CONTINUE.
 *           JSR    CHCKBRK      ; CHECK FOR BREAK FROM CONSOLE.
 * *
 * CRUN7:    BSR    RSKIPSPC     ; SKIP ANY SPACES.
 *           LDAA   0,Y          ; GET THE NEXT TOKEN IN THE LINE.
 *           CMPA   #EOLTOK      ; ARE WE AT THE END OF THE LINE?
 *           BNE    CRUN3
 *           INY                 ; YES. POINT TO START OF THE NEXT LINE.
 * CRUN1:    CPY    BASEND       ; HAVE WE REACHED THE END OF THE BASIC PROGRAM?
 *           BNE    CRUN5        ; NO. GO EXECUTE THE NEXT LINE.
 *           JMP    REND         ; GO DO  AN "END".
 * CRUN3:    INY                 ; MUST BE A MID EOL.
 *           BRA    CRUN4        ; GO DO NEXT KEYWORD.
 */




/*
 *  StartNextLine      adjust the globals to start the next line; do trace, if needed
 *
 *  Upon entry, tbufptr must point to the line number of the new current line.
 *  Upon exit, tbufptr points to length byte following the line number.
 *  Additionally, tracing information is printed if trace is on.
 */
void  StartNextLine(void)
{
	adrnxlin = tbufptr + *(tbufptr+2);		// calc address of next line, if any
	curline = getU16((U16 *)tbufptr);				// curline now holds line being run
	tbufptr = tbufptr + sizeof(U16);		// move past line number

	if (trflag)
	{
		outbyte('[');
		outdeci(curline);
		outbyte(']');
		outbyte(' ');
//		sprintf(tbuff, "[%u] ", curline);
//		pl(tbuff);
	}
}



/*
 *  rnop      run-time NOP
 *
 *  This is a place-holder function for the MIDEOL token.
 */
void  rnop(void)
{
}




/*
 *  rskipspc      skip space tokens in the run-line buffer
 */
void  rskipspc(void)
{
	switch (*tbufptr)
	{
		case  SSCNTOK:			// token for single space
		tbufptr++;				// skip past the token
		break;

		case  MSCNTOK:			// token for multiple spaces
		tbufptr = tbufptr + 2;	// skip past token plus count byte
		break;

		default:				// for all other tokens...
		return;					// nothing to do
	}
}

/*
 * RSKIPSPC: LDAA   0,Y          ; GET A CHARACTER.
 *           BMI    RSKIP2
 *           CMPA   #SSCNTOK     ; IS IT A SINGLE SPACE?
 *           BEQ    RSKIP1       ; YES. BUMP IP BY 1.
 *           BLO    RSKIP2
 *           INY                 ; BUMP IP BY 2 FOR MULTIPLE SPACES.
 * RSKIP1:   INY                 ; BUMP IP.
 * RSKIP2:   RTS                 ; RETURN.
 */


/*
 * RKEYWORD: EQU    *
 *           FDB    RLET
 *           FDB    RLET
 *           FDB    RPRINT
 *           FDB    RFOR
 *           FDB    RNEXT
 *           FDB    RTRON
 *           FDB    RTROFF
 *           FDB    RPOKE
 *           FDB    RDIM
 *           FDB    RREM
 *           FDB    RPACC
 *           FDB    RDATA
 *           FDB    RREAD
 *           FDB    RRESTOR
 *           FDB    RGOSUB
 *           FDB    0
 *           FDB    0
 *           FDB    RGOTO
 *           FDB    RON
 *           FDB    RRETURN
 *           FDB    RIF
 *           FDB    RINPUT
 *           FDB    RSTOP
 *           FDB    REND
 *           FDB    RWHILE
 *           FDB    RENDWH
 *           FDB    REEP
 *           FDB    RPORTA
 *           FDB    RPORTB
 *           FDB    RPORTC
 *           FDB    RPORTD
 *           FDB    0
 *           FDB    0
 *           FDB    0
 *           FDB    RINBYTE
 *           FDB    RTIME
 *           FDB    RONTIME
 *           FDB    RONIRQ
 *           FDB    RRETI
 *           FDB    RONPACC
 *           FDB    RSLEEP
 *           FDB    RRTIME
 */


/*
 *  runline
 */
void  runline(void)
{
	unsigned char			tok;
#ifdef  AVR
	void				(*fptr)(void);
#endif

	tbufptr = tknbuf;				// point to start of token buffer
	rskipspc();						// step over any spaces
	while ((*tbufptr != EOLTOK) && (errcode == 0))		// for all commands in this line...
	{
		rskipspc();					// step over spaces within processing loop
		tok = *tbufptr++;			// get run-time token, bump pointer
#ifdef  WINDOWS		
		RTFuncs[tok-1].funcptr();	// invoke the selected function (adjust for indexing)
#endif
#ifdef  AVR
		fptr = (void *)pgm_read_word(&RTFuncs[tok-1].funcptr);	// read addr of function from pointer table
		fptr();						// run it
#endif
#if  defined(CPU78K0R) || defined(CPUML7416) || defined(CPUSTM32)
		RTFuncs[tok-1].funcptr();	// invoke the selected function (adjust for indexing)
#endif
	}
}

/*
 * RUNLINE:  JSR    NL2
 *           LDY    TKNBUFS      ; POINT TO THE TOKEN BUFFER.
 *           LDD    0,Y          ; GET CURRENT LINE NUMBER.
 *           STD    CURLINE      ; MAKE "0" THE CURRENT LINE #.
 *           LDAB   #3           ; POINT PAST THE LINE NUMBER & LENGTH.
 *           ABY
 * RUNLINE2: BSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAB   0,Y          ; GET KEYWORD.
 *           INY                 ; POINT PAST KEYWORD.
 *           BSR    RSKIPSPC     ; SKIP SPACES.
 *           DECB                ; SUBTRACT ONE FOR INDEXING.
 *           LSLB                ; MULT BY THE # OF BYTES/ADDRESS.
 *           LDX    #RKEYWORD    ; POINT TO ADDRESS TABLE.
 *           ABX                 ; POINT TO ADDRESS OF RUN TIME ROUTINE.
 *           LDX    0,X          ; GET ADDRESS.
 *           JSR    0,X          ; GO DO IT.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAA   0,Y
 *           CMPA   #EOLTOK      ; ARE WE AT THE END OF THE LINE?
 *           BNE    RUNLINE1
 *           RTS
 * RUNLINE1: INY                 ; MUST BE A MID EOL.
 *           BRA    RUNLINE2
 */



/*
 *  chkbrk
 *
 *  Modified from Gordon's original, in that this version returns
 *  TRUE if a break was detected, else FALSE.  This lets the caller
 *  unwind everything, since we can't jump directly to main() in
 *  C.
 */
unsigned char  chkbrk(void)
{
	breakcnt = BREAK_CNT;		// rewind the break counter
	if (breakflag)				// if user wanted a break...
	{
		breakflag = 0;			// erase the flag
		errcode = BRKDETECT;	// show we caught a break
		ipsave = adrnxlin;		// will continue from next line (is this good enough?)
//		curline = 0;			// set current line to start of program
		return  TRUE;
	}
	else
	{
		return  FALSE;
	}
}

/*
 * CHCKBRK:  EQU    *
 *           LDAA   #10          ; RELOAD THE BREAK CHECK COUNT.
 *           STAA   BREAKCNT
 *           JSR    CONSTAT      ; GET CONSOLE STATUS. CHARACTER TYPED?
 *           BNE    CHCKBRK1     ; YES. GO CHECK IT OUT.
 *           RTS                 ; NO. RETURN.
 * CHCKBRK1: JSR    INCONNE      ; GET BYTE FROM CONSOLE BUT DON'T ECHO.
 *           CMPA   #$03         ; WAS IT A CONTROL-C?
 *           BEQ    CHCKBRK2     ; YES. GO DO A BREAK.
 *           RTS                 ; NO. RETURN.
 * CHCKBRK2: STY    IPSAVE       ; SAVE THE IP POINTER IN CASE OF A CONTINUE.
 *           JSR    NL
 *           LDX    #BREAKS      ; POINT TO BREAK STRING.
 *           JSR    PL
 *           LDD    CURLINE
 *           JSR    OUTDECI
 *           JSR    NL
 *           JMP    MAINW
 */



/*
 *  runinit
 */
void  runinit(void)
{
//	U16					tend;

	cclear();						// clear variable storage and stacks

	breakflag = FALSE;				// erase any pending break char
	printpos = 0;
	breakcnt = 10;
	contflag = FALSE;				// FALSE means allow breaks (hey, it was Gordon's idea!)
	dataptr = 0;
}

/*
 * RUNINIT:  EQU    *
 *           JSR    CCLEAR       ; GO CLEAR ALL VARIABLE STORAGE.
 * RUNINIT1: LDX    STNUMS      ; GET START OF NUMERIC OPERAND STACK.
 *           STX    NUMSTACK     ; INITALIZE THE OPERAND STACK POINTER.
 *           LDX    STOPS       ; GET THE START OF THE OPERATOR STACK.
 *           STX    OPSTACK      ; INITALIZE THE OPREATOR STACK POINTER.
 *           LDX    STFORSTK    ; GET THE START OF THE FOR-NEXT STACK.
 *           STX    FORSTACK     ; INITALIZE THE FOR NEXT STACK POINTER.
 *           LDX    STWHSTK     ; GET THE START OF THE WHILE STACK.
 *           STX    WHSTACK      ; INITALIZE THE WHILE STACK POINTER.
 *           LDX    STGOSTK     ; GET THE START OF THE GOSUB STACK.
 *           STX    GOSTACK      ; SET THE START OF THE GOSUB STACK.
 *           LDX    VAREND       ; GET THE VARIABLE END POINTER.
 *           INX                 ; POINT TO THE NEXT AVAILABLE BYTE.
 *           STX    STRASTG      ; INITALIZE THE STRING/ARRAY STORAGE POINTER.
 *           CLR    PRINTPOS     ; SET THE CURRENT PRINT POSITION TO 0.
 *           LDAA   #10          ; SET COUNT FOR BREAK CHECK.
 *           STAA   BREAKCNT
 *           CLR    CONTFLAG     ; CLEAR THE CONTINUE FLAG.
 *           LDX    #0           ; CLEAR THE DATA POINTER.
 *           STX    DATAPTR
 *           RTS
 */



/*
 *  ccont      the CONT command
 */
void  ccont(void)
{
	if (contflag == FALSE)			// if ok to continue
	{
		tbufptr = ipsave;			// restore the IP
		runflag = 1;
		_crun();					// let 'er rip
		runflag = 0;
	}
	else
	{
		errcode = CNTCNERR;			// can't continue
		rpterr();					// complain about it
	}
}

/*
 * CCONT:    EQU    *
 *           JSR    NL2
 *           TST    CONTFLAG
 *           BNE    CCONT1
 *           LDY    IPSAVE
 *           JMP    CRUN7
 * CCONT1:   LDAA   #CNTCNERR
 *           STAA   ERRCODE
 *           JMP    RPTERR5
 */



/*
 *  cnew      the NEW command
 */
void  cnew(void)
{
	unsigned char			tbuff[20];

//	if (autostf == VALID_FLAG)  autostf = ~VALID_FLAG;		// mark program as invalid
	if (basbeg != basend)									// if there is a program stored...
	{
		pl_P(PSTR("\n\rThis will erase the current program!  Are you sure? "));
		targetgets(tbuff);
		if ((*tbuff == 'y') || (*tbuff == 'Y'))
		{
			initvars();
			pl_P(PSTR("\n\rProgram erased."));
		}
	}
	else
	{
		pl_P(PSTR("\n\rNo program in memory."));
	}
}

/*
 * CNEW:     EQU    *
 *           ldx    EEStart
 *           LDAA   AUTOSTF,X   ;  GET THE AUTO START FLAG.
 *           CMPA   #$55         ; IS IT SET?
 *           BNE    CNEW1        ; NO. GO INITIALIZE EVERYTHING.
 *           LDAA   #$FF         ; YES. RESET (ERASE) IT.
 *           STAA   AUTOSTF,X
 *           JSR    DLY10MS
 * CNEW1:    JSR    INITVARS     ; INITIALIZE EVERYTHING.
 *           RTS                 ; RETURN.
 */


/*
 *  cclear      the CLEAR command
 */
void  cclear(void)
{
	unsigned char			*vptr;

	initstacks();						// rewind the stacks
//	initstack32(&numstack, numarray, NUMSLEN);
//	initstack8(&opstack, oparray, OPSLEN);
//	forindex = 0;						// show no FOR statements yet
//	goindex = 0;						// show no GOSUB statements yet
//	whindex = 0;						// show no WHILE statements yet

	vptr = varbegin;
	while (*vptr)
	{
//		vptr = vptr + 3;						// move to data area??
		clrvar(&vptr);							// clear the variable
	}
}

/*
 * *
 * CCLEAR:   EQU    *
 *           JSR    RUNINIT1     ; GO INITALIZE ALL STACKS ETC.
 * CCLEAR3:  LDX    VARBEGIN
 * CCLEAR1:  LDAA   0,X
 *           BEQ    CCLEAR2
 *           INX
 *           INX
 *           INX
 *           JSR    CLRVAR
 *           BRA    CCLEAR1
 * CCLEAR2:  LDX    VAREND
 *           INX
 *           STX    STRASTG
 *           RTS
 * *
 * *
 */
