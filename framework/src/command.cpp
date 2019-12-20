/**
    Copyright (c) 2019 Skyley Networks, Inc. All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the Institute nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include "compiler_options.h"

// -------------------------------------------------
//   Include files
// -------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "mbed.h"

//framework
#include "skyley_base.h"
#include "hardware.h"
#include "command.h"
#include "uart_interface.h"

//Basic
#include "defines.h"
#include "funcs.h"

// -------------------------------------------------
//   State Machines
// -------------------------------------------------
SK_STATESTART(LED_BLINK);
SK_STATESTART(LED_BLINK_TASK);

SK_STATESTART(BASIC_TIMER1);
SK_STATESTART(BASIC_TIMER1_TASK);
SK_STATESTART(BASIC_TIMER2);
SK_STATESTART(BASIC_TIMER2_TASK);

// -------------------------------------------------
//   LED on/off states
// -------------------------------------------------
typedef enum {
	eLEDOn,
	eLEDOff,
} LedState;

// -------------------------------------------------
//   Basic Timer states
// -------------------------------------------------
typedef enum {
	eTimIdle,
	eTimTrigger,
} TimState;

// -------------------------------------------------
//   Import variables
// -------------------------------------------------
//from main.cpp
extern DigitalOut led1;
extern DigitalOut led2;

// -------------------------------------------------
//   Prototypes
// -------------------------------------------------
void Interface(void);
void SampleApplication(void);
static void Timer1Task(void);
static void Timer2Task(void);
SK_UB GetParam(SK_UB *lineBuf, SK_UB **ATParam,SK_UH len);
SK_UB CharToNumber(SK_UB *param, SK_UW *num, SK_UB len);
SK_UB ParamCheck(SK_UB *param, SK_UW *num, SK_UB len, SK_UW max, SK_UW min);
SK_UB CheckDigit(SK_UB *ATParam, SK_UB num);
void _putc(char ch);
SK_H _getc(void);

// -------------------------------------------------
//   Command control
// -------------------------------------------------
#define		SAMPLEAPP_STATUS_LINE			0
#define		SAMPLEAPP_STATUS_CMD			1
#define		SAMPLEAPP_STATUS_HEX			2
#define		SAMPLEAPP_STATUS_RAW			3
#define		SAMPLEAPP_STATUS_WAIT			4

#define		SAMPLEAPP_MENU_TOP				1
#define		SAMPLEAPP_MENU_TOP_WAIT			2
#define		SAMPLEAPP_MENU_WAIT_CRLF		13

#define 	MAX_LINE_SIZE					256
#define 	LINE_BUFFER_SIZE				(MAX_LINE_SIZE)
#define 	NUM_OF_ATPARAM 					10
static SK_UB gaLineBuf[LINE_BUFFER_SIZE];
static SK_UH gnLinePos = 0;
static SK_UB gnLineStatus = SAMPLEAPP_STATUS_LINE;
static SK_UB gnMenuStatus = SAMPLEAPP_MENU_TOP;

// -------------------------------------------------
//   SKBasic Timer handlers
// -------------------------------------------------
SK_UW gnTim1Interval = 0;
SK_UW gnTim2Interval = 0;

// -------------------------------------------------
//   Working variables
// -------------------------------------------------
//TRUE:Echoback on
//FALSE:Echoback off
SK_BOOL gbEchoBack;
SK_BOOL gnBasicMode;
SK_UW	gnWaitDuration;

// -------------------------------------------------
//   Command Table
// -------------------------------------------------
static void info(SK_UB NumOfATParam, SK_UB **ATParam);
static void exec_basic(SK_UB NumOfATParam, SK_UB **ATParam);

static const CMDTBL cmdtbl[] = {
	{(char*)"info", (void (*)(SK_UB, SK_UB**)) info},
	{(char*)"basic", (void (*)(SK_UB, SK_UB**)) exec_basic },
	{NULL, NULL}
};

// -------------------------------------------------
//   Command main
// -------------------------------------------------
void CommandInit(void){
	gbEchoBack = TRUE;
	gnBasicMode = FALSE;

	gnWaitDuration = 0;

	gnTim1Interval = 0;
	gnTim2Interval = 0;

	gnLinePos = 0;
	gnLineStatus = SAMPLEAPP_STATUS_LINE;
	gnMenuStatus = SAMPLEAPP_MENU_TOP;

	//Reset UART Rx ring buffer
	UART_InitBuf();

	//Initialize event/memory SK framework
	SK_Initialize();

	//Initialize state machines
	SK_INITSTATE(LED_BLINK);
	SK_INITSTATE(LED_BLINK_TASK);

	SK_INITSTATE(BASIC_TIMER1);
	SK_INITSTATE(BASIC_TIMER1_TASK);
	SK_INITSTATE(BASIC_TIMER2);
	SK_INITSTATE(BASIC_TIMER2_TASK);
}

void CommandMain(void){
	Interface();

	SampleApplication();
}

// -------------------------------------------------
//   Sample app for state machines, SK_WAITFOR, SK_SLEEP
// Brinking LED without using RTOS thread/task.
// -------------------------------------------------
void SampleApplication(void) {
	static LedState state;

	//Get current state
	state = (LedState)SK_GETSTATE(LED_BLINK);

	//Use 2 state machines to blink LED
	SK_STATEADD(LED_BLINK_TASK, 1);
	SK_STATEADD(LED_BLINK_TASK, 2);

	switch( state ){
	case eLEDOn:
		led1 = 1; //LED on

		//1 sec sleep
		SK_SLEEP(1000, LED_BLINK_TASK, 1);

		//move to LED off state
		SK_SETSTATE(eLEDOff, LED_BLINK);
		break;

	case eLEDOff:
		led1 = 0; //LED off

		//1sec sleep
		SK_SLEEP(1000, LED_BLINK_TASK, 2);

		//move to LED on state
		SK_SETSTATE(eLEDOn, LED_BLINK);
		break;

	default:
		break;
	}

	SK_STATEEND(LED_BLINK_TASK);
}

void BasicTimerTask(void) {
	Timer1Task();
	Timer2Task();
}

static void Timer1Task(void){
	static TimState state;

	//Get current state
	state = (TimState)SK_GETSTATE(BASIC_TIMER1);

	SK_STATEADD(BASIC_TIMER1_TASK, 1);

	switch( state ){
	case eTimIdle:
		if( gnTim1Interval == 0 ){
			return;
		}

		SK_SLEEP(gnTim1Interval, BASIC_TIMER1_TASK, 1);

		//Trigger timer interrupt into SKBasic Interpreter
		SK_SETSTATE(eTimTrigger, BASIC_TIMER1);
		break;

	case eTimTrigger:
		if( gnTim1Interval == 0 ){
			SK_SETSTATE(eTimIdle, BASIC_TIMER1);
			return;
		}

		//Call Timer1 hander in SKBasic
		if(runflag && gnBasicMode && ontim1lin > 0 ){
			rinterrupt(ontim1lin);
		}

		//move to Idle state
		SK_SETSTATE(eTimIdle, BASIC_TIMER1);
		break;

	default:
		break;
	}

	SK_STATEEND(BASIC_TIMER1_TASK);
}

static void Timer2Task(void){
	static TimState state;

	//Get current state
	state = (TimState)SK_GETSTATE(BASIC_TIMER2);

	SK_STATEADD(BASIC_TIMER2_TASK, 1);

	switch( state ){
	case eTimIdle:
		if( gnTim2Interval == 0 ){
			return;
		}

		SK_SLEEP(gnTim2Interval, BASIC_TIMER2_TASK, 1);

		//Trigger timer interrupt into SKBasic Interpreter
		SK_SETSTATE(eTimTrigger, BASIC_TIMER2);
		break;

	case eTimTrigger:
		if( gnTim2Interval == 0 ){
			SK_SETSTATE(eTimIdle, BASIC_TIMER2);
			return;
		}

		//Call Timer1 hander in SKBasic
		if(runflag && gnBasicMode && ontim2lin > 0 ){
			rinterrupt(ontim2lin);
		}

		//move to Idle state
		SK_SETSTATE(eTimIdle, BASIC_TIMER2);
		break;

	default:
		break;
	}

	SK_STATEEND(BASIC_TIMER2_TASK);
}

void LedControl(SK_UB idx, SK_UB onoff){
	if( onoff >= 2 ){
		return;
	}

	if( idx == 1 ){
		led1 = onoff;
	} else if( idx == 2 ){
		led2 = onoff;
	}
}

// -------------------------------------------------
//   Input a line from UART
// -------------------------------------------------
void Interface(void) {
	SK_UB  NumOfATParam;
	SK_UB* ATParam[NUM_OF_ATPARAM];
	static 	SK_H in;

	// -------------------------------------------------
	//   Get user input from PC (UART)
	// -------------------------------------------------
	in = _getc();
	if (in >= 0) {
		switch (gnLineStatus) {

		case SAMPLEAPP_STATUS_HEX:
			// Input Hex value
			if (in >= 0x20) {
				//if ((in >= 'a') && (in <='z')) { in -= 0x20; }
				if (((in >= 'a') && (in <='z')) ||
					((in >= '0') && (in <='9')) ||
					((in >= 'A') && (in <='F'))) {
					// hex value
				} else {
					in = 0;
				}
			}
		case SAMPLEAPP_STATUS_LINE:
			// Input Strings
			if (in >= 0x20) {
				if (gnLinePos < (LINE_BUFFER_SIZE-1)) {
					gaLineBuf[gnLinePos++]	= (SK_UB)in;
					gaLineBuf[gnLinePos]	= 0;
					if( gbEchoBack == 1 )_putc((SK_UB)in);
				}
				in = 0;
			} else {
				if (in == 13) {
					gaLineBuf[gnLinePos]	= 0;
					//_print("\r\n");
					if( gbEchoBack == 1 )_print("\r\n");
					in = 1;
				} else {
					if ((in == 8) || (in == 127)) {
						if (gnLinePos > 0) {
							if( gbEchoBack == 1 )_putc(8);
							if( gbEchoBack == 1 )_putc(' ');
							if( gbEchoBack == 1 )_putc(8);
							gnLinePos--;
							gaLineBuf[gnLinePos]	= 0;
						}
					}
					in = 0;
				}
			}
			break;
			
		case SAMPLEAPP_STATUS_WAIT:
			break;
			
		default:
			// Input one ASCII character
			if ((in >= 0x20) || (in == 13)) {
				//if ((in >= 'a') && (in <='z')) { in -= 0x20; }
				gaLineBuf[0] 			= (SK_UB)in;
				gaLineBuf[1] 			= 0;
				gnLinePos = 1;
				if( gbEchoBack == 1 )_putc((SK_UB)in);
				if( gbEchoBack == 1 )_print("\r\n");
				in = 1;
			} else {
				in = 0;
			}
			break;
		}
	}

	// -------------------------------------------------
	//   Process a input value
	// -------------------------------------------------
	switch(gnMenuStatus) {
		case SAMPLEAPP_MENU_TOP:
			gnLineStatus		= SAMPLEAPP_STATUS_LINE;
			//gn_nLinePos		= 0;
			gnMenuStatus		= SAMPLEAPP_MENU_TOP_WAIT;
			in = 0;

		case SAMPLEAPP_MENU_TOP_WAIT:
			if (in > 0) {
				NumOfATParam = GetParam(gaLineBuf, ATParam, gnLinePos);
				if( NumOfATParam == 0 || NumOfATParam == 255 ) {
					if( NumOfATParam == 255 ){
						SK_print(FAIL_ER04);
					}
					gnLinePos = 0;
					gnMenuStatus = SAMPLEAPP_MENU_TOP;
					break;
				}

				{
					const CMDTBL *p;
					SK_UB cmdlen;
					SK_BOOL found = FALSE;

					p = cmdtbl;
					while (p->name != NULL) {
						cmdlen = strlen((char *)p->name);
						if (cmdlen == strlen((char *)ATParam[0]) && strncmp((char *)ATParam[0], (char *)p->name, cmdlen) == 0) {
							((int (*)(SK_UB, SK_UB **))(*p->func))(NumOfATParam, ATParam);
							found = TRUE;
							break;
						}
						p++;
					}

					if( found == FALSE ){
						SK_print(FAIL_ER04);
					}

					gnLinePos = 0;
					gnMenuStatus = SAMPLEAPP_MENU_TOP;
				}
			}
			break;

		default:
			break;
	}
}

// -------------------------------------------------
// Split input SK command by white space
// -------------------------------------------------
SK_UB GetParam(SK_UB *lineBuf, SK_UB **ATParam, SK_UH bufLen){
	SK_UB param = 0, skip = 0;
	SK_UH len;

	len = 0;

	while(*lineBuf != 0x00){
		if((*lineBuf != ' ') && (*lineBuf != 0x00)){
			if(skip == 0){
				ATParam[param] = lineBuf;
				param++;
				if( param > NUM_OF_ATPARAM ) return 255;
			}
			lineBuf++;
			skip = 1;
		} else {
			*lineBuf = 0x00;
			lineBuf++;
			skip = 0;
		}
		len++;
		if( len > bufLen ) return 255;
	}
	return param;
}

// -------------------------------------------------
// Convert Hex str to number
// -------------------------------------------------
SK_UB CharToNumber(SK_UB *param, SK_UW *num, SK_UB len){

	SK_UB no[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','a','b','c','d','e','f'};
	SK_UB i, j;
	SK_UB *p = param;

	*num = 0;
	if(len > 8)
		return ER05;

	for(i=0; i < len; i++, p++){
		for(j = 0; j < 22; j++){
			if(*p == 0x00)
				return SK_E_OK;
			if(strncmp((const char *)p,(const char *)(&no[j]),1) == 0){
				*num <<= 4;
				if(j < 0xA)
					*num |= (no[j] - '0');
				else if(0xA <= j && j < 0x10)
					*num |= (no[j] - 'A' + 10);
				else
					*num |= (no[j] - 'a' + 10);
				break;
			}
		}
		if(j >= 22)
			return ER05;
	}
	return SK_E_OK;
}

// -------------------------------------------------
// Check value range
// -------------------------------------------------
SK_UB ParamCheck(SK_UB *param, SK_UW *num, SK_UB len, SK_UW max, SK_UW min){
	SK_UB res;

	res = CheckDigit(param, len);
	if(res != SK_E_OK){
		return ER06;
	}

	res = CharToNumber(param, num, len);

	if((res != SK_E_OK) || ((*num < min) || (max < *num))){
		return ER06;
	}
	return SK_E_OK;
}

// -------------------------------------------------
// Check whether the null-terminated string ATParam has a length not bigger than num
// -------------------------------------------------
SK_UB CheckDigit(SK_UB *ATParam, SK_UB num){
	SK_UB i;

	for(i = 0; i < 20; i++){
		if(ATParam[i] == 0) break;
	}

	if(i > num){
		return ER06;
	} else {
		return SK_E_OK;
	}
}

// -------------------------------------------------
// Convert hex strings to binary
// -------------------------------------------------
void HexToBin(SK_UB* bin, SK_UB* hex, SK_UH datalen){
	SK_UH i, hexi;
	SK_UW val;

	hexi = 0;
	for( i = 0; i < datalen; i++ ){
		if( hex[hexi] == 0x00 ) break;
		CharToNumber(hex + hexi, &val, 2);
		hexi+=2;
		bin[i] = (SK_UB)val;
	}
}

// -------------------------------------------------
//   UART char input/output
// -------------------------------------------------
void _putc(char ch){
	UART_PutChar(ch);
}

SK_H _getc(void){
	return UART_GetChar();
}

/* -------------------------------------------------
 * Command procedures
 *
 * コマンドの追加：
 * 1) 引数の数と値域チェックは必ず行う
 *
 * 引数仕様：
 * NumOfATParam...コマンド引数の数 (コマンド自身の数を含む）
 * ATParam...コマンド引数文字列の2次元配列（末尾はNULL終端）
 *
 * エラーコード：
 * ER04 指定されたコマンドがサポートされていない
 * ER05 指定されたコマンドの引数の数が正しくない
 * ER06 指定されたコマンドの引数形式や値域が正しくない
 * ER09 UART入力エラーが発生した
 * ER10 指定されたコマンドは受付けたが、実行結果が失敗した
 *
 ------------------------------------------------- */

// -------------------------------------------------
//   info: sample
// -------------------------------------------------
static void info(SK_UB NumOfATParam, SK_UB **ATParam){
	SK_print("Welcome to Basic for STM32 CLI\r\n");
	SK_print("FW ver    :"); SK_print(VERSION_STR);SK_print("\r\n");
	return;
}

// -------------------------------------------------
//   do_basic
// -------------------------------------------------
static void exec_basic(SK_UB NumOfATParam, SK_UB **ATParam){
	gnBasicMode = TRUE;
	basic_main();
	gnBasicMode = FALSE;
	SK_print(RESULT_OK);
	return;
}

