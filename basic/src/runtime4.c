/*
 *  runtime4.c      Run-time routines for the SKBasic project
 */

#include  <stdio.h>
#include  <ctype.h>



#include  "basictime.h"
#include  "defines.h"
#include  "funcs.h"
#include  "basictime.h"

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

extern void sleep_start();
extern Boolean sleep_callback(VClockPtr clock, void*);
extern void sleep(RuntimeContextPtr rcon);

extern RuntimeContext 	runtime;
extern byte				gSendDataBuffer[];
extern UInt8			gSleepTime;
extern UInt8			gWakeTime;
extern SK_UW			SK_Wait;

#define SET_ID(X, Y) {  X.clue[0] = (byte)(Y>>8); X.clue[1] = (byte)Y; }

void			rsksreg(void)
{
}

void			rsksend(void)
{
	fsksend(0);
}

void			fsksend(int mode_function)
{
	int			ack;
	int			selector;
	U32			address;
	int			datalen = -1;
	int			tok;
	Status		ans;
	Message		msg;
	Identity	target;

	rskipspc();
	donexp();							// calc ack
	ack = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc selector
	selector = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	tok = *tbufptr;
	if((tok != SCONTOK) && (tok != SVARTOK)){
		donexp();							// calc data length
		datalen = (int)pull32(&numstack, STKUNDER);
		if(*tbufptr++ != SKARGTOK)	goto fail;
		rskipspc();
	}
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
		if(datalen < 0){
			datalen = len;
		}
		memcpy(gSendDataBuffer, ptr, datalen);

		SET_ID(target, address);

		init_message(&msg);
		msg.payload_length = (UInt16)datalen;
		msg.payload = (byte*)gSendDataBuffer;
		msg.selector = (UInt16)selector;

		ans = do_sendto(&target, &msg, (Boolean)ack, &runtime.sender, &runtime);
		if( ans == PACKET_SEND_COMPLETED ){
			if(mode_function){
				push32(&numstack, msg.message_id, MSTKOERR);
			}
			if(immid){
				_print_hex(msg.message_id, 4); _print(" OK");
			}
		} else {
			if(mode_function){
				push32(&numstack, -10, MSTKOERR);
			}
			if(immid){
				_print("FAIL ER10");	
			}
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rskflash(void)
{
	fskflash(0);
}

void			fskflash(int mode_function)
{
	int			selector;
	U32			address;
	int			datalen = -1;
	int			tok;
	Status		ans;
	Message		msg;
	Identity	target;

	rskipspc();
	donexp();							// calc selector
	selector = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	tok = *tbufptr;
	if((tok != SCONTOK) && (tok != SVARTOK)){
		donexp();							// calc data length
		datalen = (int)pull32(&numstack, STKUNDER);
		if(*tbufptr++ != SKARGTOK)	goto fail;
		rskipspc();
	}
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
		if(datalen < 0){
			datalen = len;
		}

		memcpy(gSendDataBuffer, ptr, datalen);

		SET_ID(target, address);

		init_message(&msg);
		msg.payload_length = (UInt16)datalen;
		msg.payload = (byte*)gSendDataBuffer;
		msg.selector = (UInt16)selector;

		ans = do_delegate(&target, &msg, D_TRUE, &runtime.sender, &runtime);
		if( ans == PACKET_SEND_COMPLETED ){
			if(mode_function){
				push32(&numstack, msg.message_id, MSTKOERR);
			}
			if(immid){
				_print_hex(msg.message_id, 4); _print(" OK");
			}
		} else {
			if(mode_function){
				push32(&numstack, -10, MSTKOERR);
			}
			if(immid){
				_print("FAIL ER10");	
			}
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rskbc(void)
{
	fskbc(0);
}

void			fskbc(int mode_function)
{
	int			radius;
	int			selector;
	int			datalen = -1;
	int			tok;
	Status		ans;
	Message		msg;

	rskipspc();
	donexp();							// calc radius
	radius = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc selector
	selector = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	tok = *tbufptr;
	if((tok != SCONTOK) && (tok != SVARTOK)){
		donexp();							// calc data length
		datalen = (int)pull32(&numstack, STKUNDER);
		if(*tbufptr++ != SKARGTOK)	goto fail;
		rskipspc();
	}
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
		if(datalen < 0){
			datalen = len;
		}

		memcpy(gSendDataBuffer, ptr, datalen);

		init_message(&msg);
		msg.payload_length = (UInt16)datalen;
		msg.payload = (byte*)gSendDataBuffer;
		msg.selector = (UInt16)selector;
		msg.ttl = (UInt8)radius;

		ans = do_broadcast(&msg, &runtime.sender, &runtime);
		if( ans == PACKET_SEND_COMPLETED ){
			if(mode_function){
				push32(&numstack, msg.message_id, MSTKOERR);
			}
			if(immid){
				_print_hex(msg.message_id, 4); _print(" OK");
			}
		} else {
			if(mode_function){
				push32(&numstack, -10, MSTKOERR);
			}
			if(immid){
				_print("FAIL ER10");	
			}
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rsksync(void)
{
	fsksync(0);
}

void			fsksync(int mode_function)
{
	int			radius;
	U32			address;
	int			tok;
	Status		ans;
	Message		msg;
	Identity	target;

	rskipspc();
	donexp();							// calc radius
	radius = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);

#if defined(RL7023_BINDING)
	if(address == 0xffff){
		ans = do_sync((UInt8)radius, &runtime.sender, &runtime);
	}else{
		SET_ID(target, address);
		ans = do_sync_with(&target, &runtime.sender, &runtime);
	}
#else
	ans = do_sync((UInt8)radius, &runtime.sender, &runtime);
#endif
	if( ans == PACKET_SEND_COMPLETED ){
		if(mode_function){
			push32(&numstack, 1, MSTKOERR);
		}
		if(immid){
			_print(" OK");
		}
	} else {
		if(mode_function){
			push32(&numstack, -10, MSTKOERR);
		}
		if(immid){
			_print("FAIL ER10");
		}
	}
	return;
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rskinq(void)
{
	fskinq(0);
}

void			fskinq(int mode_function)
{
	Status		ans;

	ans = do_inquiry(runtime.self.port_num, &runtime.sender, &runtime);
	if( ans == PACKET_SEND_COMPLETED ){
		if(mode_function){
			push32(&numstack, 1, MSTKOERR);
		}
		if(immid){
			_print("OK");	
		}
	} else {
		if(mode_function){
			push32(&numstack, -10, MSTKOERR);
		}
		if(immid){
			_print("FAIL ER10");
		}
	}
}

void			rskpair(void)
{
	fskpair(0);
}

void			fskpair(int mode_function)
{
	U32			address;
	Status		ans;
	Identity	target;

	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);

	if(address == 0xffff){
		Identity wildcard;
		//mem_set(wildcard.clue, ID_SIZE, '*');
		mem_set(wildcard.clue, ID_SIZE, 0xFF);
		add_to_accept_list(&wildcard, &runtime);
		ans = PACKET_SEND_COMPLETED;
	}else{
		target.clue[0] = (UInt8)(address >> 8);
		target.clue[1] = (UInt8)(address & 0xFF);
		ans = do_pair(&target, &runtime.sender, &runtime);
	}
	if( ans == PACKET_SEND_COMPLETED ){
		if(mode_function){
			push32(&numstack, 1, MSTKOERR);
		}
		if(immid){
			_print(" OK");
		}
	} else {
		if(mode_function){
			push32(&numstack, -10, MSTKOERR);
		}
		if(immid){
			_print("FAIL ER10");
		}
	}
	return;
}

void			rskunpair(void)
{
}

void			fskunpair(int mode_function)
{
	U32			address;
	Status		ans;
	Identity	target;

	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);

	if(address == 0xffff){
		Identity wildcard;
		//mem_set(wildcard.clue, ID_SIZE, '*');
		mem_set(wildcard.clue, ID_SIZE, 0xFF);
		remove_from_accept_list(&wildcard, &runtime);
		ans = PACKET_SEND_COMPLETED;
	}else{
		target.clue[0] = (UInt8)(address >> 8);
		target.clue[1] = (UInt8)(address & 0xFF);
		ans = do_unpair(&target, &runtime.sender, &runtime);
	}
	if( ans == PACKET_SEND_COMPLETED ){
		if(mode_function){
			push32(&numstack, 1, MSTKOERR);
		}
		if(immid){
			_print(" OK");
		}
	} else {
		if(mode_function){
			push32(&numstack, -10, MSTKOERR);
		}
		if(immid){
			_print("FAIL ER10");
		}
	}
	return;
}

void			rsknow(void)
{
	_print("ECLOCK ");
	_print_dec(runtime.self.clock.hour, 2, 0); _print(" ");
	_print_dec(runtime.self.clock.minute, 2, 0); _print(" ");
	_print_dec(runtime.self.clock.second, 2, 0); _print(" ");
	_print_dec(runtime.self.clock.msec, 4, 0); _print(" ");
	_print_dec(runtime.self.clock.is_sync, 1, 0); _print("\r\n");
}

void			rsksleep(void)
{
	VClock sleep_clock;
	clear_clock(&sleep_clock);
	sleep_clock.hour = 0xFE;
	sleep_clock.minute = 0xFE;
	sleep_clock.second = 0xFE;
#if defined(RL7023_BINDING)
	set_wakeup_time(&sleep_clock, &runtime);
#else
	set_wakeup_time(&sleep_clock, 0, &runtime);
#endif

	do_sleep(&runtime);

	_print("EWAKE\r\n");
}

void			rsksetps(void)
{
	U32	sleep, wake;

	rskipspc();
	donexp();							// calc sleep
	sleep = (int)pull32(&numstack, STKUNDER);
	if(*tbufptr++ != SKARGTOK)	goto fail;
	rskipspc();
	donexp();							// calc wake
	wake = (int)pull32(&numstack, STKUNDER);

	if( sleep == 0xFF && wake == 0xFF ){
		gSleepTime = (UInt8)sleep;
		gWakeTime = (UInt8)wake;
		
		remove_job(sleep_callback, &runtime.scheduler, &runtime);
	} else if( sleep >59 || wake > 59 ){
		_print("FAIL ER06\r\n");
	} else {
		gSleepTime = (UInt8)sleep;
		gWakeTime = (UInt8)wake;
		
		sleep_start();
	}

	return;
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rskclrtbl(void)
{
	clear_inquiry_context(&runtime.inquiry_context);
}

void			rskclrcache(void)
{
	clear_lookup_context(&runtime.lookup);
}

void			rsklkup(void)
{
	fsklkup(0);
}

void			fsklkup(int mode_function)
{
	int			datalen = -1;
	int			tok;
	Identity	answer;
	Status		ans;

	rskipspc();
	tok = *tbufptr;
	if((tok != SCONTOK) && (tok != SVARTOK)){
		donexp();							// calc data length
		datalen = (int)pull32(&numstack, STKUNDER);
		if(*tbufptr++ != SKARGTOK)	goto fail;
		rskipspc();
	}
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
		if(datalen < 0){
			datalen = len;
		}

		memset(gSendDataBuffer, 0, MAX_NAME_LEN+1);
		memcpy(gSendDataBuffer, ptr, datalen);

		ans = do_lookup((byte*)gSendDataBuffer, strlen((const char*)gSendDataBuffer), &answer, D_TRUE, &runtime);
		if( ans == D_TRUE ){
			if(mode_function){
				push32(&numstack, ((UInt16)answer.clue[0] << 8) | answer.clue[1], MSTKOERR);
			}
			if(immid){
				_print("EFOUNDC ");
					_print_hex(answer.clue[0], 2);
					_print_hex(answer.clue[1], 2);
					_print(" ");
					_print((char*)gSendDataBuffer);
			}
		}else if(mode_function){
			push32(&numstack, -10, MSTKOERR);
		}
		return;
	}else if(mode_function){
		push32(&numstack, -10, MSTKOERR);
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rskrevlkup(void)
{
	fskrevlkup(0);
}

void			fskrevlkup(int mode_function)
{
	U32			address;
	int			tok;
	Identity	target;
	Status		ans;

	rskipspc();
	donexp();							// calc address
	address = pull32(&numstack, STKUNDER);

	target.clue[0] = (byte)(address>>8);
	target.clue[1] = (byte)address;

	ans = do_rev_lookup(&target, gSendDataBuffer, MAX_NAME_LEN, D_TRUE, &runtime);
	if( ans == D_TRUE ){
		if(mode_function){
			push32(&numstack, ((UInt16)target.clue[0] << 8) | target.clue[1], MSTKOERR);
		}
		if(immid){
			_print("EFOUNDC ");
				_print_hex(target.clue[0], 2);
				_print_hex(target.clue[1], 2);
				_print(" ");
				_print((char*)gSendDataBuffer);
		}
	}else if(mode_function){
		push32(&numstack, -10, MSTKOERR);
	}
}

void			rsksetname(void)
{
	int			datalen = -1;
	int			tok;

	rskipspc();
	tok = *tbufptr;
	if((tok != SCONTOK) && (tok != SVARTOK)){
		donexp();							// calc data length
		datalen = (int)pull32(&numstack, STKUNDER);
		if(*tbufptr++ != SKARGTOK)	goto fail;
		rskipspc();
	}
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
		if(datalen < 0){
			datalen = len;
		}

		memset(runtime.self.name, 0, MAX_NAME_LEN+1);
		mem_copy(runtime.self.name, ptr, (UInt8)datalen);

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
	SK_Wait = pull32(&numstack, STKUNDER);
	if(*tbufptr == SKARGTOK){
		tbufptr++;
	}
	return;
}

void			rwait_jne(void)
{
	if(SK_Wait > 0){
		--tbufptr;
	}
}

#ifdef USE_SECURITY
void			rsksetkey(void)
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
		if(len == 16){
			mem_copy(runtime.self.network_key, ptr, (UInt8)len);
			return;
		}
		errcode = MUST16ERR;
		goto fail;
	}
	errcode = ILTOKERR;
fail:
	rpterr();
}
#endif

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

void			rrxdata(void)
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
		onrxdatalin = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);
		
		return;
	}else if(*tbufptr == SCONTOK){
		onrxdatalin = findlabelline(tbufptr);
		tbufptr += *(tbufptr + 1) + 2;
		if(onrxdatalin == 0){
			errcode = LNFERR;
			rpterr();
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rack(void)
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
		onacklin = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);
		
		return;
	}else if(*tbufptr == SCONTOK){
		onacklin = findlabelline(tbufptr);
		tbufptr += *(tbufptr + 1) + 2;
		if(onrxdatalin == 0){
			errcode = LNFERR;
			rpterr();
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			revent(void)
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
		oneventlin = getU16((U16 *)tbufptr);		// determine the target line number
		tbufptr = tbufptr + sizeof(U16);
		
		return;
	}else if(*tbufptr == SCONTOK){
		oneventlin = findlabelline(tbufptr);
		tbufptr += *(tbufptr + 1) + 2;
		if(onrxdatalin == 0){
			errcode = LNFERR;
			rpterr();
		}
		return;
	}
fail:
	errcode = ILTOKERR;
	rpterr();
}

void			rsetvar(unsigned char vartype, char *varname, unsigned long value)
{
	int		offset = findvar(vartype, varname);
	if(offset >= 0){
	}
}

void			rsetstr(char *varname, U8 *data, int length)
{
	int		offset = findvar(SVARTOK, varname);
	if(offset >= 0){
		U8		*varptr;
		varptr = (U8*)targetgetvarptr((U16)(offset+MAX_VAR_NAME_LEN+1));
		if(length < 0)		length = 0;
		if(length > 255)	length = 255;
		varptr[0] = length;
		memcpy(&varptr[1], data, length);
	}
}

#endif
