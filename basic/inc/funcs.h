/*
 *  funcs.h      include file for common function declarations
 */

#ifndef  FUNCS_H
#define  FUNCS_H

#ifdef __cplusplus
extern "C" {
#endif

void			basic_main(void);			// main entry point, declared in basiclb1.c

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
void			parse_cmdline(void);
#endif
#if	defined(CPUSTM32)
void			parse_cmdline(void);
#endif

void			initvars(void);
void			initstacks(void);


int				match(char  *lit);
void			blanks(void);
void			puttok(unsigned char  b);
void			getline(void);
void			putcommandline(char *line);

void			putint32(I32  itok);
void			putint16(I16  ival);
void			putlinum(U16  lnum);

int				streq(unsigned char  *str1, unsigned char  *str2);
U16				getlinum(void);
void			xlate(void);
void			nl(void);
void			pl(char  *ptr);
void			pl_P(FARPTR char  *ptr);		// same as pl(), but string is stored in non-volatile memory
void			skipspcs(void);
unsigned char	getchr(void);
void			incibp(void);
int				match(char  *lit);
void			innumd(void);
I32				indeci(void);
int				rinrdc(void);

void			xmideol(void);
void			xdim(void);
void			xgo(char  gotok);
void			xgosub(void);
void			xgoto(void);
void			xreturn(void);
void			xstop(void);
void			xend(void);
void			xtron(void);
void			xtroff(void);
void			xrem(void);
void			xqrem(void);
void			xdata(void);
void			xlet(void);
void			ximplet(void);
void			xon(void);
void			xif(void);
void			xfor(void);
void			xnext(void);
void			xprint(void);
void			xqprint(void);
void			xinput(void);
void			xinbyte(void);
void			inreadcm(void);
void			xread(void);
void			xrestore(void);
void			xwhile(void);
void			xendwh(void);
void			xlabel(void);
void			xsleep(void);
void			rpterr(void);
void			outdeci(I32  num);
void			outstr(unsigned char  *str, int  len);
void			DescribeError(unsigned char  errnum);


U8				getfun(void);
int				getcon(void);
//int			xfdiv(void);
int				xchrs(void);
int				xabs(void);
int				xrnd(void);
int				xsgn(void);
int				xtab(void);
//int			xadc(void);
U8				fxcall(void);				// the function version of CALL
void			xcall(void);				// statement version of CALL
U8				xhex(void);
U8				xhex2(void);
U8				xhex4(void);
U8				xpeek(void);
U8				xpeek16(void);
U8				xpeek32(void);
void			xeep(void);
void			xeep16(void);
void			xeep32(void);
U8				xfeep(void);
U8				xfeep16(void);
U8				xfeep32(void);
U8				xaddr(void);
int				xsqrt(void);
U8				fxtimestr(void);			// the function version of TIME$

void			xstrcat(void);
U8				xfstrcmp(void);
void			xstrind(void);
U8				xfstrind(void);
U8				xfstrlen(void);

void			xtimestr(void);				// the statement version of TIME$
void			xdatestr(void);				// the statement version of DATE$

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
void			xsksreg(void);
int				xsksend(void);
int				xskflash(void);
int				xskbc(void);
int				xsksync(void);
int				xskinq(void);
int				xskpair(void);
int				xskunpair(void);
void			xsknow(void);
void			xsksleep(void);
void			xsksetps(void);
void			xskclrtbl(void);
void			xskclrcache(void);
int				xsklkup(void);
int				xskrevlkup(void);
void			xsksetname(void);
void			xsksetkey(void);
void			xwait(void);
#elif	defined(SKBASIC_EMBEDDED)

#endif

U8				unumfun(unsigned char  token);
int				emptyfunc(unsigned char  token);
void			dofunct(unsigned char  functok, int  nargs, unsigned char  *type);


void			letcom(char  letok);
void			asignmt(void);
int				xexpres(U8  type);
char			cknumop(void);
char			ckbolop(void);
char			cklogop(void);
int				alphanum(char  c);
int				chcktyp(void);
int				getvar(void);
void			clrvar(unsigned char  **varptr);
int				findvar(unsigned char  vartype, unsigned char  *varname);
int				putvar(unsigned char  vartype, unsigned char  *varname);
I32				getdeci(void);
U32				gethex(void);
void			getscon(void);
int				hexdig(char  c);
int				a2hex(char c);
int				chckcmds(void);
void			clistprog(int firstlin, int lastlin);
void			clist(void);
void			cmd_time(void);
void			cdate(void);
void			lvarcon(void);
void			lfvar(void);
void			livar(void);
void			lsvar(void);
void			lpvar(void);			// print port name to console
void			lfcon(void);
void			licon(void);
void			lscon(void);
void			llcon(void);
void			lbcon(void);
void			lkeyword(void);
unsigned char	*findline(int  linenum);
int				findhlin(void);
void			cautost(void);
//void			cnoauto(void);
void			autoload(void);
void			bsc_cmd_free(void);
void			cdump(void);
void			crun(void);
void			_crun(void);
void			cnew(void);
void			ccont(void);
void			cclear(void);
void			cllist(void);
void			apendlin(void);
void			insrtlin(unsigned char  *ptr);
void			repline(unsigned char  *ptr);
void			closespc(int  bytes, unsigned char  *ptr);
void			openspc(int  bytes, unsigned char  *ptr);
void			putline(unsigned char  *cptr);
void			runline(void);
void			rskipspc(void);

void			_xprint(unsigned char  tok);

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
void			getbcon(void);
void			lbcon(void);
void			cskinfo(void);
void			cskreset(void);
void			csksave(void);
void			cskload(void);
void			cskrfctrl(void);
void			cskrfreg(void);
void			cskver(void);
void			cskpow(void);
void			csktable(void);
#endif
#if	defined(SKBASIC_CMD) || defined(SKBASIC_EMBEDDED)
void			cexit(void);
#endif

/*
 *  Time-related functions
 */
void			tmtotimestr(struct tm  *pt, char  *s);	// convert broken time (tm struct) to time string
void			tmtodatestr(struct tm  *pt, char  *s);	// convert broken time (tm struct) to date string






/*
 *  Target-specific routines (targetxxx.c)
 */
void			outbyte(U8  c);
void  			outbyte_xlate(unsigned char  c);
unsigned char	inbyte(void);
void			iodevinit(void);
void			targetgets(unsigned char  *ptr);
int				getioport(void);				// added to test for I/O port assignment or use
int				getconst(I32  *pval);			// added to support target-specific constants, such as F_CPU
void			targetinitvars(void);
void			targetwriteport(U16  index, I32  val);
I32				targetreadport(U16  index);
U32				targetgetportaddr(U16  index);
void			targetgetportname(U16  index, char  *namestr);
void			targetmarkautostart(int  flag);
U8				targetreadautostart(void);
int				targetcheckautorun(void);
U32				*targetgetvarptr(U16  addr);
void			targetdisplayheader(void);		// display info about the target firmware
void			csave(void);
void			cload(void);
void			cfiles(void);
void			cformat(void);
void			cdelete(void);
void			targetgetdynmeminfo(U8 **start, U16  *numbytes);
//void			cflsave(void);
//void			cflload(void);
void			targetwriteeeprom(U32  addr, U8  val);		// writes one byte to EEPROM
void			targetwriteeeprom16(U32  addr, U16  val);	// writes one word to EEPROM
void			targetwriteeeprom32(U32  addr, U32  val);	// writes one double word to EEPROM
U32				targetreadeeprom(U32  addr);				// reads one byte from EEPROM, returns as U32
U32				targetreadeeprom16(U32  addr);				// reads one word from EEPROM, returns as U32
U32				targetreadeeprom32(U32  addr);				// reads one double word from EEPROM, returns as U32
U32				targetgetsystime(TM  *pt);					// returns broken time in struct pointed to by pt
void			targetsetsystime(TM  *pt);					// sets system time from broken time
U32				targetgetsysdate(TM  *pt);					// returns broken date in struct pointed to by pt
void			targetsetsysdate(TM  *pt);					// sets system date from broken date
void			targetruntask(void);

/*
 *  This function originally returned a pointer to a U8.  That doesn't work in the
 *  AVR world, since a pointer is not 32 bits.  This routine has been changed to
 *  return a U32; the caller can deal with casting.
 */
U32				targetgetflashprogramaddr(U8  filenum);		// returns addr of program in selected flash file or 0 if no such file
void			targetcopyfromflash(U8 *buff, U32  fladdr, U16  num);

void			runline(void);
unsigned char	chkbrk(void);
void			runinit(void);
void			delline(unsigned int  num);
void			storlin(void);


/*
 *  Run-time function declarations
 */
void			rlet(void);
void			rprint(void);
void			rfor(void);
void			rnext(void);
void			rtron(void);
void			rtroff(void);
void			rpoke(void);
void			rdim(void);
void			rrem(void);
void			rpacc(void);
void			rdata(void);
void			rread(void);
void			rrestor(void);
//void			rgosub(void);
void			rpreparegosub(void);
void			rinterrupt(unsigned int targetln);
void			rgoto(void);
//void			rpreparegoto(void);
void			processgoto(unsigned int targetln);
void			ron(void);
void			rreturn(void);
void			rif(void);
void			rinput(void);
void			rstop(void);
void			rend(void);
void			rwhile(void);
void			rendwh(void);
void			rlabel(void);
void			reep(void);
void			reep16(void);
void			reep32(void);
void			rporta(void);
void			rportb(void);
void			rportc(void);
void			rportd(void);
void			rinbyte(void);
void			rtime(void);
void			rontime(void);
void			ronirq(void);
void			rreti(void);
void			ronpacc(void);
void			rsleep(void);
void			rrtime(void);
void			rtab(void);
void			rchrs(void);
void			rhex2(void);
void			rhex4(void);
void			rhex(void);
void			outhexbyte(U8  c);

void			rabs(void);
void			reor(void);
void			rorv(void);
void			r_and(void);			// avoid conflict with C' rand() function
void			rplus(void);
void			rminus(void);
void			rdiv(void);
void			rmod(void);
void			rmult(void);
void			rindir(void);
void			rindir32(void);
void			rindir16(void);
void			rnot(void);
void			rneg(void);
void			rlt(void);
void			rgt(void);
void			rlteq(void);
void			rgteq(void);
void			req(void);
void			rnoteq(void);
void			rnop(void);
void  			rrnd(void);
void			rsgn(void);
void			rsqrt(void);			// 32-bit integer square root
void			rpwr(void);				// raise a 32-bit integer to a power (both signed)

void			rpeek(void);
void			rpeek16(void);
void			rpeek32(void);

void			rfeep(void);
void			rfeep16(void);
void			rfeep32(void);

void			rstrcat(void);
void			rstrind(void);
void			rfstrcmp(void);
void			rfstrind(void);
void			rfstrlen(void);

//void			rtimestr(void);
//void			rdatestr(void);

void			raddr(void);

//void			rcall(void);

#if	defined(SKBASIC_IMPL) || defined(SKBASIC_CMD)
void			fsksend(int mode_function);
void			fskflash(int mode_function);
void			fskbc(int mode_function);
void			fsksync(int mode_function);
void			fskinq(int mode_function);
void			fskpair(int mode_function);
void			fskunpair(int mode_function);
void			fsklkup(int mode_function);
void			fskrevlkup(int mode_function);

void			rsksreg(void);
void			rsksend(void);
void			rskflash(void);
void			rskbc(void);
void			rsksync(void);
void			rskinq(void);
void			rskpair(void);
void			rskunpair(void);
void			rsknow(void);
void			rsksleep(void);
void			rsksetps(void);
void			rskclrtbl(void);
void			rskclrcache(void);
void			rsklkup(void);
void			rskrevlkup(void);
void			rsksetname(void);
void			rsksetkey(void);
void			rrxdata(void);
void			rack(void);
void			revent(void);
void			rsetvar(unsigned char vartype, char *varname, unsigned long value);
void			rsetstr(char *varname, U8 *data, int length);
void			rwait_set(void);
void			rwait_jne(void);
#elif defined(SKBASIC_EMBEDDED)
void			rtim1(void);
void			rtim2(void);
#endif

void			printonly(void);
void			pfuncom(void);

void			StartNextLine(void);

void			pshnum(unsigned char  tok);
void			pshaddr(unsigned char  tok);
unsigned int	pulnum(void);
void			pshop(unsigned char  op);
unsigned char	chckee(unsigned char  tok);
unsigned char	chknfun(unsigned char  tok);
void			donexp(void);
void			doop(unsigned char  op);

U16				rvarptr(void);
void			rstrset(U8 *dststrptr, U8 *srcstrptr);

#if defined(CPU78K0R) || defined(CPUML7416) || defined(CPUSTM32)
U16				getU16(U16 *p);
void			setU16(U16 *p, U16 v);
U32				getU32(U32 *p);
void			setU32(U32 *p, U32 v);
int				strlen_P(FARPTR char *p);
void			strcpy_P(char *dst, FARPTR char *src);
#else
#define	getU16(a)	*(a)
#define	setU16(a, b)	(*(a) = (b))
#define	getU32(a)	*(a)
#define	setU32(a, b)	(*(a) = (b))
#endif

#if defined(CPU78K0R)
#include "hardware.h"
#include "d2/debug.h"
#include "uart_interface.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
