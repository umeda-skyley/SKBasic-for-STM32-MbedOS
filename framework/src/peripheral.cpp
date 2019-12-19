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

// -------------------------------------------------
//   Compiler Options
// -------------------------------------------------
#include "compiler_options.h"

// -------------------------------------------------
//   Include
// -------------------------------------------------
#include <string.h>
#include "skyley_type.h"
#include "skyley_base.h"
#include "hardware.h"
#include "uart_interface.h"

// -------------------------------------------------
//   Private functions
// -------------------------------------------------

// -------------------------------------------------
//   Ring buffer
// -------------------------------------------------
#define		SK_UART_RX_BUFSIZ			256			// Serial port buffer (must be 2^n)

SK_UB gaSK_UART_RXbuf[SK_UART_RX_BUFSIZ];			// Ring buffer for Recv
static SK_UH gnSK_UART_RXst = 0;					// Buffer start
static SK_UH gnSK_UART_RXed = 0;					// Buffer end

// -------------------------------------------------
//   Working variables
// -------------------------------------------------

// -------------------------------------------------
//   Input one char
// -------------------------------------------------
void UART_RxInt(SK_UB ch) {
	SK_UH				next;

	// Store buffer
	next = (gnSK_UART_RXed + 1) & (SK_UART_RX_BUFSIZ - 1);
	if (gnSK_UART_RXst != next) {
		gaSK_UART_RXbuf[gnSK_UART_RXed] = ch;
		gnSK_UART_RXed = next;
	} else {
		//buffer over run
	}
}

// -------------------------------------------------
//   Output one char
// -------------------------------------------------
#if 0
//-> main.cpp
void UART_PutChar(SK_UB ch) {

}
#endif

// -------------------------------------------------
//   Read one char from ring buffer
// -------------------------------------------------
SK_H UART_GetChar(void) {
	SK_UB ch;
	
	if (gnSK_UART_RXst == gnSK_UART_RXed) {
		return -1;
	} else {
		ch = gaSK_UART_RXbuf[gnSK_UART_RXst];
		gnSK_UART_RXst = (gnSK_UART_RXst + 1) & (SK_UART_RX_BUFSIZ - 1);
		return (SK_H)ch;
	}
}

// -------------------------------------------------
//   Check num of chars in the ring buffer 
// -------------------------------------------------
SK_H UART_GetLen(void) {
	SK_H	len;

	len = (SK_H)gnSK_UART_RXed - (SK_H)gnSK_UART_RXst;
	if (len < 0) { len += SK_UART_RX_BUFSIZ; }
	return len;
}


void UART_InitBuf(void){
	gnSK_UART_RXst = 0;
	gnSK_UART_RXed = 0;
}
