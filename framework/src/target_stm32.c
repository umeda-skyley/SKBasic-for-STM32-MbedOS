#include  <stdio.h>
#include  <ctype.h>
#include  <string.h>

#define	OWNER
#include "basictime.h"
#include "defines.h"
#include "funcs.h"
#include "stacks.h"

#include "command.h"
#include "hardware.h"
#include "skyley_base.h"
#include "uart_interface.h"

#define	SKBASIC_DEBUG	1

#define  MAX_VARRAM			2048
#define  MAX_PRGRAM			8192
#define  MAX_DYN_MEM		1000
#define  MAX_EEPROM       	4096		/* total amount of EEPROM on device */

unsigned char				variableram[MAX_VARRAM];
unsigned char				prgramram[MAX_PRGRAM];
unsigned char				dynmem[MAX_DYN_MEM];	// dynamic storage for arrays, see strastg

#define  TARGET_VERSION		"0.9"
#define  TARGET_INFO		"SKBasic for STM32/Mbed5"

static char save_flag = 0;
static SK_UH save_bytes = 0;
SK_UB gnLED1; //led1
SK_UB gnLED2; //led2
extern SK_UW gnTim1Interval; //tim1_interval
extern SK_UW gnTim2Interval; //tim2_interval

typedef struct
{
	char					name[12];					// name of I/O port
	void					*addr;						// address of I/O port
	char					size;						// number of bytes in port
	char					div;
	char					reg_id;
	I32						min;
	I32						max;
}  IOPORT;

static const IOPORT	porttable[]  =
{
//    name            addr								  size
//----------------------------------------
	{"countdown",	&gnWaitDuration,							4, 1,	0,	1,	0x7fffffff},
	{"led1",		&gnLED1,									1, 1,	1,	0,	1},
	{"led2",		&gnLED2,									1, 1,	2,	0,	1},
	{"timer1",		&gnTim1Interval,							4, 1,	3,	0,	0x7FFFFFFF},
	{"timer2",		&gnTim2Interval,							4, 1,	4,	0,	0x7FFFFFFF},
	// Null entry, marks end of table
	{"",			0,											0, 0, 	0, 	0,	0}		// last entry MUST be blank!
};

static	void		targetformatflash(void);
static	void		targetlistdirentryentry(void);
static	void		targetloadfile(char *filename);
static	void		targetdeletefile(char *filename);
static	void		targetsavefile(char *filename);
static	U32			count_free_bytes(void);
static	void		flash_buffer_putc(char c);
static	FARPTR U8	*find_free_entry(void);
static	FARPTR U8	*find_file_entry(char *filename);

int				strlen_P(char *p)
{
	int		ret = 0;
	while(*p++ != 0){
		ret++;
	}
	return(ret);
}

void			strcpy_P(char *dst, FARPTR char *src)
{
	while(*src){
		*dst++ = *src++;
	}
	*dst++ = 0;
}

void			nl(void)
{
	if(save_flag == 0){
		_print("\r\n");
	}else if(save_flag == 1){
		save_bytes += 1;
	}else{
		flash_buffer_putc(0x0d);
	}
}

void			pl(char  *ptr)
{
	unsigned char				c;

	while ((c = *ptr++))					// assignment, not test!
	{
		outbyte_xlate(c);					// output byte with ESCAPE translation
	}
}

void			pl_P(FARPTR char  *ptr)		// same as pl(), but string is stored in non-volatile memory
{
	unsigned char				c;

	while ((c = *ptr++))					// assignment, not test!
	{
		outbyte_xlate(c);					// output byte with ESCAPE translation
	}
}

void			outbyte(U8  c)
{
	if(save_flag == 0){
		SK_putc(c);
	}else if(save_flag == 1){
		save_bytes += 1;
	}else{
		flash_buffer_putc(c);
	}
}

void  			outbyte_xlate(unsigned char  c)
{
	static char				esc = FALSE;

   	if (c)									// don't do nulls
   	{
		if (esc)							// if escape sequence is active...
		{
			switch  (c)						// need to escape the character
			{
				case  'e':					// \e = ESCAPE
				c = 0x1b;					// get the ASCII ESCAPE char
				break;

				case  '\\':					// 0x3c = single backslash
				break;

				case  't':					// \t = tab char
				c = 0x09;					// get the TAB char
				break;

				case  'b':					// \b = BKSP char
				c = 0x08;					// get the BKSP char
				break;

				case  'a':					// \a = ALARM (bell) char
				c = 0x07;					// get the BELL char
				break;

				case  'n':					// \n = newline
				c = 0x0a;					// get LF char
				break;

				case  'r':					// \r = CR
				c = 0x0d;					// get CR char
				break;

				case  'f':					// \f = formfeed
				c = 0x0c;					// get FF char
				break;

				default:					// no clue, discard the '\' char
				break;
			}
			esc = FALSE;					// always clears the ESC code
	    	outbyte(c);						// send translated char
		}
		else if (c == '\\')					// if just got backslash...
		{
			esc = TRUE;
		}
		else
		{
	    	outbyte(c);						// send actual char
		}
	}
}

unsigned char	inbyte(void)
{
	return(SK_getc());
}

void			iodevinit(void)
{
	save_flag = 0;
}

#define 	LINE_BUFFER_SIZE				255
static SK_UB		gn_aLineBuf[LINE_BUFFER_SIZE];
static SK_UW		gn_nLinePos = 0;
extern SK_BOOL		gbEchoBack;

void			targetgets(unsigned char  *ptr)
{
	gn_nLinePos = 0;
	while(1){
		SK_H	in = SK_getc();
		if (in >= 0x20) {
			if (gn_nLinePos < (LINE_BUFFER_SIZE-1)) {
				gn_aLineBuf[gn_nLinePos++]	= (SK_UB)in;
				gn_aLineBuf[gn_nLinePos]	= 0;
				//outbyte((UInt8)in);
				if( gbEchoBack == 1 )outbyte((SK_UB)in);
			}
			in = 0;
		} else {
			if(in == 0x03){
				breakflag = 1;
				break;
			} else if (in == 13) {
				gn_aLineBuf[gn_nLinePos]	= 0;
				//outbyte("\r\n");
				if( gbEchoBack == 1 )pl(PSTR("\r\n"));
				in = 1;
				strcpy((char *)ptr, (char *)gn_aLineBuf);
				gn_nLinePos = 0;
				break;
			} else {
				if ((in == 8) || (in == 127)) {
					if (gn_nLinePos > 0) {
						//_putc(8);
						//_putc(' ');
						//_putc(8);
						if( gbEchoBack == 1 )outbyte(8);
						if( gbEchoBack == 1 )outbyte(' ');
						if( gbEchoBack == 1 )outbyte(8);
						gn_nLinePos--;
						gn_aLineBuf[gn_nLinePos]	= 0;
					}
				}else{
					targetruntask();
				}
				in = 0;
			}
		}
	}
}

int				getioport(void)				// added to test for I/O port assignment or use
{
	unsigned char			n;						// index into name buffer
	char					foundit;				// true if found name in table
	char					tname[MAX_VAR_NAME_LEN+1];	// temp copy for the name search

	foundit = 0;									// show haven't found name yet
	for (n=0; ; n++)								// for all entries in port name table...
	{
		if (strlen_P((FARPTR char *)porttable[n].name) == 0)  break;	// if hit the end, outta here
		strcpy_P(tname, (FARPTR char *)porttable[n].name);			// get a RAM copy of the name
		if (match(tname))							// if found the desired port name...
		{
			foundit = 1;							// show we found the name
			break;									// stop looking
		}
	}
	if (!foundit)  return 0;						// if no luck with name, all done
	*tbufptr++ = PVARTOK;							// show we found a port name
	putint16(n);									// save index into port table after token
	return  NUM;									// show all is well
}

int				getconst(I32  *pval)		// added to support target-specific constants, such as F_CPU
{
	return(0);
}

void			targetinitvars(void)
{
	unsigned char				*x;

	varbegin = variableram;									// point to start of variable RAM
	varend = variableram;									// show no variables defined yet
	varmend = &variableram[MAX_VARRAM-1];					// point to end of variable RAM
	for (x=varbegin; x<=varmend; x++) *x=0;				// zero all of variable memory

	basbeg = prgramram;									// point to start of program RAM
	basend = prgramram;									// show no program stored yet
	basmend = &prgramram[MAX_PRGRAM-1];					// point to end of program RAM
}

void			targetwriteport(U16  index, I32  val)
{
	void	*p = porttable[index].addr;
	int		size = porttable[index].size;
	int		div = porttable[index].div;
	int		reg_id = porttable[index].reg_id;
	I32		min = porttable[index].min;
	I32		max = porttable[index].max;
	if(p){
		if((val >= min) && (val <= max) && (reg_id >= 0)){
			switch(size){
				case 1:	*((U8 *)p) = (U8)(val * div);	break;
				case 2:	*((U16 *)p) = (U16)(val * div);	break;
				case 4:	*((U32 *)p) = (U32)(val * div);	break;
			}

			//post process
			if(reg_id == 1){
				//*p == gnLED1
				LedControl(1, gnLED1);
			} if(reg_id == 2){
				//*p == gnLED2
				LedControl(2, gnLED2);
			}
		}else{
			//errcode = INVARGERR;
			errcode = OUTOFRANGEERR;
		}
	}
}

I32				targetreadport(U16  index)
{
	U32		ret = 0;
	void	*p = porttable[index].addr;
	int		size = porttable[index].size;
	int		div = porttable[index].div;
	int		reg_id = porttable[index].reg_id;
	if(p){
		switch(size){
			case 1:	ret = *((U8 *)p) / div;		break;
			case 2:	ret = *((U16 *)p) / div;	break;
			case 4:	ret = *((U32 *)p) / div;	break;
		}
		//post process calculation sample
		switch(reg_id){
			case 1:
				//ret = ((U16)((U8 *)p)[0] << 8) | ((U8 *)p)[1];
				break;
		}
	}
	return(ret);
}

U32				targetgetportaddr(U16  index)
{
	return((U32)porttable[index].addr);
}

void			targetgetportname(U16  index, char  *namestr)
{
	strcpy_P(namestr, (FARPTR char *)porttable[index].name);
}

void			targetmarkautostart(int  flag)
{
#if SKBASIC_DEBUG
	_print("targetmarkautostart()\r\n");
#endif
}

U8				targetreadautostart(void)
{
	return(0);
}

int				targetcheckautorun(void)
{
	int		ret = 0;
	if(find_file_entry(PSTR("autorun.bas")) != NULL){
		targetloadfile(PSTR("autorun.bas"));
		ret = 1;
	}
	return(ret);
}

U32				*targetgetvarptr(U16  addr)
{
	return  (U32 *)(variableram + addr);
}

void			targetdisplayheader(void)		// display info about the target firmware
{
	pl_P(PSTR("\n\r"  TARGET_INFO  " v"  TARGET_VERSION));
}

static	int		check_filename(U8 *p)
{
	int		invalid = 0;
	char	c;
	while((c = *p++) != 0x0d){
		if((c == ' ') || (c == '"') || (c == '\'')){
			invalid++;
			break;
		}
	}
	return(invalid == 0);
}

static	int		check_filenamelen(U8 *p)
{
	int		length = 0;
	char	c;
	while((c = *p++) != 0x0d){
		length++;
	}
	return((length > 0) && (length <= MAX_FILE_NAME_LEN));
}

void			csave(void)
{
	skipspcs();
	if(basbeg != basend){
		save_bytes = 0;
		save_flag = 1;
		intptr = basbeg;
		lastlin = hiline;
		firstlin = getU16((U16 *)intptr);			// get first line number
		clistprog(firstlin, lastlin);
		save_flag = 0;
		if(find_file_entry((char *)ibufptr) == NULL){
			if(find_free_entry() != NULL){
				if(save_bytes < count_free_bytes()){ 
					if(check_filenamelen(ibufptr)){
						if(check_filename(ibufptr)){
							targetsavefile((char *)ibufptr);
						}else{
							pl_P(PSTR("Bad filename"));
						}
					}else{
						pl_P(PSTR("Filename too long"));
					}
				}else{
					pl_P(PSTR("No space"));
				}
			}else{
				pl_P(PSTR("No more entry"));
			}
		}else{
			pl_P(PSTR("File already exist"));
		}
	}else{
		pl_P(PSTR("No Program"));
	}
}

void			cload(void)
{
	skipspcs();
	if(isalpha(*ibufptr)){
		if(find_file_entry((char *)ibufptr) != NULL){
			targetloadfile((char *)ibufptr);
		}else{
			pl_P(PSTR("File not exist"));
			nl();
		}
	}else{
		pl_P(PSTR("No Filename"));
	}
}

void			cformat(void)
{
	unsigned char			tbuff[20];
	pl_P(PSTR("\n\rThis will erase the all programs!  Are you sure? "));
	targetgets(tbuff);
	if ((*tbuff == 'y') || (*tbuff == 'Y'))
	{
		targetformatflash();
		pl_P(PSTR("\n\rFlash erased."));
	}
}

void			cfiles(void)
{
	nl();
	targetlistdirentryentry();
	nl();
	outdeci(count_free_bytes());
	pl_P(PSTR(" bytes available."));
}

void			cdelete(void)
{
	skipspcs();
	if(isalpha(*ibufptr)){
		if(find_file_entry((char *)ibufptr) != NULL){
			targetdeletefile((char *)ibufptr);
		}else{
			pl_P(PSTR("File not exist"));
		}
	}else{
		pl_P(PSTR("No Filename"));
	}
}

void			targetgetdynmeminfo(U8 **start, U16  *numbytes)
{
	*start = dynmem;						// save addr of dynamic memory array
	*numbytes = MAX_DYN_MEM;				// save number of bytes in array
}

//void			cflsave(void);
//void			cflload(void);
void			targetwriteeeprom(U32  addr, U8  val)		// writes one byte to EEPROM
{
#if SKBASIC_DEBUG
	_print("targetwriteeeprom()\r\n");
#endif
}

void			targetwriteeeprom16(U32  addr, U16  val)	// writes one word to EEPROM
{
#if SKBASIC_DEBUG
	_print("targetwriteeeprom16()\r\n");
#endif
}

void			targetwriteeeprom32(U32  addr, U32  val)	// writes one double word to EEPROM
{
#if SKBASIC_DEBUG
	_print("targetwriteeeprom32()\r\n");
#endif
}

U32				targetreadeeprom(U32  addr)				// reads one byte from EEPROM, returns as U32
{
#if SKBASIC_DEBUG
	_print("targetreadeeprom()\r\n");
#endif
	return(0);
}

U32				targetreadeeprom16(U32  addr)				// reads one word from EEPROM, returns as U32
{
#if SKBASIC_DEBUG
	_print("targetreadeeprom16()\r\n");
#endif
	return(0);
}

U32				targetreadeeprom32(U32  addr)				// reads one double word from EEPROM, returns as U32
{
#if SKBASIC_DEBUG
	_print("targetreadeeprom32()\r\n");
#endif
	return(0);
}

U32				targetgetsystime(TM  *pt)					// returns broken time in struct pointed to by pt
{
	pt->tm_sec = 0;				// stubbed until RTC support is added
	pt->tm_min = 0;
	pt->tm_hour = 0;
	return  0;
}

void			targetsetsystime(TM  *pt)					// sets system time from broken time
{
#if SKBASIC_DEBUG
	_print("targetsetsystime()\r\n");
#endif
}

U32				targetgetsysdate(TM  *pt)					// returns broken date in struct pointed to by pt
{
#if SKBASIC_DEBUG
	_print("targetgetsysdate()\r\n");
#endif
	pt->tm_wday = 1;					// 1 Jan 12; stubbed until RTC support is added
	pt->tm_mday = 1;
	pt->tm_yday = 1;
	pt->tm_mon = 0;
	pt->tm_year = 112;

	return  0;
}

void			targetsetsysdate(TM  *pt)					// sets system date from broken date
{
#if SKBASIC_DEBUG
	_print("targetsetsysdate()\r\n");
#endif
}

U32				targetgetflashprogramaddr(U8  filenum)		// returns addr of program in selected flash file or 0 if no such file
{
#if SKBASIC_DEBUG
	_print("targetgetflashprogramaddr()\r\n");
#endif
	return(0);
}

void			targetcopyfromflash(U8 *buff, U32  fladdr, U16  num)
{
#if SKBASIC_DEBUG
	_print("targetcopyfromflash()\r\n");
#endif
}

U16 			getU16(U16 *p)
{
	U8	*b = (U8 *)p;
	return(((U16)b[1] << 8) + b[0]);
}

void			setU16(U16 *p, U16 v)
{
	U8	*b = (U8 *)p;
	b[0] = (U8)v;
	b[1] = (U8)(v >> 8);
}

U32				getU32(U32 *p)
{
	U8	*b = (U8 *)p;
	return(((U32)b[3] << 24) + ((U32)b[2] << 16) + ((U32)b[1] << 8) + b[0]);
}

void			setU32(U32 *p, U32 v)
{
	U8	*b = (U8 *)p;
	b[0] = (U8)v;
	b[1] = (U8)(v >> 8);
	b[2] = (U8)(v >> 16);
	b[3] = (U8)(v >> 24);
}

void			targetruntask(void)
{
	if(runflag){
		char	in = (char)SK_getc();
		if(in == 0x03){
			breakflag = 1;
		}
	}

	//
	// Write user run task here
	BasicTimerTask();
	//
}

#if 0
#prgama mark - FAT8
#endif

#if defined(CPU78K0R)

#include "fsl.h"
#include "SelfLib.h"

#define	TOTAL_FLASH_SIZE		(512L * 1024L)
#define	VOLUME_SECTOR_BYTES		1024L
#define	VOLUME_TOTAL_SECTORS	124L
#define	VOLUME_RESERVED_SECTORS	4L
#define	FAT_BYTES				1024
#define	DIR_BYTES				1024
#define	DIR_ENTRY_BYTES			(1 + MAX_FILE_NAME_LEN + 1)
#define	DIR_PLANE_TEXT			0x01
#define	DIR_FREE_SPACE			0xff
#define	FAT_RESERVED_SECTORS	((DIR_BYTES + FAT_BYTES) / VOLUME_SECTOR_BYTES)
#define	FAT_FREE_SECTOR			0xff
#define	FAT_LAST_SECTOR			0xf8

static	U8				sector_list[VOLUME_TOTAL_SECTORS];
static	U8				flash_buffer[VOLUME_SECTOR_BYTES];
static	int				sector_list_index;

static	void	flush_flash_buffer(void);

static __far U8 *flash_top(void)
{
	return((__far U8 *)(TOTAL_FLASH_SIZE - (VOLUME_SECTOR_BYTES * (VOLUME_TOTAL_SECTORS + VOLUME_RESERVED_SECTORS))));
}

static	void	copy_fat_sector(void)
{
	__far U8	*fat_ptr = flash_top();
	int			i;
	for(i = 0; i < sizeof(flash_buffer); i++){
		flash_buffer[i] = *fat_ptr++;
	}
}

static	void	copy_dir_sector(__far U8 *dir_ptr)
{
	int			i;
	dir_ptr = (__far U8 *)((U32)dir_ptr & ~(VOLUME_SECTOR_BYTES - 1));
	for(i = 0; i < sizeof(flash_buffer); i++){
		flash_buffer[i] = *dir_ptr++;
	}
}

static	int		count_free_sectors(void)
{
	__far U8	*fat_ptr = flash_top();
	int			i, ret = 0;
	for(i = FAT_RESERVED_SECTORS; i < VOLUME_TOTAL_SECTORS; i++){
		if(fat_ptr[i] == 0xff){
			ret++;
		}
	}
	return(ret);
}

static	U32		count_free_bytes(void)
{
	U32		ret = count_free_sectors();
	ret *= VOLUME_SECTOR_BYTES;
	return(ret);
}

static	int		find_next_sector(int now)
{
	__far U8	*fat_ptr = flash_top();
	if(now <= 0){
		now = FAT_RESERVED_SECTORS;
	}else{
		now += 1;
	}
	while((fat_ptr[now] != 0xff) && (now < VOLUME_TOTAL_SECTORS)){
		now++;
	}
	return(now);
}

static	__far U8	*find_free_entry(void)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = &fat_ptr[FAT_BYTES];
	int			i = 0;
	while(i < DIR_BYTES){
		if(dir_ptr[i] == 0xff){
			break;
		}else{
			i += DIR_ENTRY_BYTES;
		}
	}
	return(&dir_ptr[i]);
}

static	__far U8	*find_file_entry(char *filename)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = &fat_ptr[FAT_BYTES];
	__far U8	*ret = NULL;
	int			i = 0;
	while(i < DIR_BYTES){
		if(dir_ptr[i] == 0xff){
			i += DIR_ENTRY_BYTES;
		}else{
			int		j = 0;
			while((dir_ptr[i + j + 1] != 0xff) && (filename[j] != 0) && (filename[j] != 0x0d) && (dir_ptr[i + j + 1] == filename[j])){
				j++;
				if(j >= MAX_FILE_NAME_LEN){
					break;
				}
			}
			if((j >= MAX_FILE_NAME_LEN) || ((dir_ptr[i + j + 1] == 0xff) && ((filename[j] == 0x0d) || (filename[j] == 0)))){
				ret = &dir_ptr[i];
				break;
			}
			i += DIR_ENTRY_BYTES;
		}
	}
	return(ret);
}

static	void	targetformatflash(void)
{
	__far U8	*flash_ptr = flash_top();
	int			i;
	memset(flash_buffer, -1, sizeof(flash_buffer));
	for(i = 0; i < VOLUME_TOTAL_SECTORS; i++){
		int		j;
		int		erase = 0;
		for(j = 0; j < VOLUME_SECTOR_BYTES; j++){
			if(flash_ptr[j] != 0xff){
				erase++;
				break;
			}
		}
		if(erase){
			FlashBlockWrite((SK_SlfH)((U32)flash_ptr / VOLUME_SECTOR_BYTES), 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));
			pl_P(PSTR("Block "));
			outdeci(i);
			pl_P(PSTR(" erased."));
			nl();
		}
		flash_ptr += VOLUME_SECTOR_BYTES;
	}
}

void	targetlistdirentryentry(void)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = &fat_ptr[FAT_BYTES];
	int			i = 0, number_of_files = 0;
	while(i < DIR_BYTES){
		if(dir_ptr[0] == DIR_PLANE_TEXT){
			char	filename[MAX_FILE_NAME_LEN + 1];
			int		j;
			memset(filename, 0, sizeof(filename));
			for(j = 1; j < MAX_FILE_NAME_LEN + 1; j++){
				if(dir_ptr[j] == 0xff){
					break;
				}
				filename[j - 1] = dir_ptr[j];
			}
			number_of_files++;
			pl(filename);
			nl();
		}
		dir_ptr += DIR_ENTRY_BYTES;
		i += DIR_ENTRY_BYTES;
	}
	if(number_of_files == 0){
		pl_P(PSTR("No such files."));
		nl();
	}
}

static	void	targetloadfile(char *filename)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = find_file_entry(filename);
	__far U8	*sector_ptr;
	int			srcoffset = 0, dstoffset = 0;
	int			sector;

	initvars();
	sector = dir_ptr[DIR_ENTRY_BYTES - 1];
	sector_ptr = &fat_ptr[(U32)sector * VOLUME_SECTOR_BYTES];
	immid = 0;					// not in immediate mode (yet)
	errcode = 0;				// clear error status
	runflag = 0;				// clear the run mode flag
	while(sector_ptr[srcoffset] != 0xff){
		if((inbuff[dstoffset++] = sector_ptr[srcoffset++]) == 0x0d){
			ibufptr = inbuff;
			skipspcs();					// ignore leading spaces in input buffer
	//		if (chckcmds()) continue;	// check for commands
			initstack8(&opstack, oparray, OPSLEN);	// always clear opcode stack
			initstack32(&numstack, numarray, NUMSLEN); 	// always clear numeric stack

			if (chckcmds() == 0)		// if line was not a known command...
			{
				unsigned char				*fence;		// temp pointer storage in case of error
				fence = varend;			// save current end of variables
				parse_cmdline();				// translate/execute line
				if (errcode)			// if an error occured somewhere...
				{
					rpterr();			// report the error
					varend = fence;		// restore the end of variables
				}
			}
			dstoffset = 0;
		}
		if(srcoffset >= VOLUME_SECTOR_BYTES){
			sector = fat_ptr[sector];
			if(sector >= 0xf0){
				break;
			}
			sector_ptr = &fat_ptr[(U32)sector * VOLUME_SECTOR_BYTES];
			srcoffset = 0;
		}
	}
}

static	void	targetdeletefile(char *filename)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = find_file_entry(filename);
	int			sector;
	int			i;
	int			dir_offset;

	copy_fat_sector();
	sector = dir_ptr[DIR_ENTRY_BYTES - 1];
	while(flash_buffer[sector] < 0xf0){
		int		next = flash_buffer[sector];
		flash_buffer[sector] = 0xff;
		sector = next;
	}
	flash_buffer[sector] = 0xff;
	FlashBlockWrite((SK_SlfH)((U32)fat_ptr / VOLUME_SECTOR_BYTES), 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));

	copy_dir_sector(dir_ptr);
	dir_offset = (int)((U32)dir_ptr & (VOLUME_SECTOR_BYTES - 1));
	for(i = 0; i < DIR_ENTRY_BYTES; i++){
		flash_buffer[dir_offset + i] = 0xff;
	}
	FlashBlockWrite((SK_SlfH)((U32)dir_ptr / VOLUME_SECTOR_BYTES), 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));
}

static	void	targetsavefile(char *filename)
{
	__far U8	*fat_ptr = flash_top();
	__far U8	*dir_ptr = find_free_entry();
	int			i;
	int			dir_offset;

	memset(sector_list, -1, sizeof(sector_list));
	memset(flash_buffer, -1, sizeof(flash_buffer));

	save_flag = 2;
	sector_list_index = 0;
	save_bytes = 0;
	intptr = basbeg;
	lastlin = hiline;
	firstlin = getU16((U16 *)intptr);			// get first line number
	clistprog(firstlin, lastlin);
	if(save_bytes > 0){
		flush_flash_buffer();
	}
	save_flag = 0;

	copy_fat_sector();
	i = 0;
	while(sector_list[i] != 0xff){
		flash_buffer[sector_list[i]] = sector_list[i + 1] != 0xff ? sector_list[i + 1] : 0xf8;
		i++;
	}
	FlashBlockWrite((SK_SlfH)((U32)fat_ptr / VOLUME_SECTOR_BYTES), 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));

	copy_dir_sector(dir_ptr);
	dir_offset = (int)((U32)dir_ptr & (VOLUME_SECTOR_BYTES - 1));
	for(i = 0; i < DIR_ENTRY_BYTES; i++){
		flash_buffer[dir_offset + i] = 0xff;
	}
	flash_buffer[dir_offset + 0] = DIR_PLANE_TEXT;
	i = 1;
	while(*filename != 0x0d){
		flash_buffer[dir_offset + i++] = *filename++;
	}
	flash_buffer[dir_offset + (DIR_ENTRY_BYTES - 1)] = sector_list[0];
	FlashBlockWrite((SK_SlfH)((U32)dir_ptr / VOLUME_SECTOR_BYTES), 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));
}

static	void	flush_flash_buffer(void)
{
	int		sector_index;
	U32		block = (U32)flash_top() / VOLUME_SECTOR_BYTES;
	if(sector_list_index == 0){
		sector_index = find_next_sector(0);
	}else{
		sector_index = find_next_sector(sector_list[sector_list_index - 1]);
	}
	sector_list[sector_list_index] = (U8)sector_index;
	FlashBlockWrite((SK_SlfH)block + sector_index, 0, (SK_SlfB *)flash_buffer, sizeof(flash_buffer));
	sector_list_index += 1;
	save_bytes = 0;
	memset(flash_buffer, -1, sizeof(flash_buffer));
}

static	void	flash_buffer_putc(char c)
{
	flash_buffer[save_bytes++] = c;
	if(save_bytes >= sizeof(flash_buffer)){
		flush_flash_buffer();
	}
}

#else


static	void	targetformatflash(void)
{
}

void	targetlistdirentryentry(void)
{
}

static	void	targetloadfile(char *filename)
{

}

static	void	targetdeletefile(char *filename)
{
}

static	void	targetsavefile(char *filename)
{
}

static	void	flash_buffer_putc(char c)
{
}

static	U32		count_free_bytes(void)
{
	return 0;
}

static	U8		*find_free_entry(void)
{
	return 0;
}

static	U8		*find_file_entry(char *filename)
{
	return 0;
}
#endif
