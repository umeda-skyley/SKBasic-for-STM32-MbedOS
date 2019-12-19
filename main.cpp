/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "stats_report.h"
#include <stdio.h>

#include "compiler_options.h"
#include "command.h"
#include "SimpleUART.h"
#include "skyley_base.h"
#include "hardware.h"

//Stack and heap report for debug
SystemReport report(1000);

// -------------------------------------------------
//   Debug LED
// -------------------------------------------------
DigitalOut led1(LED1);
DigitalOut led2(LED2);

// -------------------------------------------------
//   SKBasic CLI
// -------------------------------------------------
//UART I/F
SimpleUART serial(PD_8, PD_9, 38400);

//CLI thread
Thread cmd_thread(osPriorityNormal, 2048, NULL, "command");
//thread main
void command_main(void);

//10 msec systick to CLI
Ticker cmd_tick;


int main()
{
	//Start SKBasic CLI thread
	cmd_thread.start(command_main);
	cmd_tick.attach_us(&Timer_Interrupt, 10000); // 10msec

    while (true) {
        thread_sleep_for(10000);
        report.report_state();
    }
}


// -------------------------------------------------
//   SKBadic CLI thread main
// -------------------------------------------------
void command_main(void)
{
	CommandInit();

    while (true)
    {
        CommandMain();
    }
}


/* -------------------------------------------------
 * mbed - CLI glue code
 -------------------------------------------------　*/
#ifdef __cplusplus
extern "C" {
#endif

//Put one character to UART TX
//this func must be blocking mode
void UART_PutChar(unsigned char c){
	serial.putc((int)c);
}

//CLI systick, 10 msec interval
void Timer_Interrupt(void){
	SK_IncrementTimeTick(10);
}

#ifdef __cplusplus
}
#endif

