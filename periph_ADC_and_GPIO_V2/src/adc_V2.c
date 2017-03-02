/*
 * @brief LPC5410x ADC example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2015
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
//const uint32_t ExtClockIn = BOARD_EXTCLKINRATE;
static const uint8_t GPIOPins[] = {21,23};//{23, 24};
volatile int toggle=1;

static volatile int ticks;
static bool sequenceComplete, thresholdCrossed;

#define TICKRATE_HZ (200000)	/* 100 ticks per second */

#if defined(BOARD_NXP_LPCXPRESSO_54102)
#define BOARD_ADC_CH 8

#else
#warning "Using ADC channel 8 for this example, please select for your board"
#define BOARD_ADC_CH 8
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
		/* Manual start for ADC conversion sequence A */
		Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
}

/* SEQ-A interrupt handler */
void ADC_SEQA_IRQHandler(void)
{
	uint32_t pending;

	/* Get pending interrupts */
	pending = Chip_ADC_GetFlags(LPC_ADC);

	/* Sequence A completion interrupt */
	if (pending & ADC_FLAGS_SEQA_INT_MASK) {
		sequenceComplete = true;
	}

	/* Clear any pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, ADC_FLAGS_SEQA_INT_MASK);
}

/* ADC threashold interrupt handler */
void ADC_THCMP_IRQHandler(void)
{
	uint32_t pending;

	/* Get pending interrupts */
	pending = Chip_ADC_GetFlags(LPC_ADC);

	/* Threshold crossing interrupt on ADC input channel */
	if (pending & ADC_FLAGS_THCMP_MASK(BOARD_ADC_CH)) {
		thresholdCrossed = true;
	}

	/* Clear any pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, ADC_FLAGS_THCMP_MASK(BOARD_ADC_CH));
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
	DEBUGOUT("GPIO Pins Initialized \n");
}


void flipBit(void){
		Chip_GPIO_SetPinToggle(LPC_GPIO, 0, GPIOPins[0]);


}

/* main function (C entry point) */
int main(void)
{
	int loop = 1;	/* Prevents unreachable statement warning */
	uint32_t rawSample;
	int j;

	SystemCoreClockUpdate();
	Board_Init();
	Board_GPIO_Init();
	SystemCoreClockUpdate();


	DEBUGSTR("ADC sequencer demo\r\n");

	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC, (ADC_CR_RESOL(0) | ADC_CR_TSAMP(ADC_TSAMP_7CLK5)));

	/* FIXME: Enable ADC, should be at least 10mS after ADC is powered up */
	LPC_ADC->STARTUP = 0x1;
	LPC_ADC->STARTUP = 0x3;	/* Dummy calibration */

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_Calibration(LPC_ADC);

	/* Setup for maximum ADC clock rate using sycnchronous clocking */
	Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);

	/* Optionally, you can setup the ADC to use asycnchronous clocking mode.
	   To enable this, mode use 'LPC_ADC->CTRL |= ADC_CR_ASYNMODE;'.
	   In asycnchronous clocking mode mode, the following functions are
	   used to set and determine ADC rates:
	   Chip_Clock_SetADCASYNCSource();
	   Chip_Clock_SetADCASYNCClockDiv();
	   Chip_Clock_GetADCASYNCRate();
	   clkRate = Chip_Clock_GetADCASYNCRate() / Chip_Clock_GetADCASYNCClockDiv; */

	/* Setup sequencer A for ADC channel 0, EOS interrupt */
#if defined(BOARD_NXP_LPCXPRESSO_54102)
	/* Setup a sequencer to do the following:
	   Perform ADC conversion of ADC channels 0 and 8 only */
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, (ADC_SEQ_CTRL_CHANSEL(BOARD_ADC_CH) |
													ADC_SEQ_CTRL_CHANSEL(BOARD_ADC_CH) | ADC_SEQ_CTRL_MODE_EOS));

	/* ADC input 8 is on PIO0_12 mapped to FUNC2 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, (IOCON_FUNC3 | IOCON_MODE_INACT |
											IOCON_ANALOG_EN));
#endif

	/* Setup threshold 0 low and high values to about 25% and 75% of max
	Chip_ADC_SetThrLowValue(LPC_ADC, 0, 0xf50);	// ((1 * 0xFFF) / 4));
	Chip_ADC_SetThrHighValue(LPC_ADC, 0, 0xf70);// ((3 * 0xFFF) / 4));
	*/

	/* Clear all pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));

	/* Enable ADC overrun and sequence A completion interrupts */
	Chip_ADC_EnableInt(LPC_ADC, (ADC_INTEN_SEQA_ENABLE | ADC_INTEN_OVRRUN_ENABLE));

	/* Use threshold 0 for ADC channel and enable threshold interrupt mode for
	   channel as crossing */

	/* Enable ADC NVIC interrupt */
	NVIC_EnableIRQ(ADC_SEQA_IRQn);
	NVIC_EnableIRQ(ADC_THCMP_IRQn);

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);

	/* This example uses the periodic sysTick to manually trigger the ADC,
	   but a periodic timer can be used in a match configuration to start
	   an ADC sequence without software intervention. */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);


	Chip_Clock_SetSysTickClockDiv((uint32_t) 1 );
	volatile uint32_t SysClock = Chip_Clock_GetSysTickClockRate();
	volatile uint32_t ADCClock = Chip_Clock_GetADCClockRate();
	volatile uint32_t SysDiv= Chip_Clock_GetSysTickClockDiv();
	volatile uint32_t ADCDiv= Chip_Clock_GetADCClockDiv();

	volatile uint32_t ADCDivReal= Chip_ADC_GetDivider(LPC_ADC);

	volatile uint32_t MainClock =  Chip_Clock_GetMainClockRate();
	volatile uint32_t SystemClock = Chip_Clock_GetSystemClockRate();

	DEBUGOUT("GPIO Pins Initialized \n \n");
	DEBUGOUT("System Tick Clock Rate: %10d \n",SysClock);
	DEBUGOUT("System Tick Divider: %d \n \n",SysDiv);
	DEBUGOUT("System Core Clock Rate: %d \n\n",SystemCoreClock);
	DEBUGOUT("ADC Clock Rate: %d \n",ADCClock);
	DEBUGOUT("REAL ADC Divider: %d \n",ADCDivReal);
	DEBUGOUT("ADC Divider: %d \n \n",ADCDiv);

	DEBUGOUT("Main Clock Rate: %d \n", MainClock);
	DEBUGOUT("System Clock Rate: %d \n", SystemClock);



	/* Endless loop */
	while (loop) {
		/* Sleep until something happens */
		__WFI();

		/* Is a conversion sequence complete? */
		if (sequenceComplete) {
			sequenceComplete = false;

			/* Get raw sample data for channels 0-11 */

				rawSample = Chip_ADC_GetDataReg(LPC_ADC, 8);

				/* Show some ADC data */

					if (ADC_DR_RESULT(rawSample)<32){
						if (toggle==0){
						flipBit();
						toggle=1;
						}
					}
					else if(ADC_DR_RESULT(rawSample)>32 && toggle){
						toggle=0;
						flipBit();
					}
					//DEBUGOUT("Toggle = %d \n",toggle);

			}


	}

	/* Should not run to here */
	return 0;
}
