/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include "fsl_common.h"
#include "fsl_port.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_pit.h"

// Camera ISR
#include "flexio_ov7670.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool pitIsrFlag = false;
volatile unsigned long timer0_millis;

static struct timeval offset = { 0, 0 };

/*******************************************************************************
 * Code
 ******************************************************************************/

void PORTB_IRQHandler(void)
{
    /* Clear the interrupt flag for PTB2 */
    PORT_ClearPinsInterruptFlags(PORTB, 1U << BOARD_FLEXIO_VSYNC_PIN_INDEX);
    FLEXIO_Ov7670AsynCallback();
}

/* PIT ISR  */
void PIT_DELAY_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, PIT_TFLG_TIF_MASK);
    timer0_millis++;
    pitIsrFlag = true;
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    /* SIM_SOPT2[27:26]:
     *  00: Clock Disabled
     *  01: MCGFLLCLK, or MCGPLLCLK, or IRC48M
     *  10: OSCERCLK
     *  11: MCGIRCCLK
     */
    CLOCK_SetLpuartClock(2);

    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(BOARD_DEBUG_UART_BASEADDR, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

/* Timers used for delay and millis functions. */
void BOARD_InitTimers(void){
	pit_config_t pitConfig;
	// Init the PIT
	// TIMER 0 is used for millis() replacement
    PIT_GetDefaultConfig(&pitConfig);
    /* Init pit module */
    PIT_Init(PIT, &pitConfig);
    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, MSEC_TO_COUNT(1U, PIT_SOURCE_CLOCK));
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);
    /* Start channel 0 */
    PIT_StartTimer(PIT, kPIT_Chnl_0);
    // TIMER 1 is used for 1 msec delay - delay() replacement
    /* Disable timer interrupts for channel 1 */
	PIT_DisableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
}

unsigned long _millis(void)
{
	unsigned long m;
	// disable interrupt so we can read timer variable
	PIT_DisableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	m = timer0_millis;
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	return m;
}

void _delay_ms(int time)
{
	/* Set timer period for channel 1 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, MSEC_TO_COUNT(time, PIT_SOURCE_CLOCK));
	/* Start channel 1 */
	PIT_StartTimer(PIT, kPIT_Chnl_1);
	while(!PIT_GetStatusFlags(PIT, kPIT_Chnl_1));
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, PIT_TFLG_TIF_MASK);
	PIT_StopTimer(PIT, kPIT_Chnl_1);
}

void _delay_us(int time)
{
	/* Set timer period for channel 1 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(time, PIT_SOURCE_CLOCK));
	/* Start channel 1 */
	PIT_StartTimer(PIT, kPIT_Chnl_1);
	while(!PIT_GetStatusFlags(PIT, kPIT_Chnl_1));
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, PIT_TFLG_TIF_MASK);
	PIT_StopTimer(PIT, kPIT_Chnl_1);
}


// osip_time.c port

int osip_gettimeofday_k82f (struct timeval *tp, void *tz){
	unsigned long timemillis = _millis ();

	tp->tv_sec = (timemillis / 1000) + offset.tv_sec;
	tp->tv_usec = (timemillis - (tp->tv_sec * 1000)) * 1000;
	return 0;
}

int _osip_gettimeofday_realtime_k82f (struct timeval *tp, void *tz)
{
	/* TODO */
	tp->tv_sec = 0;
	tp->tv_usec = 0;
	return 0;
}
