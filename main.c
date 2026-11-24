#include "stm32l476xx.h"
#include "SysClock.h"
#include "UART.h"
#include "trigger.h"
#include "capture.h"
#include "cmd.h"

/*
	ENGIN 346 Final Project, Team 2: radar trigger and timestamping unit
	Week 1: hardware trigger on PA.8, trigger and strobe timestamps on TIM2

	Tera Term: STLink Virtual Com Port, 460800 baud, Setup > Terminal > New-line Receive = AUTO
	Jumper PA.8 (D7) to PA.0 (A0). Generator strobe (0 to 3.3 V) to PB.10 (D6).

	TRIG 1000      start the trigger at 1 kHz
	CAPTURE 100    record 100 triggers and the strobes between them, then dump
	STAT?          trigger, PSC, ARR, capture state
	TRIGOFF        stop the trigger
*/
int main(void){

	System_Clock_Init(); // Switch System Clock = 80 MHz
	UART2_Init();
	Trigger_Init();
	Capture_Init();

	while(1){
		if (line_ready)
			Cmd_Handle();

		if (cap_state == CAP_DONE)
			Capture_Dump();
	}
}
