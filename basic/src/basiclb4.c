/*
 *  basiclb4      part of the Basic11 source files
 */

#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>

#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"




/*
 *  xon      process the ON token
 */

void  xon(void)
{
	I16						num;

	*tbufptr++ = ONTOK;					// put ON token in buffer
	blanks();							// skip any blanks
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	if(match(RXDATA_STR)){
		*tbufptr++ = RXDATATOK;
		blanks();							// skip any blanks
		if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;				// if error, return
	}else if(match(ACK_STR)){
		*tbufptr++ = ACKTOK;
		blanks();							// skip any blanks
		if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;				// if error, return
	}else if(match(EVENT_STR)){
		*tbufptr++ = EVENTTOK;
		blanks();							// skip any blanks
		if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;				// if error, return
	}else{
#elif	defined(SKBASIC_EMBEDDED)
	if(match(TIM1_STR)){
		*tbufptr++ = TIM1OK;
		blanks();							// skip any blanks
		if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;
	} else 	if(match(TIM2_STR)){
		*tbufptr++ = TIM2OK;
		blanks();							// skip any blanks
		if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;
	} else {
#else
	{
#endif
		xexpres(NUM);						// get the expression
		if (errcode) return;				// if error, return
		blanks();							// skip any blanks
		if      (match("GOTO"))  xgoto();	// check for GOTO
		else if (match("GOSUB"))  xgosub(); // check for GOSUB
		else     errcode = IONSYERR;		// if neither, flag an error
		if (errcode) return;				// if error, return
		blanks();							// skip blanks

	/*
	 * XON:      EQU    *
	 * *        JSR    BLANKS
	 *           LDAA   #NUM
	 *           JSR    XEXPRES
	 * XON1:     JSR    BLANKS
	 *           LDX    #GOTO
	 *           JSR    STREQ
	 *           BCC    XON2
	 *           LDAA   #GOTOTOK
	 *           JSR    PUTTOK
	 *           JSR    BLANKS
	 *           JSR    XGOTO
	 *           BRA    XON5
	 * XON2:     LDX    #GOSUB
	 *           JSR    STREQ
	 *           BCS    XON4
	 *           LDAA   #IONSYERR
	 *           JMP    RPTERR
	 * XON4:     LDAA   #GOSUBTOK
	 *           JSR    PUTTOK
	 *           JSR    BLANKS
	 *           JSR    XGOSUB
	 * XON5:     JSR BLANKS
	 */


		while (*ibufptr == ',')				// for all arguments (comma-separated list)...
		{
			*tbufptr++ = COMMATOK;			// put COMMA token in buffer
			++ibufptr;						// advance the input buffer pointer
			blanks();						// skip blanks
			*tbufptr++ = LCONTOK;			// put line number constant token in buffer
			num = getlinum();				// get line number
			if (num == 0) errcode = LINENERR;   // if 0, line number error
			if (errcode) return;			// if error, return
			putlinum(num);					// put line number in buffer
			blanks();						// skip blanks
		}
	}
	return;
}

/* 
 *           JSR    CHKCOMA
 *           BCS    XON6
 *           RTS
 * XON6:     JSR    BLANKS
 *           LDAA   #LCONTOK
 *           JSR    PUTTOK
 *           JSR    GETLINUM
 * XON8:     JSR    PUTDTOK
 *           BRA    XON5
 * *
 * *
 * XONIRQ:   EQU    *            ; "ONIRQ" HAS A FUNCTION CODE & LINE NUMBER.
 * XONTIME:  EQU    *
 *           LDAA   #NUM         ; GO GET THE VALUE OF THE TIMER WE SHOULD GO ON.
 *           JSR    XEXPRES
 *           JSR    BLANKS       ; SKIP BLANKS.
 *           JSR    CHKCOMA      ; GO CHECK FOR A COMMA.
 *           BCS    XONTIME1     ; IF PRESENT, IT'S OK.
 * XONTIME2: LDAA   #MCOMAERR    ; IF NOT, REPORT AN ERROR.
 *           JMP    RPTERR
 * XONTIME1: JSR    BLANKS
 *           LDAA   #LCONTOK     ; GET THE LINE CONSTANT TOKEN.
 *           JSR    PUTTOK       ; PUT IT IN THE TOKEN BUFFER.
 *           JSR    GETLINUM     ; GO GET A LINE NUMBER.
 *           JMP    PUTDTOK      ; PUT THE LINE NUMBER IN THE TOKEN BUFFER.
 * *
 * *
 * XONPACC:  EQU    *
 *           BSR    GETARG       ; GET AN ARGUMENT AND A COMMA.
 *           BRA    XONTIME      ; GO USE SOME OTHER CODE.
 * *
 * *
 * GETARG:   LDAA   #NUM         ; GO GET THE "OPERATING MODE" EXPRESSION.
 *           JSR    XEXPRES
 *           JSR    BLANKS       ; SKIP BLANKS.
 *           JSR    CHKCOMA      ; GO CHECK FOR COMMA.
 *           BCC    XONTIME2     ; NO COMMA. REPORT ERROR.
 *           JMP    BLANKS       ; SKIP BLANKS AFTER COMMA AND RETURN.
 */


/*
 *  xif      process the IF statement
 */

void  xif(void)
{
	U16						num;
//	unsigned char			*elseoffptr;

	*tbufptr++ = IFTOK;					// put if token in the buffer
	blanks();							// skip any blanks
	ifwhflag++;							// let xexpres() know we are doing an IF
	xexpres(NUM);						// get relational expression
	if (errcode) return;				// if error, return
	blanks();							// if not, skip blanks

/*
 * XIF:      EQU    *
 * *        JSR    BLANKS
 *           INC    IFWHFLAG
 *           LDAA   #NUM
 *           JSR    XEXPRES
 * XIF1:     JSR    BLANKS
 */


	if (match(THEN_STR))				// check for "THEN" clause
	{
		*tbufptr++ = THENTOK;			// put THEN token in the buffer
//		elseoffptr = tbufptr;			// record addr for storing offset to ELSE later
//		tbufptr++;						// skip over cell for offset
		blanks();						// skip any blanks after "THEN"

/*
 *           LDX    #THENS
 *           JSR    STREQ
 *           BCS    XIF2
 *           LDAA   #MTHENERR
 *           JMP    RPTERR
 */

//	if (numeric(*ibufptr))					// if a line number is present after THEN...
		if (isdigit(*ibufptr))					// if a line number is present after THEN...
		{
			*tbufptr++ = LCONTOK;				// put line # const. token in buffer
			num = getlinum();					// get the line #
			if (num == 0) errcode = LINENERR;	// no line number is an error
			if (errcode) return;				// if error, return
			putint16(num);						// put number in buffer
		}
		else if (*ibufptr == '"')
		{
			getcon();							// get label string
		}
		else									// not a line #, check for statement
		{
			xlate();							// try to make a statement out of what follows
			if (errcode) return;				// if error, return
		}
	}

/*
 * XIF2:     EQU    *
 *           LDAA   #THENTOK
 *           JSR    PUTTOK
 *           JSR    BLANKS
 *           JSR    GETCHR
 *           JSR    NUMERIC
 *           BCC    XIF9
 * *
 * XIF3:     LDAA   #LCONTOK
 *           JSR    PUTTOK
 *           JSR    GETLINUM
 * XIF6:     JSR    PUTDTOK
 * *
 */

	else						// if "THEN" not present
	{
		errcode = MTHENERR;		// flag a missing THEN error
		return;
	}

	blanks();					// skip any blanks after object of THEN
//	*elseoffptr = tbufptr - elseoffptr;		// update offset from THEN clause to ELSE or EOL token
	if (match(ELSE_STR))		// is "ELSE" clause present?
	{                           // yes
		*tbufptr++ = ELSETOK;   // put ELSE token in the buffer
//		elseoffptr = tbufptr;	// reuse elseoffptr to hold offset to EOL
//		tbufptr++;				// leave room for the offset later
		blanks();               // skip any blanks after ELSE

/*
 * XIF4:     EQU    *
 *           JSR    BLANKS
 *           LDX    #ELSES
 *           JSR    STREQ
 *           BCS    XIF7
 *           RTS
 * XIF7:     LDAA   #ELSETOK
 *           JSR    PUTTOK
 *           JSR    BLANKS
 */

//	if (numeric(*ibufptr))			// if a line # is present after ELSE...
		if (isdigit(*ibufptr))			// if a line # is present after ELSE...
		{
			*tbufptr++=LCONTOK;			// put line # const. token in buffer
			num = getlinum();			// get the line #
			if (num == 0) errcode = LINENERR;
			if (errcode) return;		// if error, return
			putint16(num);				// put number in buffer
		}
		else if (*ibufptr == '"')
		{
			getcon();							// get label string
		}
		else							// line # not present, try to xlate a statement
		{
			xlate();
		}
//		*elseoffptr = tbufptr - elseoffptr;		// update offset from ELSE clause to EOL token
	}

/*
 *           JSR    GETCHR
 *           JSR    NUMERIC
 *           BCS    XIF8
 * XIF9:     LDAA   #LINENERR
 *           JMP    RPTERR
 * XIF8:     LDAA   #LCONTOK
 *           JSR    PUTTOK
 *           JSR    GETLINUM
 * XIF10:    JMP    PUTDTOK
 */

	return;                // in any case, return
}

/*
 * THENS:    FCC    "THEN"
 *           FCB    0
 * ELSES:    FCC    "ELSE"
 *           FCB   0
 */


/*
 *  xfor      process the FOR statement
 */
void  xfor(void)
{
	char					type;

	forflag = TRUE;						// show we are processing a FOR statement
	*tbufptr++ = FORTOK;				// put for token in buffer
	blanks();							// skip blanks between FOR & assignment statement
	type = (char)getvar();					// get variable
	blanks();							// skip any blanks after the variable
	if ((type != NUM) || (*ibufptr++ != '='))	// if this is not a numerical variable...
	{
		errcode = IVEXPERR;				// show invalid expression and leave
		return;
	}

/*
 * XFOR:     EQU    *
 * *        JSR    BLANKS
 *           JSR    GETVAR
 *           CMPA   #NUM
 *           BEQ    XFOR1
 * XFOR2:    LDAA   #IVEXPERR
 *           JMP    RPTERR
 * XFOR1:    JSR    GETNXCHR
 *           CMPA   #'='
 *           BNE    XFOR2
 */

	*tbufptr++ = EQUALTOK;				// put equals token in buffer
	xexpres(NUM);						// go get a numerical expression
	if (errcode) return;				// if error, return
	blanks();							// skip blanks

/* 
 *           LDAA   #EQUALTOK
 *           JSR    PUTTOK
 *           LDAA   #NUM
 *           JSR    XEXPRES
 *           JSR    BLANKS
 */

	if (match(TO_STR))					// if TO is present...
	{
		*tbufptr++ = TOTOK;				// put TO token in buffer
		blanks();						// skip blanks
		xexpres(NUM);					// get the "TO" expression
		if (errcode) return;			// return if error
	}
	else								// "TO" not present
	{
		errcode=MTOERR;					// set error flag & return
		return;
	}

/*
 *           LDX    #TO
 *           JSR    STREQ
 *           BCS    XFOR4
 *           LDAA   #MTOERR
 *           JMP    RPTERR
 * XFOR4:    LDAA   #TOTOK
 *           JSR    PUTTOK
 *           JSR    BLANKS
 *           LDAA   #NUM
 *           JSR    XEXPRES
 */

	blanks();						// skip blanks
	if (match("STEP"))				// is optional "STEP" clause present?
	{
		*tbufptr++ = STEPTOK;		// put STEP token in buffer
		blanks();					// skip blanks
		xexpres(NUM);				// get expression
	}
	forflag = FALSE;				// done processing a FOR statement
	return;
}

/*
 *           JSR    BLANKS
 *           LDX    #STEP
 *           JSR    STREQ
 *           BCS    XFOR3
 *           RTS
 * XFOR3:    LDAA   #STEPTOK
 *           JSR    PUTTOK
 *           JSR    BLANKS
 *           LDAA   #NUM
 *           JMP    XEXPRES
 *
 * *
 * TO:       FCC    "TO"
 *           FCB    0
 * STEP:     FCC    "STEP"
 *           FCB    0
 * *
 */

/*
 *  xnext      process the NEXT token
 */
void  xnext(void)
{
	*tbufptr++ = NEXTTOK;			// put NEXT token in buffer
	blanks();						// skip blanks
	if (getvar() != NUM) errcode = SYTXERR;		// get variable, must be numeric
	return;
}

/*
 * XNEXT:    EQU    *
 * *        JSR    BLANKS
 *           JSR    GETVAR
 *           CMPA   #NUM
 *           BEQ    XNEXT1
 *           LDAA   #SYTXERR
 *           JMP    RPTERR
 * XNEXT1:   RTS
 */

/*
 *  xprint      process the PRINT token
 */
void  xprint(void)
{
	_xprint(PRINTTOK);				// process the print operation, using PRINT token
}



/*
 *  xqprint      process the QUICK_PRINT (?) token
 */
void  xqprint(void)
{
	_xprint(QPRINTTOK);
}



/*
 *  _xprint      low-level routine for processing the PRINT and QPRINT tokens
 */
void  _xprint(unsigned char  tok)
{
	*tbufptr++ = tok;				// put token in buffer
	blanks();						// skip blanks

/*
 * XPRINT:   EQU    *
 * *        JSR    BLANKS
 *           JSR    GETCHR
 *           CMPA   #'#'       ; HAS AN ALTERNATE PORT BEEN SPECIFIED?
 *           BNE    XPRINT9    ; NO. GO PROCESS THE REST OF THE PRINT STATEMENT.
 *           LDAA   #PNUMTOK   ; YES. PUT THE TOKEN INTO THE BUFFER.
 *           JSR    PUTTOK     ; DO IT.
 *           JSR    INCIBP     ; POINT PAST THE "#".
 *           JSR    BLANKS     ; SKIP SPACES BETWEEN '#' AND EXPRESION.
 *           BRA    XPRINT7    ; GO GET EXPRESSION & CONTINUE.
 * *
 */

          
	while((*ibufptr!=EOL) && (*ibufptr!=MIDEOL))    // do until end of line
	{
		xexpres(0);						// get expression
		if (errcode) return;			// if error, return
		blanks();						// skip blanks
		if      (*ibufptr==COMMA) *tbufptr = COMMATOK;        // check for comma
		else if (*ibufptr==SEMI) *tbufptr = SEMITOK;     // check for semicolin
		else    return;					// if neither, return
		++ibufptr;						// advance input buffer pointer
		++tbufptr;						// advance token buffer pointer
		blanks();						// skip blanks after delimeter
	}
	return;
}

/*
 * XPRINT9:  JSR    BLANKS
 *           JSR    GETCHR
 *           CMPA   #EOL
 *           BEQ    XPRINT2
 *           CMPA   #MIDEOL
 *           BNE    XPRINT3
 * XPRINT2:  RTS
 * XPRINT3:  JSR    GETCHR       ; GET THE NEXT CHARACTER IN THE BUFFER.
 *           CMPA   #'"'         ; IS IT A STRING CONSTANT?
 *           BNE    XPRINT7
 *           JSR    GETSCON      ; YES. GO GET A STRING CONSTANT.
 *           BRA    XPRINT8      ; CONTINUE.
 * XPRINT7:  LDAA   #NUM
 *           JSR    XEXPRES
 * XPRINT8:  JSR    BLANKS
 *           JSR    GETCHR
 *           CMPA   #EOL
 *           BEQ    XPRINT2
 *           CMPA   #MIDEOL
 *           BEQ    XPRINT2
 *           BSR    CHKCOMA
 *           BCS    XPRINT9
 * XPRINT4:  CMPA   #SEMI
 *           BEQ    XPRINT6
 *           LDAA   #MCMSMERR
 *           JMP    RPTERR
 * XPRINT6:  LDAA   #SEMITOK
 *           BSR    CHKCOMA2
 *           BRA    XPRINT9
 * *
 * *
 * CHKCOMA:  JSR    GETCHR          ; GET CHARACTER FROM INPUT BUFFER.
 *           CMPA   #COMMA          ; IS IT A COMMA?
 *           BEQ    CHKCOMA1        ; YES. PUT IT IN THE TOKEN BUFFER.
 *           CLC                    ; NO. FLAG NO COMMA FOUND.
 *           RTS                    ; RETURN.
 * CHKCOMA1: LDAA   #COMMATOK       ; GET THE COMMA TOKEN.
 * CHKCOMA2: JSR    PUTTOK          ; PUT THE TOKEN IN THE BUFFER.
 *           JSR    INCIBP          ; BUMP THE INPUT BUFFER POINTER.
 *           SEC
 *           RTS                    ; RETURN.
 */

/*
 *  xinput      process the INPUT token
 */
void  xinput(void)
{
	*tbufptr++ = INPUTTOK;			// put INPUT token in buffer
	blanks();						// skip blanks

/*
 * XINPUT:   EQU    *
 * *         JSR    BLANKS
 *           BSR    XCHKDEV         ; GO CHECK FOR AN ALTERNATE DEVICE NUMBER.
 */

	if (*ibufptr == '"')			// if a prompt is included?
	{
		getscon();					// get the string constant
		if (errcode) return;		// if error, return
		if (*ibufptr != COMMA)		// is there a field delimiter?
		{
			errcode = SYTXERR;		// no, that's bad
			return;
		}
		*tbufptr++ = COMMATOK;		// save the comma token
		++ibufptr;					// next char in buffer
	}
	inreadcm();						// get the input variable list
	return;
}


/*
 * XINPUT2:  JSR    BLANKS
 *           JSR    GETCHR
 *           CMPA   #'"'
 *           BNE    INREADCM
 *           JSR    GETSCON
 *           BSR    CHKCOMA         ; IF COMMA PRESENT, PUT IN TOKEN BUFFER.
 *           BCS    INREADCM
 * XINPUT3:  LDAA   #MCOMAERR
 *           JMP    RPTERR
 * *
 */

/*
 * inreadcm      process a comma-delimited list of variables
 */
void  inreadcm(void)
{
	while (1)					// do forever
	{
		blanks();				// skip blanks
		getvar();				// get a variable
		if (errcode) return;	// if error, return
		blanks();				// skip blanks
		if (*ibufptr == COMMA) 
		{
			*tbufptr++ = COMMATOK;	// put delimiter in buffer
			++ibufptr;				// and point to the next char in the buffer
		}
		else return;			// if no delimiter return
	}
}

/*
 * XDIM:     EQU    *
 * INREADCM: EQU    *
 * XREAD:    EQU    *
 * *        JSR    BLANKS
 *           JSR    GETVAR
 * XREAD1:   JSR    BLANKS
 *           BSR    CHKCOMA
 *           BCS    XREAD
 *           RTS
 * *
 * *
 * XCHKDEV:  EQU    *
 *           JSR    GETCHR
 *           CMPA   #'#'       ; HAS AN ALTERNATE PORT BEEN SPECIFIED?
 *           BEQ    XCHKDEV1   ; NO. GO PROCESS THE REST OF THE PRINT STATEMENT.
 *           RTS               ; RETURN.
 * XCHKDEV1: LDAA   #PNUMTOK   ; YES. PUT THE TOKEN INTO THE BUFFER.
 *           JSR    PUTTOK     ; DO IT.
 *           JSR    INCIBP     ; POINT PAST THE "#".
 *           JSR    BLANKS     ; SKIP SPACES BETWEEN '#' AND EXPRESION.
 *           LDAA   #NUM       ; EXPRESSION MUST BE NUMERIC.
 *           JSR    XEXPRES    ; GO GET THE EXPRESSION.
 *           JSR    BLANKS     ; SKIP SPACES.
 *           BSR    CHKCOMA    ; GO GET COMMA THAT MUST FOLLOW THE EXPRESSION.
 *           BCC    XINPUT3    ; MISSING COMMA. GO REPORT THE ERROR.
 *           RTS               ; IT WAS THERE. GO PROCESS THE REST OF THE STATEMENT.
 * *
 * *
 * */



void  xinbyte(void)
{
//	pl("xinbyte ");
	*tbufptr++ = INBYTTOK;			// put INBYTE token in buffer
	blanks();
	getvar();
}

/*
 * XINBYTE:  EQU    *
 *           BSR    XCHKDEV    ; GO CHECK FOR ALTERNATE DEVICE.
 *           JSR    BLANKS     ; SKIP BLANKS AFTER COMMA.
 *           JMP    GETVAR     ; GO TRY TO GET A VARIABLE.
 * *
 */


/*
 * xread      process the READ token
 */
void xread(void)
{
	*tbufptr++ = READTOK;		// put read token in buffer
	inreadcm();					// get the variable list
}



/*
 * xdim      process the DIM token
 */
void  xdim(void)
{
	xdimflag = TRUE;			// show we are dimensioning an array
	*tbufptr++ = DIMTOK;		// put the DIM token in buffer
	inreadcm();					// get the variable list
	xdimflag = FALSE;			// done with the array
}




/*
 *  xrestore      process the RESTORE token
 */
void  xrestore(void)
{
	*tbufptr++ = RESTRTOK;				// put RESTORE token in buffer
	return;
}


/*
 *  xwhile      process the WHILE token
 */
void  xwhile(void)
{
//	blanks();							// skip blanks
	ifwhflag++;							// bump the while-flag counter
	*tbufptr++ = WHILETOK;				// mark the WHILE (??)
	xexpres(0);							// get expression
	return;
}

/*
 * XWHILE:   EQU    *
 * *        JSR    BLANKS
 *           INC    IFWHFLAG
 *           LDAA   #NULL
 *           JMP    XEXPRES
 * *
 */

/*
 *  xendwh      process the ENDWH token
 */
void  xendwh(void)
{
	*tbufptr++ = ENDWHTOK;				// put ENDWH token in buffer
	return;
}


/*
 *  xlabel      process the LABEL token
 */
void	xlabel(void)
{
	*tbufptr++ = LABELTOK;
	blanks();						// skip blanks
	if(*ibufptr == '"'){
		xexpres(STRING);
	}else{
		errcode = DTMISERR;
	}
	return;
}


/*
 *  xsleep      process the SLEEP token
 */
void	xsleep(void)
{
	*tbufptr++ = SLEEPTOK;
	xexpres(0);							// get expression
	return;
}


/*
 *  xtimestr	process the TIMESTR token
 */
void  xtimestr(void)
{
	*tbufptr++ = TIMESTRTOK;
	return;
}




/*
 *  xdatestr     process the DATESTR token
 */
//void  xdatestr(void)
//{
//	*tbufptr++ = DATESTRTOK;
//	return;
//}




/*
 * XPACC:    EQU    *
 * XTIME:    EQU    *
 *           LDAB   #NUM         ; SETUP TO USE CODE IN "ASIGNMT".
 *           JMP    ASIGNMT1     ; GO DO ASSIGNMENT STATEMENT.
 */

/*
 *  rpterr      report an error during processing or run-time
 */
void  rpterr(void)
{
	unsigned char					*ptr;
	unsigned char					c;

	if (runflag)						// if this is a run-time error...
	{
		pl_P(PSTR("ERROR #"));
		outdeci(errcode);
		pl_P(PSTR(" in line "));
		outdeci(curline);
		pl_P(PSTR(": "));
		DescribeError(errcode);
		contflag = TRUE;				// don't allow user to continue
		return;
	}
/*
 *  Immediate mode, echo the offending line and list the error.
 */
	ptr = inbuff;						// point to start of input buffer
	nl();
	nl();
	while ((c = *ptr++) != EOL) outbyte(c);		// print the input buffer

/*
 * RPTERR:   EQU    *
 *           STAA   ERRCODE
 *           JSR    NL2
 *           LDX    INBUFFS
 * RPTERR1:  LDAA   0,X
 *           CMPA   #EOL
 *           BEQ    RPTERR2
 *           JSR    OUTBYTE
 *           INX
 *           BRA    RPTERR1
 */

	nl();									// go to next line
	ptr = inbuff;							// point to begining of input buffer
	while (ptr++ < ibufptr-2) outbyte('*');	// output '*' to point of error
	pl_P(PSTR("^^^"));						// point to error
	nl();

/*
 * RPTERR2:  EQU    *
 *           JSR    NL
 *           LDX    IBUFPTR
 *           DEX
 *           DEX
 *           CPX    INBUFFS
 *           BLS    RPTERR4
 *           STX    IBUFPTR
 *           LDX    INBUFFS
 *           LDAA   #'*'
 * RPTERR3:  JSR    OUTBYTE
 *           INX
 *           CPX    IBUFPTR
 *           BNE    RPTERR3
 * RPTERR4:  LDX    #ARROWS
 *           JSR    PL
 *           JSR    NL
 *           BSR    RPTERR5
 * RPTERR6:  LDAA   #1
 *           STAA   CONTFLAG
 *           JMP    MAIN3
 */

	pl_P(PSTR("ERROR #"));
	outdeci(errcode);
	pl_P(PSTR(": "));
	DescribeError(errcode);
	contflag = TRUE;			// don't allow user to continue
	return;
}

/*
 * RPTERR5:  LDX    #ERRORS
 *           JSR    PL
 *           LDAB   ERRCODE
 *           CLRA
 *           JMP    OUTDECI
 * *
 * *
 * ARROWS:   FCC    "^^^"
 *           FCB    0
 * ERRORS:   FCC    "ERROR # "
 *           FCB    0
 * *
 * *
 */




/*
 *  rptrerr      report a run-time error
 *
 *  This was originally a separate routine.  I have rolled it into
 *  rpterr() above and test the runflag value to determine which
 *  type of error (immediate or run-time) to process.
 */


/* RPTRERR:  EQU    *             ; REPORT A RUN TIME ERROR.
 *           STAA   ERRCODE
 *           JSR    RPTERR5
 * RPTRERR1: LDX    #INLINE
 *           JSR    PL
 *           LDD    CURLINE
 *           JSR    OUTDECI
 *           JSR    NL
 *           LDAA   #1
 *           STAA   CONTFLAG
 *           JMP    MAINW
 * *
 * *
 * BREAKS:   FCC    "BREAK"
 * INLINE:   FCC    " IN LINE # "
 *           FCB    0
 *           $IFNDEF  HC11
 */




void  DescribeError(unsigned char  errnum)
{
	pl_P((char*)ErrStrs[errnum-1]);
}





/*
 *  outdeci      display decimal integer to output device
 */
void  outdeci(I32  num)
{
	char				tbuff[20];
	unsigned char		n;
	U32					unum;

//	sprintf(tbuff, "%ld", num);
	unum = num;						// always work with unsigned
	n = sizeof(tbuff)-1;			// start at end of buffer
	tbuff[n--] = 0;					// add terminating null
	if (num < 0)					// if doing a negative...
	{
		outbyte('-');				// print leading minus sign
		unum = -num;				// always work with unsigned (to allow for 0x80000000)
	}
	else if (num == 0)				// if simple case...
	{
		outbyte('0');				// just take the shortcut
		return;						// all done
	}

	while (unum)					// for so long as we have a number...
	{
		tbuff[n--] = (char)(unum % 10UL) + '0';		// stick in the next digit
		unum = unum / 10UL;			// see what's left
	}
	pl(&tbuff[n+1]);				// string is in RAM, use pl(), not pl_P()

//	int						k;
//	int						zs;
//	char					c;

//	zs = 0;
//	k = 10000;
//	if (num < 0)					// if value is negative...
//	{
//		num = (-num);				// convert to positive
//		outbyte('-');				// show negative
//	}

/*
 * OUTDECI:  EQU    *
 *           CPD    #0
 *           BNE    OUTDECI7
 *           LDAA   #'0'
 *           JMP    OUTBYTE
 * OUTDECI7: PSHY
 * *        PSHD
 *           PSHB
 *           PSHA
 *           CLRB
 *           PSHB
 *           PSHB
 *           TSY
 *           LDD    2,Y
 *           BPL    OUTDECI1
 *           COMA
 *           COMB
 *           ADDD   #1
 *           STD    2,Y
 *           LDAA   #'-'
 *           JSR    OUTBYTE
 */

//	while (k >= 1)						// while number is not zero...
//	{ 
//		c = num/k + '0';				// calc next digit and convert to char
//		if ((c!='0') || (k==1) || (zs))	 
//		{
//			zs = 1;						// show need to print
//			outbyte(c);					// print current char
//		}
//		num = num % k;					// remove digit just printed
//		k = k / 10;						// adjust divisor
//	}
//	return;
}

/*
 * OUTDECI1: EQU    *
 *           LDX    #PWRTEN
 * OUTDECI2: LDD    2,Y
 *           CLR    1,Y
 * OUTDECI3: SUBD   0,X
 *           BMI    OUTDECI5
 *           INC    1,Y
 *           BRA    OUTDECI3
 * OUTDECI5: ADDD   0,X
 *           STD    2,Y
 *           LDAA   1,Y
 *           BNE    OUTDECI6
 *           TST    0,Y
 *           BEQ    OUTDECI4
 * OUTDECI6: ADDA   #$30
 *           LDAB   #1
 *           STAB   0,Y
 *           JSR    OUTBYTE
 * OUTDECI4: INX
 *           INX
 *           TST    1,X
 *           BNE    OUTDECI2
 *           INS
 *           INS
 *           INS
 *           INS
 *           PULY
 *           RTS
 *           
 * *
 * *
 * PWRTEN:   FDB    10000
 *           FDB    1000
 *           FDB    100
 *           FDB    10
 *           FDB    1
 *           FDB    0
 * *
 *           $ELSE
 * *
 * OUTDECI:  EQU    *
 *           CPD    #0
 *           BNE    OUTDECI1
 *           LDAA   #'0'
 *           JMP    OUTBYTE
 * OUTDECI1: EQU    *
 *           PSHY
 * *        PSHD                ; SAVE THE NUMBER TO PRINT.
 *           PSHB
 *           PSHA
 *           LDD    #10000       ; NUMBER TO START DIVIDING BY.
 * *        PSHD
 *           PSHB
 *           PSHA
 *           CLRB                ; SET INITAL VALUE OF LEADING ZERO SUPRESS FLAG.
 *           PSHB
 *           TSY
 *           LDD    3,Y          ; IS THE NUMBER NEGATIVE?
 *           BPL    OUTDECI2     ; NO. GO PRINT THE NUMBER.
 *           COMA                ; YES. MAKE THE NUMBER POSITIVE.
 *           COMB                ; (TWO'S COMPLEMENT)
 *           ADDD   #1
 *           STD    3,Y          ; SAVE THE RESULT.
 *           LDAA   #'-'         ; PRINT A MINUS SIGN TO SHOW IT'S NEGATIVE.
 *           JSR    OUTBYTE
 * OUTDECI2: LDD    3,Y          ; GET THE DIVIDEND.
 *           LDX    1,Y          ; GET THE DIVISOR.
 *           IDIV                ; DO THE DIVIDE.
 *           STD    3,Y          ; SAVE THE REMAINDER.
 *           XGDX                ; PUT QUOTIENT IN D.
 *           CPD    #0           ; IS THE QUOTIENT 0?
 *           BNE    OUTDECI3     ; NO. GO OUTPUT THE NUMBER.
 *           TST    0,Y          ; YES. ARE WE STILL SUPRESSING LEADING ZEROS?
 *           BEQ    OUTDECI4     ; YES. DON'T PRINT THE NUMBER.
 * OUTDECI3: TBA                 ; PUT THE NUMBER IN THE A-REG.
 *           ADDA   #$30         ; MAKE THE NUMBER ASCII.
 *           LDAB   #1           ; MAKE THE ZERO SUPRESS FLAG NON-ZERO.
 *           STAB   0,Y
 *           JSR    OUTBYTE      ; OUTPUT THE NUMBER.
 * OUTDECI4: LDD    1,Y          ; GET CURRENT DIVISOR.
 *           LDX    #10          ; DIVIDE IT BY 10.
 *           IDIV
 *           STX    1,Y          ; SAVE RESULT. ARE WE DONE?
 *           BNE    OUTDECI2     ; NO KEEP GOING.
 *           LDAB   #5           ; DEALLOCATE LOCALS.
 *           ABY
 *           TYS
 *           PULY                ; RESTORE Y.
 *           RTS                 ; RETURN.
 * *
 * *
 *          $ENDIF
 * *
 * *
 */
