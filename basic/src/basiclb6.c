/*
 *  basiclb6      part of the SKBasic compiler
 */

#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>


#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)

void			xsksreg(void)
{
	*tbufptr++ = SKSREGTOK;
	return;
}

int				xsksend(void)
{
	int		type;

	*tbufptr++ = SKSENDTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a ack expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a selector expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a address expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	type = xexpres(0);					// go get a data_len or data
	if(type == NUM){
		blanks();							// skip blanks
		*tbufptr++ = SKARGTOK;
		if (errcode) goto exit;				// if error, return
		type = xexpres(0);					// go get a data expression
		if(type == NUM){
			errcode = DTMISERR;
		}

		if (errcode) goto exit;				// if error, return
	}
exit:
	forflag = FALSE;
	return(NUM);
}

int				xskflash(void)
{
	int		type;

	*tbufptr++ = SKFLASHTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a selector expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a address expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	type = xexpres(0);					// go get a data_len or data
	if(type == NUM){
		blanks();							// skip blanks
		*tbufptr++ = SKARGTOK;
		if (errcode) goto exit;				// if error, return
		type = xexpres(0);					// go get a data expression
		if(type == NUM){
			errcode = DTMISERR;
		}

		if (errcode) goto exit;				// if error, return
	}
exit:
	forflag = FALSE;
	return(NUM);
}

int				xskbc(void)
{
	int		type;

	*tbufptr++ = SKBCTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a radius expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a selector expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	type = xexpres(0);					// go get a data_len or data
	if(type == NUM){
		blanks();							// skip blanks
		*tbufptr++ = SKARGTOK;
		if (errcode) goto exit;				// if error, return
		type = xexpres(0);					// go get a data expression
		if(type == NUM){
			errcode = DTMISERR;
		}

		if (errcode) goto exit;				// if error, return
	}
exit:
	forflag = FALSE;
	return(NUM);
}

int				xsksync(void)
{
	*tbufptr++ = SKSYNCTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a radius expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a address expression
	if (errcode) goto exit;				// if error, return
exit:
	forflag = FALSE;
	return(NUM);
}

int				xskinq(void)
{
	*tbufptr++ = SKINQTOK;
	return(NUM);
}

int				xskpair(void)
{
	*tbufptr++ = SKPAIRTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a address expression
	if (errcode) goto exit;				// if error, return
exit:
	forflag = FALSE;
	return(NUM);
}

int				xskunpair(void)
{
	*tbufptr++ = SKUNPAIRTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a address expression
	if (errcode) goto exit;				// if error, return
exit:
	forflag = FALSE;
	return(NUM);
}

void			xsknow(void)
{
	*tbufptr++ = SKNOWTOK;
	return;
}

void			xsksleep(void)
{
	*tbufptr++ = SKSLEEPTOK;
	return;
}

void			xsksetps(void)
{
	*tbufptr++ = SKSETPSTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a sleep expression
	*tbufptr++ = SKARGTOK;
	if (errcode) goto exit;				// if error, return
	blanks();							// skip blanks
	xexpres(NUM);						// go get a wake expression
	if (errcode) goto exit;				// if error, return
exit:
	forflag = FALSE;
	return;
}

void			xskclrtbl(void)
{
	*tbufptr++ = SKCLRTBLTOK;
	return;
}

void			xskclrcache(void)
{
	*tbufptr++ = SKCLRCACHETOK;
	return;
}

int				xsklkup(void)
{
	int		type;

	*tbufptr++ = SKLKUPTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	type = xexpres(0);					// go get a data_len or data
	if(type == NUM){
		blanks();							// skip blanks
		*tbufptr++ = SKARGTOK;
		if (errcode) goto exit;				// if error, return
		type = xexpres(0);					// go get a data expression
		if(type == NUM){
			errcode = DTMISERR;
		}

		if (errcode) goto exit;				// if error, return
	}
exit:
	forflag = FALSE;
	return(NUM);
}

int				xskrevlkup(void)
{
	*tbufptr++ = SKREVLKUPTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a numerical expression
	forflag = FALSE;
	return(NUM);
}

void			xsksetname(void)
{
	int		type;

	*tbufptr++ = SKSETNAMETOK;
	forflag = TRUE;
	blanks();							// skip blanks
	type = xexpres(0);					// go get a data_len or data
	if(type == NUM){
		blanks();							// skip blanks
		*tbufptr++ = SKARGTOK;
		if (errcode) goto exit;				// if error, return
		type = xexpres(0);					// go get a data expression
		if(type == NUM){
			errcode = DTMISERR;
		}

		if (errcode) goto exit;				// if error, return
	}
exit:
	forflag = FALSE;
	return;
}

void			xwait(void)
{
	*tbufptr++ = WAITSETTOK;
	forflag = TRUE;
	blanks();							// skip blanks
	xexpres(NUM);						// go get a numerical expression
	*tbufptr++ = SKARGTOK;
	*tbufptr++ = WAITJNETOK;
	forflag = FALSE;
	return;
}

#ifdef USE_SECURITY
void			xsksetkey(void)
{
	int		type;

	*tbufptr++ = SKSETKEYTOK;
	forflag = TRUE;
	type = xexpres(0);					// go get a data expression
	if(type == NUM){
		errcode = DTMISERR;
	}
	forflag = FALSE;
	return;
}
#endif


#endif
