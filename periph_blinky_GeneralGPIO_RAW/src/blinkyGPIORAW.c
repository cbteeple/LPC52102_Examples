/*
 * @brief Blinky example using SysTick and interrupt
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
//#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
const uint32_t ExtClockIn = BOARD_EXTCLKINRATE;
#define TICKRATE_HZ (2400000)	/* 10 ticks per second */
static const uint8_t GPIOPins[] = {21,23};//{23, 24};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/


/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Systick handler ISR */
void SysTick_Handler(void)
{
	//int i;
	//for (i = 0; i < sizeof(GPIOPins); i++) {
		Chip_GPIO_SetPinToggle(LPC_GPIO, 0, GPIOPins[0]);
	//}
	//DEBUGOUT("GPIO Toggled \n");
}


static void Board_GPIO_Init(void)
{

	/* INMUX and IOCON are used by many apps, enable both INMUX and IOCON clock bits here. */
		Chip_Clock_EnablePeriphClock(SYSCON_CLOCK_INPUTMUX);
		Chip_Clock_EnablePeriphClock(SYSCON_CLOCK_IOCON);
		Chip_Clock_EnablePeriphClock(SYSCON_CLOCK_GPIO0);

		/* Sets up DEBUG UART */
		DEBUGINIT();

		/* Initialize GPIO */
		Chip_GPIO_Init(LPC_GPIO);

	int i;

	/* Pin muxing setup as part of board_sysinit */
	for (i = 0; i < sizeof(GPIOPins); i++) {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, GPIOPins[i]);

		//If there's no remainder of i-MOD-2, i is even
		if ( i%2 == 0 )
			Chip_GPIO_SetPinState(LPC_GPIO, 0, GPIOPins[i], (bool) false);
		else
			Chip_GPIO_SetPinState(LPC_GPIO, 0, GPIOPins[i], (bool) true);


	}

	Chip_Clock_SetSysTickClockDiv((uint32_t) 1 );
	volatile uint32_t SysClock = Chip_Clock_GetSysTickClockRate();
	volatile uint32_t ADCClock = Chip_Clock_GetADCClockRate();
	volatile uint32_t SysDiv= Chip_Clock_GetSysTickClockDiv();
	volatile uint32_t ADCDiv= Chip_Clock_GetADCClockDiv();

	volatile uint32_t MainClock =  Chip_Clock_GetMainClockRate();
	volatile uint32_t SystemClock = Chip_Clock_GetSystemClockRate();

	DEBUGOUT("GPIO Pins Initialized \n \n");
	DEBUGOUT("System Tick Clock Rate: %10d \n",SysClock);
	DEBUGOUT("System Tick Divider: %d \n \n",SysDiv);
	DEBUGOUT("System Core Clock Rate: %d \n\n",SystemCoreClock);
	DEBUGOUT("ADC Clock Rate: %d \n",ADCClock);
	DEBUGOUT("ADC Divider: %d \n \n",ADCDiv);
	DEBUGOUT("Main Clock Rate: %d \n", MainClock);
	DEBUGOUT("System Clock Rate: %d \n", SystemClock);


}




/* main function (C entry point) */
int main(void)
{
	int loop = 1;	/* Used to fix the unreachable statement warning */
	Board_GPIO_Init();
	SystemCoreClockUpdate();

	/* Enable SysTick Timer */
	//SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	while (loop) {
		//__WFI();
		Chip_GPIO_SetPinToggle(LPC_GPIO, 0, GPIOPins[0]);
	}

	return 0;
}
