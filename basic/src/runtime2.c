/*
 *  runtime2      run-time routines for the Basic11 project
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"

#if defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)

#include	"uart_interface.h"
#include 	"hardware.h"
#if defined(RL7023_BINDING)
#include 	"adf7023.h"
#include 	"adf15d4g.h"
#include 	"SelfLib.h"
#endif

#include 	"decentra_base.h"
#include 	"debug.h"
#include	"profile.h"
#include	"memory.h"
#include	"context.h"
#include	"routing.h"
#include 	"util.h"
#include 	"framework.h"
#include 	"selector.h"
#include 	"lookup.h"
#include 	"scheduler.h"
#include 	"misc.h"
#include 	"impl.h"
#include 	"rl7023_impl.h."

extern RuntimeContext 		runtime;

#endif

static void					DumpRam(U8  *addr);
unsigned char				*varram;

/*
 *  rdim      execute DIM token
 */
void  rdim(void)
{
	U32					*dynptr;
	U16					varind;
	U8					*tmem;				// temp copy of dynamic memory ptr
	I16					n;	
	U16					k;
	U8					*tstrastg;			// temp copy of strastg
//	U16					voff;				// offset into variable RAM area

	while (1)								// for all fields in DIM statement
	{
		tstrastg = strastg;					// in case this doesn't work
		rskipspc();							// move to array identifier
		if ((*tbufptr & ARRAY_MASK) == 0)	// if variable does not have a subscript...
		{
			errcode = NOSUBERR;				// missing subscript
			return;
		}
		varind = getU16((U16 *)(tbufptr+1));		// get index into variable RAM

		varind = varind + 1 + MAX_VAR_NAME_LEN;	// adjust to point to variable's value
		dynptr = targetgetvarptr(varind);	// point to "value", really pointer into dyn mem pool

		tbufptr = tbufptr+4;				// move to 1st token in expression
		donexp();							// resolve the expression
		tbufptr++;							// step past closing paren
		if (getU32(dynptr) == 0)					// if not already dimensioned...
		{
			tmem = strastg;						// need temp copy of dynamic mem index
			n = pull32(&numstack, STKUNDER);		// get number of cells in array
			if (n<0)							// if negative index...
			{
				errcode = NEGSUBER;				// that's not allowed
				return;
			}
			n = n + 1;							// allow for starting at 0
			strastg = strastg + sizeof(U16);	// create placeholder for max subscript
#ifdef  AVR
			*(U16 *)dynptr = (U16)tmem;			// write the low half of the address
			dynptr = (U16)dynptr + 2;			// advance two bytes
			*(U16 *)dynptr = 0;					// high half of the address is always zero
//			*(U8 *)(dynptr+2) = 0;				// high half of the address is always zero
#endif
#ifdef  WINDOWS
			*dynptr = (U32)tmem;				// write start of array dyn mem into dictionary
#endif
#if  defined(CPU78K0R) || defined(CPUML7416) || defined(CPUSTM32)
			setU32(dynptr, (U32)tmem);				// write start of array dyn mem into dictionary
#endif
//			*(U16 *)tmem = n;					// save max subscript in reserved area
			setU16((U16 *)tmem, n);					// save max subscript in reserved area
			tmem = strastg;						// save start of storage for this array
			strastg = strastg + (n*ISIZ);		// now point to next open dyn memory cell
			if (strastg > dynmemend)			// if out of dynamic memory...
			{
				errcode = OMEMERR;				// show out of memory
				strastg = tstrastg;				// restore the dyn mem pointer to last good value
				return;
			}	
			for (k=0; k<n; k++)					// need to clear out all bytes in array
			{
//				*(U32 *)tmem = 0;				// write a zero
				setU32((U32 *)tmem, 0);				// write a zero
				tmem = tmem + sizeof(U32);		// move to next cell
			}
		}
		rskipspc();							// move to next field, if any
		if (*tbufptr == EOLTOK)  return;	// if finished, leave now
		tbufptr++;							// another to do, move past comma
	}
}


/*
static void  DumpRam(U8  *addr)
{
	char				tbuff[60];
	U8					*ptr;
	U16					n;

	ptr = addr;
	pl_P(PSTR("\n\rDumpRam()"));
	for (n=0; n<64; n++)
	{
		if (n%16 == 0)
		{
			sprintf_P(tbuff, PSTR("\n\r%08x: "), ptr);
			pl(tbuff);
		}
		sprintf_P(tbuff, PSTR("%02x "), *ptr);
		pl(tbuff);
		ptr++;
	}
}
*/






/*
 * RDIM:     EQU    *
 *           LDAA   0,Y          ; GET VARIABLE FLAG/TYPE.
 *           BITA   #$10         ; IS IT A SUBSCRIPTED VARIABLE?
 *           BNE    RDIM1        ; YES. GO DIMENSION IT.
 *           LDAA   #NOSUBERR    ; NO. GET ERROR.
 * RDIM3:    JMP    RPTRERR      ; GO REPORT THE ERROR.
 * RDIM1:    LDD    1,Y          ; GET THE OFFSET INTO THE DICTIONARY.
 *           ADDD   VARBEGIN     ; ADD IN THE START OF THE DICTIONARY.
 *           XGDX                ; PUT THE ADDRESS INTO X.
 *           LDD    3,X          ; GET THE POINTER TO THE STORAGE. BEEN DIMENSIONED?
 *           BEQ    RDIM2        ; NO. GO DIMENSION IT.
 *           LDAA   #REDIMERR    ; YES. ERROR.
 *           BRA    RDIM3
 * RDIM2:    PSHX                ; SAVE THE POINTER TO THE DICTIONARY.
 *           LDAB   #4           ; POINT TO 1ST TOKEN IN EXPRESSION.
 *           ABY
 *           JSR    DONEXP       ; EVALUATE THE SUBSCRIPT.
 *           INY                 ; PASS UP THE CLOSING PAREN.
 *           PULX                ; RESTORE POINTER TO DICTIONARY.
 *           LDD    STRASTG      ; GET THE DYNAMIC MEMORY POOL POINTER.
 *           STD    3,X          ; PUT THE POINTER IN THE DICTIONARY ENTRY.
 *           ADDD   #2           ; UP THE POINTER.
 *           STD    STRASTG      ; SAVE NEW POINTER FOR NOW.
 *           JSR    PULNUM       ; GET SUBSCRIPT OFF OF NUMERIC STACK.
 *           BPL    RDIM8        ; ONLY POSITIVE SUBSCRIPTS ALLOWED.
 *           LDAA   #NEGSUBER    ; NEGATIVE NUMBER.
 *           BRA    RDIM9        ; REPORT ERROR.
 * RDIM8:    PSHX
 *           LDX    3,X          ; GET POINTER TO STORAGE.
 *           STD    0,X          ; PUT MAX SUBSCRIPT IN POOL STORAGE.
 *           ADDD   #1           ; COMPENSATE FOR "0" SUBSCRIPT.
 *           PULX                ; RESTORE POINTER TO DICTIONARY ENTRY.
 *           LSLD                ; MULT. BY 2 (2 BYTES/INTEGER).
 *           ADDD   STRASTG      ; ADD IN CURRENT POINTER TO POOL.
 *           CPD    STRASTG      ; WAS THE SUBSCRIPT SO BIG WE WRAPPED AROUND?
 *           BLS    RDIM4        ; YES. ERROR.
 *           CPD    VARMEND      ; DO WE HAVE ENOUGH MEMORY?
 *           BLS    RDIM5        ; YES.
 * RDIM4:    LDAA   #OMEMERR     ; NO. ERROR.
 * RDIM9:    JMP    RPTRERR      ; GO REPORT THE ERROR.
 * RDIM5:    STD    STRASTG      ; SAVE POINTER.
 *           LDX    3,X          ; POINT TO START OF STORAGE.
 *           INX
 *           INX                 ; POINT PAST THE SUBSCRIPT LIMIT.
 * RDIM6:    CLR    0,X          ; CLEAR THE STORAGE.
 *           INX                 ; POINT TO THE NEXT LOCATION.
 *           CPX    STRASTG      ; ARE WE DONE?
 *           BNE    RDIM6        ; NO. KEEP GOING.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAA   0,Y          ; GET THE NEXT CHARACTER.
 *           CMPA   #EOLTOK      ; ARE WE AT THE END OF THE LINE.
 *           BEQ    RDIM7        ; YES.
 *           INY                 ; BUMP IP PAST THE COMMA.
 *           JSR    RSKIPSPC     ; SKIP SPACES.
 *           BRA    RDIM         ; DO DIMENSION THE NEXT VARIABLE.
 * RDIM7:    RTS                 ; BACK TO MAIN INTERPRET LOOP.
 */





/*
 *  rtron      execute TRON token
 */
void  rtron(void)
{
	trflag = 0xff;
}

/* 
 * RTRON:    EQU    *
 *           LDAA   #$FF         ; SET FLAG TO TURN TRACE MODE ON.
 *           STAA   TRFLAG       ; PUT IT IN THE FLAG BYTE.
 * RTRON1:   RTS                 ; BACK TO THE INTERPRET LOOP.
 */


/*
 *  rtroff      execute TROFF token
 */
void  rtroff(void)
{
	trflag = 0;
}

/* RTROFF:   EQU    *
 *           CLR    TRFLAG       ; TURN THE TRACE MODE OFF.
 *           RTS                 ; BACK TO THE INTERPRET LOOP.
 */



/*
 *  rsleep      execute SLEEP token
 */
void  rsleep(void)
{
	uint32_t	val;
	rskipspc();							// skip any embedded spaces
	donexp();					// parse the expression
	if (errcode)  return;			// if any problems, leave now
	val = pull32(&numstack, STKUNDER);
	if(val > 0){
#if defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
		VClock sleep_clock;
		get_current_clock(&sleep_clock, &runtime);
		add_time(&sleep_clock, val * 1000);
	#if defined(RL7023_BINDING)
		set_wakeup_time(&sleep_clock, &runtime);
	#else
		set_wakeup_time(&sleep_clock, 0, &runtime);
	#endif

		do_sleep(&runtime);
#endif
	}
}

/*
 * RSLEEP:   EQU       *
 *           SEI                 ; DON'T ALLOW AN INTERRUPT TO BRING US OUT OF THE SLEEP MODE.
 *           TPA                 ; GET THE CONDITION CODE REGISTER.
 *           ANDA      #$7F      ; CLEAR THE STOP BIT
 *           TAP                 ; TRANSFER THE RESULT BACK TO THE CCR.
 *           STOP                ; HALT THE CPU.
 *           TPA                 ; ON EXIT FROM THE STOP MODE, GET THE CCR.
 *           ORAA      #$80      ; DISABLE THE STOP INSTRUCTION.
 *           TAP                 ; TRANSFER THE RESULT BACK TO THE CCR.
 *           CLI                 ; ALLOW INTERRUPTS.
 *           RTS                 ; RETURN TO WHAT WE WERE DOING.
 */



/*
 *  rprint      execute PRINT token
 */
void  rprint(void)
{
	unsigned char			tok;
	I32						val;
	char					printspc;
	char					withholdnl;
	U16						offset;
	U8						*strptr;

	withholdnl = FALSE;					// usually need a CR/LF after printing
	rskipspc();							// skip any embedded spaces
	tok = *tbufptr;
	while ((tok != EOLTOK) && (tok != MEOLTOK))
	{
		printspc = FALSE;					// show no space after printing
		switch  (tok)
		{
/*
 *  Commented this block out because the while() makes sure we never see this condition.
 */
//			case  EOLTOK:
//			case  MEOLTOK:
//			tbufptr++;						// need to move to next token explicitly
//			break;

			case  SCONTOK:					// string constant?
			val = *(tbufptr+1);				// get length of string
			tbufptr += 2;					// move to start of string
			outstr(tbufptr, val);			// write the string
			tbufptr += val;					// move to next token
			withholdnl = FALSE;				// follow with CR/LF if last print field
			break;

			case  SVARTOK:
			tbufptr++;							// move to offset address
			offset = getU16((U16 *)tbufptr);			// get offset to start of variable entry
			tbufptr = tbufptr + sizeof(U16);	// move pointer past offset
			strptr = (U8 *)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
			val = *strptr++;					// adjust for two double-quotes
			outstr(strptr, val);			// write the string
			withholdnl = FALSE;				// follow with CR/LF if last print field
			break;

			case  COMMATOK:					// comma delimiter?
			outbyte('\t');					// tab to next field
			tbufptr++;						// need to move to next token explicitly
			withholdnl = TRUE;
			break;

			case  SEMITOK:					// semicolon delimiter?
			tbufptr++;						// need to move to next token explicitly
			withholdnl = TRUE;
			break;							// nothing to do, just skip token

			case  FUNCTFLG:					// function?
//			tbufptr++;						// move past the function token
			tok = *(tbufptr+1);				// get function identifier
			switch  (tok)					// based on required function...
			{
				case  TABTOK:				// tab?
				rtab();
				break;

				case  CHRTOK:				// chr$ function?
				rchrs();
				printspc = TRUE;			// need a space after
				break;

				case  HEXTOK:				// hex(); four-byte hex value?
				rhex();
				printspc = TRUE;			// need a space after
				break;

				case  HEX2TOK:				// hex2(); one-byte hex value?
				rhex2();
				printspc = TRUE;			// need a space after
				break;

				case  HEX4TOK:				// hex4(); two-byte hex value?
				rhex4();
				printspc = TRUE;			// need a space after
				break;

//				case  TIMESTRTOK:			// timestr; print current time as string
//				rtimestr();
//				break;

//				case  DATESTRTOK:			// datestr; print current date as string
//				rdatestr();
//				break;

				default:					// not a printing function, process regular function
				donexp();					// parse the expression
				if (errcode)  return;		// leave if problems
				val = (I32)pull32(&numstack, STKUNDER);
//				sprintf(tbuff, "%ld", val);
//				pl(tbuff);					// string is in RAM, use pl() not pl_P()
				outdeci(val);
				printspc = TRUE;				// need a space after
				break;
			}
			withholdnl = FALSE;				// follow with CR/LF if last print field
			break;

			default:
			donexp();					// parse the expression
			if (errcode)  return;			// if any problems, leave now
			val = pull32(&numstack, STKUNDER);
//			sprintf(tbuff, "%ld", val);
//			pl(tbuff);						// string is in RAM, use pl(), not pl_P()
			outdeci(val);
			printspc = TRUE;				// need a space after
			withholdnl = FALSE;				// follow with CR/LF if last print field
			break;
		}
		if (printspc)  outbyte(' ');
		rskipspc();							// skip any embedded spaces
		tok = *tbufptr;						// try next field
	}
	if (!withholdnl)  nl();					// only print CR/LF if not following comma or semi!
}

/*
 * RPRINT:   EQU    *
 *           JSR    CHCKDEV      ; GO CHECK FOR ALTERNATE OUTPUT DEVICE.
 *           LDAA   0,Y          ; GET FIRST TOKEN.
 *           CMPA   #EOLTOK      ; IS IT AN EOL TOKEN?
 *           BEQ    RPRINT1      ; YES. JUST PRINT A CR/LF.
 *           CMPA   #MEOLTOK     ; IS IT A MID EOL TOKEN?
 *           BNE    RPRINT2      ; NO. GO PRINT A STRING OR NUMBER.
 * RPRINT1:  JSR    NL           ; YES. JUST PRINT A CR/LF.
 *           CLR    DEVNUM       ; GO BACK TO DEVICE #0.
 *           RTS                 ; BACK TO MAIN INTERPRET LOOP.
 * RPRINT2:  CMPA   #SCONTOK     ; IS IT A STRING CONSTANT?
 *           BNE    RPRINT3      ; NO. GO CHECK FOR A "PRINT FUNCTION".
 *           PSHY
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
 *           BRA    RPRINT4      ; GO DO NEXT EXPRESSION.
 * RPRINT3:  CMPA   #FUNCTFLG    ; IS IT A FUNCTION? 
 *           BNE    RPRINT10     ; NO. GO EVALUATE A NUMERIC EXPRESSION.
 *           LDAA   1,Y          ; GET THE FUNCTION TYPE.
 *           CMPA   #TABTOK      ; IS IT A TAB?
 *           BNE    RPRINT11     ; NO GO CHECK FOR "CHR$".
 *           JSR    RTAB         ; GO DO TAB.
 *           BRA    RPRINT4      ; GO SEE IF THERE'S MORE TO PRINT.
 * RPRINT11: CMPA   #CHRTOK      ; IS IT THE CHR$ FUNCTION.
 *           BNE    RPRINT12     ; NO. GO CHECK FOR HEX().
 *           JSR    RCHRS        ; YES. GO DO CHR$.
 *           BRA    RPRINT4      ; GO SEE IF THERE'S MORE TO PRINT.
 * RPRINT12: CMPA   #HEXTOK      ; IS IT THE HEX() FUNCTION?
 *           BNE    RPRINT10     ; NO. GO DO A NUMERIC EXPRESSION.
 *           JSR    RHEX         ; YES. GO PRINT THE NUMBER AS HEX.
 *           BRA    RPRINT4      ; GO SEE IF THERE'S MORE TO PRINT.
 * RPRINT10: CMPA   #HEX2TOK     ; IS IT THE HEX2() FUNCTION?
 *           BNE    RPRINT14     ; NO. GO DO A NUMERIC EXPRESSION.
 *           JSR    RHEX2        ; YES GO PRINT A NUMBER >=255 AS 2 HEX BYTES.
 *           BRA    RPRINT4      ; GO SEE IF THERE'S MORE TO PRINT.
 * RPRINT14: JSR    DONEXP       ; GO DO A NUMERIC EXPRESSION.
 *           JSR    PULNUM       ; GET THE NUMBER OFF THE NUMERIC STACK.
 *           JSR    OUTDECI      ; PRINT IT.
 *           LDAA   #SPC          ; PUT A TRAILING SPACE AFTER ALL NUMBERS.
 *           JSR    OUTBYTE      ; PRINT IT.
 * RPRINT4:  JSR    RSKIPSPC     ; SKIP SPACES.
 *           LDAA   0,Y          ; GET SEPERATOR CHARACTER.
 *           CMPA   #COMMATOK    ; IS IT A COMMA?
 *           BEQ    RPRINT5      ; NO.
 *           CMPA   #SEMITOK     ; IS IT A SEMICOLIN?
 *           BNE    RPRINT6      ; NO. MUST BE AN EOLTOK.
 *           INY                 ; DO NOTHING BUT BUMP THE IP.
 *           BRA    RPRINT7      ; GO CHECK FOR EOL AFTER COMMA OR SEMICOLIN.
 * RPRINT5:  INY                 ; BUMP IP PAST THE COMMATOK.
 *           LDAB   PRINTPOS     ; YES. "TAB" TO NEXT PRINT FIELD.
 *           ANDB   #$07         ; MASK OFF ALL BUT THE FIELD WIDTH.
 *           NEGB                ; MAKE IT NEGATIVE.
 *           ADDB   #8           ; ADD IN THE FIELD WIDTH. ARE WE ON A FIELD BOUND?
 *           BEQ    RPRINT7      ; YES. GO CHECK FOR AN EOL.
 *           LDAA   #SPC          ; NO. GET A SPACE & PRINT TILL WE GET THERE.
 * RPRINT8:  JSR    OUTBYTE      ; PRINT A SPACE.
 *           DECB                ; DECREMENT THE COUNT. ARE WE DONE?
 *           BNE    RPRINT8      ; NO. KEEP GOING.
 * RPRINT7:  JSR    RSKIPSPC     ; SKIP ANY SPACES.
 *           LDAA   0,Y          ; GET THE NEXT TOKEN IN THE LINE.
 *           CMPA   #EOLTOK      ; IS IT AN EOL TOKEN?
 *           BEQ    RPRINT9      ; YES. DONT DO A CR/LF AFTER A COMMA OR SEMI.
 *           CMPA   #MEOLTOK     ; NO. IS IT A MID EOL?
 *           BEQ    RPRINT9      ; SAME AS BEFORE.
 *           JMP    RPRINT2      ; IF NEITHER, GO PRINT THE NEXT EXPRESSION.
 * RPRINT6:  JSR    NL           ; DO A CR/LF IF EOL OR MIDEOL FOLLOWS EXPRESSION.
 * RPRINT9:  CLR    DEVNUM       ; GO BACK TO DEVICE #0.
 *           RTS                 ; GO DO NEXT LINE.
 */



/*
 *  rtab      execute TAB token
 */
void  rtab(void)
{
	int					c;

	pfuncom();							// process the expression
	c = pull32(&numstack, STKUNDER);		// get the result 
	outbyte('\t');						// not right, need to fix this
}

/*
 * RTAB:     EQU    *
 *           BSR    PFUNCOM      ; GO GET ARG. & CHECK MAGNITUDE. IS ARG. OK?
 *           BEQ    RTAB1        ; YES. GO DO TAB.
 *           LDAA   #TABARGER    ; NO. ERROR.
 * RTAB3:    JMP    RPTRERR      ; REPORT ERROR.
 * RTAB1:    CMPB   PRINTPOS     ; ARE WE ALREADY PAST THE "TAB" POSITION?
 *           BLS    RTAB2        ; YES. DONE.
 *           LDAA   #SPC          ; GET A SPACE.
 *           JSR    OUTBYTE      ; PRINT IT.
 *           BRA    RTAB1
 * RTAB2:    RTS                 ; RETURN.
 */


/*
 *  rchrs      execute CHRS token
 */
void  rchrs(void)
{
	U32					c;

	pfuncom();							// process the expression
	c = pull32(&numstack, STKUNDER);		// get the result 
//	sprintf(tbuff, "%c", c);
	outbyte((U8)c);							// just send the char as ASCII
//	pl(tbuff);							// string is in RAM, use pl(), not pl_P()
}

/*
 * RCHRS:    EQU    *
 *           BSR    PFUNCOM      ; GO GET ARG. & CHECK MAGNITUDE. IS ARG. OK?
 *           BEQ    RCHRS1       ; YES. GO DO TAB.
 *           LDAA   #CHRARGER    ; NO. ERROR.
 *           BRA    RTAB3        ; REPORT ERROR.
 * RCHRS1:   TBA                 ; PUT BYTE INTO A
 *           JMP    OUTBYTE      ; PRINT THE BYTE & RETURN.
 */



/*
 *  rhex2      execute HEX2 token
 *  rhex4       execute HEX token
 */
void  rhex2(void)
{
	U32					c;

	pfuncom();							// process the expression
	c = pull32(&numstack, STKUNDER);		// get the result
	outhexbyte((U8)(c & 0xff));
}


void  rhex4(void)
{
	U32					c;

	pfuncom();							// process the expression
	c = pull32(&numstack, STKUNDER);		// get the result
	outhexbyte((U8)(c >> 8));			// print top byte of 16-bit value
	outhexbyte((U8)(c & 0xff));			// print bottom byte of 16-bit value
}


void  rhex(void)
{
	U32					c;

	pfuncom();							// process the expression
	c = pull32(&numstack, STKUNDER);		// get the result
	outhexbyte((U8)(c >> 24));			// print top byte of 32-bit value
	outhexbyte((U8)(c >> 16));			// print 2nd byte of 32-bit value
	outhexbyte((U8)(c >> 8));			// print 3rd byte of 32-bit value
	outhexbyte((U8)(c & 0xff));			// print bottom byte of 32-bit value
}


/*
 * RHEX2:    EQU    *
 *           BSR    PFUNCOM      ; GO GET ARG. & CHECK MAGNITUDE. IS ARG. OK?
 *           BEQ    RHEX1        ; YES. GO PRINT 2 HEX CHARACTERS & RETURN.
 *           LDAA   #HEX2AERR    ; NO. ARG. MUST BE >=0 & <=255.
 *           BRA    RTAB3        ; GO REPORT ERROR.
 * *
 * *
 * RHEX:     EQU    *
 *           BSR    PFUNCOM      ; GO DO COMMON CODE FOR PRINT FUNCTIONS
 *           BSR    PRNT2HEX     ; GO PRINT 2 HEX CHARACTERS.
 * RHEX1:    TBA                 ; PUT LOWER BYTE IN A.
 * *                             ; FALL THRU TO PRINT 2 HEX CHARACTERS & RETURN.
 * *
 * *
 * PRNT2HEX: EQU    *
 *           PSHA                ; SAVE THE CHARACTER.
 *           BSR    PRNTHXL      ; PRINT THE LEFT HEX NYBBLE.
 *           PULA                ; GET BYTE BACK.
 *           BRA    PRNTHXR      ; PRINT RIGHT NYBBLE & RETURN.
 * *
 * *
 * PRNTHXL:  LSRA                ; GET UPPER NYBBLE INTO LOWER ONE.
 *           LSRA
 *           LSRA
 *           LSRA
 * PRNTHXR:  ANDA   #$0F         ; MASK OFF UPPER NYBBLE.
 *           ADDA   #$30         ; MAKE IT A HEX NUMBER.
 *           CMPA   #$39         ; IS IT?
 *           BLS    PRNTHXR1     ; YES. PRINT IT.
 *           ADDA   #$07         ; NO. MAKE IT A HEX LETTER.
 * PRNTHXR1: JMP    OUTBYTE      ; PRINT IT & RETURN.
 */


/*
 *  outhexbyte      print byte as two hex chars
 */
void  outhexbyte(U8  c)
{
	U8				t;

	t = c >> 4;
	if (t > 9)  t = t + 7;
	outbyte((U8)(t + '0'));
	t = c & 15;
	if (t > 9)  t = t + 7;
	outbyte((U8)(t + '0'));
}





/*
 * pfuncom      common PRINT function code
 */
void  pfuncom(void)
{
	tbufptr = tbufptr + 3;		// move past function flag, function ID, and open paren
	donexp();					// process expression
	tbufptr++;					// move past closing paren
}

/*
 * PFUNCOM:  EQU    *
 *           LDAB   #3           ; POINT PAST FUNCTION FLAG, FUNCTION TOKEN, &
 *           ABY                 ; OPEN PAREN.
 *           JSR    DONEXP       ; GO GET POSITION TO TAB TO.
 *           INY                 ; BUMP IP PAST CLOSING PAREN.
 *           JSR    PULNUM       ; GET OPERAND OFF STACK.
 *           TSTA                ; CHECK THAT OPERAND IS >0 & <=255 FOR FUNCTIONS
 * *                             ; THAT REQUIRE IT.
 *           RTS                 ; RETURN.
 */


