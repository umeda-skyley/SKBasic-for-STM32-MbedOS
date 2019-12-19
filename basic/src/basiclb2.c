/*
 *  basiclb2.c      library functions for KLBasic compiler
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
static void				processremdata(unsigned char  tok);






/*
 *  xmideol()
 */

void  xmideol(void)
{
	*tbufptr++ = MEOLTOK;
	++ibufptr;
	return;
}




/*
 * common code for GOSUB and GOTO
 */

void  xgo(char  gotok)
{
	U16						num;

	*tbufptr++ = gotok;				// put GOTO or GOSUB token in buffer
	blanks();						// skip blanks before line number
	if(*ibufptr == '"'){
		getcon();
	}else{
		*tbufptr++ = LCONTOK;			// put line number constant token in buffer
		num=getlinum();					// get line number
		if(num==0) errcode=LINENERR;	// if 0, line number error
		if(errcode) return;				// if error, return
		putint16(num);					// put line number in buffer (don't use putlinum()!
	}
	return;
}


/*
 * XGOSUB:   EQU    *
 * XGOTO:    EQU    *
 *           JSR    BLANKS
 *           LDAA   #LCONTOK
 *           BSR    PUTTOK
 *           JSR    GETLINUM
 * XGOTO2:   BRA    PUTDTOK
 *
 *
 *<><><><><><> ROUTINE NOT NEEDED <><><><><><>
 */


/*
 *  xgosub
 */

void  xgosub(void)
{
	xgo(GOSUBTOK);
	return;
}


 
/*
 *  xgoto
 *  This routine is not needed in the 68hc11 (assembler) version
 */

void  xgoto(void)
{
	xgo(GOTOTOK);
	return;
}



      
/*
 *  xreturn
 */

void  xreturn(void)
{
	*tbufptr++ = RETNTOK;		// put RETURN token in buffer
	return;
}




/*
 *  xstop
 */

void  xstop(void)
{
	*tbufptr++ = STOPTOK;		// put STOP token in buffer
	return;
}



/*
 *  xend
 */

void  xend(void)
{
	*tbufptr++=ENDTOK;			// put end token in buffer
	return;
}




/*
 *  xtron
 */

void  xtron(void)
{
	*tbufptr++ = TRONTOK;		// put TRON token in buffer
	return;
}





/*
 *  xtroff
 */

void  xtroff(void)
{
	*tbufptr++ = TROFFTOK;		// put TROFF token in buffer
	return;
}


/*
 * XRETURN:  EQU    *
 * XSTOP:    EQU    *
 * XEND:     EQU    *
 * XTRON:    EQU    *
 * XTROFF:   EQU    *
 * XRESTORE: EQU    *
 * XENDWH:   EQU    *
 * XRETI:    EQU    *
 * XSLEEP:   EQU    *
 * XRTIME:   EQU    *
 *           RTS                ; NULL FUNCTIONS BECAUSE TOKEN PLACEMENT IS DONE IN
 *                              ; XLATE FUNCTION.
 */



          
/*
 *  xrem
 */

void  xrem(void)
{
	processremdata(REMTOK);			// process the REM statement
}




/*
 *  xqrem
 */
void  xqrem(void)
{
	processremdata(QREMTOK);		// process the quick REM statement
}




/*
 *  xdata
 */

void  xdata(void)
{
	processremdata(DATATOK);
}

  

static void  processremdata(unsigned char  tok)
{
	unsigned char				len;
	unsigned char				*tptr;
	unsigned char				c;

	len = 2;						// start at 2 to allow for length byte and EOL
	*tbufptr++ = tok;				// put REM or DATA token in buffer
	tptr = tbufptr;					// save pointer to length field in token string
	*tbufptr++ = 0;					// placeholder length byte, will be overwritten later
	while (1)
	{
		c = *ibufptr++;				// get char from input buffer
		*tbufptr++ = c;				// save char in token buffer
		if (c == EOL)  break;		// leave if we hit the EOL char
		len++;						// count length of remark
	}
	ibufptr--;						// back up, we must end pointing to EOL
	*tptr = len;
	return;
}



	
/*
 * XDATA:    EQU    *
 * XREM:     EQU    *
 *           LDX    TBUFPTR      ; GET POINTER TO TOKEN BUFFER.
 *           PSHX                ; SAVE IT. (POINTER TO LENGTH OF REM OR DATA)
 *           LDAA   #0           ; SAVE A BYTE FOR THE LENGTH.
 *           BSR    PUTTOK
 *           LDAB   #2           ; INITALIZE LENGTH  TO 2 (INCLUDES LENGTH & EOL.
 * XREM1:    BSR    GETCHR
 *           CMPA   #EOL
 *           BEQ    XREM2
 *           BSR    PUTTOK
 *           BSR    INCIBP
 *           INCB                ; UP THE BYTE COUNT.
 *           BRA    XREM1
 * XREM2:    BSR    PUTTOK
 *           PULX                ; GET POINTER TO LENGTH BYTE.
 *           STAB   0,X          ; PUT IT IN THE TOKEN BUFFER.
 *           RTS
 *
 *
 * XPORTA:   EQU    *
 * XPORTB:   EQU    *
 * XPORTC:   EQU    *
 * XPORTD:   EQU    *
 *           LDAB   #NUM         ; WE'RE XLATING A NUMERICAL STATEMENT.
 *           BRA    ASIGNMT1     ; GO DO IT LIKE AN ASIGNMENT STATEMENT.
 *
 *
 *
 */

          
/*
 *  xlet
 */

void  xlet(void)
{
	letcom(LETTOK);                // pass LET token to common code
	return;
}

  
	

/*
 *  ximplet (implied LET)
 */

void  ximplet(void)
{
	letcom(IMLETTOK);
	return;
}





/*
 *  letcom     common code for explicit & implicit LET
 *
 *  This routine checks for an indirection token (@, @16, or @32) before the
 *  variable name and equals sign.
 */

void  letcom(char  letok)
{
	*tbufptr++=letok;				// put LET token in buffer
	blanks();						// skip blanks before (possible) indirection token
	if 		(match(INDIR32_STR))  *tbufptr++ = INDIR32TOK;
	else if (match(INDIR16_STR))  *tbufptr++ = INDIR16TOK;
	else if (match(INDIR_STR))    *tbufptr++ = INDIRTOK;
	blanks();						// skip blanks before assignment (only really needed after indirection)
	asignmt();						// evaluate expression
	return;
}

  
	
/*
 * XLET:     EQU    *
 * XIMPLET:  EQU    *
 *           JSR    BLANKS
 * XLET1     JMP    ASIGNMT
 *
 */


          
/*
 *  asignmt
 */

void  asignmt(void)
{
	int				type;

	if ((type = getioport()) == 0)				// if unable to find I/O port...
	{
		if ((type = getvar()) == 0)  return;	// and unable to find variable, return
	}
	if (errcode)  return;
// ASIGNMT1:
	blanks();									// allow spaces before '='
	if (*ibufptr++ != '=')						// if no equals sign...
	{
		errcode = IVEXPERR;						// invalid expression
		return;
	}
	*tbufptr++ = EQUALTOK;						// put equals token in buffer
	xexpres((U8)type);								// build expression in token buffer
	return;
}

  
	
/*
 * ASIGNMT:  EQU    *
 *           JSR    GETVAR
 *           TAB
 * ASIGNMT1: BSR    GETNXCHR
 *           CMPA   #'='
 *           BEQ    ASIGNMT2
 *           LDAA   #IVEXPERR
 *           JMP    RPTERR
 * ASIGNMT2: LDAA   #EQUALTOK
 *           BSR    PUTTOK
 *           TBA
 *                               FALL THROUGH TO XEXPRES.
 *
 */


          
/*
 *  xexpres
 */

int   xexpres(U8  type)
{
	U8						c;
	char					thenflag;

	thenflag = 0;						// used in processing IF-THEN clause
	while (1)
	{
		blanks();										// ADDED  strip out cosmetic blanks
		if      (match(NEG_STR))    	*tbufptr++ = NEGTOK;
		else if (match(INDIR16_STR))  	*tbufptr++ = INDIR16TOK;	// must do this indirection check first!
		else if (match(INDIR32_STR))  	*tbufptr++ = INDIR32TOK;	// must do this indirection check second!
		else if (match(INDIR_STR))  	*tbufptr++ = INDIRTOK;		// now safe to do basic indirection check
		else if (match(NOT_STR))    	*tbufptr++ = NOTTOK;

/*
 * XEXPRES:  EQU    *
 *           PSHY
 *           PSHA
 *           TSY
 * XEXPRS29: LDX    #UINARYOP
 *           JSR    TBLSRCH
 *           BCC    XEXPRS30
 *           BSR    PUTTOK
 *
 */

		blanks();						// ADDED  needed for proper handling of NOT token
		if (*ibufptr == '(')			// open paren?
		{
			*tbufptr++ = OPARNTOK;		// put in token buffer
			++ibufptr;					// point to next char in input buffer
			xexpres(type);				// go get sub expression
			if (errcode) return -1;
			if (*ibufptr != ')')
			{
				errcode = UPARNERR;
				return -1;
			}
			*tbufptr++ = CPARNTOK;		// put it in the token buffer
			++ibufptr;					// point to the next char in the input buffer
			type = NUM;					// show we have resolved a number (ADDED)
			goto chkoprtr;
		}

/*
 * XEXPRS30: JSR    GETCHR
 *           CMPA   #'('
 *           BNE    XEXPRS1
 *           JSR    INCIBP
 *           LDAA   #OPARNTOK
 *           JSR    PUTTOK
 *           LDAA   0,Y
 *           JSR    XEXPRES
 * XEXPRS2:  JSR    GETNXCHR
 *           CMPA   #')'
 *           BEQ    XEXPRS3
 *           LDAA   #UPARNERR
 *           JMP    RPTERR
 * XEXPRS3:  LDAA   #CPARNTOK
 *           JSR    PUTTOK
 *           JMP    CHKOPRTR
 */

		c = (U8)getcon();							// first, try for a constant
		if (!c)  c = (U8)getfun();					// next, try a function
		if (!c)  c = (U8)getioport();				// next, could be an I/O port
		if (!c)  c = (U8)getvar();					// last chance, see if it's a variable
		if (errcode)  return -1;					// if any known problems, bail now
		if (type == 0)  type = c;
		if (c != type)
		{
			errcode = DTMISERR;
			return -1;
		}
/*
		if ((isdigit(*ibufptr)) || (*ibufptr == '$') || (*ibufptr == '"'))	// original used numeric()
		{
			c = getcon();
//			if (errcode)  return;
		}
		else
		{
			c = getfun();
			if (!c)  c = getioport();			// first, try for a port
			if (!c)  c = getvar();				// finally, try for a variable
		}
		if (errcode)  return;
		if (type == 0) type = c;
		if (c != type)
		{
			errcode = DTMISERR;
			return;
		}
 */
 
  
/*
 * XEXPRS1:  EQU    *
 *           JSR    NUMERIC
 *           BCS    XEXPRS4
 *           CMPA   #'$'
 *           BEQ    XEXPRS4
 *           CMPA   #'"'
 *           BNE    XEXPRS5
 * XEXPRS4:  JSR    GETCON
 *           BRA    XEXPRS7
 * XEXPRS5:  JSR    GETFUN
 *           TSTA
 *           BNE    XEXPRS7
 *           JSR    GETVAR
 * XEXPRS7:  LDAB   0,Y
 *           CMPB   #NULL
 *           BNE    XEXPRS8
 *           STAA   0,Y
 * XEXPRS8:  CMPA   0,Y
 *           BEQ    XEXPRS9
 *           LDAA   #DTMISERR
 *           JMP    RPTERR
 * XEXPRS9:  EQU    *
 *
 */


// now look for operator or end of expression

chkoprtr:
		blanks();				// skip any spaces
		c = *ibufptr;
//		if (c == EOL || c == MIDEOL || c == SPC || c == COMMA || c == SEMI || c == ')')
		if (c == EOL || c == MIDEOL || c == COMMA || c == SEMI || c == ')')
		{
			return type;
		}

/*
 * CHKOPRTR: EQU    *
 *           JSR    GETCHR
 *           CMPA   #EOL
 *           BEQ    XEXPRS24
 *           CMPA   #MIDEOL
 *           BEQ    XEXPRS24
 *           CMPA   #SPC
 *           BEQ    XEXPRS24
 *           CMPA   #COMMA
 *           BEQ    XEXPRS24
 *           CMPA   #SEMI
 *           BEQ    XEXPRS24
 *           CMPA   #')'
 *           BEQ    XEXPRS24
 */


		if (type == NUM)
		{
			if      ((c = cknumop())) ;
			else if ((c = ckbolop())) ;
			else if (ifwhflag)
			{
				if (thenflag)  return type;		// if looking for THEN, exit now
				c = cklogop();
				thenflag++;						// next time through we will be looking for THEN
			}
			else if (forflag)  return type;		// FOR with numeric clause, leave now so xfor() can handle TO or STEP clause
			else    c = 0;
		}

/*
 * XEXPRS15: EQU    *
 *           LDAA   0,Y
 *           CMPA   #NUM
 *           BNE    XEXPRS21
 *           JSR    CKNUMOP
 *           BCS    XEXPRS17
 *           JSR    CKBOLOP
 *           BCS    XEXPRS17
 *           TST    IFWHFLAG
 *           BEQ    XEXPRS18
 *           JSR    CKLOGOP
 *           BRA    XEXPRS17
 * XEXPRS18: LDAA   #NULL
 *           BRA    XEXPRS17
 */


		else
		{
			errcode = IDTYERR;
			return -1;
		}

/*
 * XEXPRS21: EQU    *
 *           LDAA   #IDTYERR
 *           JMP    RPTERR
 */


		if (c == 0)
		{
			errcode = OPRTRERR;
			return -1;
		}
		*tbufptr++ = c;
	}
	return type;
}



/*
 * XEXPRS17: EQU    *
 *           TSTA
 *           BNE    XEXPRS23
 *           LDAA   #OPRTRERR
 *           JMP    RPTERR
 * XEXPRS24: INS
 *           PULY
 *           RTS
 * XEXPRS23: JSR    PUTTOK
 *           JMP    XEXPRS29
 */



/*
 *  cknumop
 */

char  cknumop(void)
{
//	if      (match("+")) return(PLUSTOK);
//	else if (match("-")) return(MINUSTOK);  
//	else if (match("*")) return(MULTTOK);
//	else if (match("/")) return(DIVTOK);
//	else if (match("%")) return(MODTOK);
//	else if (match("^")) return(PWRTOK);
//	else    return(NULL);


	if      (match(PLUS_STR)) return(PLUSTOK);
	else if (match(MINUS_STR)) return(MINUSTOK);  
	else if (match(MULT_STR)) return(MULTTOK);
	else if (match(DIV_STR)) return(DIVTOK);
	else if (match(MOD_STR)) return(MODTOK);
	else if (match(PWR_STR)) return(PWRTOK);
	else    return(0);
}

/*
 * CKNUMOP:  EQU    *
 *           LDX    #NUMOPTBL
 *
 * CKOP:     JSR    TBLSRCH
 *           BCS    CKOP1   
 *           LDAA   #NULL
 * CKOP1:    RTS
 */


          
/*
 *  ckbolop
 */

char  ckbolop(void)
{
	if      (match("AND")) return(ANDTOK);
	else if (match("OR")) return(ORTOK);
	else if (match("EOR")) return(EORTOK);
//	else    return(NULL);
	else    return(0);
}

/*
 * CKBOLOP:  EQU    *
 *           LDX    #BOLOPTBL
 *           BRA    CKOP
 *
 */







          
/*
 *  cklogop
 */

char  cklogop(void)
{
	if      (match("<=")) return(LTEQTOK);
	else if (match(">=")) return(GTEQTOK);
	else if (match("<>")) return(NOTEQTOK);
	else if (match("<")) return(LTTOK);
	else if (match(">")) return(GTTOK);
	else if (match("=")) return(EQTOK);
//	else    return(NULL);
	else    return(0);
}

/*
 * CKLOGOP:  EQU    *
 *           LDX    #LOGOPTBL
 *           BRA    CKOP
 */



/*<><><><><> NOTE: THIS ROUTINE HAS NO 'C' COUNTER PART <><><><><><>
 *
 * TBLSRCH:  EQU    *
 *           JSR    STREQ        ; SEARCH FOR STRING.
 *           BCS    TBLSRCH1     ; IF FOUND GO GET TOKEN & RETURN.
 * TBLSRCH2: INX                 ; BUMP POINTER TO NEXT CHAR.
 *           LDAA   0,X          ; GET IT.
 *           BNE    TBLSRCH2     ; KEEP LOOKING FOR END OF ENTRY.
 *           INX                 ; FOUND IT. BUMP POINTER TO NEXT ENTRY.
 *           INX
 *           LDAA   0,X          ; AT THE END OF THE TABLE?
 *           BNE    TBLSRCH      ; NO. GO CHECK THE NEXT ENTRY.
 *           CLC                 ; YES. FLAG AS NOT FOUND.
 *           RTS                 ; RETURN.
 * TBLSRCH1: LDAA   1,X          ; GET TOKEN.
 *           SEC                 ; FLAG AS FOUND.
 *           RTS                 ; RETURN.
 *
 *
 * NUMOPTBL: EQU    *
 * PLUS:     FCC    "+"
 *           FCB    0
 *           FCB    PLUSTOK
 * MINUS:    FCC    "-"
 *           FCB    0
 *           FCB    MINUSTOK
 * MULT:     FCC    "*"
 *           FCB    0
 *           FCB    MULTTOK
 * DIV:      FCC    "/"
 *           FCB    0
 *           FCB    DIVTOK
 * MODS:     FCB    $5C,$00
 *           FCB    MODTOK
 *           FCB    0            ; END OF TABLE FLAG.
 *
 * BOLOPTBL: EQU    *
 * ANDS:     FCC    ".AND."
 *           FCB    0
 *           FCB    ANDTOK
 * ORS:      FCC    ".OR."
 *           FCB    0
 *           FCB    ORTOK
 * EORS:     FCC    ".EOR."
 *           FCB    0
 *           FCB    EORTOK
 *           FCB    0            ; END OF TABLE FLAG.
 *
 * LOGOPTBL: EQU    *
 * LTEQ:     FCC    "<="
 *           FCB    0
 *           FCB    LTEQTOK
 * GTEQ:     FCC    ">="
 *           FCB    0
 *           FCB    GTEQTOK
 * NOTEQ:    FCC    "<>"
 *           FCB    0
 *           FCB    NOTEQTOK
 * LT:       FCC    "<"
 *           FCB    0
 *           FCB    LTTOK
 * GT:       FCC    ">"
 *           FCB    0
 *           FCB    GTTOK
 * EQ:       FCC    "="
 *           FCB    0
 *           FCB    EQTOK
 *           FCB    0            ; END OF TABLE FLAG.
 *
 *
 * UINARYOP: EQU    *
 * NEGS:     FCC    "-"
 *           FCB    0
 *           FCB    NEGTOK
 * NOTS:     FCC    "NOT"
 *           FCB    0
 *           FCB    NOTTOK
 *           FCB    0            ; END OF TABLE MARKER.
 *
 */

