/*
 *  rexpres.c      run-time support routines for Basic11 project
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


#include  "basictime.h"
#include  "defines.h"

#ifdef  AVR
#include  "farmem.h"
#endif

#include  "funcs.h"


/*
 *  We need a custom random-number generator for both the Windows
 *  target and for the small MCUs such as the Atmel ATmega1284p.
 *
 *  The following function declarations and variable define a 32-bit
 *  generator, purportedly derived from "Numerical Recipies in C++."
 *  See also rrnd() below.
 */
static U32					b_random(void);
static void  				b_srandom(U32  newseed);

static U32					seed = RANDOM_SEED_VAL;



/*
 *  Local functions
 */
static void					_rfeep(U8  size);
static U32					isqrt(U32  val);
static I32					ipow(I32  base, I32  pwr);



/*
 *  donexp      run-time expression evaluator
 */
void  donexp(void)
{
	unsigned char				tok;

	pshop(OPARNTOK);			// push an open paren token as an end of expression marker
	while (1)
	{
		rskipspc();				// always skip spaces
		tok = *tbufptr;
		if (tok == OPARNTOK)	// if start of an expression...
		{
			tbufptr++;			// move to next token
			donexp();			// evaluate the subexpression (RECURSION!)
			tbufptr++;			// move to next token
		}
		else
		{
			if (tok & 0x80)		// if this is a constant or variable...
			{
				pshnum(tok);	// push value onto stack
				if (errcode)  return;	// if hit an error, bail now
			}
			else
			{
				tok = chknfun(tok);		// check for function that returns a number
				if (errcode)  return;	// if hit an error, bail now
				if (chckee(tok))  return;	// if at end, push close paren and execute stack, then return
				pshop(tok);		// not end of expression, push the operator onto stack
				tbufptr++;		// point to next token
			}
		}
	}
}

/*
 * DONEXP:   LDAA   #OPARNTOK    ; USE AN OPEN PAREN AS AN END OF EXPRESSION MARKER.
 *           JSR    PSHOP        ; PUSH OPEN PAREN ON THE STACK.
 * DONEXP1:  LDAA   0,Y          ; GET THE NEXT CHARACTER IN THE EXPRESSION.
 *           CMPA   #OPARNTOK    ; IS IT AN OPEN PAREN?
 *           BNE    DONEXP4      ; NO. CONTINUE.
 *           INY                 ; POINT TO NEXT TOKEN.
 *           BSR    DONEXP       ; GO DO A SUBEXPRESSION.
 *           INY                 ; MOVE THE IP PAST THE CLOSING PAREN.
 *           BRA    DONEXP1      ; GO GET THE NEXT CHARACTER.
 * DONEXP4:  TSTA                ; CHECK FOR OPERATOR OR OPERAND.
 *           BPL    DONEXP2      ; IF NOT VARIABLE OR CONSTANT, GO CHECK FOR FUNCT.
 *           BSR    PSHNUM       ; GO PUSH OPERAND ONTO STACK.
 *           BRA    DONEXP1      ; GO GET NEXT TOKEN.
 * DONEXP2:  JSR    CHKNFUN      ; GO CHECK FOR FUNCTION THAT RETURNS A NUMBER.
 *           JSR    CHCKEE       ; GO CHECK FOR END OF EXPRESSION.
 *           BCC    DONEXP3      ; IF NOT END OF EXPRESSION, GO PUSH OPERATOR.
 *           RTS                 ; IF AT END, RETURN.
 * DONEXP3:  INY                 ; POINT TO THE NEXT TOKEN.
 *           JSR    PSHOP        ; PUSH OPERATOR ONTO STACK.
 *           BRA    DONEXP1      ; GO GET NEXT TOKEN.
 */




/*
 *  pshnum      push a numeric operand onto the operand stack
 */
void  pshnum(unsigned char  tok)
{
	U16								offset;
	U32								val;
	U32								*dynptr;
	U16								sub;

	switch (tok)
	{
		case  IVARTOK:						// integer variable
//		case  FVARTOK:						// floating-point variable (for now)
		tbufptr++;							// move to offset address
		offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		val = getU32((U32 *)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1)));
		break;

		case  ICONTOK:
		tbufptr++;							// move to constant value
		val = getU32((U32 *)tbufptr);				// get constant value from buffer
		tbufptr = tbufptr + sizeof(U32);	// move past constant value
		offset = *tbufptr++;				// get length of constant string from buffer
		tbufptr = tbufptr + offset;			// move pointer past embedded string
		break;

		case  IAVARTOK:						// integer array
		tbufptr++;							// point to index int varram
		offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		dynptr = targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));	// get addr in dyn mem
#ifdef  AVR
		dynptr = (U32 *)((U16)*dynptr);		// now point to start of variable in dyn mem
#else
		dynptr = (U32 *)(getU32(dynptr));			// now point to start of variable in dyn mem
#endif
		tbufptr++;							// step over open paren
		donexp();							// compute subscript
		tbufptr++;							// step over closing paren
		sub = (U16)pull32(&numstack, STKUNDER);	// get the subscript
		if (sub >= getU16((U16 *)dynptr))			// if the subscript is too large...
		{
			errcode = SUBORERR;				// show subscript out of range
			return;
		}
//		(U8 *)dynptr = (U8 *)dynptr + sizeof(U16);	// move past subscript to start of data
		dynptr = (U32 *)((U8 *)dynptr + sizeof(U16));	// move past subscript to start of data
		dynptr = dynptr + sub;				// point to desired cell
		val = getU32(dynptr);						// get the value of selected cell
		break;

		case  PVARTOK:						// I/O port variable
		tbufptr++;							// move to offset address
		offset = getU16((U16 *)tbufptr);			// get index into port address table
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		val = (U32)targetreadport(offset);	// do target-specific read of port
		break;

		default:
		errcode = ILTOKERR;
		rpterr();
		return;
	}
	push32(&numstack, val, MSTKOERR);			// save value on run-time number stack
}

/*
 * *        PSHNUM SUBROUTINE
 * *
 * *        PUSHES A NUMERIC OPERAND (CONSTANT OR VARIABLE) VALUE ONTO THE
 * *        OPERAND STACK.
 * *
 * *
 * PSHNUM:   CMPA   #IVARTOK     ; IS IT AN INTEGER SCALER VARIABLE?
 *           BNE    PSHNUM1      ; NO. GO CHECK FOR CONSTANT.
 *           LDD    1,Y          ; YES. GET THE "OFFSET" ADDRESS.
 *           ADDD   VARBEGIN     ; ADD IN THE START ADDRESS OF THE VARIABLE TABLE.
 *           XGDX                ; GET THE ADDRESS INTO X.
 *           LDAB   #$03         ; BUMP INTERPRETER POINTER PAST "VARIABLE".
 *           ABY
 *           LDD    3,X          ; GET THE VARIABLE VALUE.
 *           BRA    PSHNUM4      ; GO PUT IT ON THE STACK.
 * PSHNUM1:  CMPA   #ICONTOK     ; IS IT AN INTEGER CONSTANT?
 *           BNE    PSHNUM2      ; NO. GO CHECK FOR AN INTEGER ARRAY VARIABLE.
 *           LDX    1,Y          ; GET THE CONSTANT VALUE INTO X.
 *           LDAB   #$04
 *           ADDB   3,Y
 *           ABY
 *           XGDX                ; PUT THE CONSTANT VALUE INTO D.
 *           BRA    PSHNUM4      ; GO PUT IT ON THE STACK.
 * PSHNUM2:  CMPA   #IAVARTOK    ; IS IT AN INTEGER ARRAY?
 *           BNE    PSHNUM3      ; NO. GO CHECK FOR A STRING VARIABLE.
 *           BSR    CALCSUB      ; GO GET BASE ADDR. & SUBSCRIPT OF ARRAY.
 *           PSHY                ; SAVE THE INTERPRETER POINTER.
 *           PSHX                ; PUT THE BASE ADDRESS OF THE ARRAY ON THE STACK.
 *           ASLD                ; MULTIPLY THE SUBSCRIPT BY THE # OF BYTES/ELEMENT.
 *           TSY                 ; POINT TO THE BASE ADDRESS.
 *           ADDD   0,Y          ; GET ADDRESS OF THE ELEMENT.
 *           PULX                ; RESTORE X.
 *           PULY                ; RESTORE Y
 *           XGDX                ; PUT ELEMENT ADDRESS INTO X.
 *           LDD    0,X          ; GET VALUE OF ELEMENT IN D.
 *           BRA    PSHNUM4 
 * PSHNUM3:  LDAA   #ILTOKERR
 *           JMP    RPTRERR
 * PSHNUM4:  LDX    NUMSTACK     ; GET THE OPERAND STACK POINTER.
 *           DEX                 ; MAKE ROOM ON THE STACK FOR NEW OPERAND.
 *           DEX
 *           CPX    ENUMSTK      ; HAS THE STACK OVERFLOWED?
 *           BHS    PSHNUM5      ; NO. GO STACK THE VALUE.
 *           LDAA   #MSTKOERR    ; YES.
 *           STAA   ERRCODE
 *           JMP    RPTRERR      ; GO REPORT THE ERROR.
 * PSHNUM5:  STX    NUMSTACK     ; SAVE THE STACK POINTER.
 *           STD    0,X          ; PUT THE VALUE ON THE STACK.
 *           RTS                 ; RETURN.
 */


/*
 *  pshaddr      push the address of a variable or port onto the operand stack
 */
void  pshaddr(unsigned char  tok)
{
	U16								offset;
	U32								val;
	U32								*dynptr;
	U16								sub;

	switch (tok)
	{
		case  IVARTOK:						// integer variable
//		case  FVARTOK:						// floating-point variable (for now)
		tbufptr++;							// move to offset address
		offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		val = (U32)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
		break;

		case  ICONTOK:
		errcode = CONADDRERR;				// cannot use ADDR() with constant
		rpterr();
		return;

		case  IAVARTOK:						// integer array
		tbufptr++;							// point to index int varram
		offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		dynptr = targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));	// get addr in dyn mem
#ifdef  AVR
		dynptr = (U32 *)((U16)*dynptr);		// now point to start of variable in dyn mem
#else
		dynptr = (U32 *)(getU32(dynptr));			// now point to start of variable in dyn mem
#endif
		tbufptr++;							// step over open paren
		donexp();							// compute subscript
		tbufptr++;							// step over closing paren
		sub = (U16)pull32(&numstack, STKUNDER);	// get the subscript
		if (sub >= getU16((U16 *)dynptr))			// if the subscript is too large...
		{
			errcode = SUBORERR;				// show subscript out of range
			return;
		}
//		(U8 *)dynptr = (U8 *)dynptr + sizeof(U16);	// move past subscript to start of data
		dynptr = (U32 *)((U8 *)dynptr + sizeof(U16));	// move past subscript to start of data
		dynptr = dynptr + sub;				// point to desired cell
		val = (U32)dynptr;						// get the address of selected cell
		break;

		case  PVARTOK:						// I/O port variable
		tbufptr++;							// move to offset address
		offset = getU16((U16 *)tbufptr);			// get index into port address table
		tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
		val = (U32)targetgetportaddr(offset);	// fetch address of port
		break;

		default:
		errcode = ILTOKERR;
		rpterr();
		return;
	}
	push32(&numstack, val, MSTKOERR);			// save value on run-time number stack
}




/*
 * *        THIS SUBROUTINE CALCULATES BOTH THE BASE ADDRESS AND THE
 * *        SUBSCRIPT OF THE ARRAY VARIABLE THAT IS CURRENTLY POINTED TO BY
 * *        THE Y-REG. IT CHECKS TO SEE IF THE VARIABLE HAS BEEN DIMENTIONED
 * *        AND IF THE SUBSCRIPT IS IN RANGE. THE ROUTINE RETURNS WITH THE
 * *        ADDRESS OF THE ARRAY IN THE X-REG. & THE SUBSCRIPT IN THE D-REG.
 * *
 * CALCSUB:  LDD    1,Y          ; GET THE VARIABLE OFFSET ADDRESS.
 *           ADDD   VARBEGIN     ; ADD IN THE START OF THE VARIABLE AREA.
 *           XGDX                ; PUT ADDRESS INTO X.
 *           LDX    3,X          ; GET THE ACTUAL STORAGE ADDRESS.
 * *                             ; HAS THE ARRAY BEEN DIMENTIONED?
 *           BNE    CALCSUB2     ; YES. CONTINUE.
 *           LDAA   #UNDIMERR    ; NO. UNDIMENTIONED ARRAY REFERENCE.
 * CALCSUB1: JMP    RPTRERR      ; GO REPORT THE ERROR.
 * CALCSUB2: LDAB   #$4          ; SET POINTER TO START OF SUBSCRIPT EXPRESSION.
 *           ABY
 *           PSHX                ; SAVE THE POINTER TO THE ARRAY STORAGE AREA.
 *           JSR    DONEXP       ; GO GET THE SUBSCRIPT.
 *           INY                 ; BUMP IP PAST THE CLOSING PAREN OF THE SUBSCRIPT.
 *           PULX                ; RESTORE X.
 *           JSR    PULNUM       ; GET SUBSCRIPT FROM THE OPERAND STACK.
 *           CPD    0,X          ; IS THE SUBSCRIPT WITHIN RANGE?
 *           BLS    CALCSUB3     ; YES. CONTINUE.
 *           LDAA   #SUBORERR    ; NO. SUBSCRIPT OUT OF RANGE ERROR.
 *           BRA    CALCSUB1     ; GO REPORT IT.
 * CALCSUB3: INX                 ; BYPASS THE SUBSCRIPT LIMIT.
 *           INX
 *           RTS
 */




/*
 *  pulnum      pull a value from the number stack
 */
unsigned int  pulnum(void)
{
	return  pull32(&numstack, STKUNDER);
}

/*
 * PULNUM:   PSHX                ; SAVE THE X-REG.
 *           LDX    NUMSTACK     ; GET THE OPERAND STACK POINTER.
 *           LDD    0,X          ; GET THE OPERAND.
 *           INX                 ; BUMP THE STACK POINTER.
 *           INX
 *           STX    NUMSTACK     ; SAVE THE STACK POINTER.
 *           PULX                ; RESTORE THE X-REG.
 *           CPD    #0           ; "TEST" THE OPERAND BEFORE WE RETURN.
 *           RTS                 ; RETURN.
 */



/*
 *  chknfun      check for (and process) a function that returns a number
 *
 *  This routine acts as a dispatcher for functions that return a number.
 *  Upon entry, argument tok holds the token (from defines.h) for the
 *  specified function.
 *
 *  Upon exit, all global variables and stacks have been updated as
 *  required by the operation of the selected function.  Generally,
 *  this means that a computed value has been pushed onto the proper
 *  number stack.
 *
 *  In some cases, such as for the TAB() function, this routine will
 *  report an error because the function is somehow illegal.  In the
 *  cases of TAB() and CHR$(), those functions are only legal as part
 *  of a PRINT statement and will be flagged as illegal by this routine.
 */
unsigned char  chknfun(unsigned char  tok)
{
	unsigned char				funcid;

	if (tok != FUNCTFLG)  return tok;
	tbufptr++;					// move to function identifier
	funcid = *tbufptr;			// save the function identifier
	tbufptr = tbufptr + 2;		// skip past ID and opening paren
	switch  (funcid)			// based on function ID...
	{
//		case  FDIVTOK:
//		rfdiv();
//		break;

		case  CHRTOK:
		printonly();				// chr$ is illegal in an expression
		break;

//		case  ADCTOK:
//		radc();
//		break;

		case  ABSTOK:
		rabs();
		break;

		case  SQRTTOK:
		rsqrt();
		break;

		case  RNDTOK:
		rrnd();
		break;

		case  SGNTOK:
		rsgn();
		break;

		case  TABTOK:
		printonly();
		break;

//		case  CALLTOK:
//		rcall();
//		break;

		case  PEEKTOK:
		rpeek();
		break;

		case  PEEK16TOK:
		rpeek16();
		break;

		case  PEEK32TOK:
		rpeek32();
		break;

		case  FEEPTOK:
		rfeep();
		break;

		case  FEEP16TOK:
		rfeep16();
		break;

		case  FEEP32TOK:
		rfeep32();
		break;

		case  FSTRCMPTOK:
		rfstrcmp();
		break;

		case  FSTRINDTOK:
		rfstrind();
		break;

		case  FSTRLENTOK:
		rfstrlen();
		break;

//		case  UPTIMETOK:
//		push(&numstack, targetgetuptime(), MSTKOERR);
//		tbufptr++;						// skip past closing paren for this function
//		break;

//		case  FPRTBTOK;
//		rfportb();
//		break;

//		case  FPRTCTOK:
//		rfportc();
//		break;

//		case  FPRTDTOK:
//		rfportd();
//		break;

//		case  FPRTETOK:
//		rfporte();
//		break;

//		case  FTIMETOK:
//		rftime();
//		break;

		case  HEXTOK:
		printonly();				// only legal in PRINT statement
		break;

		case  HEX2TOK:
		printonly();				// only legal in PRINT statement
		break;

		case  HEX4TOK:
		printonly();				// only legal in PRINT statement
		break;

//		case  FPACCTOK:
//		rfpacc();
//		break;

//		case  TIMESTRTOK:
//		printonly();
//		break;

//		case  DATESTRTOK:
//		printonly();
//		break;

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
		case  ADDRTOK:				// there is no supporting function for the ADDR function; see donexp()
		pshaddr(*tbufptr);			// next token better be a variable or port!
		break;

		case  SKSENDTOK:
		fsksend(1);
		tbufptr--;
		break;

		case  SKFLASHTOK:
		fskflash(1);
		tbufptr--;
		break;

		case  SKBCTOK:
		fskbc(1);
		tbufptr--;
		break;

		case  SKSYNCTOK:
		fsksync(1);
		tbufptr--;
		break;

		case  SKINQTOK:
		fskinq(1);
		tbufptr--;
		break;

		case  SKPAIRTOK:
		fskpair(1);
		tbufptr--;
		break;

		case  SKUNPAIRTOK:
		fskunpair(1);
		tbufptr--;
		break;

		case  SKLKUPTOK:
		fsklkup(1);
		tbufptr--;
		break;

		case  SKREVLKUPTOK:
		fskrevlkup(1);
		tbufptr--;
		break;
#endif
		default:					// better never get here!
		break;
	}
	tbufptr++;						// point to next token
	rskipspc();						// added: don't leave pointing to a space
	return  (*tbufptr);				// return next token
}

/*
 * *        **** chcknfun() ****
 * *
 * *         checks for a numeric function and performs it if present
 * *
 * CHKNFUN:  CMPA   #FUNCTFLG    ; IS THIS A FUNCTION CALL?
 *           BEQ    CHKNFUN1     ; YES. GO DO THE FUNCTION.
 *           RTS                 ; NO. JUST RETURN.
 * CHKNFUN1: LDAA   1,Y          ; GET THE FUNCTION CODE BYTE IN B.
 *           DECA                ; SUBTRACT 1 FOR INDEXING.
 *           LDAB   #3           ; BUMP THE IP.
 *           ABY                 ; POINT TO THE FIRST ELEMENT IN THE EXPRESSION.
 *           TAB                 ; PUT THE FUNCTION NUMBER INTO B.
 *           ASLB                ; MULT BY THE NUMBER OF BYTES/ADDRESS.
 *           LDX    #RNFUNCT     ; POINT TO THE FUNCTION ADDRESS TABLE.
 *           ABX                 ; POINT TO THE PROPER FUNCTION.
 *           LDX    0,X          ; GET THE ADDRESS INTO X.
 *           JSR    0,X          ; GO DO THE FUNCTION.
 *           INY                 ; PUT IP PAST THE CLOSING PAREN.
 *           LDAA   0,Y          ; GET NEXT CHARACTER.
 *           RTS                 ; RETURN.
 */




/*
 * RNFUNCT:  EQU    *
 *           FDB    RFDIV
 *           FDB    ICHRS        ; "ICHRS" BECAUSE IT'S ILLEGAL IN AN EXPRESSION.
 *           FDB    RADC
 *           FDB    RABS
 *           FDB    RRND
 *           FDB    RSGN
 *           FDB    ITAB         ; "ITAB" BECAUSE IT'S ILLEGAL IN AN EXPRESSION.
 *           FDB    RCALL
 *           FDB    RPEEK
 *           FDB    RFEEP        ; "EEP" AS A FUNCTION.
 *           FDB    IHEX         ; "IHEX" BECAUSE IT'S ILLEGAL IN AN EXPRESSION.
 *           FDB    RFPORTA
 *           FDB    RFPORTB
 *           FDB    RFPORTC
 *           FDB    RFPORTD
 *           FDB    RFPORTE
 *           FDB    RFTIME
 *           FDB    IHEX2        ; "IHEX2" BECAUSE IT'S ILLEGAL IN AN EXPRESSION.
 *           FDB    RFPACC
 */



/*
 *  chckee      check for end of expression
 *
 *  Returns non-zero if argument is an end of expression token,
 *  such as a closed paren; else returns 0.
 */
unsigned char  chckee(unsigned char  tok)
{
	switch  (tok)
	{
		case  CPARNTOK:
		case  MEOLTOK:
		case  SEMITOK:
		case  COMMATOK:
		case  EOLTOK:
		case  SSCNTOK:				// added: space cannot end an expression
		case  MSCNTOK:				// added: spaces cannot end an expression
		case  TOTOK:				// added to process TO token in FOR-TO-NEXT loop
		case  STEPTOK:				// added to process STEP token in FOR-TO-STEP-NEXT loop
		case  THENTOK:				// added to process THEN token in IF-THEN-ELSE statement
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
		case  SKARGTOK:
#endif
		pshop(CPARNTOK);
		return  TRUE;

		default:
		return  FALSE;
	}
}

/*
 * *        **** chckee() ****
 * *
 * *         if the current token is a semicolin, comma, colin, or space 
 * *         all pending operations on the math stack are performed and 
 * *         we return with the carry set 
 * *
 * CHCKEE:   EQU    *
 *           CMPA   #CPARNTOK    ; IS IT A CLOSED PAREN?
 *           BEQ    CHCKEE2      ; YES.
 *           CMPA   #MEOLTOK     ; IS IT ONE OF THE "EXPRESSION END" TOKENS?
 *           BHS    CHCKEE1      ; YES.
 *           CLC                 ; FLAG "NOT AT THE END OF EXPRESSION".
 *           RTS                 ; RETURN.
 * CHCKEE1:  LDAA   #CPARNTOK    ; END OF EXPRESSION FOUND. PERFORM ALL PENDING
 * CHCKEE2:  BSR    PSHOP        ; OPERATIONS.
 *           SEC                 ; FLAG END OF EXPRESSION.
 *           RTS
 */




/*
 *  pshop      push operator onto operator stack
 */
void  pshop(unsigned char  op)
{
	unsigned char				currop;
	unsigned char				prevop;

	push8(&opstack, op, MSTKOERR);					// this will work in most cases

	while (opstack.index >= 2)						// while there are two or more items on stack...
	{
		if (errcode)  return;						// leave if hit an error
		currop = opstack.stk[opstack.index-1];		// reach into stack for current op
		prevop = opstack.stk[opstack.index-2];		// reach into stack for previous op

		if (currop == OPARNTOK)  return;			// if current op is an open paren, done
		if ((currop & OP_PRECED_MASK) > (prevop & OP_PRECED_MASK))	return;	// if current op is higher prec, done

		if ((currop == CPARNTOK) && (prevop == OPARNTOK))	// if ops are open and close parens...
		{
//			opstack.index = opstack.index - 2;		// drop both tokens from stack
			pull8(&opstack, STKUNDER);
			pull8(&opstack, STKUNDER);
			return;
		}
/*
 *  We need to execute the previous opcode but leave the current opcode on
 *  the top of the stack when we are done.
 */
//		opstack.index--;							// back up one cell
//		opstack.stk[opstack.index-1] = currop;		// save current op on stack
		currop = pull8(&opstack, STKUNDER);			// grab the current op
		prevop = pull8(&opstack, STKUNDER);			// grap the previous op
		push8(&opstack, currop, MSTKOERR);			// save currop on top of op stack
		doop(prevop);								// execute the previous op
	}
}
		
/*
 * PSHOP:    LDX    OPSTACK      ; GET THE OPERATOR STACK POINTER.
 *           DEX                 ; DECREMENT THE STACK POINTER.
 *           CPX    EOPSTK       ; DID THE STACK OVERFLOW?
 *           BNE    PSHOP1       ; NO. CONTINUE.
 *           LDAA   #MSTKOERR    ; YES.
 *           JMP    RPTRERR      ; GO REPORT THE ERROR.
 * PSHOP1:   STX    OPSTACK
 *           STAA   0,X          ; PUT IT ON THE STACK.
 * PSHOP2:   LDX    OPSTACK
 *           LDAA   0,X          ; GET THE NEW OPERATOR OFF THE TOP OF STACK.
 *           CMPA   #OPARNTOK    ; IS IT AN OPEN PAREN?
 *           BEQ    PSHOP5       ; YES. GO PUSH IT.
 *           LDAB   1,X          ; GET THE PREVIOUS OPERATOR OFF THE STACK.
 *           ANDB   #$F0         ; MASK ALL BUT THE PRECIDENCE VALUE.
 *           ANDA   #$F0         ; MASK ALL BUT THE OPERATOR PRECIDENCE.
 *           CBA                 ; IS THE PRECIDENCE OF THE CURRENT OPERATOR >=
 * *                             ; THE OPERATOR ON THE TOP OF THE STACK?
 *           BHI    PSHOP5       ; NO. JUST GO PUSH IT ON THE STACK.
 *           LDAA   1,X          ; YES. GET THE PREVIOUS OPERATOR FROM THE STACK.
 *           LDAB   0,X          ; GET THE CURRENT OPERATOR FROM THE STACK.
 *           CMPB   #CPARNTOK    ; IS THE CURRENT OPERATOR A CLOSED PAREN?
 *           BNE    PSHOP3       ; NO. CONTINUE.
 *           CMPA   #OPARNTOK    ; YES. IS THE PREVIOUS OPERATOR AN OPEN PAREN?
 *           BNE    PSHOP3       ; NO. CONTINUE.
 *           INX                 ; YES. KNOCK BOTH OPERATORS OFF THE STACK.
 *           INX
 *           STX    OPSTACK      ; SAVE THE STACK POINTER.
 * PSHOP5:   RTS                 ; RETURN.
 * PSHOP3:   STAB   1,X          ; PUT IT ON THE STACK.
 *           INX                 ; UPDATE THE STACK POINTER.
 *           STX    OPSTACK
 *           BSR    DOOP         ; GO DO THE OPERATION.
 *           BRA    PSHOP2       ; GO TRY FOR ANOTHER OPERATION.
 */




void  doop(unsigned char  op)
{
#ifdef  AVR
	void				(*fptr)(void);
#endif

//	outdeci(op);								// debug
//	outbyte(' ');								// debug

#ifdef  WINDOWS		
			RTFuncs[op-1].funcptr();			// invoke the selected function (adjust for indexing)
#endif
#ifdef  AVR
			fptr = (void *)pgm_read_word(&RTFuncs[op-1].funcptr);	// read addr of function from pointer table
			if (fptr)  fptr();					// if possibly legal, run it
#endif
#if  defined(CPU78K0R) || defined(CPUML7416) || defined(CPUSTM32)
			if(RTFuncs[op-1].funcptr){
				RTFuncs[op-1].funcptr();			// invoke the selected function (adjust for indexing)
			}else{
				SK_print("funcerr 0x");SK_print_hex(op, 2);SK_print("\r\n");
			}
#endif

/*
	switch (op)					// there should be a better way to do this...
	{
		case  INDIRTOK:
		rindir();
		break;

		case  NOTTOK:
		rnot();
		break;

		case  NEGTOK:
		rneg();
		break;

//		case  PWRTOK:
//		rpwr();
//		break;
//
		case  MULTTOK:
		rmult();
		break;

		case  DIVTOK:
		rdiv();
		break;

		case  MODTOK:
		rmod();
		break;

		case  PLUSTOK:
		rplus();
		break;

		case  MINUSTOK:
		rminus();
		break;

		case  SPLUSTOK:
//		string addition??
		break;

		case  LTTOK:
		rlt();
		break;

		case  GTTOK:
		rgt();
		break;

		case  LTEQTOK:
		rlteq();
		break;

		case  GTEQTOK:
		rgteq();
		break;

		case  EQTOK:
		req();
		break;

		case  NOTEQTOK:
		rnoteq();
		break;

		case  ANDTOK:
		rand();
		break;

		case  ORTOK:
		rorv();
		break;

		case  EORTOK:
		reor();
		break;
	}
*/
}
 
/*
 * DOOP:     CMPA   #$70         ; IS IT A UINARY OPERATOR?
 *           BLO    DOOP1        ; NO. GO CHECK THE NEXT GROUP.
 *           SUBA   #$70         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR7       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP1:    CMPA   #$60         ; IS IT THE "^" OPERATOR?
 *           BLO    DOOP2        ; NO. GO CHECK THE NEXT GROUP.
 *           SUBA   #$60         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR6       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP2:    CMPA   #$50         ; IS IT MULTIPLY, DIVIDE, OR MOD?
 *           BLO    DOOP3        ; NO. GO CHECK THE NEXT GROUP.
 *           SUBA   #$50         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR5       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP3:    CMPA   #$40         ; IS IT ADD OR SUBTRACT?
 *           BLO    DOOP4        ; NO. GO CHECK THE NEXT GROUP.
 *           SUBA   #$40         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR4       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP4:    CMPA   #$30         ; IS IT A LOGICAL OPERATOR?
 *           BLO    DOOP5        ; NO. GO CHECK THE NEXT GROUP.
 *           SUBA   #$30         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR3       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP5:    CMPA   #$20         ; IS IT AND, OR, OR EOR?
 *           BLO    DOOP6        ; NO. ERROR.
 *           SUBA   #$20         ; YES. SUBTRACT THE BASE VALUE OF THE GROUP.
 *           LDX    #HEIR2       ; POINT TO THE EXECUTION ADDRESS TABLE.
 *           BRA    DOOP7        ; GO DO THE OPERATION.
 * DOOP6:    LDAA   #ILTOKERR    ; ILLEGAL OPERATOR TOKEN ENCOUNTERED.
 *           JMP    RPTRERR      ; GO REPORT THE ERROR.
 * DOOP7:    TAB                 ; PUT THE OFFSET IN B.
 *           ASLB                ; MULTIPLY THE OFFSET BY 2.
 *           ABX                 ; POINT TO THE ROUTINE ADDRESS.
 *           LDX    0,X          ; GET THE ADDRESS.
 *           JMP    0,X          ; GO DO THE OPERATION & RETURN.
 */

  
	
/*
 * HEIR7:    EQU    *
 *           FDB    RINDIR
 *           FDB    RNOT
 *           FDB    RNEG
 * HEIR6:    EQU    *
 *           FDB    RPWR
 * HEIR5:    EQU    *
 *           FDB    RMULT
 *           FDB    RDIV
 *           FDB    RMOD
 * HEIR4:    EQU    *
 *           FDB    RPLUS
 *           FDB    RMINUS
 * HEIR3:    EQU    *
 *           FDB    RLT
 *           FDB    RGT
 *           FDB    RLTEQ
 *           FDB    RGTEQ
 *           FDB    REQ
 *           FDB    RNOTEQ
 * HEIR2:    EQU    *
 *           FDB    RAND
 *           FDB    RORV
 *           FDB    REOR
 */
 
 
 /*
  *  reor      run-time for EOR function
  */
void  reor(void)
{
	U32					i;

	i = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = i ^ numstack.stk[numstack.index-1];
}

/*
 * REOR:     JSR    PULNUM
 *           LDX    NUMSTACK
 *           EORA   0,X
 *           EORB   1,X
 * REOR1:    STD    0,X
 *           RTS
 */


 /*
  *  rorv      run-time for inclusive-OR function
  */
void  rorv(void)
{
	U32					i;

	i = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = i | numstack.stk[numstack.index-1];
}

/*
 * RORV:     JSR    PULNUM
 *           LDX    NUMSTACK
 *           ORAA   0,X
 *           ORAB   1,X
 *           BRA    REOR1
 */


 /*
  *  r_and      run-time for AND function
  */
void  r_and(void)
{
	U32					i;

	i = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = i & numstack.stk[numstack.index-1];
}

/*
 * RAND:     JSR    PULNUM
 *           LDX    NUMSTACK
 *           ANDA   0,X
 *           ANDB   1,X
 *           BRA    REOR1
 */



 /*
  *  rplus      run-time for addition function
  */
void  rplus(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = i + numstack.stk[numstack.index-1];
}

/*
 * RPLUS:    JSR    PULNUM
 *           LDX    NUMSTACK
 *           ADDD   0,X
 *           BRA    REOR1
 */



 /*
  *  rminus      run-time for subtraction function
  */
void  rminus(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = numstack.stk[numstack.index-1] - i;
}

/*
 * RMINUS:   LDX    NUMSTACK
 *           LDD    2,X
 *           SUBD   0,X
 *           INX
 *           INX
 *           STD    0,X
 *           STX    NUMSTACK
 *           RTS
 */



 /*
  *  rdiv      run-time for division function
  */
void  rdiv(void)
{
	I32					i;

	i = (I32)pull32(&numstack, STKUNDER);
	if (i)	numstack.stk[numstack.index-1] = ((I32)numstack.stk[numstack.index-1]) / i;
	else
	{
		errcode = ZDIVERR;				//show divide-by-0 error
		rpterr();
		return;
	}	
}

/*
 * RDIV:     BSR    RDIVS        ; GO DO A SIGNED DIVIDE.
 *           JSR    PULNUM       ; GET INTEGER RESULT OFF STACK.
 *           LDX    NUMSTACK     ; POINT TO NUMERIC STACK.
 *           STD    0,X          ; OVERWRITE REMAINDER.
 *           RTS                 ; RETURN.
 * *
 * *
 * *
 * RDIVS:    LDX    NUMSTACK     ; POINT TO NUMERIC STACK.
 *           LDAA   0,X          ; GET UPPER BYTE OF DIVISOR.
 *           EORA   2,X          ; GET SIGN OF THE RESULT.
 *           PSHA                ; SAVE RESULT.
 *           LDD    0,X          ; GET DIVISOR OFF NUMERIC STACK. IS IT ZERO?
 *           BNE    RDIV1        ; NO. CONTINUE.
 * RDIV2:    LDAA   #ZDIVERR     ; YES. GET DIVIDE BY ZERO ERROR.
 *           JMP    RPTRERR      ; GO REPORT IT.
 * RDIV1:    BPL    RDIV3        ; IF POSITIVE IT'S OK.
 *           JSR    RNEG         ; IF NOT MAKE IT POSITIVE.
 * RDIV3:    TST    2,X          ; IS THE DIVIDEND NEGATIVE?
 *           BPL    RDIV4        ; NO. CONTINUE.
 *           LDD    2,X          ; YES. GET THE NUMBER.
 *           COMA                ; NEGATE IT.
 *           COMB
 *           ADDD   #1
 *           STD    2,X          ; SAVE THE RESULT.
 * RDIV4:    LDD    0,X          ; GET THE DIVISOR.
 *           LDX    2,X          ; GET THE DIVIDEND.
 *           XGDX                ; PUT THEM IN THE PROPER REGISTERS.
 *           IDIV                ; DO AN UNSIGNED DIVIDE.
 *           PSHX                ; SAVE THE QUOTIENT.
 *           LDX    NUMSTACK     ; POINT TO THE NUMERIC STACK.
 *           STD    2,X          ; SAVE THE REMAINDER.
 * *        PULD                 ; GET THE QUOTIENT.
 *           PULA
 *           PULB
 *           STD    0,X          ; PUT IT ON THE NUMERIC STACK.
 *           PULA                ; GET THE SIGN OF THE RESULT.
 *           TSTA                ; SET THE CONDITION CODES.
 *           BPL    RDIV5        ; IF PLUS, RESULT OK AS IS.
 *           JSR    RNEG         ; MAKE THE QUOTIENT NEGATIVE.
 *           LDD    2,X          ; GET THE REMAINDER.
 *           COMA                ; MAKE IT NEGATIVE.
 *           COMB
 *           ADDD   #1
 *           STD    2,X          ; SAVE THE RESULT.
 * RDIV5:    RTS                 ; RETURN.
 */



 /*
  *  rmod      run-time for modulus function
  */
void  rmod(void)
{
	I32					i;

	i = (I32)pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = (I32)numstack.stk[numstack.index-1] % i;
}

/*
 * RMOD:     BSR    RDIVS        ; GO GET QUOTIENT & REMAINDER.
 *           JSR    PULNUM       ; REMOVE INTEGER RESULT & LEAVE REMAINDER.
 *           RTS                 ; RETURN.
 */



 /*
  *  rmult      run-time for multiplication function
  */
void  rmult(void)
{
	I32					i;

	i = (I32)pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] =  (I32)numstack.stk[numstack.index-1] * i;
}

/*
 * RMULT:    PSHY
 *           LDX    NUMSTACK
 *           LDAA   1,X
 *           LDAB   3,X
 *           MUL
 * *        PSHD
 *           PSHB
 *           PSHA
 *           TSY
 *           LDAA   1,X
 *           LDAB   2,X
 *           MUL
 *           ADDB   0,Y
 *           STAB   0,Y
 *           LDAA   0,X
 *           LDAB   3,X
 *           MUL
 *           ADDB   0,Y
 *           STAB   0,Y
 *           INX
 *           INX
 * *        PULD
 *           PULA
 *           PULB
 *           STD    0,X
 *           STX    NUMSTACK
 *           PULY
 *           RTS
 */



 /*
  *  rindir      run-time for indirection function (pointer to U32, U16, or U8)
  *
  *  WARNING:  As of 7 May 2011, the AVR GCC compiler uses a U16 as a (U32 *).
  *  This means you can access at most 64K of SDRAM on an Xplained board using the
  *  customary *(U32 *) cast in C, plus you'll get a compiler warning about
  *  differing pointer sizes.
  */
void  rindir32(void)
{
	U32					p;
	U32					v;
	U32					t;

	p = numstack.stk[numstack.index-1];
#ifdef  AVR
	v = __read_U16_far(p);
#else
	v = getU32((U32 *)p);
#endif
	p++;
	p++;
#ifdef  AVR
	t = ((U32)__read_U16_far(p) << 16);
#else
	t = 0;				// for true 32-bit systems, v already has the correct value
#endif
	v = v + t;
	numstack.stk[numstack.index-1] = v;
}



void  rindir(void)
{
	U32					p;
	U32					v;

	p = numstack.stk[numstack.index-1];
#ifdef  AVR
	v = __read_byte_far(p);
#else
	v = getU32((U32 *)p) & 0xff;
#endif
	numstack.stk[numstack.index-1] = v;
}



void  rindir16(void)
{
	U32					p;
	U16					v;

	p = numstack.stk[numstack.index-1];
#ifdef  AVR
	v = __read_U16_far(p);
#else
	v = (U16)(getU32((U32 *)p) & 0xffff);
#endif
	numstack.stk[numstack.index-1] = v;
}



/*
 * *
 * RINDIR:   EQU    *
 *           RTS
 */




 /*
  *  rnot      run-time for NOT function
  */
void  rnot(void)
{
	numstack.stk[numstack.index-1] = ~numstack.stk[numstack.index-1];
}

/*
 * *
 * RNOT:     EQU    *
 *           LDX    NUMSTACK
 *           LDD    0,X
 *           COMA
 *           COMB
 *           STD    0,X
 *           RTS
 */




 /*
  *  rneg      run-time for negation function
  */
void  rneg(void)
{
	numstack.stk[numstack.index-1] = -(int)numstack.stk[numstack.index-1];
}

/*
 * RNEG:     EQU    *
 *           BSR    RNOT
 *           ADDD   #1
 *           STD    0,X
 *           RTS
 */



 /*
  *  rlt      run-time for less-than comparison
  */
void  rlt(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	if ((I32)numstack.stk[numstack.index-1] < i)  numstack.stk[numstack.index-1] = 1;
	else									 numstack.stk[numstack.index-1] = 0;
}

/*
 * RLT:      EQU    *
 *           BSR    CMPNUM
 *           BGE    RLT1
 * RLT2:     INC    3,X
 * RLT1:     INX
 *           INX
 *           STX    NUMSTACK
 *           RTS
 */




 /*
  *  rgt      run-time for greater-than comparison
  */
void  rgt(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	if ((I32)numstack.stk[numstack.index-1] > i)  numstack.stk[numstack.index-1] = 1;
	else								     numstack.stk[numstack.index-1] = 0;
}

/*
 * RGT:      EQU    *
 *           BSR    CMPNUM
 *           BLE    RLT1
 *           BRA    RLT2
 */




 /*
  *  rlteq      run-time for less-than-or-equal comparison
  */
void  rlteq(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	if ((I32)numstack.stk[numstack.index-1] <= i)  numstack.stk[numstack.index-1] = 1;
	else									  numstack.stk[numstack.index-1] = 0;
}

/*
 * RLTEQ:    EQU    *
 *           BSR    CMPNUM
 *           BGT    RLT1
 *           BRA    RLT2
 */





 /*
  *  rgteq      run-time for greater-than-or-equal comparison
  */
void  rgteq(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	if ((I32)numstack.stk[numstack.index-1] >= i)  numstack.stk[numstack.index-1] = 1;
	else									  numstack.stk[numstack.index-1] = 0;
}

/*
 * RGTEQ:    EQU    *
 *           BSR    CMPNUM
 *           BLT    RLT1
 *           BRA    RLT2
 */




 /*
  *  req      run-time for equal comparison
  */
void  req(void)
{
	I32					i;

	i = (I32)pull32(&numstack, STKUNDER);			// added (I32)  
	if ((I32)numstack.stk[numstack.index-1] == i)  	numstack.stk[numstack.index-1] = 1;
	else								      		numstack.stk[numstack.index-1] = 0;
}

/*
 * REQ:      EQU    *
 *           BSR    CMPNUM
 *           BNE    RLT1
 *           BRA    RLT2
 */




 /*
  *  rnoteq      run-time for not-equal comparison
  */
void  rnoteq(void)
{
	I32					i;

	i = pull32(&numstack, STKUNDER);
	if ((I32)numstack.stk[numstack.index-1] != i)	numstack.stk[numstack.index-1] = 1;
	else											numstack.stk[numstack.index-1] = 0;
}


 /*
  *  rpwr      run-time for value to a power
  */
void  rpwr(void)
{
	I32					pwr;

	pwr = pull32(&numstack, STKUNDER);
	numstack.stk[numstack.index-1] = ipow((I32)numstack.stk[numstack.index-1], pwr);
}


static I32  ipow(I32  base, I32  exp)
{
    I32				pow;
	I32				v;

	pow = base;
	v = 1;							// set now, in case exp == 0

	if (exp < 0)  return  0;		// actually this is a divide/0 fault

    while (exp)
    {
        if (exp & 1)  v *= pow;
        pow *= pow;
		exp >>= 1;
    }

    return  v;
}


/*
 * RNOTEQ:   EQU    *
 *           BSR    CMPNUM
 *           BEQ    RLT1
 *           BRA    RLT2
 * *
 * *
 * CMPNUM:   EQU    *
 *           LDX    NUMSTACK
 *           LDD    2,X
 *           CLR    2,X
 *           CLR    3,X
 *           CPD    0,X
 *           RTS
 * *
 * *
 * RPWR:     EQU    *
 *           RTS
 */




void  rabs(void)
{
	donexp();							// process the argument
	numstack.stk[numstack.index-1] = abs(numstack.stk[numstack.index-1]);
}

/*
 * RABS:     EQU    *
 *           JSR    DONEXP
 *           LDX    NUMSTACK
 *           LDD    0,X
 *           BPL    RABS1
 * RABS2:    COMA
 *           COMB
 *           ADDD   #1
 * RABS1:    STD    0,X
 *           RTS
 */



/*
 *  rsgn      return the sign of the argument on the stack
 *
 *  Returns 0 if arg = 0, 1 if arg is positive, or -1 if
 *  arg is negative.
 */
void  rsgn(void)
{
	I32					val;

	donexp();							// process the argument
	val = numstack.stk[numstack.index-1];
	if (val == 0)      numstack.stk[numstack.index-1] = 0;
	else if (val < 0)  numstack.stk[numstack.index-1] = -1;
	else               numstack.stk[numstack.index-1] = 1;
}


/*
 * *
 * RSGN:     EQU    *
 *           JSR    DONEXP
 *           LDX    NUMSTACK
 *           LDD    0,X
 *           BEQ    RABS1
 *           LDD    #1
 *           TST    0,X
 *           BPL    RABS1
 *           BRA    RABS2
 * */



/*
 * *
 * RCALL:    EQU    *
 *           JSR    DONEXP
 *           LDX    NUMSTACK
 *           LDX    0,X
 *           JSR    0,X
 *           BRA    RPEEK1
 * */


void  raddr(void)
{
	donexp();									// process the argument; donexp will push addr on stack
}




void  rpeek(void)
{
	U32					p;

	donexp();									// process the argument
	p = numstack.stk[numstack.index-1];			// get addr as pointer
	numstack.stk[numstack.index-1] = *(U8 *)p;	// return the value at that address
}




void  rpeek16(void)
{
	U32					p;

	donexp();									// process the argument
	p = numstack.stk[numstack.index-1];			// get addr as pointer
	numstack.stk[numstack.index-1] = getU16((U16 *)p);	// return the value at that address
}




void  rpeek32(void)
{
	U32					p;

	donexp();									// process the argument
	p = numstack.stk[numstack.index-1];			// get addr as pointer
	numstack.stk[numstack.index-1] = getU32((U32 *)p);	// return the value at that address
}




/*
 *  rfeep      read a byte from EEPROM
 *
 *  Use:  100 x = EEP(y)
 *
 *  Always reads a single byte.  Actual reading of EEPROM is handled by target-specific
 *  function.
 */
void  rfeep(void)
{
	_rfeep(sizeof(U8));
}



/*
 *  rfeep16      read a 16-bit word from EEPROM
 *
 *  Use:  100 x = EEP16(y)
 */
void  rfeep16(void)
{
	_rfeep(sizeof(U16));
}



/*
 *  rfeep32     read a 32-bit long word from EEPROM
 *
 *  Use:  100 x = EEP32(y)
 */
void  rfeep32(void)
{
	_rfeep(sizeof(U32));
}



static void  _rfeep(U8  size)
{
	U32					p;
	volatile U32		v;

	donexp();									// process the argument
	p = numstack.stk[numstack.index-1];			// get addr as pointer

	switch (size)
	{
		case  sizeof(U8):
		v = (U32)targetreadeeprom(p);			// let the target sort it out
		break;

		case  sizeof(U16):
		v = (U32)targetreadeeprom16(p);			// let the target sort it out
		nl();
		break;

		case  sizeof(U32):
		v = (U32)targetreadeeprom32(p);			// let the target sort it out
		break;

		default:
		errcode = IVEXPERR;						// call it an invalid expression
		v = 0;									// got to save something
		break;
	}

	numstack.stk[numstack.index-1] = v;			// return the value read
}


void	rfstrcmp(void)
{
	int			tok;
	U8			*ptr1 = NULL, *ptr2 = NULL;
	int			len1 = 0, len2 = 0;
	int			i, ret = 0;

	rskipspc();
	tok = *tbufptr;
	if((tok == SCONTOK) || (tok == SVARTOK)){

		if(tok == SCONTOK){
			len1 = *(tbufptr+1);
			tbufptr += 2;
			ptr1 = tbufptr;
			tbufptr += len1;
		}else{
			int		offset;
			U8		*varptr;
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			len1 = varptr[0];
			ptr1 = &varptr[1];
		}
	}else{
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
		goto exit;
	}

	for(i = 0; (i < len1) && (ret == 0); i++){
		if(i < len2){
			ret += ((int)ptr1[i]) - ((int)ptr2[i]);
		}else{
			ret = ptr1[i];
		}
	}
	if((ret == 0) && (len1 < len2))	ret = -((int)ptr2[len1]);

	push32(&numstack, ret, MSTKOERR);
exit:
	return;
}

void	rfstrind(void)
{
	int			tok;

	rskipspc();
	tok = *tbufptr;
	if((tok == SCONTOK) || (tok == SVARTOK)){
		U8		*ptr = NULL;
		int		len = 0;
		int		address;

		if(tok == SCONTOK){
			len = *(tbufptr+1);
			tbufptr += 2;
			ptr = tbufptr;
			tbufptr += len;
		}else{
			int		offset;
			U8		*varptr;
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			len = varptr[0];
			ptr = &varptr[1];
		}
		rskipspc();
		if(*tbufptr++ != COMMATOK){
			errcode = SYTXERR;
			goto exit;
		}
		rskipspc();
		donexp();							// calc address
		address = pull32(&numstack, STKUNDER);
		if((address < 0) || (address >= len)){
			errcode = OUTOFRANGEERR;
		}else{
			push32(&numstack, ptr[address], MSTKOERR);
		}
	}
exit:
	return;
}

void	rfstrlen(void)
{
	int			tok;

	rskipspc();
	tok = *tbufptr;
	if((tok == SCONTOK) || (tok == SVARTOK)){
		U8		*ptr = NULL;
		int		len = 0;

		if(tok == SCONTOK){
			len = *(tbufptr+1);
			tbufptr += 2;
			ptr = tbufptr;
			tbufptr += len;
		}else{
			int		offset;
			U8		*varptr;
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			len = varptr[0];
			ptr = &varptr[1];
		}
		push32(&numstack, len, MSTKOERR);
	}
}

void  rsqrt(void)
{
	U32					v;

	donexp();									// process the argument
	v = numstack.stk[numstack.index-1];			// get addr as pointer
	numstack.stk[numstack.index-1] = isqrt(v);	// return the integer square-root of the value
}



/*
 *  Integer square-root function.  Automatically rounds result to next
 *  higher integer.
 *
 *  Taken from: http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
 */
static U32  isqrt(U32  val)
{
    U32					op;
    U32					res;
    U32					one;


	op  = val;
	res = 0;
	one = 1uL << 30;		// The second-to-top bit is set:
							// use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

// "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

    return res;
}


/* *
 * RPEEK:    EQU    *
 *           JSR    DONEXP
 *           LDX    NUMSTACK
 *           LDX    0,X
 *           LDAB   0,X
 *           CLRA
 * RPEEK1:   LDX    NUMSTACK
 *           STD    0,X
 *           RTS
 * *
 * *
 * RFEEP:    EQU    *
 *           JSR    DONEXP       ; GO GET SUBSCRIPT OF EEPROM ARRAY.
 *           LDX    NUMSTACK     ; POINT TO THE OPERAND STACK.
 *           LDD    0,X          ; GET THE SUBSCRIPT OFF THE STACK.
 *           CPD    #MAXEESUB    ; IS IT WITHIN THE LIMIT?
 *           BLS    RFEEP1       ; YES. GO GET THE VALUE.
 *           LDAA   #EESUBERR    ; NO. SUBSCRIPT ERROR.
 * RFEEP2:   JMP    RPTRERR      ; REPORT THE ERROR.
 * RFEEP1:   LSLD                ; MULT THE SUBSCRIPT BY 2.
 *           ADDD   #EEPBASAD    ; ADD IN THE BASE ADDRESS OF THE EEPROM ADDRESS.
 *           XGDX                ; PUT THE ADDRESS IN X.
 *           LDD    0,X          ; GET THE DATA.
 *           BRA    RPEEK1       ; GO STEAL SOME CODE.
 * *
 * *
 * RFDIV:    EQU    *
 *           JSR    DONEXP       ; GO EVALUATE THE DIVIDEND EXPRESSION.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           INY                 ; PASS UP THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES AFTER THE COMMA.
 *           JSR    DONEXP       ; EVALUATE THE DIVISOR EXPRESSION.
 *           LDX    NUMSTACK     ; POINT TO OPERAND STACK.
 *           LDD    2,X          ; GET THE DIVIDEND.
 *           LDX    0,X          ; GET THE DIVISOR.
 *           FDIV                ; DO THE FRACTIONAL DIVIDE.
 *           BVC    RFDIV1       ; ALL IS OK IF V=0. (IX > D).
 *           LDAA   #OVDV0ERR    ; ERROR. EITHER OVERFLOW OR /0 ERROR.
 * RFDIV2:   BRA    RFEEP2       ; GO REPORT IT.
 * RFDIV1:   XGDX                ; PUT QUOTIENT IN D.
 *           LDX    NUMSTACK     ; POINT TO OPERAND STACK.
 *           INX                 ; REMOVE DIVISOR FROM STACK.
 *           INX
 *           STD    0,X          ; PUT QUITIENT ON OPERAND STACK.
 *           STX    NUMSTACK     ; SAVE NEW VALUE OF STACK POINTER.
 *           RTS                 ; RETURN.
 * *
 * *
 * RADC:     EQU    *
 *           JSR    DONEXP       ; GO GET THE CHANNEL NUMBER TO CONVERT.
 *           LDX    NUMSTACK     ; POINT TO THE RESULT.
 *           LDD    0,X          ; GET THE CHANNEL NUMBER.
 *           BMI    RADC4        ; NEGATIVE CHANNEL NUMBERS ARE ILLEGAL.
 *           CPD    #7           ; IS IT A VALID CHANNEL NUMBER?
 *           BLS    RADC1        ; YES. GO CONVERT IT.
 * RADC4:    LDAA   #INVCHERR    ; NO. INVALID CHANNEL NUMBER.
 *           BRA    RFDIV2       ; GO REPORT THE ERROR.
 * RADC1:    LDX    IOBaseV
 *           STAB   ADCTL,X      ; START THE CONVERSION ON THE SELECTED.
 * RADC2:    TST    ADCTL,X      ; IS THE CONVERSION COMPLETE?
 *           BPL    RADC2        ; NO. WAIT FOR 4 CONVERSIONS ON 1 CHANNEL.
 *           CLRA                ; YES. NOW AVERAGE THE 4 CONVERSIONS.
 *           LDAB   ADR1,X       ; GET 1ST RESULT.
 *           ADDB   ADR2,X       ; ADD IN THE SECOND.
 *           ADCA   #0           ; ADD IN CARRY.
 *           ADDB   ADR3,X       ; ADD IN THE THIRD.
 *           ADCA   #0           ; ADD IN CARRY.
 *           ADDB   ADR4,X       ; ADD IN THE FOURTH.
 *           ADCA   #0           ; ADD IN CARRY.
 *           LSRD                ; DIVIDE RESULT BY 4.
 *           LSRD
 *           LDX    NUMSTACK     ; POINT TO THE RESULT.
 *           STD    0,X          ; PUT THE RESULT ON THE OPERAND STACK.
 *           RTS                 ; RETURN.
 * *
 * */



/*
 *  The following functions try to duplicate the ANSI random() and
 *  srandom() functions for MSVC 6.0 and for 8-bit MCUs such as the
 *  Atmel ATmega1284p..
 */
static U32 b_random(void)
{
	seed = 1664525L * seed + 1013904223L;
	return seed;
}

static void  b_srandom(U32  newseed)
{
	seed = newseed;
}



void  rrnd(void)
{
	U32							val;
	U32							t;

	donexp();							// get function argument
	val = numstack.stk[numstack.index-1];
	if (val == 0L)  b_srandom(RANDOM_SEED_VAL);		// for rnd(0), reseed the generator, return 0
	else
	{
		t = b_random();					// compute a 32-bit random number
		val = t % val + 1L;
		numstack.stk[numstack.index-1] = val;	// put back on stack
	}
}
	
/*
 * RRND:     EQU    *
 *           JSR    DONEXP       ; GO GET FUNCTION ARGUMENT.
 *           LDX    NUMSTACK     ; GET ARGUMENT OFF STACK. GET NEW RANDOM NUMBER?
 *           LDD    0,X
 *           BEQ    RRND2        ; YES. GO GET NEXT RANDOM NUMBER IN THE SERIES.
 *           BMI    RRND1        ; IF NEG., START A NEW SERIES.
 *           LDD    RANDOM       ; IF POSITIVE, GET LAST RANDOM NUMBER.
 *           BRA    RRND3        ; RETURN.
 * RRND1:    LDX    IOBaseV
 *           LDD    TCNT,X       ; USE THE TIMER VALUE AS THE NEW SEED.
 *           STD    RANDOM       ; SAVE IT.
 * RRND2:    LDD    RANDOM       ; GET PREVIOUS RANDOM NUMBER (USE AS SEED).
 *           ASLB                ; DO SOME OPERATIONS.
 *           ABA
 *           LDAB   RANDOM+1
 *           ASLD
 *           ASLD
 *           ADDD   RANDOM
 *           ADDD   #$3619
 *           STD    RANDOM
 * RRND3:    LSRD                ; MAKE THE NUMBER POSITIVE.
 *           STD    0,X          ; PUT THE NUMBER ON THE STACK.
 *           RTS                 ; RETURN.
 */




/*
 *  itab()
 *  ichrs()
 *  ihex()
 *  ihex2()
 *
 *  These routines have been consolidated into a single routine with an error
 *  code indicating that the routine is only legal in a PRINT statement.
 */
void  printonly(void)
{
	errcode = PRFUNERR;					// print only!
//	rpterr();							// whine
}

	
/*
 * *
 * ITAB:     EQU    *
 * ICHRS:    EQU    *
 * IHEX:     EQU    *
 * IHEX2:    EQU    *
 *           LDAA   #PRFUNERR    ; THESE FUNCTIONS MUST BE USED ONLY IN
 *           JMP    RPTRERR      ; PRINT STATEMENTS.
 * *
 * *
 * RFTIME:   LDD    TIMEREG      ; GET THE TIME IN SECONDS.
 *           BRA    RFPORTA2     ; GO PUT NUMBER ON THE STACK.
 * *
 * *
 * RFPACC:   LDX    IOBaseV
 *           LDAB   PACNT,X      ; GET THE CURRENT VALUE OF THE PULSE ACCUMULATOR.
 *           CLRA
 *           BRA    RFPORTA2     ; GO PUT THE NUMBER ON THE STACK.
 * *
 * *
 * RFPORTA:  EQU    *
 *           LDAB   #PORTAIO      ; GET DATA FROM PORTA.
 * RFPORTA1: LDX    IOBaseV
 *           ABX
 *           LDAB   0,X
 *           CLRA                ; CLEAR UPPER BYTE OF WORD.
 * RFPORTA2: DEY                 ; DECREMENT IP BECAUSE CALLING ROUTINE WILL TRY
 *           DEY                 ; TO BUMP IT PAST AN OPENING & CLOSING PAREN
 * *                             ; WHICH ISN'T THERE.
 *           JMP    PSHNUM4      ; GO PUSH VALUE ON OPERAND STACK & RETURN.
 * *
 * RFPORTB:  EQU    *
 *           LDAB   #PORTBIO
 *           BRA    RFPORTA1
 * *
 * RFPORTC:  EQU    *
 *           LDAB   #PORTCIO
 *           BRA    RFPORTA1
 * *
 * RFPORTD:  EQU    *
 *           LDAB   #PORTDIO
 *           BRA    RFPORTA1
 * *
 * RFPORTE:  EQU    *
 *           LDAB   #PORTEIO
 *           BRA    RFPORTA1
 * *
 * *
 */
