/*
 *  runtime1      run-time routines for the Basic11 project
 */

#include  <stdio.h>
#include  <ctype.h>
#include  <string.h>


#include  "basictime.h"
#include  "defines.h"
#ifdef  AVR
#include  "farmem.h"
#endif
#include  "funcs.h"




/*
 *  rrem      execute REM token
 */
void  rrem(void)
{
	tbufptr = tbufptr + *tbufptr;		// skip non-executable statement
}


/*
 *  rdata      execute DATA token
 */
void  rdata(void)
{
	tbufptr = tbufptr + *tbufptr;		// skip non-executable statement
}


/*
 * RREM:     EQU    *            ; NON-EXECUTIBLE STATEMENT JUST SKIP IT.
 * RDATA:    EQU    *
 *           LDAB   0,Y          ; GET LENGTH OF REMARK OR DATA LINE.
 *           ABY                 ; POINT TO THE EOLTOK.
 *           RTS                 ; RETURN.
 */
void	rstrset(U8 *dststrptr, U8 *srcstrptr)
{
	int		dststrlength = dststrptr[0];
	int		srcstrlength = srcstrptr[0];
	memcpy(&dststrptr[1], &srcstrptr[1], srcstrlength);
	dststrptr[0] = srcstrlength;
}


/*
 *  rlet      execute LET token
 */
void  rlet(void)
{
	U16						voff;
	U32						val;
	U32						*vptr;
	unsigned char			vflag;
	U16						sub;
	U8						indir;
	U32						taddr;

	vptr = 0;							// stupid compiler!
	indir = 0;							// show no indirection (yet)
	vflag = *tbufptr;					// save a copy of the variable type
	if ((vflag == INDIRTOK)	|| (vflag == INDIR32TOK) || (vflag == INDIR16TOK))  // if this is an indirect store...
	{
		indir = vflag;					// show we are doing indirection
		tbufptr++;						// move to the variable type
		rskipspc();						// allow for spaces after indirection operator
		vflag = *tbufptr;				// get variable type
	}

	voff = rvarptr();					// set up pointer to variable
	if (vflag == IAVARTOK)				// if this is an array...
	{
		tbufptr++;						// step over open paren
		donexp();						// resolve the subscript (no leading spaces!)
		tbufptr++;						// step over close paren
		if (errcode)  return;			// if problems, leave now
		vptr = targetgetvarptr(voff);	// get pointer into dyn mem pool
#ifdef  AVR
		vptr = (U32 *)((U16)*vptr);		// now get AVR (16-bit) pointer to data area for array
#else
		vptr = (U32 *)(getU32(vptr));			// now get pointer to data area for array
#endif
		sub = pull32(&numstack, STKUNDER);	// get subscript from stack
		if (sub >= getU16((U16 *)vptr))		// if subscript is out of range...
		{
			errcode = SUBORERR;			// show subscript error
			return;						// and done
		}
//		(U8 *)vptr = (U8 *)vptr + sizeof(U16);		// move to start of array data area
//		(U8 *)vptr = (U8 *)vptr + (sub * sizeof(U32));	// move to desired cell
		vptr = (U32 *)((U8 *)vptr + sizeof(U16));		// move to start of array data area
		vptr = vptr + sub;				// move to desired cell
	}

	rskipspc();							// step over any spaces
	tbufptr++;							// step past the '=' token
	if (vflag == SVARTOK)
	{
		U8	tok = tbufptr[0];
		if (tok == SVARTOK)
		{
			U16	dstvoff = rvarptr();
			rstrset((U8*)targetgetvarptr(voff), (U8*)targetgetvarptr(dstvoff));
		}
		else
		if (tok == SCONTOK)
		{
			tbufptr++;
			rstrset((U8*)targetgetvarptr(voff), tbufptr);
			tbufptr += *tbufptr + 1;
		}
		else
		{
			errcode = DTMISERR;
			return;
		}
	}
	else
	{
		donexp();							// evaluate the expression
		if (errcode)  return;				// if problems, leave now
		val = pull32(&numstack, STKUNDER);	// pull value from number stack
		if (vflag == PVARTOK)				// if saving to a port...
		{
			targetwriteport(voff, val);		// use target code to modify the I/O port
		}
		else if (vflag == IAVARTOK)			// if writing to an array...
		{
			setU32(vptr, val);					// pointer is all set, just write it
		}
		else								// must be a simple pointer
		{
	#ifdef  AVR
			taddr = (U32)targetgetvarptr(voff);
			if (indir)						// if using indirection, use contents of variable as address
			{
				taddr = *(U32 *)taddr;		// this only works if variables are stored at address below 64K!
			}
			if (indir == INDIRTOK)			// if doing 8-bit indirection...
			{
				__write_byte_far(taddr, val);
			}
			else if (indir == INDIR16TOK)	// if doing 16-bit indirection...
			{
				__write_U16_far(taddr, val);
			}
			else							// either doing straight store or 32-bit indirection...
			{
				__write_U16_far(taddr, val);	// write low half
				taddr = taddr + 2;
				__write_U16_far(taddr, (val >> 16));
			}
	#else
			vptr = (U32 *)targetgetvarptr(voff);
			if (indir)						// if using indirection, use contents of variable as address
			{
				vptr = (U32 *)getU32(vptr);
			}
			if (indir == INDIRTOK)			// if doing 8-bit indirection...
			{
				*(U8 *)vptr = val;
			}
			else if (indir == INDIR16TOK)	// if doing 16-bit indirection...
			{
				setU16((U16 *)vptr, val);
			}
			else							// either doing straight store or 32-bit indirection...
			{
				setU32(vptr, val);					// save value to variable RAM
			}
	#endif
		}
	}
}

/*
 * RLET:     EQU    *
 *           LDAA   0,Y          ; GET VARIABLE FLAG.
 *           BSR    RVARPTR      ; GET POINTER TO ASIGNMENT VARIABLE.
 * *        PSHD                SAVE POINTER TO VARIABLE.
 *           PSHB
 *           PSHA
 *           INY                 ; PUT IP PAST THE "=" TOKEN.
 *           JSR    DONEXP       ; EVALUATE THE EXPRESSION.
 *           JSR    PULNUM       ; GET VALUE INTO D.
 *           PULX                ; POINT TO THE DICTIONARY ENTRY.
 *           STD    0,X          ; STORE VALUE.
 *           RTS                 ; BACK TO MAIN INTERPRET LOOP.
 */


/*
 *  rvarptr      execute the VARPTR token
 */
U16  rvarptr(void)
{
	unsigned char				vflag;
	unsigned int				voff;

	voff = 0;						// just in case
	vflag = *tbufptr++;				// get variable type
//	if (vflag & ARRAY_MASK)			// if this is an array...
//	{
//	}

	if (vflag == IAVARTOK)			// if this is an array...
	{
		voff = getU16((U16 *)tbufptr);		// compute offset from start of variable area
		tbufptr = tbufptr + sizeof(U16);	// point past offset
		voff = voff + MAX_VAR_NAME_LEN + 1;		// magic number!  offset now points to data area of variable entry
	}
	else if (vflag == SVARTOK)		// if this is a string variable...
	{
		voff = getU16((U16 *)tbufptr);		// compute offset from start of variable area
		tbufptr = tbufptr + sizeof(U16);	// point past offset
		voff = voff + MAX_VAR_NAME_LEN + 1;		// magic number!  offset now points to data area of variable entry
	}
	else if (vflag == PVARTOK)		// if this is a port address...
	{
		voff = getU16((U16 *)tbufptr);		// get address of port
		tbufptr = tbufptr + sizeof(U16);	// point past offset
	}
	else							// must be a simple variable...
	{
		voff = getU16((U16 *)tbufptr);		// compute offset from start of variable area
		tbufptr = tbufptr + sizeof(U16);	// point past offset
		voff = voff + MAX_VAR_NAME_LEN + 1;		// magic number!  offset now points to data area of variable entry
	}
	return  voff;
}

/*
 * RVARPTR:  LDAA   0,Y          ; GET VARIABLE FLAG.
 *           BITA   #$02         ; IS IT A STRING VARIABLE?
 *           BNE    RVARPTR2     ; YES. GO GET POINTER FOR A STRING DESCRIPTOR.
 *           BITA   #$10         ; IS IT A NUMERIC ARRAY VARIABLE?
 *           BNE    RVARPTR1     ; YES. GO CALCULATE THE SUBSCRIPT.
 * RVARPTR3: LDD    1,Y          ; GET THE OFFSET TO THE DICTIONARY ENTRY.
 *           ADDD   VARBEGIN     ; ADD IN THE START ADDRESS OF THE DICTIONARY.
 *           ADDD   #3           ; MAKE POINTER POINT TO THE ACTUAL STORAGE LOCATION
 *           PSHB                ; SAVE B.
 *           LDAB   #3           ; POINT TO THE FIRST ELEMENT PAST THE VARIABLE.
 *           ABY
 *           PULB                ; RESTORE B.
 *           RTS
 * RVARPTR1: EQU    *
 *           JSR    CALCSUB      ; GO GET BASE ADDR & EVALUATE SUBSCRIPT EXPRESSION.
 *           PSHX                ; PUSH BASE ADDRESS ONTO STACK.
 *           TSX                 ; POINT TO IT.
 *           LSLD                ; MULT THE SUBSCRIPT BY THE # OF BYTES/ELEMENT.
 * RVARPTR4: ADDD   0,X          ; GET ADDRESS OF ELEMENT.
 *           PULX                ; RESTORE X.
 *           RTS                 ; RETURN.
 * RVARPTR2: EQU    *
 *           BITA   #$10         ; IS IT A STRING ARRAY?
 *           BEQ    RVARPTR3     ; NO. JUST GO GET POINTER TO DESCRIPTOR.
 *           JSR    CALCSUB      ; GET BASE ADDR. & CALC SUBSCRIPT.
 *           PSHX                ; SAVE THE BASE ADDRESS.
 * *        PSHD                SAVE THE SUBSCRIPT VALUE.
 *           PSHB
 *           PSHA
 *           TSX                 ; POINT TO THE VALUES.
 *           LSLD                ; MULT BY 2.
 *           ADDD   0,X          ; MULT BY 3.
 *           INS                 ; GET RID OF SUBSCRIPT VALUE.
 *           INS
 *           TSX                 ; POINT TO BASE ADDRESS.
 *           BRA    RVARPTR4
 */


/*
 *  rgoto      execute GOTO token
 *
 *  In Gordon's original code, this routine set up several
 *  global variables, then jumped into the middle of crun().
 *  Since this is not an option in C programs, I've recoded
 *  rgoto() to do the preparation, then control returns to
 *  crun().  crun() must be modified to treat the GOTOTOK as
 *  a special case and call this routine.
 */

void  rgoto(void)
{

	rskipspc();						// step over any spaces
	if(*tbufptr == LCONTOK){
		unsigned int					nextln;
		unsigned char					*srchptr;
		unsigned int					targetln;
		tbufptr++;						// step over the LCONTOK token
		targetln = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);

		if (immid)						// if goto was entered from console...
		{
			adrnxlin = basend;			// set search addr to end of program
		}
		srchptr = adrnxlin;				// start the search at the next line
		if (srchptr == basend)  srchptr = basbeg;	// rewind if hit the end
		nextln = getU16((U16 *)srchptr);		// get next line number
		if (nextln > targetln)  srchptr = basbeg;	// if current line is too high, start at beginning

		while (1)
		{
			nextln = getU16((U16 *)srchptr);	// get next line number
			if (nextln == targetln)		// if found the line we want...
			{
				tbufptr = srchptr;		// use new line as current line
	//			tbufptr = tbufptr + sizeof(U16);	// move to length byte
	//			tbufptr++;				// step over length byte to first token in line
	//			rskipspc();				// cannot point to space at start of line!
				if (immid)
				{
					immid = 0;			// show we are running, not in immediate mode
					runflag = TRUE;		// turn on the run flag
					_crun();			// restart the inner interpreter
					return;				// return to the outer interpreter
				}
				else					// not in immediate mode...
				{
					tbufptr = tbufptr + sizeof(U16);	// move to length byte
					tbufptr++;			// step over length byte to first token in line
					rskipspc();			// cannot point to space at start of line!
					return;				// let crun() finish the transfer
				}
			}
			srchptr = srchptr + *(srchptr+2);	// move to next line
			if ((nextln > targetln)	||	// if we are past the target...
				(srchptr == basend))	// or hit the end of the program...
			{
				errcode = LNFERR;		// line number doesn't exist (don't report yet)
				return;
			}
		}
	}else if(*tbufptr == SCONTOK){
		unsigned char					*nextlnptr;
		unsigned char					*srchptr;

		srchptr = basbeg;				// start the search at the next line

		while (1)
		{
			nextlnptr = srchptr;
			nextlnptr += sizeof(U16);	// move to length byte
			nextlnptr++;				// step over length byte to first token in line
			switch (*nextlnptr)
			{
				case  SSCNTOK:			// token for single space
				nextlnptr++;				// skip past the token
				break;

				case  MSCNTOK:			// token for multiple spaces
				nextlnptr += 2;	// skip past token plus count byte
				break;
			}
			if(*nextlnptr == LABELTOK){
				nextlnptr++;
				switch (*nextlnptr)
				{
					case  SSCNTOK:			// token for single space
					nextlnptr++;				// skip past the token
					break;

					case  MSCNTOK:			// token for multiple spaces
					nextlnptr += 2;	// skip past token plus count byte
					break;
				}
				if(memcmp(tbufptr, nextlnptr, *(tbufptr + 1)) == 0){
					tbufptr = srchptr;		// use new line as current line
		//			tbufptr = tbufptr + sizeof(U16);	// move to length byte
		//			tbufptr++;				// step over length byte to first token in line
		//			rskipspc();				// cannot point to space at start of line!
					if (immid)
					{
						immid = 0;			// show we are running, not in immediate mode
						runflag = TRUE;		// turn on the run flag
						_crun();			// restart the inner interpreter
						return;				// return to the outer interpreter
					}
					else					// not in immediate mode...
					{
						tbufptr = tbufptr + sizeof(U16);	// move to length byte
						tbufptr++;			// step over length byte to first token in line
						rskipspc();			// cannot point to space at start of line!
						return;				// let crun() finish the transfer
					}
				}
			}
			srchptr = srchptr + *(srchptr+2);	// move to next line
			if (srchptr >= basend)		// hit the end of the program...
			{
				errcode = LNFERR;		// line number doesn't exist (don't report yet)
				return;
			}
		}
	}
}

/*
 * RGOTO:    EQU    *
 *           TST    IMMID        ; DID WE ENTER HERE FROM THE IMMIDIATE MODE?
 *           BEQ    RGOTO7       ; NO. JUST GO DO A GOTO.
 *           LDD    BASEND       ; YES. SET ADRNXLIN TO END OF PROGRAM SO THE
 *           STD    ADRNXLIN     ; SEARCH STARTS AT THE FIRST LINE.
 * RGOTO7:   LDX    ADRNXLIN     ; POINT TO THE START OF THE NEXT LINE.
 *           CPX    BASEND       ; IS THIS THE LAST LINE OF THE PROGRAM?
 *           BNE    RGOTO1       ; NO. SEARCH STARTING AT THE NEXT LINE.
 * RGOTO3:   LDX    BASBEG       ; YES. POINT TO THE START OF THE BASIC PROGRAM.
 *           BRA    RGOTO2
 * RGOTO1:   LDD    0,X          ; GET THE NEXT LINE NUMBER IN THE PGM.
 *           CPD    1,Y          ; IS IT > THE LINE NUMBER WE ARE TO "GOTO"?
 *           BHI    RGOTO3       ; YES. START THE SEARCH AT THE BEGINING.
 * RGOTO2:   LDD    0,X          ; GET THE NEXT LINE NUMBER INTO D.
 *           CPD    1,Y          ; IS THIS THE CORRECT LINE?
 *           BEQ    RGOTO4       ; YES. "GOTO" THE NEW LINE.
 *           BLO    RGOTO5       ; NO. IS IT LESS THAN THE "TARGET LINE"?
 * RGOTO6:   LDAA   #LNFERR      ; NO. THE LINE MUST NOT EXIST.
 *           JMP    RPTRERR      ; REPORT THE ERROR & RETURN TO MAIN LOOP.
 * RGOTO5:   LDAB   2,X          ; GET THE LENGTH OF THIS LINE.
 *           ABX                 ; POINT TO THE START OF THE NEXT LINE.
 *           CPX    BASEND       ; DID WE HIT THE END OF THE PROGRAM?
 *           BEQ    RGOTO6       ; YES. THE LINE DOESN'T EXIST.
 *           BRA    RGOTO2       ; NO. GO SEE IF THIS IS THE CORRECT LINE.
 * RGOTO4:   XGDX                ; MAKE IT THE NEW IP.
 *           XGDY
 *           TST    IMMID
 *           BEQ    RGOTO8
 *           CLR    IMMID
 * RGOTO9:   JMP    CRUN1
 * RGOTO8:   INS
 *           INS
 *           BRA    RGOTO9
 */



/*
 *  rpreparegosub      prepare to execute the GOSUB token
 *
 *  This routine was originally named rgosub().  In Gordon's
 *  implementation, it adjusted the IP (Y-reg), then jumped
 *  into the middle of the crun() function.  Since jumping
 *  into the middle of a C function isn't safe, I've changed
 *  this routine so it prepares the global variables for
 *  a change to a different line number.  This routine
 *  should be called from inside crun() as a special case.
 *  When control returns from this routine, the globals,
 *  including tbufptr, will be set up properly for crun()
 *  to execute.
 *
 *  Note that this routine might detect an error.  After
 *  calling this routine, crun() should test errcode to
 *  make sure no error was detected before continuing.
 */
void  rpreparegosub(void)
{
	unsigned char			*tptr;

	rskipspc();						// move past any spaces
	tptr = tbufptr;					// tbufptr points to line number constant of target line
	if (immid)						// if immediate
	{
		tbufptr = basend - 1;		// use last EOL as return address
	}
	else if(*tbufptr == LCONTOK)
	{
		tbufptr = tbufptr + 3;		// step past line number constant and line number to next token
	}
	else if(*tbufptr == SCONTOK)
	{
		tbufptr += *(tbufptr + 1) + 2;
	}
	rskipspc();						// step over spaces
	if (goindex == GOSLEN)			// if already full...
	{
		errcode = GOSOVERR;			// record the error
		return;
	}
	gostack[goindex++] = tbufptr;	// record the return address
	tbufptr = tptr;					// restore addr of line number constant of target line
}

/*
 * RGOSUB:   EQU    *
 *           PSHY                ; SAVE THE I.P. TO THE LINE NUMBER.
 *           TST    IMMID        ; DID WE GET HERE FROM THE IMMIDIATE MODE?
 *           BEQ    RGOSUB3      ; NO. GO DO A NORMAL GOSUB.
 *           LDY    BASEND       ; YES. MAKE RETURN POINT TO THE LAST EOL TOKEN
 *           DEY                 ; IN THE PROGRAM.
 *           BRA    RGOSUB2      ; GO PUT IT ON THE ARGUMENT STACK.
 * RGOSUB3:  LDAB   #3           ; BYPASS THE LINE NUMBER.
 *           ABY
 * RGOSUB2:  JSR    RSKIPSPC     ; SKIP SPACES AFTER THE LINE NUMBER.
 *           LDX    GOSTACK      ; GET THE GOSUB STACK POINTER.
 *           DEX                 ; POINT TO THE NEXT ENTRY ON THE STACK.
 *           DEX
 *           CPX    EGOSTK       ; OUT OF STACK SPACE?
 *           BHS    RGOSUB1      ; NO. GO PUSH THE "RETURN ADDRESS" ON THE STACK.
 *           LDAA   #GOSOVERR    ; YES. GET THE ERRCODE.
 *           JMP    RPTRERR      ; GO REPORT THE ERROR.
 * RGOSUB1:  STX    GOSTACK      ; SAVE THE "GOSUB" STACK POINTER.
 *           STY    0,X          ; PUT THE RETURN ADDRESS ON THE STACK.
 *           PULY                ; GET THE POINTER TO THE LINE NUMBER.
 *           JMP    RGOTO        ; GO DO A "GOTO".
 */

void  rinterrupt(unsigned int targetln)
{
	unsigned int					nextln;
	unsigned char					*srchptr;

	if (goindex == GOSLEN)			// if already full...
	{
		errcode = GOSOVERR;			// record the error
		return;
	}
	gostack[goindex++] = tbufptr;	// record the return address

	srchptr = adrnxlin;				// start the search at the next line
	if (srchptr == basend)  srchptr = basbeg;	// rewind if hit the end
	nextln = getU16((U16 *)srchptr);		// get next line number
	if (nextln > targetln)  srchptr = basbeg;	// if current line is too high, start at beginning

	while (1)
	{
		nextln = getU16((U16 *)srchptr);	// get next line number
		if (nextln == targetln)		// if found the line we want...
		{
			tbufptr = srchptr;		// use new line as current line
			tbufptr = tbufptr + sizeof(U16);	// move to length byte
			tbufptr++;			// step over length byte to first token in line
			rskipspc();			// cannot point to space at start of line!
			return;				// let crun() finish the transfer
		}
		srchptr = srchptr + *(srchptr+2);	// move to next line
		if ((nextln > targetln)	||	// if we are past the target...
			(srchptr == basend))	// or hit the end of the program...
		{
			errcode = LNFERR;		// line number doesn't exist (don't report yet)
			return;
		}
	}
}


/*
 *  rreturn      execute RETURN token
 */
void  rreturn(void)
{
	if (goindex == 0)				// if stack is empty...
	{
		errcode = RWOGERR;			// record the error
	}
	else
	{
		tbufptr = gostack[--goindex];	// pull the return address
	}
}

/*
 * RRETURN:  EQU    *
 *           LDX    GOSTACK      ; GET THE GOSUB STACK POINTER.
 *           CPX    STGOSTK      ; IS THERE A RETURN ADDRESS ON THE GOSUB STACK?
 *           BNE    RRETURN1     ; YES. GO RETURN.
 *           LDAA   #RWOGERR     ; NO. RETURN W/O GOSUB ERROR.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * RRETURN1: LDY    0,X          ; GET THE RETURN ADDRESS IN THE IP.
 *           INX                 ; REMOVE THE ADDRESS FROM THE STACK.
 *           INX
 *           STX    GOSTACK      ; SAVE THE STACK POINTER.
 *           RTS                 ; BACK TO THE MAIN INTERPRET LOOP.
 */


/*
 *  rstop      execute STOP token
 */
void  rstop(void)
{
	pl_P(PSTR("\n\rStopped at line "));
	outdeci(curline);
	contflag = FALSE;				// show ok to continue
	ipsave = adrnxlin;				// will continue from next line (is this good enough?)
	curline = 0;					// set current line to start of program
	tbufptr = basend - 1;			// point to final EOL token in program (can't jump to main() in C)
}

/*
 * RSTOP:    EQU    *
 *           LDX    #STOPSTR
 *           JSR    PL
 *           LDD    CURLINE
 *           JSR    OUTDECI
 *           STY    IPSAVE
 *           BRA    REND1
 */
  
/*
 *  rend      execute END token
 */
void  rend(void)
{
	nl();									// make it pretty
	contflag = TRUE;						// show NOT ok to continue
	curline = 0;							// show current line is 0 (start of program)
	tbufptr = basend - 1;					// point to final EOL token in program (can't jump to main() in C)
}

/* REND:     EQU    *
 *           JSR    NL
 *           LDAA   #1
 *           STAA   CONTFLAG
 * REND1:    LDD    #0
 *           STD    CURLINE
 *           JMP    MAINW
 *
 *
 * STOPSTR:  FCB    $0A,$0D
 *           FCC    "STOPPED AT LINE # "
 *           FCB    0
 */

/*
 *  rwhile      execute WHILE token
 */
void  rwhile(void)
{
	int					count;

	if (whindex == WHSLEN)				// at top of stack?
	{
		errcode = WHSOVERR;				// show overflow
		rpterr();
		return;
	}
	whstack[whindex] = tbufptr;			// save current token buffer pointer
	whindex++;							// move to next cell
	count = 1;
	while (1)
	{
		tbufptr = adrnxlin;				// check next line
		if (tbufptr == 0)  return;		// if none, leave now
		if (tbufptr == basend)			// if hit the end of the program...
		{
			rend();						// end the run
			return;
		}
		adrnxlin = adrnxlin + *(adrnxlin+2);	// move to next line
		tbufptr = tbufptr + 3;			// point past line number and len
		rskipspc();						// move past any spaces
		if (*tbufptr == WHILETOK)  count++;		// if found a WHILE statement, count it
		else if (*tbufptr == ENDWHTOK)			// if found an ENDWH statement...
		{
			count--;					// show found a matching ENDWH
			if (count == 0)				// if found the one we want...
			{
				return;
			}
		}
	}
}

/*
 * RWHILE:   EQU    *
 *           LDX    WHSTACK      ; GET THE WHILE STACK POINTER.
 *           DEX                 ; POINT TO THE NEXT STACK LOCATION.
 *           DEX
 *           CPX    EWHSTK       ; ARE WE AT THE END OF THE STACK?
 *           BHS    RWHILE4      ; NO. GO STACK IT.
 *           LDAA   #WHSOVERR    ; YES. WHILE STACK OVER FLOW.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * RWHILE4:  STX    WHSTACK      ; SAVE THE WHILE STACK POINTER.
 *           STY    0,X          ; PUT IT ON THE STACK.
 *           LDAB   #$01         ; GET THE WHILE COUNT INTO B. (FOR NESTED WHILE'S)
 * RWHILE3:  PSHB
 *           LDY    ADRNXLIN     ; GET THE ADDRESS OF THE NEXT LINE.
 *           BNE    RWHILE2      
 *           RTS
 * RWHILE2:  PSHY                ; SAVE THE IP.
 *           CPY    BASEND       ; ARE WE AT THE END OF THE PROGRAM?
 *           BEQ    REND         ; YES. DO AN END.
 *           LDX    ADRNXLIN     ; NO. GET THE ADDRESS OF THE NEXT LINE IN X.
 *           LDAB   2,X          ; GET THE LENGTH OF THIS LINE.
 *           ABX                 ; POINT TO THE START OF THE NEXT LINE.
 *           STX    ADRNXLIN     ; SAVE IT.
 *           LDAB   #3           ; POINT PAST THE LINE NUMBER & LINE LENGTH.
 *           ABY
 *           JSR    RSKIPSPC     ; SKIP ANY SPACES.
 *           LDAA   0,Y          ; GET THE KEYWORD TOKEN.
 *           PULY                ; RESTORE THE IP.
 *           PULB                ; GET THE NESTED WHILE COUNT.
 *           CMPA   #WHILETOK    ; IS IT ANOTHER WHILE?
 *           BNE    RWHILE1      ; NO. GO CHECK FOR ENDWH.
 *           INCB                ; YES. UP THE NESTED WHILE COUNT.
 * RWHILE1:  CMPA   #ENDWHTOK    ; IS IT THE END WHILE STATEMENT?
 *           BNE    RWHILE3      ; NO. GO LOOK AT THE NEXT LINE.
 *           DECB                ; YES. IS IT THE CORRECT 'ENDWH'?
 *           BNE    RWHILE3      ; NO. LOOK FOR ANOTHER ONE.
 *           JMP    RGOTO8       ; BACK TO INTERPRET LOOP.
 */


/*
 *  rendwh      execute ENDWH token
 */
void  rendwh(void)
{
	unsigned char					*tptr;

	if (whindex == 0)						// if no WHILE statement has been executed yet...
	{
		errcode = ENDWHERR;					// show ENDWH with no WHILE
		rpterr();							// tell the world
		return;
	}
	tptr = tbufptr;							// save in case this doesn't work
	tbufptr = whstack[whindex-1];			// point to WHILE expression
	donexp();								// evaluate the expression
	if (pull32(&numstack, STKUNDER) == 0)		// if expression is FALSE... 
	{
		tbufptr = tptr;						// need to continue after ENDWH
		whindex--;							// back up to valid cell
	}
}

/*
 * RENDWH:   EQU    *
 *           LDX    WHSTACK      ; GET THE WHILE STACK POINTER.
 *           CPX    STWHSTK      ; HAS A WHILE STATEMENT BEEN EXECUTED?
 *           BNE    RENDWH1      ; YES. GO GET THE ADDRESS OF THE WHILE STATEMENT.
 *           LDAA   #ENDWHERR    ; NO. GET ENDWHILE ERROR.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * RENDWH1:  PSHY                ; SAVE THE IP IN CASE THE WHILE TEST FAILS.
 *           LDY    0,X          ; GET THE IP POINTER TO THE WHILE EXPRESSION.
 *           JSR    DONEXP       ; YES. GO EVALUATE A NUMERIC EXPRESSION.
 *           JSR    PULNUM       ; GET RESULT OFF NUMERIC STACK. IS IT TRUE?
 *           BNE    RENDWH3      ; YES. GO EXECUTE CODE BETWEEN WHILE & ENDWH.
 *           PULY                ; NO. GET THE ADDRESS OF THE NEXT LINE/STATEMENT.
 *           LDX    WHSTACK      ; GET WHILE STACK POINTER.
 *           INX                 ; TAKE ADDRESS OFF OF WHILE STACK.
 *           INX
 *           STX    WHSTACK      ; SAVE STACK POINTER.
 *           BRA    RENDWH5      ; GO TO INTERPRET LOOP.
 * RENDWH3:  INS                 ; REMOVE POINTER TO STATEMENT AFTER "ENDWH"
 *           INS                 ; FROM STACK.
 * RENDWH5:  RTS                 ; GO EXECUTE LINES TILL "ENDWH".
 */


/*
 *  ron      execute ON token
 */
void  ron(void)
{
	U8	tok;
	I32	i;

	rskipspc();						// move past any spaces
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
	if(*tbufptr == RXDATATOK){
		tbufptr++;
		rrxdata();
	}else if(*tbufptr == ACKTOK){
		tbufptr++;
		rack();
	}else if(*tbufptr == EVENTTOK){
		tbufptr++;
		revent();
	} else {
#elif defined(SKBASIC_EMBEDDED)
	if(*tbufptr == TIM1OK){
		tbufptr++;
		rtim1();
	} else if(*tbufptr == TIM2OK){
		tbufptr++;
		rtim2();
	} else {
#else
	{
#endif
		donexp();						// evaluate the expression
		rskipspc();						// move past any spaces
		tok = *tbufptr++;
		rskipspc();						// move past any spaces
		i = pull32(&numstack, STKUNDER);
		if(i > 0){
		}else{
			errcode = ONARGERR;				//show divide-by-0 error
			rpterr();
			return;
		}
	}
}

/*
 * RON:      EQU    *
 *           JSR    DONEXP       ; GO EVALUATE THE EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER EXPRESSION.
 *           LDAA   0,Y          ; GET EITHER "GOTO" OR "GOSUB" TOKEN.
 *           PSHA                ; SAVE IT.
 *           INY                 ; POINT TO NEXT TOKEN.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDX    NUMSTACK     ; POINT TO THE OPERAND STACK.
 *           LDD    0,X          ; GET EXPRESSION VALUE.
 *           BPL    RON1         ; IS IT NEGATIVE?
 *           BNE    RON1         ; OR ZERO?
 * RON5:     LDAA   #ONARGERR    ; YES. REPORT ERROR.
 *           JMP    RPTRERR
 * RON1:     LDD    0,X          ; GET THE EXPRESSION VALUE.
 *           SUBD   #1           ; SUBTRACT 1. HAVE WE FOUND THE LINE NUMBER?
 *           BEQ    RON4         ; YES. GO DO "GOTO" OR "GOSUB".
 *           STD    0,X          ; NO. SAVE REMAINDER.
 *           LDAB   #3           ; POINT PAST THE LINE NUMBER VALUE.
 *           ABY
 *           JSR    RSKIPSPC     ; SKIP SPACES PAST THE LINE NUMBER.
 *           LDAA   0,Y          ; GET NEXT TOKEN.
 *           CMPA   #EOLTOK      ; HAVE WE HIT THE END OF THE LINE?
 *           BEQ    RON5         ; YES. ERROR.
 * RON3:     INY                 ; NO. MUST BE A COMMA. BYPASS IT.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER THE COMMA.
 *           BRA    RON1         ; GO SEE IF THE NEXT LINE NUMBER IS THE ONE.
 * RON4:     JSR    PULNUM       ; GET RID OF ARGUMENT.
 *           PULA                ; GET "GO" TOKEN.
 *           CMPA   #GOTOTOK     ; IS IT A "GOTO" TOKEN?
 *           BNE    RON6         ; NO. MUST BE A "GOSUB"
 *           JMP    RGOTO        ; GO DO A "GOTO".
 * RON6:     PSHY                ; SAVE THE POINTER TO THE LINE NUMBER.
 * RON8:     LDAB   #3           ; POINT PAST THE LINE NUMBER.
 *           ABY
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER LINE NUMBER.
 *           LDAA   0,Y          ; GET NEXT TERMINATOR CHARACTER.
 *           CMPA   #EOLTOK      ; HIT THE END OF THE LINE YET?
 *           BEQ    RON7         ; YES. GO DO THE GOSUB.
 *           CMPA   #MEOLTOK     ; NO. HIT THE LOGICAL END OF THE LINE YET?
 *           BEQ    RON7         ; YES. GO DO THE GOSUB.
 *           INY                 ; NO. MUST BE A COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER THE COMMA.
 *           BRA    RON8         ; GO FIND THE END OF THE LINE.
 * RON7:     JMP    RGOSUB2      ; GO DO A "GOSUB".
 */

/*
 *  rpoke      execute POKE token
 */
void  rpoke(void)
{
}

/* RPOKE:    EQU    *
 *           INY                 ; PASS UP THE OPEN PAREN.
 *           JSR    RSKIPSPC     ; PASS UP ANY SPACES.
 *           JSR    DONEXP       ; GO EVALUATE THE ADDRESS EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP ANY SPACES.
 *           INY                 ; SKIP THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP ANY SPACES.
 *           JSR    DONEXP       ; GET THE VALUE TO PUT INTO MEMORY.
 *           INY                 ; PASS UP THE CLOSING PAREN.
 *           JSR    PULNUM       ; GET THE MEMORY VALUE.
 *           XGDX                ; SAVE IT.
 *           JSR    PULNUM       ; GET THE ADDRESS.
 *           XGDX                ; PUT ADDRESS INTO X & MEM VALUE INTO D.
 *           STAB   0,X          ; PUT VALUE INTO MEMORY.
 *           RTS                 ; BACK TO THE INTERPRET LOOP.
 */



/*
 *  rporta      execute PORTA, PORTB, PORTC, PORTD tokens
 *
 *  These are all target-specific and have been moved to the
 *  appropriate targetxxx.c file.
 */

/*
 * RPORTA:   LDAB   #PORTAIO
 * RPORTA1:  LDX    IOBaseV     ;  GET ADDRESS OF PORTA I/O REGISTER.
 *           ABX
 *           PSHX                ; SAVE POINTER TO VARIABLE.
 *           INY                 ; PUT IP PAST THE "=" TOKEN.
 *           JSR    DONEXP       ; EVALUATE THE EXPRESSION.
 *           JSR    PULNUM       ; GET VALUE INTO D.
 *           TSTA                ; IS THE VALUE <0 AND >255?
 *           BEQ    RPORTA2      ; NO. GO PUT THE VALUE IN THE PORT.
 *           LDAA   #PRTASERR    ; YES. ERROR.
 *           JMP    RPTRERR      ; REPORT THE ERROR.
 * RPORTA2:  PULX                ; POINT TO THE DICTIONARY ENTRY.
 *           STAB   0,X          ; STORE VALUE.
 *           RTS                 ; BACK TO MAIN INTERPRET LOOP.
 *  
 * *
 * *
 * RPORTB:   LDAB   #PORTBIO     ; GET ADDRESS OF PORTB I/O REGISTER.
 *           BRA    RPORTA1      ; GO DO AN ASIGNMENT.
 * *
 * *
 * RPORTC:   LDAB   #PORTCIO     ; GET ADDRESS OF PORTC I/O REGISTER.
 *           BRA    RPORTA1      ; GO DO AN ASIGNMENT.
 * *
 * *
 * RPORTD:   LDAB   #PORTDIO     ; GET ADDRESS OF PORTD I/O REGISTER.
 *           BRA    RPORTA1      ; GO DO AN ASIGNMENT.
 * *
 */
