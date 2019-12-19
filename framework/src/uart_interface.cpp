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
//   Includes
// -------------------------------------------------
#include "uart_interface.h"
#include "hardware.h"


// --------------------------------------------------------
//   Input char (blocking)
// --------------------------------------------------------

SK_UB _getcw(void) {
	while(SK_getlen() == 0) { ; };
	return (SK_UB)SK_getc();
}


// --------------------------------------------------------
//   Print string
// --------------------------------------------------------

void _print(const char *str) {
	while (*str != 0) SK_putc((SK_UB)*str++);
}


// --------------------------------------------------------
//   Print number in hex
// --------------------------------------------------------

static SK_UB hex[] = "0123456789abcdef";

void _print_hex(SK_UW l, SK_UB len) {
	while(len>0) {
		SK_putc(hex[(l >> ((len-1)*4)) & 15]);
		len--;
	}
}


// --------------------------------------------------------
//   Print number in dec
// --------------------------------------------------------

void _print_dec(SK_W d, SK_UB len, SK_UB pre) {
	SK_UB flg;
	SK_W p;
	SK_UH n;

	p = 1; for(n=1;n<len;n++) p *= 10;
	flg = 0;
	
	if (d<0) { SK_putc('-'); d = -d; }
	
	while(p>0) {
		n = (SK_UH)((d / p) % 10);
		if (n==0) {
			if ((flg != 0) || (p == 1)) { SK_putc('0'); } else { if (pre != 0) { SK_putc(pre); } }
		} else {
			SK_putc((SK_UB)('0'+n));
			flg = 1;
		}
		d = d - (n * p);
		p = p / 10;
	}
}
