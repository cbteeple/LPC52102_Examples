/*
 * @brief Frequency Measurement example
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

#include "board.h"
#include <stdio.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Measurement cycle with value display */
static uint32_t measureDisplay(char *str, FREQMSR_SRC_T src, uint32_t freqRef, uint32_t freqExp)
{
	uint32_t freqComp;

	/* Setup to measure the selected target */
	Chip_INMUX_SetFreqMeasTargClock(src);

	/* Start a measurement cycle and wait for it to complete. If the target
	   clock is not running, the measurement cycle will remain active
	   forever, so a timeout may be necessary if the target clock can stop */
	Chip_SYSCON_StartFreqMeas();
	while (!Chip_SYSCON_IsFreqMeasComplete()) {}

	/* Get computed frequency */
	freqComp = Chip_SYSCON_GetCompFreqMeas(freqRef);

	/* Show the raw capture value and the compute frequency */
	DEBUGOUT("Capture source : %s, reference frequency = %dHz\r\n", str, freqRef);
	DEBUGOUT("Raw frequency capture value = %d\r\n", Chip_SYSCON_GetRawFreqMeasCapval());
	DEBUGOUT("Computed frequency value = %dHz\r\n", freqComp);
	DEBUGOUT("Expected frequency value = %dHz\r\n\r\n", freqExp);

	return freqComp;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/* main function (C entry point) */
int main(void)
{
	int loop = 1;	/* used to avoid unreachable statement warning */
	uint32_t freqRef;

	/* Board Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Note that the accuracy of a target measurement requires a reference
	   clock rate faster than the target clock. */

	/* Powerup WDT oscillator */
	Chip_SYSCON_PowerUp(SYSCON_PDRUNCFG_PD_WDT_OSC);

	/* Setup to use the main oscillator for the frequency reference since
	   we already know this rate */
	Chip_INMUX_SetFreqMeasRefClock(FREQ_MEAS_MAIN_CLK);
	freqRef = Chip_Clock_GetMainClockRate();

	/* Start RTC oscillator frequency measurement and display results */
	measureDisplay("RTC32K oscillator (main clock reference)", FREQMSR_32KHZOSC,
				   freqRef, Chip_Clock_GetRTCOscRate());

	/* Start watchdog oscillator frequency measurement and display results */
	measureDisplay("Watchdog oscillator (main clock reference)", FREQMSR_WDOSC,
				   freqRef, Chip_Clock_GetWDTOSCRate());

	/* Start IRC oscillator frequency measurement and display results. This may not be
	   accuare as it will be too close to the main oscillator frequency. */
	measureDisplay("IRC oscillator (main clock reference)", FREQMSR_IRC, freqRef,
				   Chip_Clock_GetIntOscRate());

	while (loop) {
		__WFI();
	}

	return 0;
}
