/*
 * @brief Brown-out detector example
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

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Brown-out detector interrupt */
void BOD_IRQHandler(void)
{
	/* Turn on LED */
	Board_LED_Set(1, true);
}

/* Main function (C entry point) */
int main(void)
{
	int loop = 1;	/* Prevents unreachable statement warning */
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);

	/* If the board was reset due to a BOD event, the reset can be
	   detected here. Turn on LED0 if reset occured due to BOD. */
	if ((Chip_SYSCON_GetSystemRSTStatus() & SYSCON_RST_BOD) != 0) {
		Chip_SYSCON_ClearSystemRSTStatus(SYSCON_RST_BOD);
		Board_LED_Set(0, true);
	}

	/* Enable BOD power */
	Chip_SYSCON_PowerUp(SYSCON_PDRUNCFG_PD_BOD_RST | SYSCON_PDRUNCFG_PD_BOD_INTR);

	/* Set BOD detection interrupt to 3.05v and device reset to 1.5v */
	Chip_PMU_SetBODLevels(PMU_BODRSTLVL_1_50V, PMU_BODINTVAL_3_05v);

	/* Enable BOD reset and interrupt on low power level */
	Chip_PMU_EnableBODReset();
	Chip_PMU_EnableBODInt();

	/* Enable BOD interrupt */
	NVIC_EnableIRQ(BOD_IRQn);

	/* Wait forever */
	while (loop) {
		__WFI();
	}

	return 0;
}
