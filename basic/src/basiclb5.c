/*
 *  basiclb5      part of the Basic11 compiler
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
static void						_xeep(U8  tok);





/*
 * getfun      process a function token
 */
U8  getfun(void)
{
	U8						type;
	unsigned char			*fptr;

	fptr = tbufptr;					// record tbufptr, in case this is not a legal function call
	*tbufptr = FUNCTFLG;			// this may not be necessary, depends on if the function is legal
	tbufptr++;						// a function call will write a subcode to this cell

//	if      (match("FDIV"))		type = (U8)xfdiv();
	if	    (match("CHR$"))		type = (U8)xchrs();
	else if	(match(ABS_STR))	type = (U8)xabs();
	else if (match(SQRT_STR))	type = (U8)xsqrt();
	else if	(match("RND"))		type = (U8)xrnd();
	else if	(match("SGN"))		type = (U8)xsgn();
	else if	(match("TAB"))		type = (U8)xtab();
//	else if	(match("ADC"))		type = (U8)xadc();	// removed, handled by targetxxx.c
	else if (match(HEX4_STR))	type = (U8)xhex4();		// check for hex4() before hex()!
	else if (match(HEX2_STR))	type = (U8)xhex2();		// check for hex2() before hex()!
	else if (match(HEX_STR))	type = (U8)xhex();
	else if (match(PEEK16_STR))	type = (U8)xpeek16();
	else if (match(PEEK32_STR))	type = (U8)xpeek32();
	else if (match(PEEK_STR))	type = (U8)xpeek();
	else if (match(ADDR_STR))	type = (U8)xaddr();
	else if (match(FEEP16_STR))	type = (U8)xfeep16();
	else if (match(FEEP32_STR))	type = (U8)xfeep32();
	else if (match(FEEP_STR))	type = (U8)xfeep();
//	else if	(match("CALL"))		type = (U8)xcall();
	else if (match(TIMESTR_STR))	type = (U8)fxtimestr();
	else if (match(STRCMP_STR))	type = (U8)xfstrcmp();
	else if (match(STRIND_STR))	type = (U8)xfstrind();
	else if (match(STRLEN_STR))	type = (U8)xfstrlen();
#if defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	else if (match(SKSEND_STR))		type = (U8)xsksend();
	else if (match(SKFLASH_STR))	type = (U8)xskflash();
	else if (match(SKBC_STR))		type = (U8)xskbc();
	else if (match(SKSYNC_STR))		type = (U8)xsksync();
	else if (match(SKINQ_STR))		type = (U8)xskinq();
	else if (match(SKPAIR_STR))		type = (U8)xskpair();
	else if (match(SKUNPAIR_STR))	type = (U8)xskunpair();
	else if (match(SKLKUP_STR))		type = (U8)xsklkup();
	else if (match(SKREVLKUP_STR))	type = (U8)xskrevlkup();
#endif
	else							// not a legal function call...
	{
		tbufptr = fptr;				// restore tbufptr
		return(0);
	}
	return (type);
}

/*
 * GETFUN:   EQU    *
 *           LDX    #FUNCTBL
 * GETFUN1:  JSR    STREQ
 *           BCS    GETFUN2
 * GETFUN3:  INX
 *           LDAA   0,X
 *           BNE    GETFUN3
 *           LDAB   #4
 *           ABX
 *           TST    0,X
 *           BNE    GETFUN1
 *           CLRA
 *           RTS
 * GETFUN2:  LDAA   #FUNCTFLG
 *           JSR    PUTTOK
 *           LDAA   1,X
 *           LDX    2,X
 *           JMP    0,X
 * *
 * *
 * FUNCTBL:  EQU    *
 * FDIVS:    FCC    "FDIV"
 *           FCB    0
 *           FCB    FDIVTOK
 *           FDB    BNUMFUN
 * CHRS:     FCC    "CHR$"
 *           FCB    0
 *           FCB    CHRTOK
 *           FDB    UNUMFUN
 * ABS:      FCC    "ABS"
 *           FCB    0
 *           FCB    ABSTOK
 *           FDB    UNUMFUN
 * RND:      FCC    "RND"
 *           FCB    0
 *           FCB    RNDTOK
 *           FDB    UNUMFUN
 * SGN:      FCC    "SGN"
 *           FCB    0
 *           FCB    SGNTOK
 *           FDB    UNUMFUN
 * TABS:     FCC    "TAB"
 *           FCB    0
 *           FCB    TABTOK
 *           FDB    UNUMFUN
 * ADCS:     FCC    "ADC"
 *           FCB    0
 *           FCB    ADCTOK
 *           FDB    UNUMFUN
 * CALL:     FCC    "CALL"
 *           FCB    0
 *           FCB    CALLTOK
 *           FDB    UNUMFUN
 * PEEK:     FCC    "PEEK"
 *           FCB    0
 *           FCB    PEEKTOK
 *           FDB    UNUMFUN
 *           FCC    "EEP"
 *           FCB    0
 *           FCB    FEEPTOK
 *           FDB    UNUMFUN
 * HEX2:     FCC    "HEX2"
 *           FCB    0
 *           FCB    HEX2TOK
 *           FDB    UNUMFUN
 * HEX:      FCC    "HEX"
 *           FCB    0
 *           FCB    HEXTOK
 *           FDB    UNUMFUN
 *           FCC    "PORT"
 *           FCB    0
 *           FCB    FPRTATOK
 *           FDB    FINDPORT
 *           FCC    "TIME"
 *           FCB    0
 *           FCB    FTIMETOK
 *           FDB    XTIMEF
 *           FCC    "PACC"
 *           FCB    0
 *           FCB    FPACCTOK
 *           FDB    XPACCF
 *           FCB    0            ; END OF TABLE MARKER.
 * *
 * *
 * XPOKE:    EQU    *
 *           LDX    TBUFPTR      ; GET TOKEN BUFFER POINTER.
 *           DEX                 ; DEC. TO COMPENSATE FOR PUTTOK DONE IN XLATE.
 *           STX    TBUFPTR      ; SAVE NEW POINTER VALUE. FALL THROUGH TO BNUMFUN.
 *           LDAA   0,X          ; GET TOKEN BACK INTO THE A-REG.
 * *
 * *
 */

/*
 *  xfdiv      process FP division
 */
 /*
int  xfdiv(void)
{
	unsigned char					type[2];

	type[0] = NUM;
	type[1] = NUM;				// both arguments must be type NUM
	dofunct(FDIVTOK, 2, type);
	return (NUM);
}
*/

/*
 * BNUMFUN:  EQU    *
 *           PSHY
 *           LDAB   #NUM
 *           PSHB
 *           PSHB
 *           TSY
 *           LDAB   #2
 *           JSR    DOFUNCT
 * *        LDAA   #NUM
 *           PULA
 *           PULA
 *           PULY
 *           RTS
 */


/***** xchrs *****/
int  xchrs(void)
{
	return(unumfun(CHRTOK));
}

/***** xabs() *****/
int  xabs(void)
{
	return(unumfun(ABSTOK));
}

/***** xrnd() *****/
int  xrnd(void)
{
	return(unumfun(RNDTOK));
}

/***** xsgn() *****/
int  xsgn(void)
{
	return(unumfun(SGNTOK));
}

/***** xtab() *****/
int  xtab(void)
{
	return(unumfun(TABTOK));
}

/***** xadc() *****/
//int  xadc(void)
//{
//	return(unumfun(ADCTOK));
//}

/***** xcall() *****/
U8  fxcall(void)
{
	return(unumfun(CALLTOK));
}



U8  xaddr(void)
{
	return(unumfun(ADDRTOK));
}


int  xsqrt(void)
{
	return(unumfun(SQRTTOK));
}


U8  fxtimestr(void)
{
	return(unumfun(TIMESTRTOK));
}




/*
 *  xhex
 */
U8  xhex(void)
{
	return(unumfun(HEXTOK));
}


/*
 *  xhex2
 */
U8  xhex2(void)
{
	return(unumfun(HEX2TOK));
}


/*
 *  xhex4
 */
U8  xhex4(void)
{
	return(unumfun(HEX4TOK));
}



/*
 *  xpeek
 */
U8  xpeek(void)
{
	return(unumfun(PEEKTOK));
}



/*
 *  xpeek16
 */
U8  xpeek16(void)
{
	return(unumfun(PEEK16TOK));
}



/*
 *  xpeek32
 */
U8  xpeek32(void)
{
	return(unumfun(PEEK32TOK));
}



/*
 *  xfeep      read byte from EEPROM
 */
U8  xfeep(void)
{
	return(unumfun(FEEPTOK));
}



/*
 *  xfeep16      read word from EEPROM
 */
U8  xfeep16(void)
{
	return(unumfun(FEEP16TOK));
}


/*
 *  xfeep32      read double word from EEPROM
 */
U8  xfeep32(void)
{
	return(unumfun(FEEP32TOK));
}





/*
 *  xeep      write byte to EEPROM
 */
void  xeep(void)
{
	_xeep(EEPTOK);
}


/*
 *  xeep16      write word to EEPROM
 */
void  xeep16(void)
{
	_xeep(EEP16TOK);
}


/*
 *  xeep32      write double word to EEPROM
 */
void  xeep32(void)
{
	_xeep(EEP32TOK);
}



/*
 *  _xeep      write data to EEPROM
 */
static void  _xeep(U8  tok)
{
	unumfun(tok);

/*
 *  WARNING:  The remaining code is a duplicate of a section of asignmt().
 *  Gordon's original design simply jumped into the body of asignmt(), which
 *  I obviously can't do.  So I copied the necessary code to here.
 *
 *  If you need to change asignmt(), be sure to make any necessary changes
 *  here as well.
 */
	blanks();						// allow spaces before '='
	if (*ibufptr++ != '=')			// if no equals sign...
	{
		errcode = IVEXPERR;			// invalid expression
		return;
	}
	*tbufptr++ = EQUALTOK;			// put equals token in buffer
	xexpres(NUM);					// build numeric expression in token buffer
	return;
}

void			xstrcat(void)
{
	unsigned char					type[2];		// need an array for dofunct()
	type[0] = STRING;
	type[1] = STRING;
	dofunct(STRCATTOK, 2, type);			// go do the function
	return;
}

U8				xfstrcmp(void)
{
	unsigned char					type[2];		// need an array for dofunct()
	type[0] = STRING;
	type[1] = STRING;
	dofunct(FSTRCMPTOK, 2, type);			// go do the function
	return(NUM);						// return the function type
}

void			xstrind(void)
{
	unsigned char					type[2];		// need an array for dofunct()
	type[0] = STRING;
	type[1] = NUM;
	dofunct(STRINDTOK, 2, type);			// go do the function
	blanks();						// allow spaces before '='
	if (*ibufptr++ != '=')			// if no equals sign...
	{
		errcode = IVEXPERR;			// invalid expression
		return;
	}
	*tbufptr++ = EQUALTOK;			// put equals token in buffer
	xexpres(NUM);					// build numeric expression in token buffer
	return;
}

U8				xfstrind(void)
{
	unsigned char					type[2];		// need an array for dofunct()
	type[0] = STRING;
	type[1] = NUM;
	dofunct(FSTRINDTOK, 2, type);			// go do the function
	return(NUM);						// return the function type
}

U8				xfstrlen(void)
{
	unsigned char					type[1];		// need an array for dofunct()
	type[0] = STRING;
	dofunct(FSTRLENTOK, 1, type);			// go do the function
	return(NUM);						// return the function type
}


/*
 *  unumfun      common code for a uniary numerical function
 */
U8  unumfun(unsigned char  token)
{
	unsigned char					type[1];		// need an array for dofunct()

	type[0] = NUM;						// set the 1st (only) argument type to NUM
	dofunct(token, 1, type);			// go do the function
	return(NUM);						// return the function type
}

/*
 * XEEP:     EQU    *            ; PROGRAM A WORD OF EEPROM.
 *           LDX    TBUFPTR      ; COMPENSATE FOR TOKEN PLACEMENT BU UNUMFUN
 *           DEX                 ; ROUTINE.
 *           STX    TBUFPTR      ; SAVE POINTER.
 *           LDAA   0,X          ; GET TOKEN FROM BUFFER.
 *           BSR    UNUMFUN      ; GO TREAT AS A UNIARY NUMERIC FUNCTION.
 *           JMP    ASIGNMT1     ; GO USE ASSIGNMENT CODE FOR REST OF FUNCTION.
 * *
 *
 * *
 * UNUMFUN:  EQU    *
 *           PSHY
 *           LDAB   #NUM
 *           PSHB
 *           LDAB   #1
 *           TSY
 *           BSR    DOFUNCT
 * *        LDAA   #NUM
 *           PULA
 *           PULY
 *           RTS
 */

/*
 *  dofunct      general-purpose function handler
 */
void  dofunct(unsigned char  functok, int  nargs, unsigned char  *type)
{
	*tbufptr++ = functok;				// put function token in buffer
	if(*ibufptr != '(')					// must find opening paren
	{ 
//		errcode = ILFSYERR;				// oops, that's an error
		errcode = SYTXERR;				// oops, that's an error
		return;							// leave now
	}
	*tbufptr++ = OPARNTOK;				// put open paren in token buffer
	++ibufptr;

/*
 * DOFUNCT:  EQU    *
 *           JSR    PUTTOK
 *           JSR    GETCHR
 *           CMPA   #'('
 *           BEQ    DOFUNCT1
 * DOFUNCT5: LDAA   #MPARNERR
 *           JMP    RPTERR
 * DOFUNCT1: JSR    INCIBP
 *           LDAA   #OPARNTOK
 *           JSR    PUTTOK
 */

	while(1)
	{
		xexpres(*type++);					// get the argument/expression
		if (errcode) return;				// return if error
		if (--nargs == 0) break;			// if we have all the arguments, quit
		if (*ibufptr != ',')				// if delimiter not present, return
		{
//			errcode = ILFSYERR;
			errcode = SYTXERR;				// oops, that's an error
			return;
		}
		*tbufptr++ = COMMATOK;				// if it is, put it in the token buffer
		++ibufptr;							// point to the next character
	}

/*
 * DOFUNCT4: LDAA   0,Y
 *           INY
 *           PSHB
 *           JSR    XEXPRES
 *           PULB
 *           DECB
 *           BEQ    DOFUNCT3
 *           JSR    CHKCOMA
 *           BCC    DOFUNCT5
 *           BRA    DOFUNCT4
 */

	if (*ibufptr != ')')				// must see closing paren
	{
//		errcode = ILFSYERR;				// if not, error
		errcode = MPARNERR;				// if not, error
		return;
	}
	else								// saw closing paren
	{
		*tbufptr++ = CPARNTOK;			// put it in the token buffer
		++ibufptr;						// advance input buffer pointer
	}
	return;
}

/*
 * DOFUNCT3: EQU    *
 *           JSR    GETCHR
 *           CMPA   #')'
 *           BNE    DOFUNCT5
 *           JSR    INCIBP
 *           LDAA   #CPARNTOK
 *           JMP    PUTTOK       ; PUT TOKEN IN BUFFER & RETURN.
 * *
 * *
 * *
 * FINDPORT: EQU    *
 *           JSR    GETNXCHR     ; GO GET PORT "NUMBER".
 *           JSR    ToUpper      ; Translate the character to upper case.
 *           CMPA   #'A'         ; IS IT AN A OR HIGHER?
 *           BHS    FINDPRT1     ; YES. GO CHECK UPPER LIMIT.
 * FINDPRT2: LDAA   #ILPRTERR    ; NO. ILLEGAL PORT "NUMBER".
 *           JMP    RPTERR       ; REPORT ERROR.
 * FINDPRT1: CMPA   #'E'         ; IS IT HIGHER THAN AN "E"?
 *           BHI    FINDPRT2     ; YES. ILLEGAL PORT.
 *           SUBA   #'A'         ; SUBTRACT "BASE" PORT OF A
 *           ADDA   #FPRTATOK    ; ADD IN "BASE" TOKEN.
 * *
 * *                            ; STEAL SOME CODE.
 * XPACCF:   EQU    *
 * XTIMEF:   JSR    PUTTOK       ; PUT TOKEN IN BUFFER.
 *           LDAA   #NUM         ; RETURN TYPE "NUM".
 *           RTS                 ; RETURN.
 * *
 */



