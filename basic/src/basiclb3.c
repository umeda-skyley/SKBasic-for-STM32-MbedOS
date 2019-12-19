/*
 *  basiclb3.c      library functions for KLBasic compiler
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

static void						getvarname(void);				// parse a variable name from the input buffer



/*
 *  Local variables
 */

static  unsigned char			varname[MAX_VAR_NAME_LEN+1];		// holds possible variable name, null-terminated
static 	unsigned char			vartype;						// holds variable type, based on trailing char


/*
 * IVARTOK 0x84 varname 14 bytes 0xnn 0xnn 0xnn 0xnn
 * PVARTOK 
 * SVARTOK 0x84 varname 14 bytes 1 byte length string include ""
 */
 
int				a2hex(char c)
{
	c -= '0';
	if(c > 9)	c -= 7;
	if(c > 16)	c -= 0x20;
	return(c);
}

/*
 *  getvarname      try to parse a variable name from the input buffer
 */
void  getvarname(void)
{
	unsigned char					cnt;
	unsigned char					n;

	for (cnt=0; cnt<MAX_VAR_NAME_LEN+1; cnt++)		// for all chars in variable name...
	{
		varname[cnt] = 0;					// init the variable name plus terminating null
	}
	if (isalpha(*ibufptr))					// if first char is an alpha...
	{ 
		varname[0] = *ibufptr++;			// save the char as part of the name
	}
	else									// this is not a variable!
	{
		errcode = ILVARERR;					// show illegal variable
	} 

/*
 * GETVAR:   EQU    *
 *           PSHY
 *           CLRA
 *           PSHA
 *           PSHA
 *           PSHA
 *           PSHA
 *           TSY
 *           JSR    GETCHR
 *           JSR    ALPHA
 *           BCS    GETVAR1
 *           LDAA   #ILVARERR
 *           JMP    RPTERR
 * GETVAR1:  jsr    ToUpper
 *           STAA   0,Y
 *           JSR    INCIBP
 */


	for (n=1; n<MAX_VAR_NAME_LEN; n++)			// need to do remaining chars in name
	{
		if (alphanum(*ibufptr))				// if next char is alphanumeric...
		{
			varname[n]=*ibufptr++;			// save and advance to next char
		}
		else
		{
			break;						// if not, done for now
		}
	}

	if ((vartype=(U8)chcktyp()) == 0)			// check type identifier of variable in varname 
	{
		vartype = IVARTOK;					// no special case, assume integer variable
	}
	else									// found identifier, must be int or string
	{
		++ibufptr;							// on to next char
	}
}


/*
 *           JSR    GETCHR
 *           JSR    ALPHANUM
 *           BCC    GETVAR2
 *           jsr    ToUpper
 *           STAA   1,Y
 *           JSR    INCIBP
 * GETVAR2:  JSR    CHCKTYP
 *           STAA   3,Y
 */




/*
 *  getvar      handle variable reference/definition
 *
 *  This routine tries to make a variable out of what is currently being
 *  pointed to by ibufptr and places it into the variable symbol table
 *  if it is not already there.
 *
 *  Variable names are of the form:  <alpha>(<alpha+digit>*7)
 *  Variable name may end with '%' to indicate integer or '$' to indicate
 *  string.  If no ending character, assume integer (no FP support).
 */

int  getvar(void)
{
	I16						offset;

	getvarname();									// try to get a variable name from the input buffer
	if (errcode)  return(0);						// any problems, leave now

	offset = findvar(vartype, varname);				// try to find variable in table
	if (errcode)  return(0);						// oops, got an error, outta here
	if (offset == -1)								// if var is not already in table...
	{
		if (vartype == IAVARTOK)					// if adding an array...
		{
			if (!xdimflag)							// but not in xdim()...
			{
				errcode = UNDIMERR;					// show array is not DIMensioned
				return  0;							// and leave
			}
		}
		if ((offset=putvar(vartype,varname)) == -1)  return(0);  // try to add to variable
		if (errcode) return(0);						// had problems, leave now
	}
	else											// variable already exists in table
	{
//		if (xdimflag)								// if we are trying to DIMension it...
//		{
//			errcode = REDIMERR;						// show we are redimensioning it
//			return  0;								// and leave
//		}
	}
 
/*
 *           JSR    FINDVAR
 *           CPD    #-1
 *           BNE    GETVAR5
 * GETVAR4:  LDAA   3,Y
 *           JSR    PUTVAR
 */

	*tbufptr++ = vartype;				// put variable type byte in token buffer
	putint16(offset);					// put 16-bit offset after it
	if (vartype == IAVARTOK)			// if this is an integer array...
	{
//		ibufptr++;						// advance past open paren
		*tbufptr++ = OPARNTOK;			// save open-paren token to buffer
		xexpres(NUM);					// resolve size of array
		if (*ibufptr != ')')			// if missing close paren...
		{
			errcode = MPARNERR;			// show the error
			return 0;
		}
		ibufptr++;						// step over closing paren in input buffer
		*tbufptr++ = CPARNTOK;			// save the closing paren token
	}
	if ((vartype==IVARTOK) || (vartype==IAVARTOK))	// for ints or int arrays...
	{
		return(NUM);					// show we found a number
	}
	else								// must be a string variable...
	{
		return(STRING);					// show that
	}
}


/*
 * GETVAR5:  EQU    *
 * *       PSHD
 *           PSHB
 *           PSHA
 *           LDAA   3,Y
 *           JSR    PUTTOK
 * *        PULD
 *           PULA
 *           PULB
 *           JSR    PUTDTOK
 *           LDAA   3,Y           ; GET VARIABLE TYPE AGAIN.
 *           BITA   #$10          ; IS IT AN ARRAY VARIABLE?
 *           BEQ    GETVAR7       ; NO. CONTINUE.
 *           JSR    INCIBP        ; MOVE THE INPUT BUFFER POINTER PAST THE OPEN (.
 *           LDAA   #OPARNTOK
 *           JSR    PUTTOK
 *           LDAA   #NUM          ; YES. SUBSCRIPT EXPRESSION MUST BE NUMERIC.
 *           JSR    XEXPRES       ; GO GET THE SUBSCRIPT.
 *           JSR    GETNXCHR      ; GET THE TERMINATING CHARACTER.
 *           CMPA   #')'          ; IS IT A CLOSING PAREN?
 *           BEQ    GETVAR8       ; YES. GO FINISH UP.
 *           LDAA   #MPARNERR     ; NO. ERROR.
 *           JMP    RPTERR
 * GETVAR8:  LDAA   #CPARNTOK     ; GET CLOSING PAREN TOKEN.
 *           JSR    PUTTOK        ; PUT TOKEN IN BUFFER.
 * GETVAR7:  LDAA   #NUM          ; NO. RETURN PROPER TYPE.
 *           LDAB   3,Y
 *           BITB   #2
 *           BEQ    GETVAR6
 *           LDAA   #STRING
 * GETVAR6:  INS
 *           INS
 *           INS
 *           INS
 *           PULY
 *           RTS
 */



/*
 *  chcktyp      check type of variable passed in ibufptr
 *
 *  The assembly source shows special treatment of a subscript
 *  indicator ('(' following name) but that treatment is
 *  not reflected in the C psuedocode.
 *
 *  This routine MUST return 0 if the variable is just a standard
 *  integer variable without the trailing '%' identifier!
 */
int  chcktyp(void)
{
	int					r;

//	if      (*ibufptr == '%')  return(IVARTOK);		// name followed by % means integer
//	else if (*ibufptr == '$')  return(SVARTOK);		// name followed by $ means string
//	else    return (0);								// nothing special

	r = 0;									// default (0) means integer, no added identifier
	if (*ibufptr == '(')					// if subscripted variable...
	{
		r = IVARTOK | ARRAY_MASK;			// show a subscripted integer variable
	}
	else
	if (*ibufptr == '$')
	{
		r = SVARTOK;
	}

	return  r;
}


/*
 * CHCKTYP:  EQU    *
 *           LDAA   #IVARTOK        ; IN V1.0 ONLY INTEGER VARIABLES ARE SUPPORTED.
 *           PSHA                   ; IN V2.0 FLOATING POINT VARIABLES WILL BE
 *           JSR    GETCHR          ; SUPPORTED.
 *           CMPA   #'('            ; IS A SUBSCRIPT FOLLOWING THE NAME?
 *           PULA                   ; RESTORE THE TOKEN TYPE.
 *           BNE    CHCKTYP4        ; NO. RETURN.
 *           ADDA   #$10            ; YES. MAKE IT AN ARRAY VARIABLE.
 * CHCKTYP4: RTS                    ; RETURN.
 */


/*
 *  findvar    look through table of variables for string in varname
 *
 *  Upon entry, vartype holds the variable type identifier and varname
 *  points to the null-terminated variable name to locate in the variable
 *  table.
 */
int  findvar(unsigned char  vartype, unsigned char  *varname)
{
	U8								*varptr;
	U8								k;
	U8								len;

	len = (U8)strlen((char *)varname);			// save length of target string in varname
	varptr = varbegin;				// point to the start of the var table
	while (*varptr)					// for all variables in the table...
	{

/*
 * FINDVAR:  EQU    *
 *           LDX    VARBEGIN
 * FINDVAR1: TST    0,X
 *           BEQ    FINDVAR2
 */

/*
 *  Gordon's original code used single-letter variables so streq() didn't
 *  need to be very bright.  When I moved to multi-char variables, streq()
 *  no longer worked properly.  The failure was that it would return a match
 *  if a name in the variable table matched OR WAS A SUBSET of the target.
 *
 *  For example, c, ca, and cat all matched a search for the variable cat.
 *
 *  Since streq() returns the number of chars it checked when it found a
 *  match, I now test the returned value against the length of the target
 *  variable.  A match isn't a match unless the lengths match as well.
 */
 		if (*varptr == vartype)				// is the current var the same type?
		{
			k = (U8)streq(varname, varptr+1);	// returns 0 if no match, else num of chars matched in varptr+1
			if (k == len)					// if num of chars matched is length of variable name sought...
			{
				return (varptr-varbegin);	// return offset from the table start
			}
		}

/*
 *           CMPA   0,X
 *           BNE    FINDVAR3
 *           LDAB   1,X
 *           CMPB   0,Y
 *           BNE    FINDVAR3
 *           LDAB   2,X
 *           CMPB   1,Y
 *           BNE    FINDVAR3
 *           XGDX
 *           SUBD   VARBEGIN
 *           RTS
 */

/*
 *  Not the variable we want, so we need to adjust varptr to the
 *  the next entry in the variable table.
 */
		if      (*varptr==IVARTOK) varptr = varptr + ISIZ + MAX_VAR_NAME_LEN + 1;	// magic number!
		else if (*varptr==SVARTOK) varptr = varptr + SSIZ + MAX_VAR_NAME_LEN + 1;	// magic number!
//		else if (*varptr==FVARTOK) varptr = varptr + FSIZ + MAX_VAR_NAME_LEN + 1;	// magic number!
		else if (*varptr==IAVARTOK) varptr = varptr + ASIZ + MAX_VAR_NAME_LEN + 1;	// array
		else								// unknown! (should not happen!)
		{
			errcode = ILTOKERR;				// show it's broken
			return (-1);					// and outta here
		}
	}

/*
 *  The assembler source shows that the real version of this program handles
 *  subscripted variables (arrays) but that code is not in the psuedocode.
 *  Need to add array support later.
 */

/*
 * FINDVAR3: EQU    *
 *           LDAB   0,X
 *           BITB   #$10          ; IS IT AN ARRAY VARIABLE?
 *           BEQ    FINDVAR8      ; NO CONTINUE.
 *           LDAB   #ASIZ+3       ; YES. GET ARRAY SIZE +3.
 *           BRA    FINDVAR7
 * FINDVAR8: CMPB   #IVARTOK
 *           BNE    FINDVAR6
 *           LDAB   #ISIZ+3
 * FINDVAR7: ABX
 *           BRA    FINDVAR1
 * FINDVAR6: LDAA   #ILTOKERR
 *           JMP    RPTERR
 * FINDVAR2: LDD    #-1
 *           RTS
 */

	return(-1);						// arrays not supported yet, bail
}



/*
 *  putvar      add a variable to the variable table
 */
int  putvar(unsigned char  vartype, unsigned char  *varname)
{
	U16						count;
	U16						n;
	unsigned char			*varadd;

	varadd = varend;			// save begining addr of var we are storing
	*varend++ = vartype;		// put token/type in variable symbol table
	for (n=0; n<MAX_VAR_NAME_LEN; n++)
	{
		*varend++ = *varname;	// copy char (could be terminating null!)
		if (*varname)  varname++;	// only advance name pointer if copied a char!
	}


/*
 * PUTVAR:   EQU    *
 *           LDX    VAREND
 *           PSHX
 *           STAA   0,X
 *           INX
 *           LDAB   0,Y
 *           STAB   0,X
 *           INX
 *           LDAB   1,Y
 *           STAB   0,X
 *           INX
 */

	if      (vartype == IVARTOK)  count = ISIZ + 1;		// determine # of bytes for this var
	else if (vartype == IAVARTOK)  count = ISIZ + 1;	// integer array
	else if (vartype == SVARTOK)  count = SSIZ + 1;
	else if (vartype == FVARTOK)  count = FSIZ + 1;
	else 
	{ 
		errcode = ILTOKERR;
		return (-1);
	}

	for (n=1; n<=count; n++) *varend++ = 0;			// zero the storage
	--varend;
	if (varend > varmend)					// if wrote past end of variable area...
	{
		errcode = OMEMERR;					// show out of memory
		return(-1);
	}

//	pl_P(PSTR("varend = "));			// debug
//	outhexbyte((unsigned int)varend>>8);						// debug
//	outhexbyte((unsigned int)varend&0xff);					// debug
//	pl_P(PSTR("\n\r"));;

	return (varadd - varbegin);				// return offset
}


/*
 *           BSR    CLRVAR
 *           CLR    0,X          ; CLEAR 1 BYTE BEYOND THE END OF THE VAR AREA.
 *           STX    VAREND
 *           CPX    VARMEND
 *           BLS    PUTVAR5
 *           LDAA   #OMEMERR
 *           BRA    CLRVAR6
 * PUTVAR5:  EQU    *
 * *        PULD
 *           PULA
 *           PULB
 *           SUBD   VARBEGIN
 * *        PSHD                ; SAVE THE OFFSET TO THIS VARIABLE.
 *           PSHB
 *           PSHA
 *           JSR    CCLEAR3      ; CLEAR ALL VARIABLES SINCE WE MAY HAVE TRASHED
 * *                            ANY ARRAYS THAT HAD BEEN ALLOCATED.
 * *        PULD                RESTORE THE "NEW" VARIABLE OFFSET.
 *           PULA
 *           PULB
 *           RTS
 */



/*
 *  clrvar
 */
void  clrvar(unsigned char  **varptr)
{
	int						n;
	U8						*tptr;
	U16						count;
	U32						*tptr32;

	if (**varptr == IAVARTOK)			// is this an integer array variable?
	{
		tptr32 = (U32 *)(*varptr + MAX_VAR_NAME_LEN + 1);	// point to "value" in array variable
		*varptr = *varptr + MAX_VAR_NAME_LEN + 1 + ISIZ;	// adjust argument for next invocation
#ifdef  AVR
		tptr32 = (U32 *)((U16)*tptr32);			// AVR pointers are only 16 bits
#else
		tptr32 = (U32 *)(getU32(tptr32));				// now point into dyn memory pool
#endif
		if (tptr32 == 0)  return;		// if array is not yet set up, leave
		count = getU16((U16 *)tptr32);			// get max subscript
		tptr = (U8 *)tptr32 + sizeof(U16);	// move to start of actual data
		count = count * ISIZ;			// calc number of bytes to clear
	}
	else if (**varptr == IVARTOK)		// is this an integer variable?
	{
		count = ISIZ;					// yes, set counter to integer size
		tptr = *varptr + MAX_VAR_NAME_LEN + 1;	// set up pointer
		*varptr = tptr + count;			// adjust argument for next invocation
	}
//	else if (**varptr == FVARTOK)		// is this an FP variable?
//	{
//		count = FSIZ;					// yes, set counter to FP size
//	}
	else if (**varptr == SVARTOK)		// is this an FP variable?
	{
		count=SSIZ;
		tptr = *varptr + MAX_VAR_NAME_LEN + 1;	// set up pointer
		*varptr = tptr + count;			// adjust argument for next invocation
	}
	else								// oops, this is bad
	{
		errcode = ILTOKERR;				// show illegal token error
		rpterr();
		return;
	}
//	*varptr = *varptr + MAX_VAR_NAME_LEN + 1;	// point to data area inside the variable
	for (n=0; n<count; n++)
	{
//		**varptr = 0;					// zero the bytes in the variable
//		*varptr = *varptr + 1;			// move to next cell
		*tptr = 0;						// zero the bytes in the variable
		tptr++;							// move to next cell
	}
}

/*
 * CLRVAR:   EQU    *
 *           BITA   #$10          ; IS IT AN ARRAY VARIABLE?
 *           BEQ    CLRVAR8       ; NO. CONTINUE.
 *           LDAB   #ASIZ         ; YES. GET THE DICTIONARY SIZE+1.
 *           BRA    CLRVAR1       ; PUT THE VARIABLE IN THE DICTIONARY.
 * CLRVAR8:  CMPA   #IVARTOK
 *           BNE    CLRVAR4
 *           LDAB   #ISIZ
 * CLRVAR1:  EQU    *
 *           CLR    0,X
 *           INX
 *           DECB
 *           BNE    CLRVAR1
 *           RTS
 * CLRVAR4:  LDAA   #ILTOKERR
 * CLRVAR6:  JMP    RPTERR
 */




/*
 *  getcon      parse a constant and put it in the token buffer
 */
int  getcon(void)
{
	I32					tconst;
	unsigned char		*litp;
	char				count;
	unsigned char		len;

	litp = ibufptr;						// save a pointer to start of constant
	if (*ibufptr == '"')				// if found start of quoted string...
	{
		getscon();						// add a string constant
		return(STRING);					// tell the world
	}

/*
 * GETCON:   EQU    *
 *           JSR    GETCHR
 */


	else if (*ibufptr == '$')			// if found a hex constant...
	{
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
		++ibufptr;						// move past $ identifier
		if(*ibufptr == '"')
		{
			getbcon();						// add a string constant
			return(STRING);					// tell the world
		}
		else
		{
			tconst = gethex();				// add a hex constant
		}
#else
		tconst = gethex();				// add a hex constant
#endif
	}
	else if (isalpha(*ibufptr))			// if alphabetic, might be target-specific constant
	{
		len = (U8)getconst(&tconst);		// try to lookup the constant (advances ibufptr if matches!)
		if (len == 0)					// if unknown but started with alphabetic...
		{
			return  0;					// can't be a decimal constant, leave now
		}
	}
	else tconst = getdeci();			// better be a decimal constant
	if (errcode)  return(0);			// if error occurred, bail now

/*
 * GETCON2:  EQU    *
 *           LDX    IBUFPTR
 *           PSHX
 *           CMPA   #'$'
 *           BNE    GETCON3
 *           JSR    INCIBP
 *           JSR    GETHEX
 *           BRA    GETCON4
 * GETCON3:  JSR    GETDECI
 */


	*tbufptr++ = ICONTOK;					// add token for integer constant
	putint32(tconst);						// now add the value
	count = (I8)(ibufptr-litp);					// get number of bytes in source form
	*tbufptr++ = count;						// add that count to token buffer
	while (litp < ibufptr)  *tbufptr++ = *litp++;	// copy original string into buffer
	return (NUM);							// show we found a number
}


/*
 * GETCON4:  EQU    *
 *           PSHA
 *           LDAA   #ICONTOK
 *           JSR    PUTTOK
 *           PULA
 *           JSR    PUTDTOK
 *           LDD    IBUFPTR
 *           TSX
 *           SUBD   0,X
 *           TBA
 *           JSR    PUTTOK
 *           PULX
 * GETCON5:  LDAA   0,X
 *           JSR    PUTTOK
 *           INX
 *           DECB
 *           BNE    GETCON5
 *           LDAA   #NUM
 *           RTS
 */


/*
 *  getdeci      get decimal integer constant from input buffer
 *
 *  Rewritten to use isdigit().  This routine only does postive numbers;
 *  see indeci() for handling of negative numbers.
 */
I32  getdeci(void)
{
	char					c;
	I32						num;

	num = 0;
	if (isdigit(*ibufptr) == 0)			// if first char is not a decimal digit...
	{
		errcode = SYTXERR;				// that's bad, throw a syntax error
		return(0);
	}	
	
	while (isdigit(c = *ibufptr))		// for all chars that are decimal digits...
	{
		num = num * 10 + (c-'0');		// convert the number field to a value
		if (num < 0)					// if value rolls past top limit...
		{	
			errcode = INTOVERR;			// number is out of range (what about negative numbers?)
			return(0);
		}
		++ibufptr;
	}
	return (num);
}


/*
 * GETDECI:  EQU    *
 *           PSHY
 *           CLRA
 *           PSHA
 *           PSHA
 *           TSY
 *           LDX    IBUFPTR
 *           LDAA   0,X
 *           JSR    NUMERIC
 *           BCS    GETDECI1
 *           LDAA   #SYTXERR
 *           BRA    CHCKERR
 * GETDECI1: LDAA   0,X
 *           JSR    NUMERIC
 *           BCC    GETDECI3
 *           JSR    ADDDIG
 *           BPL    GETDECI1
 *           LDAA   #INTOVERR
 *           BRA    CHCKERR
 * GETDECI3: STX    IBUFPTR
 *           LDD    0,Y
 *           INS
 *           INS
 *           PULY
 *           RTS
 */

/*
 *  gethex      process a hex integer constant from the input stream
 *
 *  This routine uses the input buffer (ibufptr).
 */

U32  gethex(void)
{
	char						c;
	char						count;
	U32							num;

	num = 0;
	count = 0;
	if (!hexdig(*ibufptr))			// if first char is not a hex digit...
	{
		errcode = IVHEXERR;
		return(0);
	}

/*
 * GETHEX:   EQU    *
 *           PSHY
 *           CLRA
 *           PSHA
 *           PSHA
 *           TSY
 *           LDX    IBUFPTR
 *           LDAA   0,X
 *           JSR    HEXDIG
 *           BCS    GETHEX1
 *           LDAA   #IVHEXERR
 * CHCKERR:  TST    RUNFLAG
 *           BEQ    GETHEX5
 *           JMP    RPTRERR
 * GETHEX5:  JMP    RPTERR
 */


	while (hexdig(c = (char)toupper((char)*ibufptr)))			// for all hex digits in the input buffer...
	{
		num = num * 16 + a2hex(c);
//		if (count++ > 4)					// if too many digits in the field 
		if (count++ > (sizeof(U32)*2))		// if too many digits in the field
		{
			errcode = HEXOVERR;				// show too many digits in a hex number
			return(0);
		}
		++ibufptr;
	}
	return(num);							// return computed value
}

/*
 * GETHEX1:  EQU    *
 *           LDAA   0,X
 *           JSR    HEXDIG
 *           BCC    GETDECI3
 *           LDD    0,Y
 *           LSLD
 *           BCS    GETHEX3
 *           LSLD
 *           BCS    GETHEX3
 *           LSLD
 *           BCS    GETHEX3
 *           LSLD
 *           BCS    GETHEX3
 *           STD    0,Y
 *           LDAA   0,X
 *            JSR      ToUpper
 *            TAB
 *           INX
 *           SUBB   #'0'
 *           CMPB   #9
 *           BLS    GETHEX4
 *           SUBB   #7
 * GETHEX4:  CLRA
 *           ADDD   0,Y
 *           STD    0,Y
 *           BRA    GETHEX1
 * GETHEX3:  LDAA   #HEXOVERR
 *           BRA    CHCKERR
 */


int  hexdig(char  c)
{
//	return (isdigit(c) || (c>='A' && c<='F'));	// TRUE if 0-9, A-F
	return  (isxdigit(c));
}


/*
 * HEXDIG:   EQU    *
 *           JSR    NUMERIC
 *           BCC    HEXDIG1
 *           RTS
 * HEXDIG1:  JSR    ToUpper
 *           CMPA   #'A'
 *           BLO    HEXDIG2
 *           CMPA   #'F'
 *           BHI    HEXDIG2
 *           SEC
 *           RTS
 * HEXDIG2:  CLC
 *           RTS
 */


void  getscon(void)
{
	unsigned char				count;
	unsigned char				*bufptr;
	unsigned char				c;

	count = 0;								// init to allow for overhead in buffer
	*tbufptr++ = SCONTOK;					// add string const token
	bufptr = tbufptr++;						// record token buffer pointer, then bump it
											// and reserve a byte for string length
	ibufptr++;

/*
 * GETSCON:  EQU    *
 *           LDAB   #2
 *           LDAA   #SCONTOK
 *           JSR    PUTTOK
 *           LDX    TBUFPTR
 *           PSHX
 *           CLRA
 *           JSR    PUTTOK
 *           JSR    GETNXCHR     ; PUT FIRST QUOTE IN TOKEN BUFFER.
 *           JSR    PUTTOK
 */


	while (((c = *ibufptr) != '"'))			// for all chars until the ending double-quote...
	{
		if (c == EOL)						// if hit the end of the line...
		{
			errcode = MISQUERR;				// error, no ending quote
			return;
		}
		*tbufptr++ = c;						// if not, put next char in buffer
		++ibufptr;							// advance input buffer pointer
		++count;							// up byte count
	}

 
/* GETSCON1: EQU    *
 *           JSR    GETNXCHR
 *           CMPA   #'"'
 *           BEQ    GETSCON2
 *           CMPA   #EOL
 *           BNE    GETSCON3
 *           LDAA   #MISQUERR
 *           JMP    RPTERR
 * GETSCON3: JSR    PUTTOK
 *           INCB
 *           BRA    GETSCON1
 */

	++ibufptr;								// advance input buffer pointer
	*bufptr = count;						// put string byte count in token buffer
	return;
}

/*
 * GETSCON2: EQU    *
 *           JSR    PUTTOK
 * GETSCON4: PULX
 *           STAB   0,X
 *           RTS
 */
 
#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
void  getbcon(void)
{
	unsigned char				count;
	unsigned char				*bufptr;
	unsigned char				c;
	U8							num = 0;

	count = 0;								// init to allow for overhead in buffer
	*tbufptr++ = SCONTOK;					// add string const token
	bufptr = tbufptr++;						// record token buffer pointer, then bump it
											// and reserve a byte for string length
	ibufptr++;

	while (((c = *ibufptr) != '"'))			// for all chars until the ending double-quote...
	{
		if (c == EOL)						// if hit the end of the line...
		{
			errcode = MISQUERR;				// error, no ending quote
			return;
		}
		if (!hexdig(c))						// if first char is not a hex digit...
		{
			errcode = IVHEXERR;
			return;
		}
		num = a2hex(c) << 4;
		++ibufptr;							// advance input buffer pointer
		if((c = *ibufptr) != '"')
		{
			if (!hexdig(c))						// if first char is not a hex digit...
			{
				errcode = IVHEXERR;
				return;
			}
			num += a2hex(c);
			++ibufptr;							// advance input buffer pointer
		}
		*tbufptr++ = num;						// if not, put next char in buffer
		++count;							// up byte count
	}
	++ibufptr;								// advance input buffer pointer
	*bufptr = count;						// put string byte count in token buffer
	return;
}
#endif
