/*
 *  runtime5.c      Run-time routines for the SKBasic Embedded
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"
#include  "basictime.h"

#if defined(SKBASIC_EMBEDDED)

#include	"uart_interface.h"
#include 	"hardware.h"

extern SK_UW gnWaitDuration;

static	int		findlabelline(U8 *label)
{
	U16								ret = 0;
	unsigned char					*nextlnptr;
	unsigned char					*srchptr;

	srchptr = basbeg;				// start the search at the next line

	while (srchptr < basend)
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
			if(memcmp(label, nextlnptr, *(label + 1)) == 0){
				ret = getU16((U16 *)srchptr);		// get next line number
			}
		}
		srchptr = srchptr + *(srchptr+2);	// move to next line
	}
	return(ret);
}

void			rtim1(void)
{
	int			tok;

	rskipspc();
	if(*tbufptr != GOSUBTOK){
		goto fail;
	}
	tbufptr++;
	rskipspc();
	if(*tbufptr == LCONTOK){
		tbufptr++;
		ontim1lin = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);
		
		return;
	}else if(*tbufptr == SCONTOK){
		ontim1lin = findlabelline(tbufptr);
		tbufptr += *(tbufptr + 1) + 2;
		if(ontim1lin == 0){
			errcode = LNFERR;
			rpterr();
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rtim2(void)
{
	int			tok;

	rskipspc();
	if(*tbufptr != GOSUBTOK){
		goto fail;
	}
	tbufptr++;
	rskipspc();
	if(*tbufptr == LCONTOK){
		tbufptr++;
		ontim2lin = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);
		
		return;
	}else if(*tbufptr == SCONTOK){
		ontim2lin = findlabelline(tbufptr);
		tbufptr += *(tbufptr + 1) + 2;
		if(ontim2lin == 0){
			errcode = LNFERR;
			rpterr();
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rwait_set(void)
{

	rskipspc();
	donexp();							// calc address
	gnWaitDuration = pull32(&numstack, STKUNDER);
	if(*tbufptr == SKARGTOK){
		tbufptr++;
	}
	return;
}

void			rwait_jne(void)
{
	if(gnWaitDuration > 0){
		--tbufptr;
	}
}

#endif
