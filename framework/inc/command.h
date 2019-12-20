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

#ifndef __command_h__
#define __command_h__

#include "skyley_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SK_E_OK
#define K_E_OK 0
#endif
#ifndef SK_E_ER
#define SK_E_ER (-1)
#endif

#define		ER01	1
#define		ER02	2
#define		ER03	3
#define		ER04	4
#define		ER05	5
#define		ER06	6
#define		ER07	7
#define		ER08	8
#define		ER09	9
#define		ER10	10

#define FAIL_ER04 "fail err04\r\n"
#define FAIL_ER05 "fail err05\r\n"
#define FAIL_ER06 "fail err06\r\n"
#define FAIL_ER10 "fail err10\r\n"
#define RESULT_OK "ok\r\n"

extern SK_UW gnWaitDuration;

extern void CommandInit(void);
extern void CommandMain(void);

//called from basic main
extern void BasicTimerTask(void);
extern void LedControl(SK_UB inx, SK_UB val);

typedef struct {
	const char *name;
	void (*func)(SK_UB, SK_UB **);
} CMDTBL;

#ifdef __cplusplus
}
#endif

#endif
