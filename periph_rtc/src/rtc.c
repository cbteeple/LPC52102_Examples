/*
 * @brief RTC example with wakeup
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"

/* Select a mode of type 'POWER_MODE_T' for this example: mode available are
   POWER_SLEEP, POWER_DEEP_SLEEP, POWER_POWER_DOWN*/
#define PDOWNMODE   (POWER_POWER_DOWN)

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/* Saved clock source */
static  CHIP_SYSCON_MAINCLKSRC_T saved_clksrc;

/* RTC interrupt flags */
static volatile bool rtcWake, rtcAlarm;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* RTC interrupt handler */
void RTC_IRQHandler(void)
{
	uint32_t rtcStatus;

	Board_LED_Toggle(0);

	/* Get RTC status register */
	rtcStatus = Chip_RTC_GetStatus(LPC_RTC);

	/* Check RTC 1KHz match interrupt */
	if (rtcStatus & RTC_CTRL_WAKE1KHZ) {
		/* RTC high resultiuon wakeup interrupt */
		rtcWake = true;
	}

	/* Check RTC 1Khz match interrupt */
	if (rtcStatus & RTC_CTRL_ALARM1HZ) {
		/* Alarm */
		rtcAlarm = true;
	}

	/* Clear only latched RTC status */
	Chip_RTC_ClearStatus(LPC_RTC, (rtcStatus & (RTC_CTRL_WAKE1KHZ | RTC_CTRL_ALARM1HZ)));
}

/* main function (C entry point) */
int main(void)
{
	int loop = 1;	/* Used to prevent unreachable statement warning */
	int stateCounter = 0;

	/* Setup SystemCoreClock and any needed board code */
	SystemCoreClockUpdate();
	Board_Init();
	Board_LED_Set(0, false);

	/* Turn on the RTC 32K Oscillator */
	Chip_SYSCON_PowerUp(SYSCON_PDRUNCFG_PD_32K_OSC);

	/* Enable the RTC oscillator, oscillator rate can be determined by
	   calling Chip_Clock_GetRTCOscRate()	*/
	Chip_Clock_EnableRTCOsc();

	/* Initialize RTC driver (enables RTC clocking) */
	Chip_RTC_Init(LPC_RTC);

	/* Enable RTC as a peripheral wakeup event */
	Chip_SYSCON_EnableWakeup(SYSCON_STARTER_RTC);

	/* RTC reset */
	Chip_RTC_Reset(LPC_RTC);

	/* Start RTC at a count of 0 when RTC is disabled. If the RTC is enabled, you
	   need to disable it before setting the initial RTC count. */
	Chip_RTC_Disable(LPC_RTC);
	Chip_RTC_SetCount(LPC_RTC, 0);

	/* Set a long alarm time so the interrupt won't trigger */
	Chip_RTC_SetAlarm(LPC_RTC, 1000);

	/* Enable RTC and high resolution timer - this can be done in a single
	   call with Chip_RTC_EnableOptions(LPC_RTC, (RTC_CTRL_RTC1KHZ_EN | RTC_CTRL_RTC_EN)); */
	Chip_RTC_Enable1KHZ(LPC_RTC);
	Chip_RTC_Enable(LPC_RTC);

	/* Clear latched RTC interrupt statuses */
	Chip_RTC_ClearStatus(LPC_RTC, (RTC_CTRL_OFD | RTC_CTRL_ALARM1HZ | RTC_CTRL_WAKE1KHZ));

	/* Enable RTC interrupt */
	NVIC_EnableIRQ(RTC_IRQn);

	/* save the clock source, power down the PLL */
	saved_clksrc = Chip_Clock_GetMainClockSource();

	/* Enable RTC alarm interrupt */
	Chip_RTC_EnableWakeup(LPC_RTC, (RTC_CTRL_ALARMDPD_EN | RTC_CTRL_WAKEDPD_EN));

	/* Sleep and do all the work in the RTC interrupt handler */
	while (loop) {
		DEBUGOUT("Tick number: %d, 1KHZ int:%d, alarm int:%d\r\n",
				 stateCounter, rtcWake, rtcAlarm);
		rtcWake = rtcAlarm = false;

		/* 10 high resolution ticks that get slower each tick */
		if (stateCounter < 10) {
			/* Wakeup in 300, 400, 500, etc. milliSeconds */
			Chip_RTC_SetWake(LPC_RTC, (300 + (stateCounter * 100)));
			stateCounter++;
		}
		else {
			DEBUGOUT("Setting alarm to wake up in 4s\r\n");
			/* Set alarm to wakeup in 4 seconds */
			Chip_RTC_SetAlarm(LPC_RTC, Chip_RTC_GetCount(LPC_RTC) + 4);
			stateCounter = 0;
		}

		/* Disable PLL, if previously enabled, prior to sleep */
		if (saved_clksrc == SYSCON_MAINCLKSRC_PLLOUT) {
			Chip_Clock_SetMainClockSource(SYSCON_MAINCLKSRC_IRC);
			Chip_SYSCON_PowerDown(SYSCON_PDRUNCFG_PD_SYS_PLL);
		}

		Chip_POWER_SetVoltage(POWER_LOW_POWER_MODE, Chip_Clock_GetMainClockRate());

		/* Before going to sleep or power down,
		    make sure that the UART has completed transmission */
		while ((LPC_USART0->STAT & UART_TXIDLE) == 0) {}

		/* Go to sleep leaving SRAM and the 32K oscillator powered during sleep. Use lower
		    voltage during sleep. */
		Chip_POWER_EnterPowerMode(PDOWNMODE,
								  (SYSCON_PDRUNCFG_PD_SRAM0A | SYSCON_PDRUNCFG_PD_SRAM0B | SYSCON_PDRUNCFG_PD_32K_OSC));

		/* On wakeup, restore PLL power if needed */
		if (saved_clksrc == SYSCON_MAINCLKSRC_PLLOUT) {
			Chip_SYSCON_PowerUp(SYSCON_PDRUNCFG_PD_SYS_PLL);

			/* Wait for PLL lock */
			while (!Chip_Clock_IsSystemPLLLocked()) {}

			Chip_POWER_SetVoltage(POWER_LOW_POWER_MODE, Chip_Clock_GetSystemPLLOutClockRate(false));

			/* Use PLL for system clock */
			Chip_Clock_SetMainClockSource(SYSCON_MAINCLKSRC_PLLOUT);
		}
	}
	return 0;
}
