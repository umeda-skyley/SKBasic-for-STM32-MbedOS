/*
 *  leditor.c      line editor for the Basic11 project
 */


#include  <stdio.h>
#include  <ctype.h>

#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"





/***** storlin() *****/
void  storlin(void)
{
	unsigned char		*plinum;
	unsigned int		tline;
	unsigned int		t2;

	contflag = TRUE;					// TRUE means cannot continue later
	tbufptr = tknbuf;					// rewind the token buffer pointer
//	tline = *tbufptr;					// get MSB of line number
//	tline = *(U16 *)tbufptr;			// get complete line number
	tline = getU16((U16 *)tbufptr);			// get complete line number
	if (tline > hiline)					// line # larger than current hi line
	{
		apendlin();						// append it to the end of the buffer
		hiline = tline;					// make it the current high line number
		return;
	}

/*
 * STORLIN:  EQU    *
 *           LDAA   #1           ; set the continue flag.
 *           STAA   CONTFLAG     ; we don't allow continues if the program has been altered.
 *           LDX       TKNBUFS   ; point to the start of the token buffer
 *           LDD    0,X          ; get the first 2 bytes of the token buffer (the line number).
 *           CPD    HILINE       ; was the entered lines number higher than the highest so far?
 *           BLS    STORLIN1     ; no. go do an insert or replace operation.
 *           JSR    APENDLIN     ; yes. just append the line to the end of the program buffer.
 *           LDX    TKNBUFS      ; point to the start of the token buffer
 *           LDD    0,X          ; get the first 2 bytes of the token buffer (the line number).
 *           STD    HILINE
 *           RTS                 ; return.
 */

	plinum = findline(tline);			// look for line # in the program buffer
//	t2 = *(U16 *)plinum;				// t2 holds line number from program buffer
	t2 = getU16((U16 *)plinum);				// t2 holds line number from program buffer

	if (tline == t2)					// is it the same line #?
	{
		repline(plinum);				// yes. replace it with the new line
		return;
	}
	insrtlin(plinum);					// no. insert the new line in the buffer
	return;
}

/*
 * STORLIN1: EQU    *
 *           BSR    FINDLINE
 *           LDD    0,X
 *           PSHX
 *           LDX    TKNBUFS
 *           CPD    0,X
 *           PULX
 *           BNE    INSRTLIN
 *           JMP    REPLINE
 */



/***** delline() *****/
void  delline(unsigned int  num)					// delete line from basic buffer
{
	unsigned char			*linum;
	unsigned char			*ptr;

	if (num > hiline) return;			// line number can't be there, return
	linum = findline(num);				// look for the requested line #

/*
 * DELLINE:  EQU    *
 * *        PSHD                 ; SAVE THE LINE NUMBER TO DELETE.
 *           PSHB
 *           PSHA
 *           TSY                 ; POINT TO THE LINE NUMBER WE SAVED.
 *           CPD    HILINE       ; IS IT HIGHER THAN THE HIGHEST LINE ENTERED SO FAR?
 *           BLS    DELLINE1     ; NO. GO SEE IF THE LINE EXISTS.
 * DELLINE2: LDAA   #1           ; YES. THE LINE CANNOT EXIST.
 *           STAA   CONTFLAG
 * *        PULD                 ; PULL THE LINE NUMBER OFF THE STACK.
 *           PULA
 *           PULB
 *           RTS                 ; RETURN.
 * DELLINE1: BSR    FINDLINE     ; GO SEE IF THE LINE EXISTS.
 *                              ; RETURN A POINTER TO A LINE NUMBER IN THE BASIC PROGRAM BUFFER.
 */

//	if (*(U16 *)linum != num) return;	// if the line # doesn't exist, return
	if (getU16((U16 *)linum) != num) return;	// if the line # doesn't exist, return
	ptr = linum;						// make the int pointer a char pointer
	closespc(ptr[2],ptr);				// go delete the line
	if (num == hiline) hiline = findhlin();
	return;
}

/*
 *           LDD    0,X          ; GET THE LINE NUMBER THAT WAS FOUND.
 *           CPD    0,Y          ; WAS THE LINE NUMBER FOUND THE ONE THAT WAS REQUESTED TO BE DELETED.
 *           BNE    DELLINE2     ; NO. THE LINE DOESN'T EXIST. JUST RETURN.
 *           LDAB   2,X          ; YES. GET THE LENGTH OF THE LINE.
 *           BSR    CLOSESPC     ; GO CLOSE THE SPACE IN THE PROGRAM BUFFER.
 *           LDD    HILINE       ; GET THE HIGHEST LINE NUMBER ENTERED.
 *           CPD    0,Y          ; DID WE DELETE THE HIGHEST LINE NUMBER?
 *           BNE    DELLINE2     ; NO. JUST RETURN.
 *           BSR    FINDHILN     ; YES. GO FIND THE HIGHEST LINE NUMBER.
 *           STD    HILINE       ; SAVE IT.
 *           BRA    DELLINE2     ; RETURN.
 */


/***** closespc() *****/				// close up space in the BASIC buffer
void  closespc(int  bytes, unsigned char  *ptr)
{
	unsigned char				*to;
	unsigned char				*from;

	to = ptr;						// set up destination pointer
	from=ptr+bytes;					// setup source pointer
	while (from < basend)			// while we're not at the end of the buff
	{
		*to++ = *from++;			// move source to destination
	}
	basend = to;					// set new basend pointer
	return;
}

/*
 * 
 * CLOSESPC: EQU    *            ; ENTERED WITH
 *           PSHY                ; SAVE THE CURRENT VALUE OF Y.
 *           PSHX                ; TRANSFER X TO Y BY... PUSHING X AND THEN
 *           PULY                ; PULLING Y.
 *           ABY                 ; ADD THE LENGTH TO Y.
 * CLOSESP1: CPY    BASEND       ; HAVE WE MOVED ALL THE BYTES?
 *           BHS    CLOSESP2     ; YES. RETURN.
 *           LDAA   0,Y          ; NO. GET A BYTE.
 *           STAA   0,X          ; MOVE IT.
 *           INX                 ; ADVANCE THE DESTINATION POINTER.
 *           INY                 ; ADVANCE THE SOURCE POINTER.
 *           BRA    CLOSESP1     ; GO CHECK TO SEE IF WE'RE DONE.
 * CLOSESP2: STX    BASEND       ; SAVE THE NEW 'END OF BASIC PROGRAM' POINTER.
 *           JSR    MOVEVARSDN   ; MOVE ALL THE VARIABLES DOWN.
 *           PULY                ; RESTORE Y.
 *           RTS                 ; RETURN.
 */


/***** findline() *****/
/*
 * return pointer to line number or next highest line number
 */
unsigned char  *findline(int  linenum)
{
	unsigned char			*basbufp;
	int						ln;

	basbufp = basbeg;						// set pointer to start of basic buffer
	while (1)								// do until we find line # or one higher
	{
//		ln = *(U16 *)basbufp;				// ln holds line number from NV memory
		ln = getU16((U16 *)basbufp);				// ln holds line number from NV memory
		basbufp = basbufp + sizeof(U16);	// move past line number
		if (ln >= linenum)					// if found line number or next highest...
		{
			return (basbufp-2);				// return pointer to start of line in memory
		}
		basbufp = *basbufp - 2 + basbufp;	// basbufp now points to start of next line
	}
}

/*
 * 
 * FINDLINE: EQU    *
 *           LDX    BASBEG
 * FINDLIN1: CPD    0,X
 *           BLS    FINDLIN2
 *           PSHB
 *           LDAB   2,X
 *           ABX
 *           PULB
 *           BRA    FINDLIN1
 * FINDLIN2: RTS
 */




/***** findhlin() *****/

int  findhlin(void)					// find highest line number in basic buffer
{
	unsigned char			*iptr;
	unsigned int			lnum;

	lnum = 0;						// set line # to 0
	iptr = basbeg;					// set int pointer to basbeg
	while (iptr != basend)			// while we're not to the end of the basic buffer
	{
//		lnum = *(U16 *)iptr;		// get current line #
		lnum = getU16((U16 *)iptr);		// get current line #
//		outdeci(lnum);				// debug
//		outbyte(' ');				// debug
		if (lnum == 0)  return lnum;		// leave if serious problem!
		iptr = iptr + *(unsigned char *)(iptr+2);	// move to start of next line
	}
	return (lnum);					// return highest line number
}

/*
 * 
 * FINDHILN: EQU    *
 *           LDX    BASBEG
 * FINDHIL1: CPX    BASEND
 *           BEQ    FINDHIL2
 *           LDD    0,X
 *           PSHB
 *           LDAB   2,X
 *           ABX
 *           PULB
 *           BRA    FINDHIL1
 * FINDHIL2: RTS
 */


/***** insrtlin() *****/
void  insrtlin(unsigned char  *ptr)
{
	openspc(tknbuf[2], ptr);		// go open space in the program bufer
	if (errcode) return;			// return if out of memory
	putline(ptr);					// put line into buffer
	return;
}

/*
 * 
 * INSRTLIN: EQU    *
 *           PSHX
 *           LDX       TKNBUFS
 *           LDAB   2,X
 *           PULX
 *           PSHX
 *           BSR    OPENSPC
 *           PULX
 *           BRA    PUTLINE
 */



/***** openspc() *****/				// open space in program buffer
void  openspc(int  bytes, unsigned char  *ptr)
{
	unsigned char				*to;
	unsigned char				*from;

	from = basend;					// set source at end of buffer
	to = basend + bytes;			// set destination "bytes" beyond source
	if (to > basmend)				// if out of memory, return an error
	{ 
		errcode = OMEMERR;
		return;
	}
	basend = to;					// set new end of buffer
	while (from != ptr-1)			// open up area in buffer
	{
		*to-- = *from--;
	}
	return;
}

/*
 * 
 * OPENSPC:  EQU    *
 *           PSHY
 *           PSHX
 *           LDX    VAREND
 *           ABX
 *           CPX    BASMEND
 *           BHI    OPENSPC4
 *           JSR    MOVEVARSUP
 *           LDX    BASEND
 *           PSHX
 *           ABX
 *           PSHX
 *           TSY
 *           LDD    0,Y
 * OPENSPC1: STD    BASEND
 * OPENSPC3: LDD    2,Y
 *           CPD    4,Y
 *           BLO    OPENSPC2
 *           LDX    2,Y
 *           LDAA   0,X
 *           DEX
 *           STX    2,Y
 *           LDX    0,Y
 *           STAA   0,X
 *           DEX
 *           STX    0,Y
 *           BRA    OPENSPC3
 * OPENSPC4: LDAA   #OMEMERR
 *           JMP    RPTERR
 * OPENSPC2: PULX
 *           PULX
 *           PULX
 *           PULY
 *           RTS
 */



/***** putline() *****/				// move line from token buffer to program buffer
void  putline(unsigned char  *cptr)
{
	int				count;

	count = tknbuf[2];				// get length of line in token buffer
	tbufptr = tknbuf;				// point to start of token buffer
	while (count)
	{
		*cptr++ = *tbufptr++;		// move a byte
		--count;					// decrement the byte count
	}
	return;
}

/*
 * 
 * PUTLINE:  EQU    *
 *           PSHX
 *           LDX    TKNBUFS
 *           LDAB   2,X
 *           PULX
 *           LDY    TKNBUFS
 * PUTLINE1: LDAA   0,Y
 *           INY
 *           STAA   0,X
 *           INX
 *           DECB
 *           BNE    PUTLINE1
 *           RTS
 */



/***** apendlin() *****/			// append line to end of program buffer
void  apendlin(void)
{
	if ((basend + tknbuf[2]) <= basmend)		// do we have enough memory left?
	{
		putline(basend);			// move the line
		basend += tknbuf[2];		// set the new end of basic pointer
//		outdeci((U32)basend);		// debug
	}
	else errcode = OMEMERR;			// not enough memory, error
	return;
}

/*
 * 
 * APENDLIN: EQU    *
 *           LDX    TKNBUFS
 *           LDAB   2,X
 *           LDX    VAREND
 *           ABX
 *           CPX    BASMEND
 *           BHI    APENDLN1
 * *         LDAB   TKNBUF+2
 *           JSR    MOVEVARSUP
 *           LDX    BASEND
 *           ABX
 *           XGDX
 *           LDX    BASEND
 *           STD    BASEND
 *           BRA    PUTLINE
 * APENDLN1: LDAA   #OMEMERR
 *           JMP    RPTERR
 */



/***** repline() *****/				// replace line in buffer
void  repline(unsigned char  *ptr)
{
	int					lendif;		// holds size (in bytes) of difference in two lines
	int					temp1;
	int					temp2;

	temp1 = *(ptr+2);				// convert type from char to int
	temp2 = (tknbuf[2]);
	lendif = temp1 - temp2;			// get the difference in line length
	if (lendif == 0)				// if the same, just write over the old
	{
		putline(ptr);
	}

/* 
 * REPLINE:  EQU    *
 *           LDAB   2,X
 *           PSHX
 *           LDX   TKNBUFS
 *           SUBB   2,X
 *           PULX
 *           BNE    REPLINE1
 *           BRA    PUTLINE
 */

	else if (lendif < 0)			// if line in tknbuf is larger
	{
		lendif = -lendif;			// make it a positive number
		openspc(lendif, ptr);		// try to open up a space
		if (errcode) return;		// if not enough memory, return
		putline(ptr);				// if ok, copy line to program buffer
	}

/* 
 * REPLINE1: EQU    *
 *           BPL    REPLINE2
 *           NEGB
 *           PSHX
 *           JSR    OPENSPC
 *           PULX
 *           BRA    PUTLINE
 */

	else							// if line in tknbuf is smaller
	{
		closespc(lendif, ptr);		// close up excess space
		putline(ptr);				// put new line in program buffer
	}
	return;
}


/* 
 * REPLINE2: EQU    *
 *           PSHX
 *           JSR    CLOSESPC
 *           PULX
 *           BRA    PUTLINE
 * *
 * *
 * MoveVarsUp:
 *           PSHY                ; SAVE THE Y REGISTER.
 *           PSHB                ; SAVE THE BYTE COUNT.
 *           LDX       VAREND    ; POINT TO THE END OF THE VARIABLE MEMORY SPACE.
 *           LDY       VAREND    ; POINT TO THE END OF VARIABLE MEMORY SPACE.
 *           ABX                 ; ADD THE NUMBER OF BYTES TO MOVE TO THE POINTER.
 *           LDD       VAREND    ; GET THE CURRENT VARIABLE TABLE ENDING ADDRESS.
 *           STX       VAREND    ; SAVE THE NEW END OF VARIABLE TABLE POINTER.
 *           SUBD      VARBEGIN  ; CALCULATE THE NUMBER OF BYTES TO MOVE.
 *           BEQ       MOVEUP2   ; JUST RETURN IF THERE IS NOTHING TO MOVE.
 *           std       VarSize   ; save the size of the variable table (9/12/89).
 * MOVEUP1:  LDAA      0,Y       ; GET A BYTE.
 *           STAA      0,X       ; MOVE IT.
 *           DEX
 *           DEY
 *           bsr       DecCount  ; DECREMENT THE BYTE COUNT. ARE WE DONE? (9/12/89).
 *           BPL       MOVEUP1   ; GO TILL WE'RE DONE.
 *           INX                 ; ADJUST THE POINTER
 * MOVEUP2:  STX       VARBEGIN  ; SAVE THE NEW START OF VARIABLE TABLE POINTER.
 *           PULB                ; RESTORE THE BYTE COUNT.
 *           PULY                ; RESTORE Y.
 *           RTS                 ; RETURN.
 * *
 * *
 * MoveVarsDn:
 *           PSHY                ; SAVE Y.
 *           PSHB                ; SAVE THE BYTE COUNT.
 *           LDY       VARBEGIN  ; POINT TO THE CURRENT START OF THE VARIABLE TABLE.
 *           LDAA      #$FF      ; MAKE THE BYTE COUNT NEGATIVE SO WE CAN JUST ADD.
 *           NEGB                          
 *           ADDD      VARBEGIN  ; CALCULATE THE NEW START OF THE VARIABLE TABLE.
 *           XGDX                ; PUT THE NEW STARTING ADDRESS OF THE VARIABLE TABLE INTO X.
 *           LDD       VAREND    ; GET THE OLD TABLE ENDING ADDRESS.
 *           SUBD      VARBEGIN  ; SUBTRACT THE OLD TABLE STARTING ADDRESS TO GET THE SIZE OF THE TABLE.
 *           STX       VARBEGIN  ; SAVE THE POINTER TO THE NEW START OF THE VARIABLE TABLE.
 *           std       VarSize   ; save the size of the variable table (9/12/89).
 *           BEQ       MOVEDN2   ; IF THE SIZE IS 0 (NO VARIABLES ALLOCATED) EXIT.
 * MOVEDN1:  LDAA      0,Y       ; GET A BYTE.
 *           STAA      0,X       ; MOVE IT.
 *           INX                 ; MOVE THE DESTINATION POINTER.
 *           INY                 ; MOVE THE SOURCE POINTER.
 *           bsr       DecCount  ; DECREMENT THE BYTE COUNT. ARE WE DONE? (9/12/89).
 *           BPL       MOVEDN1   ; NO. KEEP MOVIN' THEM BYTES.
 *           DEX
 * MOVEDN2:  STX       VAREND    ; SAVE THE NEW POINTER TO THE END OF THE VARIABLE TABLE.
 *           PULB                ; RESTORE THE BYTE COUNT.
 *           PULY                ; RESTORE Y.
 *           RTS                 ; RETURN.
 * *
 * *
 * DecCount:
 *           ldd       VarSize   ; get the size of the variable table.
 *           subd      #1        ; decrement it.
 *           std       VarSize   ; save the new value.
 *           rts                 ; return.
 */
