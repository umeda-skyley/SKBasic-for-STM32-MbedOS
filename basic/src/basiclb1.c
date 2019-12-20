/*
 *  basiclb1      target-independent version of Gordon Doughman's BASIC11 interpreter
 *
 *  This code is derived from the source and comments supplied by
 *  Gordon in his release of BASIC11.  I've retained the header and
 *  associated comments.
 */

/*
 ******************************************************************************
 *                                                                            *
 *                      MC68HC11 BASIC INTERPRETER                            *
 *                                                                            *
 *                             WRITTEN BY:                                    *
 *                                                                            *
 *                           GORDON DOUGHMAN                                  *
 *                                                                            *
 *                        COPYRIGHT 1985-1990 BY                              *
 *                                                                            *
 *                           GORDON DOUGHMAN                                  *
 *                                                                            *
 ******************************************************************************
 */


#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>


#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"


/*
 *  Local functions
 */
static void				outheader(void);
static void				outrdy(void);
static void				outprmpt(void);
#if	!defined(SKBASIC_IMPL) && !defined(SKBASIC_CMD) && !defined(SKBASIC_EMBEDDED)
static void				parse(void);
#endif
//static void				putlinum(U16  lnum);




/*
 *  basic_main()  entry point for the KLBasic program.
 */

#if	!defined(SKBASIC_IMPL)
void  basic_main(void)
{
	unsigned char				*fence;		// temp pointer storage in case of error
	U16							tend;

	iodevinit();				// set up the I/O devices
	initvars();					// initialize all variables and pointers
	initstacks();				// set up the various stacks
	targetgetdynmeminfo(&strastg, &tend);	// rewind dyn mem pointer and length
	dynmemend = strastg + tend;	// calc ending addr of dyn mem

	if (targetcheckautorun())		// if target device detects autorun...
	{
		crun();					// execute the program (better be in RAM!)
	}
	outheader();				// send startup message (possibly to the console)
	immid = TRUE;				// always start in immediate mode


/*	
 *           JMP    POWERUP
 * MAIN:     EQU    *
 * MAINC:    JSR    INITVARS     ; INITALIZE ALL INTERNAL VARIABLES.
 *           LDX    EEStart
 *           LDAA   AUTOSTF,X    ; get the auto start flag.
 *           CMPA   #$55
 *           BNE    MAIN9
 *           CLI                 ; ALLOW ALL INTERRUPTS TO BE SERVICED.
 *           JSR    CRUN
 * MAIN9:    JSR    OUTHEADR     ; PRINT HEADER.
 * MAINW:    EQU    *
 * MAIN2:    LDD    RAMStart     ; RESET STACK VALUE.
 *           ADDD   RAMSize
 *           XGDX
 *           TXS
 *           CLI                 ; (J.I.C.)
 *           CLRA                ; CLEAR D.
 *           CLRB
 *           STD    TIMECMP      ; DON'T ALLOW "ONTIME" INTERRUPTS TO OCCUR.
 *           STD    ONIRQLIN     ; DON'T ALLOW "ONIRQ" INTERRUPTS TO OCCUR.
 *           STD    ONPACLIN     ; DON'T ALLOW "PACC" INTERRUPTS TO OCCUR.
 *           JSR    OUTRDY       ; PRINT READY MESSAGE.
 */
	

	while(1)						// do forever
	{
		if (immid)					// if we are in immediate mode...
		{
//			pl_P(PSTR("FOR index="));
//			outdeci(forindex);
//			pl_P(PSTR(" GO index="));
//			outdeci(goindex);
//			pl_P(PSTR(" WHILE index="));
//			outdeci(whindex);
//			pl_P(PSTR(" NUM index="));
//			outdeci(numstack.index);
//			pl_P(PSTR(" OP index="));
//			outdeci(opstack.index);
			outrdy();				// send the ready message
		}

		immid = 0;					// not in immediate mode (yet)
		errcode = 0;				// clear error status
		runflag = 0;				// clear the run mode flag
		outprmpt();					// output prompt
		getline();					// getline from console
		skipspcs();					// ignore leading spaces in input buffer
//		if (chckcmds()) continue;	// check for commands
		initstack8(&opstack, oparray, OPSLEN);	// always clear opcode stack
		initstack32(&numstack, numarray, NUMSLEN); 	// always clear numeric stack

		if (chckcmds() == 0)		// if line was not a known command...
		{
			fence = varend;			// save current end of variables
#if defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
			parse_cmdline();				// translate/execute line
#else
			parse();
#endif
			if (errcode)			// if an error occured somewhere...
			{
			 	rpterr();			// report the error
			 	varend = fence;		// restore the end of variables
			}
		}
#if defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
		if (errcode == EXIT_BASIC)
		{
			break;
		}
#endif
	}
}
#endif


/*
 * MAIN1:    EQU    *
 *           CLR    IMMID        ; CLEAR THE IMMIDIATE MODE FLAG.
 *           CLR    ERRCODE      ; CLEAR THE ERROR CODE BYTE.
 *           CLR    RUNFLAG      ; CLEAR THE RUN MODE FLAG.
 *           JSR    OUTPRMPT     ; SEND PROMPT TO CONSOLE.
 *           JSR    GETLINE      ; GO GET LINE FROM OPERATOR.
 *           JSR    SKIPSPCS     ; SKIP ANY LEADING SPACES.
 *           JSR    CHCKCMDS     ; GO CHECK FOR ANY COMMANDS.
 *           CPD    #0           ; WERE THERE ANY?
 *           BNE    MAIN2        ; YES. CONTINUE.
 *           LDX    VAREND       ; SAVE CURRENT END OF VARIABLE AREA IN CASE LINE
 *           STX    FENCE        ; HAS AN ERROR IN IT. (SO NO SPURIOUS VARIABLES
 *                               ; ARE ALLOCATED)
 *           JSR    PARSE
 *           TST    IMMID        ; DID WE EXECUTE IN IMMIDATE MODE?
 *           BNE    MAIN2        ; YES. PRINT READY MESSAGE.
 *           BRA    MAIN1        ; NO. JUST PRINT PROMPT.
 *
 *
 * MAIN3:    LDX    FENCE        ; GET THE VAREND FENCE.
 *           CLR    0,X          ; MARK "OLD" END OF VARIABLE AREA IN CASE ANY
 *                               ; VARIABLES WERE ALLOCATED.
 *           STX    VAREND       ; RESTORE THE POINTER.
 *           BRA    MAIN2        ; CONTINUE AFTER ERROR.
 */


/*
 *  skipspcs    skip spaces in the input buffer
 *
 *  Upon exit, ibufptr points either to the next non-space
 *  character or to the EOL marker.
 */
void  skipspcs(void)
{
	while (*ibufptr == ' ') ++ibufptr;
	return;
}


/*
 * SKIPSPCS: EQU    *
 * SKIPSPC1: JSR    GETCHR
 *           CMPA   #SPC
 *           BNE    SKIPSPC2
 *           JSR    INCIBP
 *           BRA    SKIPSPC1
 * SKIPSPC2: RTS
 */



          
/*
 *  outheader      display the KLBasic header
 */

static void  outheader(void)
{
//	nl();
//	nl();
	targetdisplayheader();					// have the target display it's header
//	pl_P(PSTR("\n\r" CORE_INFO  " v"  CORE_VERSION));
//	pl_P(PSTR("\n\rCore written by Karl Lunt, based on Gordon Doughman's BASIC11"));
}

/*
 * OUTHEADR: EQU    *
 *           LDX    #HEADER
 *           JMP    PL
 *
 * HEADER:   EQU    *
 *           FCB    $0D,$0A,$0D,$0A
 *           FCC    "BASIC11 v1.55"
 *           FCB    $0D,$0A
 *           FCC    "Copyright 1985-1990 by"
 *           FCB    $0D,$0A
 *           FCC    "Gordon Doughman"
 *          FCB    $0D,$0A,0
 */


/*
 *  outrdy      display the ready prompt
 */
static void  outrdy(void)
{
	pl_P(PSTR("\n\rREADY"));
	return;
}


/*
 * OUTRDY:   EQU    *
 *           LDX    #READY
 *           JMP    PL
 *
 * READY:    EQU    *
 *           FCB    $0D,$0A
 *           FCC    "READY"
 *           FCB    $0D,$0A,0
 */

          

/*
 *  getline      get a line of input (normally from the console)
 *
 */

void  getline(void)
{
	targetgets(inbuff);							// use gets() for Windows
	ibufptr = inbuff + strlen((char *)inbuff);
	*ibufptr = EOL;								// put EOL in input buffer
	ibufptr = inbuff;							// initalize the input buffer pointer
	return;
}

/*
 * GETLINE:  EQU    *
 *		     LDAB   #IBUFLEN-1
 *           LDX    INBUFFS
 */


/*
 * GETLINE3: JSR    INBYTE
 *           CMPA   #EOL
 *           BEQ    GETLINE1
 *           TSTB
 *           BEQ    GETLINE1
 */


/*
 *           CMPA   #BS
 *           BNE    GETLINE2
 *           CMPB   #IBUFLEN-1
 *           BEQ    GETLINE1
 *           DEX
 *           LDAA   #SPC
 *           JSR    OUTBYTE
 *           LDAA   #BS
 *           JSR    OUTBYTE
 *           INCB
 */


/*
 * GETLINE2: EQU    *
 *           CMPA   #CNTRLC      ; IS IT A "BREAK"?
 *           BNE    GETLINE4     ; NO. GO PROCESS THE CHARACTER.
 *           INC    CONTFLAG     ; DONT ALLOW A "CONT".
 *           JMP    CHCKBRK2     ; GO DO A BREAK.
 * GETLINE4: CMPA   #SPC
 *           BLO    GETLINE3
 */


/*
 *           STAA   0,X
 *           INX
 *           DECB
 *           BNE    GETLINE3
 */

/*
 * GETLINE1: EQU    *
 *           LDAA   #EOL
 *           STAA   0,X
 *           LDX    INBUFFS
 *           STX    IBUFPTR
 *           RTS
 */

void  putcommandline(char *line)
{
	strcpy((char*)inbuff, line);
	ibufptr = inbuff + strlen((char *)inbuff);
	*ibufptr = EOL;								// put EOL in input buffer
	ibufptr = inbuff;							// initalize the input buffer pointer
	return;
}


/*
 *  ToUpper      convert character to uppercase
 *
 *  Replaced with the C library function toupper().
 */

/*
 * ToUpper:   EQU      *
 *            cmpa     #'a'                ; is the character less than a lower case 'a'?
 *            blo      ToUpper1
 *            cmpa     #'z'
 *            bhi      ToUpper1
 *            anda     #$df
 * ToUpper1:  rts
 */
          

 /*
  *  outprmpt      send prompt for new input, usually to console
  */

static void  outprmpt(void)
{
	nl();					// go to new line
	outbyte(PROMPT_CHAR);	// send prompt to console
	return;
}

/*
 * OUTPRMPT: EQU    *
 *           LDX    #PRMPTSTR
 *           BRA    PL
 *
 * PRMPTSTR: EQU    *
 *           FCB    $0D,$0A
 *           FCC    "#"
 *           FCB    0
 */

          


/*
 *  pl() and nl() were originally defined here.  They have been moved into the
 *  targetxxx.c file because they are device-dependent.
 */

          
/*
 *  parse      parse the line held in the input buffer
 */
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
void  parse_cmdline(void)
#else
static void  parse(void)
#endif
{
	U16						num;

	tbufptr = tknbuf;					// initialize the token buffer pointer
/*
 * PARSE:    EQU    *
 *           LDX    TKNBUFS                ; Get the start of the token buffer
 *           STX    TBUFPTR
 */

	xdimflag = FALSE;					// show not running xdim() function now
	if ((num = getlinum()))				// get line number if present
	{
		if (*ibufptr == EOL)			// was line # followed by CR?
		{
			delline(num);				// yes, go delete the line from the prog buffer
//			targetgetdynmeminfo(strastg, &dynmemlen);	// rewind dyn mem pointer and length
			return;
		}
		immid = 0;						// flag as not immediate
		putlinum(num);					// put line number in buffer
	}
	else								// no line number?
	{
		immid=1;						// if no line # flag as immediate
	}
/*
 *           BSR    GETLINUM
 *           BCC    PARSE2
 *           PSHA
 *           JSR    GETCHR
 *           CMPA   #EOL
 *           PULA
 *           BNE    PARSE1
 *           JSR    DELLINE
 *           LDX    VAREND
 *           INX
 *           STX    STRASTG
 * PARSE7:   RTS
 * PARSE1:   CLR    IMMID
 * PARSE8:   BSR    PUTLINUM
 *           BRA    PARSE3
 * PARSE2:   JSR    GETCHR
 *           CMPA   #EOL
 *           BEQ    PARSE7
 *           LDAA   #1
 *           STAA   IMMID
 *           LDD    #0
 *           BRA    PARSE8
 */

	if (errcode) return;				// if line number error, return
	xlate();							// try to translate
	if (errcode) return;				// if translation error, return
	if (immid) runline();				// if immediate mode run 1 line
	else								// if not immediate mode...
	{
		storlin();						// if not store pgm line
//		strastg = INITIAL_STRASTG;		// reset the dynamic memory index
	}
	return;								// go get next line
}

/*
 * PARSE3:   JSR    XLATE
 *           TST    IMMID
 *           BEQ    PARSE5
 *           JMP    RUNLINE      ; GO RUN THE LINE & RETURN.
 * PARSE5:   JSR    STORLIN      ; GO STORE LINE & RETURN.
 *           LDX    VAREND
 *           INX
 *           STX    STRASTG
 *           RTS
 */
          



/*
 *  getlinum      scan input string for a line number in first field
 *
 *  Upon exit, returns 0 if first field is not a line number, else
 *  returns line number.  If an illegal line number (such as a number
 *  outside the legal range) is found, modifies global variable
 *  errcode accordingly.
 */
U16  getlinum(void)
{
	U16						num;
	U16						prevnum;	// used to spot line number out of range

	num = 0;
	prevnum = 0;

/*
 * GETLINUM: EQU    *
 *           PSHY
 *           CLRA
 *           PSHA
 *           PSHA
 *           TSY
 */

	/*
	 *  The original code used Gordon's numeric() function.  This code uses
	 *  the C library function isdigit() instead.
	 */
	if (!isdigit(*ibufptr)) return(0);		// if 1st char not numeric, return an error

/*
 *           LDX    IBUFPTR
 *           LDAA   0,X
 *           BSR    NUMERIC
 *           BCC    GTLNUM4
 */

	while (isdigit(*ibufptr))				// while the char is a decimal digit...
	{
		num = num * 10 + (*ibufptr-'0');	// convert digit and add to number
		ibufptr++;							// advance input buffer pointer
//		if (num <= 0)						// if line number exceeds legal range
		if (num < prevnum)					// if line number wraps around...
		{
			errcode = LINRANG;				// show line number out of range
			return(0);
		}
		prevnum = num;						// update overrun test value
	}
	return (num);
}


/*
 * GTLNUM2:  LDAA   0,X
 *           BSR    NUMERIC
 *           BCS    GTLNUM3
 *           SEC
 * GTLNUM1:  LDD    0,Y
 *           BNE    GTLNUM4
 *           LDAA   #LINENERR
 *           BRA    GTLNUM5
 * GTLNUM4:  INS
 *           INS
 *           PULY
 *           STX    IBUFPTR
 *           RTS
 * GTLNUM3:  BSR    ADDDIG
 *           BPL    GTLNUM2
 *           LDAA   #LINRANG
 * GTLNUM5:  JMP    RPTERR
 *
 *
 *
 * ADDDIG:   EQU    *
 *           LDD    0,Y
 *           ASLD
 *           ASLD
 *           ADDD   0,Y
 *           ASLD
 *           STD    0,Y
 *           LDAB   0,X
 *           INX
 *           SUBB   #'0'
 *           CLRA
 *           ADDD   0,Y
 *           STD    0,Y
 *           RTS
 *
 */



/*
 *  putint16      insert 16-bit value into token buffer, advance buffer pointer
 */
void  putint16(I16  val)
{
//	*(U16 *)tbufptr = val;				// save value in token buffer
	setU16((U16 *)tbufptr, val);				// save value in token buffer
	tbufptr = tbufptr + sizeof(U16);	// move pointer past line number
	return;
}



/*
 *  putlinum      insert line number into token buffer, advance buffer pointer one extra cell
 */
void  putlinum(U16  lnum)
{
	putint16(lnum);						// put line number into buffer
	*tbufptr++ = 0;						// hold place for length of line
	return;
}

/*
 *  PUTLINUM: EQU    *
 *           JSR    PUTDTOK
 *           CLRA
 *           JMP    PUTTOK
 */



/*
 *  numeric     test char, return TRUE if numeric
 *
 *  This routine has been replaced with the C library function isdigit().
 */

/*numeric(c)
 *char c;
 *{
 * c=c&127;
 * return((c>='0')&(c<='9'));
 *}
 */

/*
 * NUMERIC:  EQU    *
 *           CMPA   #'0'
 *           BLO    NUMERIC1
 *           CMPA   #'9'
 *           BHI    NUMERIC1
 *           SEC
 *           RTS
 * NUMERIC1: CLC
 *           RTS
 */

          
/*
 *  alpha      test char, return TRUE if in range A-Z, a-z
 *
 *  This routine has been replaced with C's library function isalpha().
 */

/*
 *alpha(c)
 *char c;
 *{
 * c=c&127;
 * return((c>='A')&(c<='Z'));
 *}
 */

/*
 * ALPHA:    EQU    *
 *           CMPA   #'A'
 *           BLO    ALPHA1
 *           CMPA   #'Z'
 *           BLS    ALPHA2
 *           CMPA   #'a'
 *           BLO    ALPHA1
 *           CMPA   #'z'
 *           BHI    ALPHA1
 * ALPHA2:   SEC
 *           RTS
 * ALPHA1:   CLC
 *           RTS
 */


/*
 *  alphanum    test char, return TRUE if it is alphanumeric
 */
int  alphanum(char  c)
{
	return (isalpha(c) || isdigit(c));
}

/*
 * ALPHANUM: EQU    *
 *           BSR    ALPHA
 *           BCC    ALPHANU1
 *           RTS
 * ALPHANU1: BRA    NUMERIC
 */




/*
 ****************************************
 *              xlate()
 * translate the input buffer into tokenized
 * form placing the results into tknbuf
 ****************************************
 */
void  xlate(void)
{
	while (*ibufptr != EOL)
	{
		ifwhflag=0;					// set IF flag to zero
		blanks();					// skip all blanks
/*
 *  The following block of code tests the token against a series of
 *  keywords and, if a match occurs, executes the related function.
 *  I will likely replace this later with a structure containing the
 *  keyword string and an associated function pointer.  It won't gain
 *  any speed, but it will make the program easier to maintain.
 */
		if		(match(DATA_STR))		xdata();
		else if (match(LET_STR))		xlet();
		else if	(match(READ_STR))		xread();
		else if (match(RESTORE_STR))	xrestore();
		else if	(match(GOSUB_STR))		xgosub();
		else if (match(GOTO_STR))		xgoto();
		else if (match(ON_STR))			xon();
		else if (match(RETURN_STR))		xreturn();
		else if (match(IF_STR))			xif();
		else if (match(INPUT_STR))		xinput();
		else if (match(PRINT_STR))		xprint();
		else if (match(FOR_STR))		xfor();
		else if (match(NEXT_STR))		xnext();
		else if (match(STOP_STR))		xstop();
		else if (match(ENDWHILE_STR))	xendwh();
		else if (match(END_STR))		xend();
		else if (match(REM_STR))		xrem();
		else if (match(DIM_STR))		xdim();
//		else if (match(SWAP_STR))		xswap();
		else if (match(TRON_STR))		xtron();
		else if (match(TROFF_STR))		xtroff();
		else if (match(WHILE_STR))		xwhile();
		else if (match(LABEL_STR))		xlabel();
		else if (match(SLEEP_STR))		xsleep();
//		else if (match(ONIRQ_STR))		xonirq();
		else if (match(EEP16_STR))		xeep16();
		else if (match(EEP32_STR))		xeep32();
		else if (match(EEP_STR))		xeep();
		else if (match(INBYTE_STR))		xinbyte();
//		else if (match(TIMESTR_STR))	xtimestr();
//		else if (match(DATESTR_STR))	xdatestr();
		else if (match(QPRINT_STR))		xqprint();		// quick print (added 21 Jul 08 KEL)
		else if (match(QREM_STR))		xqrem();		// quick REM (added 16 Aug 08 KEL)
		else if (match(ELSE_STR))		return;			// bail early in case we are executing inside an IF statement
		else if (match(STRCAT_STR))		xstrcat();
		else if (match(STRIND_STR))		xstrind();
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
		else if (match(SKSREG_STR))		xsksreg();
		else if (match(SKSEND_STR))		xsksend();
		else if (match(SKFLASH_STR))	xskflash();
		else if (match(SKBC_STR))		xskbc();
		else if (match(SKSYNC_STR))		xsksync();
		else if (match(SKINQ_STR))		xskinq();
		else if (match(SKPAIR_STR))		xskpair();
		else if (match(SKUNPAIR_STR))	xskunpair();
		else if (match(SKNOW_STR))		xsknow();
		else if (match(SKSLEEP_STR))	xsksleep();
		else if (match(SKSETPS_STR))	xsksetps();
		else if (match(SKCLRTBL_STR))	xskclrtbl();
		else if (match(SKCLRCACHE_STR))	xskclrcache();
		else if (match(SKLKUP_STR))		xsklkup();
		else if (match(SKREVLKUP_STR))	xskrevlkup();
		else if (match(SKSETNAME_STR))	xsksetname();
		else if (match(WAIT_STR))		xwait();
#ifdef USE_SECURITY
		else if (match(SKSETKEY_STR))	xsksetkey();
#endif
#endif
#if defined(SKBASIC_EMBEDDED)
		else if (match(WAIT_STR))		xwait();
#endif
		else							ximplet();      // if no keyword, assume implied LET
/*
 *  The keyword has been processed, see what's left to do.
 */
		if (errcode) return;			// a non-zero error code is bad, leave now

		blanks();						// skip some more blanks
		if (*ibufptr == MIDEOL)			// if mid-line EOL...
		{
			xmideol();					// process the mid-line EOL
			continue;
		}

		if (*ibufptr != EOL)			// this had better be an EOL!
		{
			errcode=SYTXERR;			// oops, flag a syntax error
			return;
		}
	}									// while (*ibufptr != EOL)...
	*tbufptr = EOLTOK;					// put token eol in token buffer
	if (!immid)							// if we are tokenizing a line (not immediate command)...
	{
		tknbuf[2] = (unsigned char)((int)tbufptr-(int)tknbuf)+1;	// put line length into tokenized line
	}
	return;
}

/*
 * XLATE:    EQU    *
 *           JSR    GETCHR      ; GET NEXT CHAR.
 *           CMPA   #EOL        ; AT THE END OF THE LINE?
 *           BEQ    XLATE1      ; YES.
 *           CLR    IFWHFLAG    ; NOT XLATING "IF" OR "WHILE"
 *           JSR    BLANKS      ; SKIP BLANKS.
 *           LDX    #KEYWORDS   ; POINT TO KEYWORD TABLE.
 * XLATE4:   JSR    STREQ       ; IS KEYWORD IS IN THE INPUT BUFFER?
 *           BCS    XLATE2      ; YES GO PROCESS IT.
 * XLATE3:   INX                ; NO. POINT TO NEXT CHAR.
 *           LDAA   0,X         ; AT THE END OF THIS KEYWORD?
 *           BNE    XLATE3      ; NO.
 *           LDAB   #4          ; NUMBER OF BYTES TO SKIP.
 *           ABX
 *           TST    0,X         ; AT THE END OF THE TABLE?
 *           BNE    XLATE4      ; NO. CHCK FOR NEXT KEYWORD.
 *           LDAA   #IMLETTOK   ; ASSUME AN IMPLIED LET.
**           JSR    PUTTOK       ; PUT TOKEN IN BUFFER.
 *           LDX    #XIMPLET    ; GET ADDR OF XLATION ROUTINE.
**           JSR    0,X          ; GO DO IT.
**           BRA    XLATE6       ; GO FINISH UP.
 *           BRA    XLATE9
 * XLATE2:   LDAA   1,X         ; GET KEYWORD TOKEN.
 *           LDX    2,X         ; GET ADDR OF XLATION ROUTINE.
 * XLATE9:   JSR    PUTTOK      ; PUT TOKEN IN BUFFER.
 *           CMPA   #DATATOK    ; SPECIAL CASE, DONT SKIP BLANKS AFTER KEYWORD.
 *           BEQ    XLATE5
 *           CMPA   #REMTOK     ; SAME SPECIAL CASE AS FOR DATA.
 *           BEQ    XLATE5
 *           JSR    BLANKS      ; SKIP BLANKS BETWEEN KEYWORD & NEXT OBJECT.
 * XLATE5:   JSR    0,X         ; GO DO IT.
 * XLATE6:   JSR    BLANKS      ; SKIP BLANKS.
 *           JSR    GETNXCHR    ; GET NEXT CHAR.
 *           CMPA   #MIDEOL     ; IS IT A MID EOL?
 *           BNE    XLATE7      ; NO. CHCK FOR EOL.
 *           LDAA   #MEOLTOK    ; GET MID EOL TOKEN.
 *           JSR    PUTTOK      ; PUT IT IN BUFFER.
 *           BRA    XLATE       ; CONTINUE.
 * XLATE7:   CMPA   #EOL        ; EOL?
 *           BEQ    XLATE1      ; YES. FINISH UP.
 *           LDAA   #SYTXERR    ; NO. SYNTAX ERROR.
 *           JMP    RPTERR      ; REPORT XLATION ERROR.
 * XLATE1:   LDAA   #EOLTOK     ; GET EOL TOKEN.
 *           JSR    PUTTOK      ; PUT IT IN BUFFER.
 *           LDD    TBUFPTR     ; GET TOKEN BUFFER POINTER.
 *           SUBD   TKNBUFS     ; Compute the TOKEN BUFFER LENGTH.
 *           LDX    TKNBUFS     ; POINT TO BUFFER.
 *           STAB   2,X         ; STORE LENGTH.
 *           RTS                ; RETURN.
 *
 *
 *        KEYWORD LOOK UP TABLE
 *
 *
 * KEYWORDS: EQU    *
 * DATA:     FCC    "DATA"
 *           FCB    0
 *           FCB    DATATOK
 *           FDB    XDATA
 * LET:      FCC    "LET"
 *           FCB    0
 *           FCB    LETTOK
 *           FDB    XLET
 * READ:     FCC    "READ"
 *           FCB    0
 *           FCB    READTOK
 *           FDB    XREAD
 * RESTORE:  FCC    "RESTORE"
 *           FCB    0
 *           FCB    RESTRTOK
 *           FDB    XRESTORE
 * GOSUB:    FCC    "GOSUB"
 *           FCB    0
 *           FCB    GOSUBTOK
 *           FDB    XGOSUB
 * GOTO:     FCC    "GOTO"
 *           FCB    0
 *           FCB    GOTOTOK
 *           FDB    XGOTO
 * ONTIME:   FCC    "ONTIME"
 *           FCB    0
 *           FCB    ONTIMTOK
 *           FDB    XONTIME
 * ONIRQ:    FCC    "ONIRQ"
 *           FCB    0
 *           FCB    ONIRQTOK
 *           FDB    XONIRQ
 * ONPACC:   FCC    "ONPACC"
 *           FCB    0
 *           FCB    ONPACTOK
 *           FDB    XONPACC
 * ON:       FCC    "ON"
 *           FCB    0
 *           FCB    ONTOK
 *           FDB    XON
 * RETURN:   FCC    "RETURN"
 *           FCB    0
 *           FCB    RETNTOK
 *           FDB    XRETURN
 * IIF:      FCC    "IF"
 *           FCB    0
 *           FCB    IFTOK
 *           FDB    XIF
 * INPUT:    FCC    "INPUT"
 *           FCB    0
 *           FCB    INPUTTOK
 *           FDB    XINPUT
 * PRINT:    FCC    "PRINT"
 *           FCB    0
 *           FCB    PRINTTOK
 *           FDB    XPRINT
 *           FCC    "?"
 *           FCB    0
 *           FCB    PRINTTOK
 *           FDB    XPRINT
 * FOR:      FCC    "FOR"
 *           FCB    0
 *           FCB    FORTOK
 *           FDB    XFOR
 * NEXT:     FCC    "NEXT"
 *           FCB    0
 *           FCB    NEXTTOK
 *           FDB    XNEXT
 * STOPSS:   FCC    "STOP"
 *           FCB    0
 *           FCB    STOPTOK
 *           FDB    XSTOP
 * ENDWH:    FCC    "ENDWH"
 *           FCB    0
 *           FCB    ENDWHTOK
 *           FDB    XENDWH
 * ENDS:     FCC    "END"
 *           FCB    0
 *           FCB    ENDTOK
 *           FDB    XEND
 * REM:      FCC    "REM"
 *           FCB    0
 *           FCB    REMTOK
 *           FDB    XREM
 * TRON:     FCC    "TRON"
 *           FCB    0
 *           FCB    TRONTOK
 *           FDB    XTRON
 * TROFF:    FCC    "TROFF"
 *           FCB    0
 *           FCB    TROFFTOK
 *           FDB    XTROFF
 * WHILE:    FCC    "WHILE"
 *           FCB    0
 *           FCB    WHILETOK
 *           FDB    XWHILE
 * POKE:     FCC    "POKE"
 *           FCB    0
 *           FCB    POKETOK
 *           FDB    XPOKE
 * DIM:      FCC    "DIM"
 *           FCB    0
 *           FCB    DIMTOK
 *           FDB    XDIM
 * EEP:      FCC    "EEP"
 *           FCB    0
 *           FCB    EEPTOK
 *           FDB    XEEP
 * PORTA:    FCC    "PORTA"
 *           FCB    0
 *           FCB    PORTATOK
 *           FDB    XPORTA
 * PORTB:    FCC    "PORTB"
 *           FCB    0
 *           FCB    PORTBTOK
 *           FDB    XPORTB
 * PORTC:    FCC    "PORTC"
 *           FCB    0
 *           FCB    PORTCTOK
 *           FDB    XPORTC
 * PORTD:    FCC    "PORTD"
 *           FCB    0
 *           FCB    PORTDTOK
 *           FDB    XPORTD
 * INBYTES:  FCC    "INBYTE"
 *           FCB    0
 *           FCB    INBYTTOK
 *           FDB    XINBYTE
 * TIME:     FCC    "TIME"
 *           FCB    0
 *           FCB    TIMETOK
 *           FDB    XTIME
 * RETI:     FCC    "RETI"
 *           FCB    0
 *           FCB    RETITOK
 *           FDB    XRETI
 * PACC:     FCC    "PACC"
 *           FCB    0
 *           FCB    PACCTOK
 *           FDB    XPACC
 * SLEEP:    FCC    "SLEEP"
 *           FCB    0
 *           FCB    SLEEPTOK
 *           FDB    XSLEEP
 * RTIMES:   FCC    "RTIME"
 *           FCB    0
 *           FCB    RTIMETOK
 *           FDB    XRTIME
 *           FCB    0           ; END OF TABLE MARKER.
 */


/*
 *  blanks      count blanks in input stream, save count in token buffer
 *
 *  This routine is used to record the size of the inter-token blank fields
 *  as the input stream is processed.
 */
void  blanks(void)
{
	unsigned char					spcnt;

	spcnt = 0;
	while (*ibufptr == ' ')				// for all spaces in this field...
	{
		ibufptr++;						// move to the next character
		spcnt++;						// count this space
	}

/*
 * BLANKS:   EQU    *
 *           PSHX
 *           LDX    IBUFPTR
 *           CLRB
 * BLANKS1:  LDAA   0,X
 *           CMPA   #SPC
 *           BNE    BLANKS2
 *           INCB
 *           INX
 *           BRA    BLANKS1
 */

	if (spcnt == 0)  return;			// if nothing to record, done

/*
 * BLANKS2:  TSTB
 *           BNE    BLANKS3
 *           PULX
 *           RTS
 */

	if (spcnt > 1)						// if need to compress the field...
	{
		*tbufptr++ = MSCNTOK;			// use the multi-space compression token
		*tbufptr++ = spcnt;				// now embed the space count
	}

/*
 * BLANKS3:  STX    IBUFPTR
 *           LDX    TBUFPTR
 *           CMPB   #1
 *           BEQ    BLANKS4
 *           LDAA   #MSCNTOK
 *           STAA   0,X
 *           INX
 * BLANKS5:  STAB   0,X
 *           INX
 *           STX    TBUFPTR
 *           PULX
 *           RTS
 */

	else								// only a single space in the input stream
	{
		*tbufptr++=SSCNTOK;
	}
	return;
}


/*
 * BLANKS4:  LDAB   #SSCNTOK
 *           BRA    BLANKS5
 */



/*
 *  match      compare string in arg lit to string in ibufptr
 */
/*
 *************************************
 *             match()
 * try to find match between *lit and
 * *ibufptr. if match found, ibufptr is
 * advanced to point beyond *lit. the
 * string pointed to by lit must be null
 * terminated.
 **************************************
 */

int  match(char  *lit)
{
	int					k;					// holds length if match

	k = 0;									// assume this fails
	if (streq(ibufptr, (unsigned char *)lit))	// if string in lit matches input buffer...
	{
		k = strlen(lit);					// get the count of matching characters
		ibufptr += k;						// increment buffer pointer past keyword
		return  k;							// return length of matching string
	}
	return  k;								// return 0 if no match, else length
}

	
/* int k;
 * if(k=streq(ibufptr,lit))
 * {
 *  ibufptr+=k;
 *  return(1);
 * }
 * return(0);
 *}
 */


          
/*
 ***************************************
 *               streq()
 * compare str1 to str2. str2 must be null
 * terminated.
 *
 ****************************************
 */
int  streq(unsigned char  *str1, unsigned char  *str2)
{
	int					k;

	k = 0;
	while (str2[k])         // we're not at the end of string2
	{
		if ((toupper(str1[k])) != toupper((str2[k]))) return(0);		// case-insensitive compare
		k++;
	}
	return  k;
}
  
/*
 * STREQ:    EQU    *
 *           LDD    IBUFPTR     ; SAVE VALUE OF POINTER.
**           PSHD
 *           PSHB
 *           PSHA
 * STREQU4:  LDAA   0,X
 *           BEQ    STREQU2
 * STREQU1:  BSR    GETNXCHR
 *           jsr    ToUpper      ; Make the character upper case.
 *           CMPA   0,X
 *           BEQ    STREQU3
**           PULD
 *           PULA
 *           PULB
 *           STD    IBUFPTR
 *           CLC
 *           RTS
 * STREQU3:  INX
 *           BRA    STREQU4
 * STREQU2:  PULA
 *           PULB
 *           SEC
 *           RTS
 */




/*
 *  getchr      get next char from input buffer
 */
unsigned char  getchr(void)
{
	return  *ibufptr;
}

/*
 * GETCHR      THIS ROUTINE GETS THE NEXT CHARACTER FROM THE INPUT BUFFER.
 *
 *
 * GETCHR:   PSHX               ; SAVE THE X REGISTER.
 *           LDX    IBUFPTR     ; GET POINTER.
 *           LDAA   0,X         ; GET A CHARACTER.
 *           PULX               ; RESTORE X.
 *           RTS                ; RETURN.
 *
 * ------------------------------
 *
 *        THIS ROUTINE GETS THE NEXT CHARACTER FROM THE INPUT BUFFER
 *        AND ADVANCES THE POINTER TO POINT TO THE NEXT CHARACTER.
 *
 *
 * GETNXCHR: BSR    GETCHR
 *                       FALL THROUGH TO INCIBP.
 */

  

/*
 *  incibp      increment the input buffer pointer
 */
void  incibp(void)
{
	ibufptr++;
}

/*
 * INCIBP       THIS ROUTINE JUST INCREMENTS THE INPUT BUFFER POINTER.
 *
 *
 * INCIBP:   PSHX               ; SAVE X.
 *           LDX    IBUFPTR     ; GET POINTER.
 * INCIBP1:  INX                ; ADVANCE POINTER.
 *           STX    IBUFPTR     ; UPDATE POINTER.
 * INCIBP2:  PULX               ; RESTORE X
 *           RTS                ; RETURN.
 */






/*
 *  putint32      put 32-bit integer into token buffer and advance buffer pointer
 */
 void  putint32(I32  itok)
 {
//	*(I32 *)tbufptr = itok;					// save the value
	setU32((U32 *)tbufptr, itok);					// save the value
	tbufptr = tbufptr + sizeof(I32);		// move past the value
//	 *tbufptr = itok>>8;
//	 tbufptr++;
//	 *tbufptr = itok&0xff;
//	 tbufptr++;
 }

/* --------------------------------
 *
 *        THIS ROUTINE PUTS THE WORD IN THE D-REG. INTO THE TOKEN BUFFER
 *        AND ADVANCES THE TOKEN BUFFER POINTER.
 *
 *
 * PUTDTOK:  BSR    PUTTOK      ; PUT THE FIRST BYTE INTO THE TOKEN BUFFER.
 *           TBA                ; PUT THE 2ND BYTE INTO A.
 *                            ; FALL THROUGH TO PUTTOK.
 */



/*
 *  puttok      put 8-bit token into token buffer and advance token pointer
 */
void  puttok(U8  b)
{
	if (tbufptr == (tknbufs + TBUFLEN))		// if we exceeded the size of the token buffer...
	{
		errcode = EXPCXERR;					// show expression is too complex
		return;
	}
	*tbufptr = b;
	tbufptr++;
}

/*----------------------------------
 * 
 *        THIS ROUTINE PUTS THE CHARACTER IN THE A-REG. INTO THE TOKEN
 *        BUFFER AND ADVANCES THE TOKEN BUFFER POINTER.
 *
 *
 * PUTTOK:   PSHX               ; SAVE X.
 *           pshb
 *           psha                                              ; (9/12/89).
 *           LDX    TBUFPTR     ; GET POINTER.
 *           STAA   0,X         ; PUT CHARACTER.
 * PUTTOK1:  INX                ; ADVANCE POINTER.
 *           STX    TBUFPTR     ; SAVE POINTER.
 *            
 *           LDD    TKNBUFS     ; get the starting address of the token buffer.
 *           ADDD   #TBUFLEN    ; add the length of the buffer to it.
 *           CPD    TBUFPTR     ; IS THE TOKEN BUFFER FULL?
 *           pula                                              ; (9/12/89).
 *           pulb               ; restore the b reg.
 *           BHI    INCIBP2     ; NO. RESTORE X AND RETURN.
 *           LDAA   #EXPCXERR   ; YES. FLAG THE ERROR.
 *           JMP    RPTERR      ; GO REPORT IT.
 */
