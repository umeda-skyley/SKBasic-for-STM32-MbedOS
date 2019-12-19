/*
 *  runtime3.c      Run-time routines for the Basic11 project
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"



/*
 *  local functions
 */
static void				findnextdata(void);
static void				_reep(U8  size);




/*
 *  rfor      execute the FOR token
 */
void  rfor(void)
{
	U16					voff;
	unsigned char		*tptr;

	if (forindex == FORSLEN)
	{
		errcode = FORNXERR;			// show no more FOR statements
		rpterr();
		return;
	}
	rskipspc();							// step past any spaces
	tptr = tbufptr;						// save a temp copy of the token pointer
	voff = rvarptr();					// get offset to control variable
	forstack[forindex].voff = voff;		// save offset
	forstack[forindex].cur = curline;	// save current line number
	tbufptr = tptr;						// restore the token pointer
	rlet();								// do assignment part of FOR
	rskipspc();							// step past spaces
	tbufptr++;							// this should have been a TO token!
	rskipspc();							// step past spaces
	donexp();							// calc terminating loop value
	forstack[forindex].termval = pull32(&numstack, STKUNDER);	// save terminating value
	forstack[forindex].step = 1;		// default step value
	rskipspc();							// move past the spaces
	if (*tbufptr == STEPTOK)			// if this is a STEP clause...
	{
		tbufptr++;						// move past the STEP token
		rskipspc();						// step past spaces
		donexp();						// calc STEP value
		forstack[forindex].step = pull32(&numstack, STKUNDER);	// save STEP value
		rskipspc();						// move past any spaces
	}
	forstack[forindex].tbuf = tbufptr;	// save end of FOR-TO clause (pointer into tknbuff)
	forindex++;
}

/*
 * RFOR:     EQU    *
 *           LDD    FORSTACK     ; GET FOR STACK POINTER.
 *           SUBD   #10          ; ALLOCATE NEW FOR-NEXT DESCRIPTOR BLOCK.
 *           CPD    EFORSTK      ; HAVE WE RUN OUT OF FOR-NEXT STACK SPACE?
 *           BHS    RFOR1        ; NO. CONTINUE.
 *           LDAA   #FORNXERR    ; YES. ERROR.
 *           JMP    RPTRERR      ; REPORT ERROR.
 * RFOR1:    STD    FORSTACK     ; SAVE NEW STACK POINTER.
 *           PSHY                ; SAVE IP ON STACK.
 *           JSR    RVARPTR      ; GET POINTER TO ASIGNMENT VARIABLE.
 *           PULY                ; RESTORE IP.
 *           LDX    FORSTACK     ; GET FOR STACK POINTER.
 *           STD    0,X          ; PUT POINTER TO CONTROL VARIABLE IN STACK.
 *           LDD    CURLINE      ; GET CURRENT LINE NUMBER.
 *           STD    8,X          ; SAVE CURRENT LINE NUMBER IN STACK.
 *           JSR    RLET         ; GO DO ASIGNMENT PART OF FOR.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           INY                 ; SKIP PAST "TO" TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           JSR    DONEXP       ; CALCULATE THE TERMINATING LOOP VALUE.
 *           JSR    PULNUM       ; GET NUMBER OFF OF THE STACK.
 *           LDX    FORSTACK     ; GET STACK POINTER.
 *           STD    4,X          ; PUT VALUE IN STACK BLOCK.
 *           LDD    #1           ; ASSUME A "STEP" VALUE OF 1.
 * RFOR3:    STD    2,X          ; PUT IT IN THE STACK.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAA   0,Y          ; GET NEXT TOKEN.
 *           CMPA   #STEPTOK     ; IS THE STEP CLAUSE PRESENT?
 *           BEQ    RFOR2        ; YES. GO GET THE "STEP" VALUE.
 *           STY    6,X          ; PUT TERMINATING CHARACTER OF "FOR" STATEMENT ON.
 *           RTS                 ; EXECUTE NEXT STATEMENT.
 * RFOR2:    INY                 ; SKIP PAST THE "STEP" TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           JSR    DONEXP       ; GO CALCULATE THE "STEP" VALUE.
 *           JSR    PULNUM       ; GET VALUE OFF OPERAND STACK.
 *           LDX    FORSTACK     ; GET POINTER TO FOR STACK.
 *           BRA    RFOR3        ; GO PUT VALUE IN STACK.
 */

/*
 *  rnext      execute the NEXT token
 */
void  rnext(void)
{
	U16								voff;
	FOR_ENTRY						*fptr;
	I32								cv;
	U32								*vaddr;

	rskipspc();
	voff = rvarptr();
	fptr = &forstack[forindex-1];
	if (voff != fptr->voff)
	{
		errcode = MFRNXERR;			// show missing FOR index variable
		rpterr();					// tell the world
		return;
	}
	vaddr = targetgetvarptr(voff);
	cv = getU32((U32 *)vaddr);
//	cv = *(I32 *)targetgetvarptr(voff);
//	cv = *(I32 *)(varram+voff);		// get current value of control variable
	cv = cv + fptr->step;			// add step value to control variable
//	*(I32 *)(varram+voff) = cv;		// save new value of control variable
	setU32(vaddr, cv);

	if (((fptr->step < 0) && (cv < fptr->termval))	||	// if hit target w/ neg step...
		((fptr->step > 0) && (cv > fptr->termval)))		// or hit target w/ pos step...
	{
		forindex--;					// show done with this FOR loop
		rskipspc();					// move to next item in program
	}
	else
	{
		tbufptr = fptr->tbuf;		// point to end of FOR statement
		curline = fptr->cur;		// restore the current line
	}
}

/*
 * RNEXT:    EQU    *
 *           JSR    RVARPTR      ; GET POINTER TO LOOP INDEX VARIABLE.
 *           LDX    FORSTACK     ; GET "FOR" STACK POINTER.
 *           CPD    0,X          ; IS THE LOOP VARIABLE THE SAME?
 *           BEQ    RNEXT1       ; YES. CONTINUE.
 *           LDAA   #MFRNXERR    ; NO. ERROR.
 *           JMP    RPTRERR      ; GO REPORT IT.
 * RNEXT1:   PSHY                ; SAVE IP.
 *           LDY    0,X          ; GET POINTER TO CONTROL VARIABLE.
 *           LDD    0,Y          ; GET CONTROL VARIABLE VALUE.
 *           ADDD   2,X          ; ADD THE STEP VALUE TO IT.
 *           STD    0,Y          ; SAVE THE RESULT.
 *           TST    2,X          ; IS THE STEP VALUE NEGATIVE?
 *           BMI    RNEXT2       ; YES. GO DO TEST.
 *           CPD    4,X          ; NO. ARE WE DONE?
 *           BLE    RNEXT3       ; NO. GO DO THE LOOP AGAIN.
 * RNEXT4:   PULY                ; RESTORE THE CURRENT IP.
 *           XGDX                ; PUT "FOR - NEXT" STACK POINTER IN D.
 *           ADDD   #10          ; REMOVE DESCRIPTOR FROM STACK.
 *           STD    FORSTACK     ; SAVE NEW STACK VALUE.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER CONTROL VARIABLE.
 *           RTS                 ; DO THE STATEMENT AFTER THE NEXT.
 * RNEXT2:   CPD    4,X          ; ARE WE DONE?
 *           BLT    RNEXT4       ; YES. CONTINUE.
 * RNEXT3:   PULY                ; CLEAN Y OFF OF STACK.
 *           LDY    6,X          ; GET NEW IP.
 *           LDD    8,X          ; GET LINE NUMBER OF FOR STATEMENT.
 *           STD    CURLINE      ; MAKE IT THE CURRENT LINE.
 *           RTS         
 */


/*
 *  rinput      execute the INPUT token
 *
 *  RINPUT  [SCONTOK  LEN  "string"], var [, var, var...]
 */
void  rinput(void)
{
	unsigned char				*tptr;
	int							len;

//	chkdev();							// check the input device
	rskipspc();							// skip any spaces
	if (*tbufptr == SCONTOK)			// is there a string to print as a prompt?
	{
		tptr = tbufptr;					// need a working copy of the token pointer
		len = *(tptr+1);				// get the length of the prompt string + quote marks
		len = len - 2;					// subtract out the two quote marks
		tptr = tptr + 3;				// advance pointer to first char in prompt string
		outstr(tptr, len);				// print the prompt
		tbufptr = tptr + len + 1;		// point to cell just past closing quote mark (should be comma)
		rskipspc();						// just in case
		tbufptr++;						// move past comma
	}
	else
	{
	}
	rskipspc();							// skip any spaces
	while (1)							// loop until we get what we want
	{
		outbyte('?');					// ask for input
		outbyte(' ');					// make it pretty
		getline();
		nl();
		if (chkbrk())  break;			// see if the user wanted out early
		if (rinrdc() == 0)  break;		// if got all the data we need, leave
	}
	devnum = 0;							// restore input device
}

/*
 * RINPUT:   EQU    *
 *           BSR    CHCKDEV      ; CHECK FOR ALTERNATE INPUT DEVICE.
 *           LDAA   0,Y          ; GET A TOKEN.
 *           CMPA   #SCONTOK     ; IS THERE A PROMPT TO PRINT?
 *           BNE    RINPUT1      ; NO JUST GO GET THE DATA.
 *           PSHY                ; YES. SAVE POINTER.
 *           LDAB   #2           ; COMPENSATE FOR CONSTANT & LENGTH BYTE.
 *           ADDB   1,Y          ; ADD IN LENGTH BYTE.
 *           ABY                 ; POINT BEYOND PROMPT.
 *           PULX                ; GET POINTER INTO X.
 *           INX                 ; POINT TO LENGTH BYTE.
 *           LDAB   0,X          ; GET IT.
 *           SUBB   #2           ; SUBTRACT OUT THE DELIMETER COUNT.
 *           INX                 ; POINT TO STRING.
 *           INX
 *           JSR    OUTSTR       ; GO PRINT THE STRING.
 *           INY                 ; BYPASS COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER COMMA.
 *           BRA    RINPUT6
 * RINPUT1:  JSR    NL
 * RINPUT6:  EQU    *
 *           LDX    #QSP         ; POINT TO PROMPT.
 *           JSR    PL           ; PRINT IT.
 *           JSR    GETLINE      ; GET THE DATA IN THE INPUT BUFFER.
 *           BSR    RINRDC
 *           BCS    RINPUT1
 *           JSR    NL
 *           CLR    DEVNUM       ; SET DEVICE NUMBER BACK TO 0.
 *           RTS
 *
 *
 * QSP:      FCC    "? "
 *           FCB    0
 */


/*
 *  chckdev      setup for a change of I/O device
 */
void  chckdev(void)
{
}

/*
 *
 * CHCKDEV:  LDAA   0,Y          ; GET A TOKEN.
 *           CMPA   #PNUMTOK     ; IS AN ALTERNATE DEVICE SPECIFYED?
 *           BEQ    CHCKDEV1     ; YES. CONTINUE.
 *           RTS                 ; NO. RETURN.
 * CHCKDEV1: INY                 ; YES. PASS THE '#' TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           JSR    DONEXP       ; GO EVALUATE THE NUMERIC EXPRESSION.
 *           JSR    PULNUM       ; GET THE NUMBER OFF THE STACK.
 *           BPL    CHCKDEV2     ; NEGATIVE NUMBERS NOT ALLOWED.
 * CHCKDEV3: LDAA   #ILLIOERR    ; REPORT THE ERROR.
 *           JMP    RPTRERR
 * CHCKDEV2: CPD    #$0007       ; IS IT LARGER THAN 7?
 *           BHI    CHCKDEV3
 *           STAB   DEVNUM       ; MAKE IT THE NEW DEVICE NUMBER.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           CMPA   #EOLTOK      ; IF THIS IS A PRINT STATEMENT, IS IT EOL?
 *           BEQ    CHCKDEV4     ; YES. DON'T BUMP THE IP.
 *           INY                 ; BYPASS THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 * CHCKDEV4: RTS                 ; RETURN.
 */



/*
 * rinrdc      common code for INPUT and DATA statements
 */
int  rinrdc(void)
{
	while (1)									// for the entire line...
	{
		skipspcs();								// ignore blanks
		if (*ibufptr == EOL)  return TRUE;		// show we haven't read the data we need yet
		innumd();								// read/input a value, save to variable pointed to by tbufptr
		if (errcode)  return TRUE;
		rskipspc();
		if (*tbufptr == EOLTOK)	 return FALSE;	// if don't need any more data, return
		if (*tbufptr == MEOLTOK)  return FALSE;	// if don't need any more data, return
		tbufptr++;								// this should be a comma
		rskipspc();
	}
}

/*
 * RINRDC:   JSR    SKIPSPCS
 *           CMPA   #EOL
 *           BNE    RINRDC1
 *           SEC
 *           RTS
 * RINRDC1:  BSR    INNUMD
 *           JSR    RSKIPSPC
 *           LDAA   0,Y
 *           CMPA   #EOLTOK
 *           BEQ    RINRDC2
 *           CMPA   #MEOLTOK
 *           BEQ    RINRDC2
 *           INY                 ; BUMP PAST THE COMMA.
 *           JSR    RSKIPSPC
 *           BRA    RINRDC 
 * RINRDC2:  CLC
 *           RTS
 */




/*
 *  innumd      input a number from the DATA statement
 *
 *  This routine tests the contents of ibufptr, assumed to hold
 *  the first character of a number found in the input buffer.
 *  If the first character is a '$', the field pointed to by the
 *  input buffer is processed as a hex number, otherwise the
 *  field is processed as a decimal number.
 *
 *  After processing the number in the input field, this routine
 *  uses rvarptr() to compute the offset to the variable and
 *  modifies that variable to hold the value just read.
 *
 *  This routine uses the input buffer pointer (ibufptr).
 */
void  innumd(void)
{
	U32						val;
	U16						voff;

	if (*ibufptr == '$')					// if this is a hex number...
	{
		ibufptr++;							// move to next char
		val = gethex();						// read a hex number from the input buffer
	}
	else									// not hex, assume decimal
	{
		val = indeci();						// input a decimal number from the input buffer
	}
	skipspcs();								// skip spaces in the input buffer
	switch  (*ibufptr)						// based on the current char...
	{
		case  COMMA:						// comma means another field is available
		ibufptr++;
		break;

		case  EOL:							// EOL means no more data on this line
		break;

		default:							// anything else is bad!
		errcode = MCOMAERR;					// show we were expecting a comma
		rpterr();
		return;
	}
	voff = rvarptr();						// find the offset for storing the data value
//	*(U32 *)(varram+voff) = val;			// save data value
	setU32(targetgetvarptr(voff), val);			// save data value
}

/*
 * INNUMD:   EQU    *
 *           CMPA   #'$'
 *           BNE    INNUM2
 *           JSR    INCIBP
 *           JSR    GETHEX
 *           BRA    INNUM3
 * INNUM2:   JSR    INDECI
 * INNUM3:   EQU    *
 * *        PSHD
 *           PSHB
 *           PSHA
 *           JSR    SKIPSPCS
 *           CMPA   #COMMA
 *           BEQ    INNUM4
 *           CMPA   #EOL
 *           BEQ    INNUM7
 *           LDAA   #MCOMAERR
 *           JMP    RPTRERR
 * INNUM4:   JSR    INCIBP
 * INNUM7:   JSR    RVARPTR
 *           XGDX
 *           PULA
 *           PULB
 *           STD    0,X
 *           RTS
 */



/*
 *  outstr      write a fixed-length string to the active output device
 *
 *  WARNING:  The original version of this routine advanced the token buffer
 *  pointer during the string print; this version DOES NOT!  Be sure to
 *  advance the pointer in the calling routine after calling this routine.
 */
void  outstr(unsigned char  *str, int  len)
{
	while (len)
	{
		outbyte_xlate(*str++);			// write the char, with escape translation
		len--;							// count this char
	}
}

/*
 *
 * OUTSTR   EQU    *
 *           TSTB
 *           BEQ    OUTSTR2
 * OUTSTR1  LDAA   0,X
 *           INX
 *           JSR    OUTBYTE
 *           DECB
 *           BNE    OUTSTR1
 * OUTSTR2  RTS
 */



/*
 *  indeci      input a 32-bit decimal integer from the input buffer (ibufptr)
 */
I32  indeci(void)
{
	char					neg;
	I32						val;

	neg = FALSE;					// not doing negative number yet
	if (*ibufptr == '-')			// if doing a negative number...
	{
		neg = TRUE;					// show we need to do a negative
		ibufptr++;					// move to next char
	}
	val = getdeci();				// get the decimal number
	if (neg)  val = -val;			// negate if needed
	return  val;
}

/*
 * INDECI:   EQU    *
 *           JSR    GETCHR       ; GET A CHARACTER.
 *           CMPA   #'-'         ; IS IT A NEGATIVE NUMBER?
 *           BNE    INDECI1      ; NO. GO GET POSITIVE NUMBER.
 *           JSR    INCIBP       ; YES. BUMP INPUT BUFFER PAST IT.
 *           JSR    GETDECI      ; GET THE NUMBER.
 *           COMA                ; NEGATE IT.
 *           COMB
 *           ADDD   #1
 *           RTS                 ; RETURN.
 * INDECI1:  JSR    GETDECI
 *           RTS
 */



/*
 *  rread      execute the READ token
 */
void  rread(void)
{
	if (dataptr == 0)				// if data pointer is not pointing to data...
	{
		rrestor();					// rewind the data pointer
		findnextdata();
	}
	ibufptr = dataptr;				// set up the input pointer for rinrdc()

	while (1)						// for all values we need to read...
	{
		rskipspc();					// just in case there are spaces
		if (rinrdc() == TRUE)		// read a data field; if read is not complete...
		{
			dataptr = ibufptr + 2;	// point to start of next statement
			findnextdata();			// find the next DATA statement
			if (errcode)  return;	// if hit an error, bail now
			ibufptr = dataptr;		// rewind the input buffer
		}
		else						// read all we needed, time to leave
		{
			dataptr = ibufptr;		// update dataptr for next READ statement
			return;
		}
	}
}

/*
 * RREAD:    EQU    *
 *           LDX    DATAPTR      ; GET POINTER TO DATA. IS IT POINTING TO DATA?
 *           BNE    RREAD1       ; YES. CONTINUE TO READ DATA.
 *           BSR    RRESTOR      ; NO. GO GET POINTER TO FIRST DATA STATEMENT.
 *           LDX    DATAPTR      ; GET POINTER TO DATA.
 * RREAD1:   STX    IBUFPTR      ; PUT IT IN THE INPUT BUFFER POINTER.
 *           JSR    RINRDC       ; GO USE INPUT/READ COMMON CODE.
 *           BCS    RREAD2       ; IF CARRY SET, MORE DATA TO READ.
 *           LDX    IBUFPTR      ; GET POINTER TO DATA LINE.
 *           STX    DATAPTR      ; SAVE DATA POINTER FOR NEXT READ.
 *           RTS                 ; RETURN.
 * RREAD2:   PSHY                ; SAVE Y.
 *           LDY    IBUFPTR      
 *           INY
 *           INY
 *           BSR    RESTOR4      ; GO FIND NEXT "DATA" STATEMENT.
 *           PULY                ; RESTORE Y.
 *           BRA    RREAD        ; KEEP READING DATA.
 */



/*
 *  findnextdata      scan through program for next DATA statement
 */  
static void  findnextdata(void)
{
	unsigned char			*tptr;

	tptr = tbufptr;					// need to save tbufptr
	tbufptr = dataptr;				// use tbufptr to step across the DATA statements
	while (1)
	{
		tbufptr = tbufptr + 3;		// move to first token in line
		rskipspc();					// skip any spaces at start of line
		if (*tbufptr == DATATOK)	// if this is a DATA statement...
		{
			dataptr = tbufptr + 3;	// point past the length byte
			tbufptr = tptr;			// restore tbufptr
			return;
		}
		if (dataptr == basend)		// if ran out of program...
		{
			tbufptr = tptr;			// restore the token pointer
			errcode = ODRDERR;		// show out of data
//			rpterr();
			return;
		}
		dataptr = dataptr + *(dataptr+2);	// get start of next line
	}
}




/*
 *  rrestor      execute the RESTORE token
 */
void  rrestor(void)
{
	unsigned char				*tptr;

	tptr = tbufptr;						// save the current buffer pointer
	tbufptr = basbeg;					// start at beginning of program
	dataptr = basbeg;					// start this one at beginning also
	while (1)							// forever...
	{
		dataptr = tbufptr + *(tbufptr+2);	// point to start of next line
		tbufptr = tbufptr + 3;			// point to first token in this line
		rskipspc();						// skip any spaces
		if (*tbufptr == DATATOK)		// if this is a DATA statement...
		{
			dataptr = tbufptr + 2;		// point past the length byte
			tbufptr = tptr;				// restore buffer pointer
			return;
		}
		if (dataptr == basend)			// if ran out of program...
		{
			tbufptr = tptr;				// restore the token pointer
			errcode = ODRDERR;			// show out of data error
			rpterr();					// tell the world
			return;
		}
		tbufptr = dataptr;				// move to next line
	}
}



/*
 * RRESTOR:  EQU    *
 *           PSHY                ; SAVE Y.
 *           LDY    BASBEG       ; START SEARCH FOR "DATA" STATEMENTS AT THE BEGIN.
 * RESTOR2:  PSHY                ; SAVE POINTER TO THIS LINE.
 *           LDAB   2,Y          ; GET LINE LENGTH.
 *           ABY                 ; GET START OF NEXT LINE.
 *           STY    DATAPTR      ; SAVE IN "DATAPTR".
 *           PULY                ; RESTORE POINTER.
 *           LDAB   #3
 *           ABY                 ; POINT TO FIRST TOKEN IN LINE.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAA   0,Y          ; GET THE KEYWORD.
 *           CMPA   #DATATOK     ; IS IT A DATA LINE?
 *           BEQ    RESTOR1      ; YES. GO SET UP POINTER.
 *           LDY    DATAPTR      ; GET ADDRESS OF NEXT LINE.
 * RESTOR3:  CPY    BASEND       ; ARE WE AT THE END OF THE PROGRAM?
 *           BNE    RESTOR2      ; NO. KEEP LOOKING.
 *           LDAA   #ODRDERR     ; OUT OF DATA ERROR.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * RESTOR1:  INY                 ; POINT PAST DATA TOKEN & THE DATA LENGTH.
 *           INY
 *           STY    DATAPTR      ; SAVE POINTER TO DATA.
 *           PULY                ; RESTORE Y.
 *           RTS                 ; RETURN.
 * *
 * *
 * RESTOR4:  PSHY                ; CALL TO COMPENSATE FOR PULL OF Y ON RETURN.
 *           BRA    RESTOR3
 */



/*
 *  rif      execute the IF token
 */
void  rif(void)
{
	unsigned int				flag;

	donexp();							// evaluate the IF expression
	rskipspc();							// skip any spaces before THEN
	tbufptr++;							// skip over the THEN token
	rskipspc();							// skip any spaces after THEN
	flag = (unsigned int)pull32(&numstack, STKUNDER);	// get the result of the IF expression
	if (errcode)  return;				// if bad expression, leave now

	if (flag)							// if result of IF was TRUE...
	{
		if ((*tbufptr == LCONTOK) || (*tbufptr == SCONTOK))		// if THEN is followed by line number...
		{
			rgoto();					// go to line in buffer following THEN token
			return;						// all done here
		}
		else							// no line number after THEN...
		{
			return;						// let crun() handle it
		}
	}
	else								// result of IF was FALSE...
	{
		if (*tbufptr == LCONTOK)		// if THEN followed by line number...
		{
			tbufptr = tbufptr + 3;		// step over line number token and line number (total of 3 bytes)
		}
		else if(*tbufptr == SCONTOK)
		{
			tbufptr += *(tbufptr+1) + 2;
		}
		else
		{
			tbufptr = adrnxlin - 1;		// go to start of next line (if any) less one cell
			return;						// no ELSE clause, let crun() handle it
		}
		rskipspc();						// step over any spaces
		if (*tbufptr == ELSETOK)		// if hit an ELSE token...
		{
			tbufptr++;					// move past ELSE token
			rskipspc();					// move over any spaces
			if ((*tbufptr == LCONTOK) || (*tbufptr == SCONTOK))	// if ELSE is followed by a line number...
			{
				rgoto();				// go to line in buffer following ELSE token
				return;
			}
			else						// no line number, fall through and let runline() deal with it
			{
				return;
			}
		}
	}
}

/*
 * RIF:      EQU    *
 *           JSR    DONEXP       ; GO DO A NUMERIC EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           INY                 ; SKIP PAST "THEN" TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER THEN.
 *           JSR    PULNUM       ; GET RESULT OF EXPRESSION FROM OPERAND STACK.
 *           BEQ    RIF1         ; NOT TRUE. SEE IF ELSE CLAUSE PRESENT.
 * RIF3:     JMP    RGOTO        ; RESULT WAS TRUE. GOTO PROPER LINE NUMBER.
 * RIF1:     LDAB   #3           ; BUMP IP PAST LINE NUMBER.
 *           ABY
 *           JSR    RSKIPSPC     ; SKIP SPACES IF PRESENT.
 *           LDAA   0,Y          ; GET NEXT TOKEN.
 *           CMPA   #ELSETOK     ; IS IT THE "ELSE" CLAUSE.
 *           BNE    RIF2         ; NO RETURN.
 *           INY                 ; PASS ELSE TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           BRA    RIF3         ; DO A GOTO.
 * RIF2:     RTS                 ; RETURN.
 */




/*
 *  rtimestr    print a string showing current system time
 */
/*
void  rtimestr(void)
{
	time_t				_time;
	struct tm			*pt;
	char				systime[50];
		
	_time = targetgetsystime();			// get current calendar time (seconds)
	pt = localtime(&_time);				// convert to broken time starting at 1900 in tm struct
	tmtotimestr(pt, systime);			// convert to time string
	pl(systime);
}
*/


/*
 *  rdatestr      print a string showing current system date
 */
/*
void  rdatestr(void)
{
	time_t				_time;
	struct tm			*pt;
	char				sysdate[50];
		
	_time = targetgetsystime();			// get current calendar time (in seconds)
	pt = localtime(&_time);				// convert to broken time in tm struct
	tmtodatestr(pt, sysdate);			// convert to date string
	pl(sysdate);
}
*/	




/*
 *  reep      execute the EEP token when writing to EEPROM
 *
 *  Handles the EEP() token when writing:
 *
 *  100 EEP(x) = $12
 *
 *  Unlike Gordon's implementation, EEP() accesses are 8-bit; see
 *  also EEP16() and EEP32();
 *
 *  Note that the target may (probably will) use parts of the EEPROM
 *  for storing target-specific data.  The user must take care when
 *  writing data to EEPROM areas not to modify system parameters.
 */
void  reep(void)
{
	_reep(sizeof(U8));				// process a byte
}



void  reep16(void)
{
	_reep(sizeof(U16));				// process a word
}



void  reep32(void)
{
	_reep(sizeof(U32));				// process a double-word
}



/*
 *  _reep      helper function for handling writes to EEPROM
 *
 *  This routine handles the parsing of the buffer and sets up writes
 *  to EEPROM based on the number of bytes to write, passed as argument
 *  size.
 */
static void  _reep(U8  size)
{
	U32								value;
	unsigned int					index;

	tbufptr++;						// skip over the left-paren token
	rskipspc();						// step over any spaces
	donexp();						// evaluate the EEPROM index expression
	rskipspc();						// step over any spaces
	tbufptr++;						// skip over the right-paren token
	rskipspc();						// step over any spaces
	tbufptr++;						// skip over the equals-sign token (assignment)
	donexp();						// get value to write to EEPROM
	value = pull32(&numstack, STKUNDER);	// get the value to write
	index = (unsigned int)pull32(&numstack, STKUNDER);	// get the address of the EEPROM cell to write
	switch  (size)					// based on number of bytes to write...
	{
		case  sizeof(U8):			// writing a byte
		value = value & 0xff;		// enforce a byte-wide limit on write
		targetwriteeeprom(index, (U8)value);	// let the target sort it all out
		break;

		case  sizeof(U16):			// writing a word
		value = value & 0xffff;		// enforce a word-wide limit on write
		targetwriteeeprom16(index, (U16)value);	// let the target sort it all out
		break;

		case  sizeof(U32):			// writing a double word
		targetwriteeeprom32(index, value);	// let the target sort it all out
		break;

		default:					// should never happen!
		break;
	}
}

/*
 * REEP:     EQU    *            ; PROGRAM A WORD OF EEPROM.
 *           INY                 ; PASS UP THE OPEN PAREN.
 *           JSR    RSKIPSPC     ; PASS UP ANY SPACES.
 *           JSR    DONEXP       ; GO GET THE "SUBSCRIPT" OF THE EEPROM LOCATION.
 *           INY                 ; PASS UP THE CLOSING PAREN.
 *           INY                 ; PASS UP THE EQUALS TOKEN.
 *           JSR    DONEXP       ; GET VALUE TO FROGRAM INTO EEPROM.
 *           PSHY                ; SAVE THE Y REG.
 *           LDY    NUMSTACK     ; POINT TO THE NUMERIC STACK.
 *           LDD    2,Y          ; GET THE SUBSCRIPT FOR THE EEPROM LOCATION.
 *           BMI    REEP1        ; NEGATIVE SUBSCRIPTS NOT ALLOWED.
 *           CPD    #MAXEESUB    ; IS THE SUBSCRIPT WITHIN RANGE?
 *           BLS    REEP2        ; YES. CONTINUE.
 * REEP1:    LDAA   #EESUBERR    ; EEPROM SUBSCRIPT ERROR.
 *           JMP    RPTRERR      ; REPORT IT.
 * REEP2:    LSLD                ; MULT THE SUBSCRIPT BY 2.
 *           ADDD   #EEPBASAD    ; ADD IN THE EEPROM BASE ADDRESS.
 *           XGDX                ; PUT THE ADDRESS INTO X.
 *           LDAA   0,X          ; GET THE MOST SIGNIFIGANT BYTE OF THE CURRENT NUM.
 *           CMPA   #$FF         ; DOES IT NEED ERASING?
 *           BEQ    REEP3        ; NO. SEE IF NEXT BYTE NEEDS ERASING.
 *           BSR    ERASEBYT     ; YES. GO ERASE IT.
 * REEP3:    INX                 ; POINT TO NEXT BYTE.
 *           LDAA   0,X          ; GET NEXT BYTE.
 *           CMPA   #$FF         ; DOES THIS BYTE NEED TO BE ERASED?
 *           BEQ    REEP4        ; NO. GO WRITE DATA TO EEPROM.
 *           BSR    ERASEBYT     ; YES. GO ERASE THE BYTE.
 * REEP4:    LDAA   1,Y          ; GET LS BYTE OF WORD.
 *           BSR    PROGBYTE     ; GO PROGRAM THE BYTE.
 *           DEX                 ; POINT TO THE MOST SIGNIFIGANT EEPROM LOCATION.
 *           LDAA   0,Y          ; GET THE MS BYTE OF THE WORD.
 *           BSR    PROGBYTE     ; GO PROGRAM THE BYTE.
 *           PULY                ; RESTORE Y.
 *           JSR    PULNUM       ; FIX UP NUM STACK.
 *           JSR    PULNUM
 *           RTS                 ; RETURN.
 */


void			rstrcat(void)
{
	int			tok;
	U8			*varptr1;
	U8			*ptr1 = NULL, *ptr2 = NULL;
	int			len1 = 0, len2 = 0;
	//int			i, ret = 0;

	tbufptr++;						// skip over the left-paren token
	rskipspc();
	tok = *tbufptr;
	if(tok == SVARTOK){
		int		offset;
		tbufptr++;							// move to offset address
		offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		varptr1 = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
		len1 = varptr1[0];
		ptr1 = &varptr1[1];
	}else{
		errcode = SYTXERR;
		goto exit;
	}
	rskipspc();
	if(*tbufptr++ != COMMATOK){
		errcode = SYTXERR;
		goto exit;
	}
	rskipspc();
	tok = *tbufptr;
	if((tok == SCONTOK) || (tok == SVARTOK)){
		if(tok == SCONTOK){
			len2 = *(tbufptr+1);
			tbufptr += 2;
			ptr2 = tbufptr;
			tbufptr += len2;
		}else{
			int		offset;
			U8		*varptr;
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			len2 = varptr[0];
			ptr2 = &varptr[1];
		}
	}else{
		errcode = SYTXERR;
		goto exit;
	}
	tbufptr++;						// skip over the right-paren token

	if((len1 + len2) < 256){
		int		i;
		for(i = 0; i < len2; i++){
			ptr1[len1 + i] = ptr2[i];
		}
		varptr1[0] = (U8)(len1 + len2);
	}else{
		errcode = STROVERFLOWERR;
		goto exit;
	}

exit:
	return;
}

void			rstrind(void)
{
	int			tok;

	tbufptr++;						// skip over the left-paren token
	rskipspc();
	tok = *tbufptr;
	if(tok == SVARTOK){
		U8		*varptr;
		U8		*ptr = NULL;
		int		len = 0;
		int		address, value;

		if(tok == SCONTOK){
			tbufptr++;
			varptr = tbufptr++;
			len = varptr[0];
			ptr = tbufptr;
			tbufptr += len;
		}else{
			int		offset;
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			len = varptr[0];
			ptr = &varptr[1];
		}
		rskipspc();
		if(*tbufptr++ != COMMATOK){
			errcode = MCOMAERR;
			goto exit;
		}
		rskipspc();
		donexp();							// calc address
		address = (int)pull32(&numstack, STKUNDER);
		rskipspc();
		tbufptr++;						// skip over the right-paren token
		rskipspc();
		if(*tbufptr++ != EQUALTOK){
			errcode = SYTXERR;
			goto exit;
		}
		rskipspc();
		donexp();							// calc address
		value = (int)pull32(&numstack, STKUNDER);

		if((address < 0) || (address >= 256)){
			errcode = OUTOFRANGEERR;
		}else{
			if(address >= len){
				while(len < address){
					ptr[len++] = 0;
				}
				varptr[0] = (U8)(len + 1);
			}
			ptr[address] = (U8)value;
		}
	}
exit:
	return;
}


/*
 *  erasebyt      erase a byte of EEPROM memory
 */
void  erasebyt(char  c)
{
}

/*
 * ERASEBYT: EQU    *
 *           PSHY
 *           LDY       IOBaseV   ; Point to the base address of the I/O Registers.
 *           LDAB   #$16         ; SET UP BYTE ERASE MODE, ADDR LATCH, ERASE
 *           STAB   PPROG,Y      ; VOLTAGE OFF.
 *           STAA   0,X          ; LATCH ADDRESS.
 *           TPA                 ; GET CURRENT I-BIT STATUS.
 *           PSHA                ; SAVE IT.
 *           SEI                 ; INHIBIT INTERRUPTS WHILE ERASING.
 *           LDAB   #$17         ; TURN ON ERASE VOLTAGE
 *           STAB   PPROG,Y
 *           BSR    DLY10MS      ; DELAY ABOUT 10 MS.
 *           LDAB   #$16         ; TURN PROGRAMING VOLTAGE OFF.
 *           STAB   PPROG,Y
 *           PULA                ; GET ORIGINAL I-BIT STATUS.
 *           TAP                 ; RESTORE IT.
 *           CLR    PPROG,Y
 *           PULY
 *           RTS                 ; RETURN.
 */



/*
 *  progbyte      program a byte in EEPROM
 */
void  progbyte(char  c, int  addr)
{
}

/*
 * PROGBYTE: EQU    *
 *           PSHY
 *           LDY       IOBaseV   ; Point to the base address of the I/O Registers.
 * PROGBYT2: LDAB   #$02         ; SET UP NORMAL PROGRAMING MODE, ADDRESS/DATA
 *           STAB   PPROG,Y      ; LATCHED, PROGRAMING VOLTAGE OFF.
 *           STAA   0,X          ; LATCH DATA & ADDRESS.
 *           PSHA                ; SAVE THE DATA FOR COMPARE AFTER PROGRAMING.
 *           TPA                 ; GET CURRENT I-BIT STATUS.
 *           PSHA                ; SAVE IT.
 *           SEI                 ; INHIBIT INTERRUPTS WHILE PROGRAMING.
 *           LDAB   #$03         ; TURN ON PROGRAMING VOLTAGE.
 *           STAB   PPROG,Y
 *           BSR    DLY10MS      ; LEAVE IT ON FOR 10 MS.
 *           LDAB   #$02         ; NOW, TURN THE PROGRAMMING VOLTAGE OFF.
 *           STAB   PPROG,Y
 *           PULA                ; GET ORIGINAL I-BIT STATUS.
 *           TAP                 ; RESTORE IT.
 *           CLR    PPROG,Y      ; PUT THE EEPROM BACK IN THE READ MODE.
 *           PULA                ; RESTORE THE DATA TO SEE IF IT WAS PROGRAMMED.
 *           CMPA   0,X          ; WAS THE DATA WRITTEN PROPERLY?
 *           BNE    PROGBYT2     ; NO. TRY AGAIN.
 *           PULY                ; Restore Y.
 *           RTS                 ; YES. RETURN.
 */




/*
 *  dly10ms      delay 10 msecs (used solely for EEPROM programming)
 */
void  dly10ms(void)
{
}

/*
 * DLY10MS:  EQU    *
 *           PSHX                ; SAVE X.
 *           LDX    #3330        ; GET DELAY CONSTANT.
 * DLY10MS1: DEX                 ; DECREMENT THE COUNT. DONE?
 *           BNE    DLY10MS1     ; NO. DELAY SOME MORE.
 *           PULX                ; RESTORE X.
 *           RTS                 ; RETURN.
 */



/*
 *  rinbyte      execute the INBYTE token
 */
void  rinbyte(void)
{
	I32					c;
	U16					voff;

	rskipspc();
	voff = rvarptr();
//	pl("x ");
	c = inbyte();
//	*(I32 *)(varram+voff) = c;		
	setU32((U32 *)targetgetvarptr(voff), c);
}

/*
 * RINBYTE:  EQU    *
 *           JSR    CHCKDEV      ; GO CHECK FOR AN ALTERNATE DEVICE DESIGNATION.
 *           JSR    RVARPTR      ; GO GET POINTER TO THE BYTE INPUT VARIABLE.
 *           XGDX                ; PUT THE POINTER INTO X.
 *           JSR    INBYTE       ; GO GET A BYTE FROM THE SPECIFIED INPUT DEVICE.
 *           TAB                 ; PUT THE BYTE IN THE L.S.BYTE.
 *           CLRA                ; ZERO THE UPPER BYTE.
 *           STD    0,X          ; PUT IT IN THE VARIABLE.
 *           CLR    DEVNUM       ; RESET TO DEVICE #0.
 *           RTS                 ; RETURN.
 */



/*
 *  rtime      execute the TIME token
 */
void  rtime(void)
{
}

/*
 * RTIME:    EQU    *
 *           INY                 ; POINT PAST THE EQUALS TOKEN.
 *           JSR    DONEXP       ; GO EVALUATE THE EXPRESSION.
 *           JSR    PULNUM       ; GET THE NUMBER OFF THE STACK.
 *           STD    TIMEREG      ; PUT IT IN THE TIME REGISTER.
 *           RTS                 ; RETURN.
 */



/*
 *  rrtime      execute the RTIME token
 */
void  rrtime(void)
{
}

/*
 * RRTIME:   equ       *
 *           sei                 ; disable interrupts.
 *           LDAA   #SWPRE+1     ; ADD 1 TO NORMAL PRE SCALER.
 *           STAA   TIMEPRE      ; SET UP THE SOFTWARE PRESCALER.
 *           LDX       IOBaseV   ; Point to the I/O Base Address.
 *           ldd       TCNT,x    ; get the current value of the timer counter.
 *           jsr       TIMINTS3  ; go initialize the TOC using the timer interrupt code.
 *           clra
 *           clrb
 *           STD    TIMEREG      ; PUT IT IN THE TIME REGISTER.
 *           cli
 *           RTS                 ; RETURN.
 */



/*
 *  rpacc      execute the PACC token
 */
void  rpacc(void)
{
}

/*
 * RPACC:    EQU    *
 *           INY                 ; POINT PAST EQUALS TOKEN.
 *           JSR    DONEXP       ; EVALUATE THE EXPRESSION.
 *           JSR    PULNUM       ; GET THE NUMBER OFF THE STACK.
 *           TSTA                ; IS THE NUMBER WITHIN RANGE?
 *           BEQ    RPACC1       ; YES. GO SETUP THE PACC REGISTER.
 *           LDAA   #PACCARGE    ; NO. REPORT AN ERROR.
 *           JMP    RPTRERR
 * RPACC1:   LDX    IOBaseV
 *           STAB   PACNT,X      ; PUT NUMBER IN PULSE ACC.
 *           RTS                 ; RETURN.
 */




/*
 *  rontime      execute ONTIME token
 */
void  rontime(void)
{
}

/*
 * RONTIME:  EQU    *
 *           BSR    CHCKIMID    ; NOT ALLOWED IN IMMIDIATE.
 *           JSR    DONEXP      ; GO EVALUATE THE TIME "MATCH" EXPRESSION.
 *           JSR    PULNUM      ; GET THE NUMBER OFF THE STACK.
 *           STD    TIMECMP     ; PUT IN THE COMPARE REGISTER.
 *           JSR    RSKIPSPC    ; SKIP SPACES.
 *           INY                ; PASS UP COMMA.
 *           JSR    RSKIPSPC    ; SKIP SPACES.
 *           STY    ONTIMLIN    ; SAVE THE POINTER TO THE LINE NUMBER.
 *           BRA    RONIRQ2     ; GO FINISH UP.
 */


/*
 *  ronirq      execute ONIRQ token
 */
void  ronirq(void)
{
}

/*
 * RONIRQ:   EQU    *
 *           BSR    CHCKIMID
 *           JSR    DONEXP      ; GO CHECK TO SEE IF WE ARE TO ENABLE OR DISABLE.
 *           JSR    RSKIPSPC    ; SKIP SPACES UP TO COMMA.
 *           INY                ; BYPASS COMMA.
 *           JSR    RSKIPSPC    ; SKIP SPACES UP TO LINE NUMBER.
 *           JSR    PULNUM      ; GET MODE. SHOULD WE ENABLE THE FUNCTION?
 *           BNE    RONIRQ1     ; YES.
 *           STD    ONIRQLIN    ; NO. MAKE THE LINE NUMBER 0.
 *           BRA    RONIRQ2     ; GO FINISH UP.
 * RONIRQ1:  STY    ONIRQLIN    ; SAVE THE POINTER TO THE LINE NUMBER,
 * RONIRQ2:  LDAB   #3          ; MOVE IP PAST THE LINE NUMBER.
 *           ABY
 *           RTS                ; RETURN.
 */



/*
 *  rreti      execute RETI token
 */
void  rreti(void)
{
}

/*
 * RRETI:    EQU    *
 *           BSR    CHCKIMID
 *           TPA                 ; CHECK TO SEE IF THE INTERRUPT MASK IS SET.
 *           BITA   #$10         ; ARE WE IN AN INTERRUPT ROUTINE?
 *           BNE    RRETI1       ; SINCE THE IRQ MASK IS SET WE MUST BE.
 *           LDAA   #NOTINTER    ; NO. FLAG AN ERROR.
 *           JMP    RPTRERR      ; GO REPORT IT.
 * RRETI1:   LDD    SCURLINE     ; RESTORE THE MAIN PROGRAM CURRENT LINE.
 *           STD    CURLINE
 *           LDD    SADRNXLN     ; RESTORE MAIN PROGRAM "ADDRESS OF THE NEXT LINE".
 *           STD    ADRNXLIN
 *           INS                 ; TAKE THE RETURN ADDRESS OFF THE STACK.
 *           INS
 *           RTI                 ; GO BACK TO WHERE WE LEFT OFF.
 */



/*
 *  chckimid      check current mode, report error if immediate mode
 */
void  chckimid(void)
{
	if (immid)						// if now in immediate mode...
	{
		errcode = NOTALERR;			// that is not allowed!
		rpterr();					// show the error
	}
}

/*
 * CHCKIMID: EQU    *
 *           TST    IMMID        ; ARE WE IN THE IMMIDIATE MODE?
 *           BEQ    CHCKIMI1     ; NO. JUST RETURN.
 *           LDAA   #NOTALERR    ; YES. THIS COMMAND NOT ALLOWED.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * CHCKIMI1: RTS                 ; RETURN.
 */




/*
 *  ronpacc      execute ONPACC token
 */
void  ronpacc(void)
{
}

/*
 * RONPACC:  EQU    *
 *           BSR    CHCKIMID     ; THIS INSTRUCTION NOT ALLOWED IN IMMID MODE.
 *           JSR    DONEXP       ; GO EVALUATE THE COUNT MODE EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           INY                 ; BYPASS THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER COMMA.
 *           JSR    DONEXP       ; GO EVALUATE THE INTERRUPT MODE EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           INY                 ; BYPASS THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER THE COMMA.
 *           TPA                 ; GET CURRENT I-BIT STATUS.
 *           PSHA                ; SAVE IT.
 *           SEI                 ; INHIBIT INTERRUPTS.
 *           STY    ONPACLIN     ; SAVE POINTER TO INTERRUPT ROUTINE.
 *           JSR    PULNUM       ; GET INTERRUPT MODE OFF STACK.
 * RONPACC1: CPD    #1           ; IS THE ARGUMENT <=1?
 *           BLS    RONPACC2     ; YES. ARG. OK.
 * RONPACC5: LDAA   #INTMODER    ; NO. GET ERROR CODE.
 *           JMP    RPTRERR
 * RONPACC2: LDAA   #$10         ; GET BIT TO ENABLE INTERRUPT.
 *           TSTB                ; WAS THE ARGUMENT 0?
 *           BEQ    RONPACC3     ; YES. GO ENABLE INTS. ON EACH COUNT.
 *           LSLA                ; NO. ENABLE INTS. ON PACC OVERFLOW ONLY.
 * RONPACC3: LDX    IOBaseV
 *           STAA   TMSK2,X
 *           JSR    PULNUM       ; GET THE COUNT MODE OFF THE STACK.
 *           BNE    RONPACC4     ; GO SET THE MODE IF NOT 0.
 *           LDX    IOBaseV
 *           CLR    PACTL,X      ; TURN OFF THE PULSE ACCUMULATOR.
 *           STD    ONPACLIN     ; CLEAR POINTER TO LINE NUMBER.
 *           BRA    RONPACC6     ; GO CLEAN UP & RETURN.
 * RONPACC4: CPD    #4           ; IS THE ARGUMENT IN RANGE?
 *           BHI    RONPACC5     ; YES. REPORT AN ERROR.
 *           ADDB   #3           ; GET BIT TO ENABLE PACC.
 *           LSLB
 *           LSLB
 *           LSLB
 *           LSLB
 *           LDX    IOBaseV
 *           STAB   PACTL,X      ; ENABLE THE PACC & SET MODE.
 * RONPACC6: PULA                ; GET OLD I-BIT STATUS OFF STACK.
 *           TAP                 ; RESTORE OLD STATUS.
 *           LDAB   #3
 *           ABY                 ; PASS UP LINE NUMBER.
 *           RTS                 ; RETURN.
 * *
 * *
 */

/*
 *  rlabel      execute LABEL token
 */
void	rlabel(void)
{
	//int		len = 0;
	rskipspc();
	tbufptr += *(tbufptr+1) + 2;
}
