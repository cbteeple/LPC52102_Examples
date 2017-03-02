/*
 * @brief SPI slave with DMA example
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

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define APP_SPI_CH 				LPC_SPI0
#define APP_SPI_DMA_RXCH		DMAREQ_SPI0_RX
#define APP_SPI_DMA_TXCH		DMAREQ_SPI0_TX
#define RX_LENGTH				4

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
/* DMA descriptors must be aligned to 16 bytes */
#if defined(__CC_ARM)	/* Keil support */
__align(16) static DMA_CHDESC_T dmaSPISTxDesc[2];
__align(16) static DMA_CHDESC_T dmaSPISRxDesc[2];
#elif defined(__ICCARM__)	/* IAR support */
#pragma data_alignment=16
static DMA_CHDESC_T dmaSPISTxDesc[2];
#pragma data_alignment=16
static DMA_CHDESC_T dmaSPISRxDesc[2];
#elif defined( __GNUC__ )	/* GCC support */
static DMA_CHDESC_T dmaSPISTxDesc[2] __attribute__ ((aligned(16)));
static DMA_CHDESC_T dmaSPISRxDesc[2] __attribute__ ((aligned(16)));
#endif /* defined (__GNUC__) */

static uint8_t spiBuff1[RX_LENGTH];
static uint8_t spiBuff2[RX_LENGTH] = {30, 31, 32, 33};

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void spis_dma_Start(void);
static void dmaClearChannel(DMA_CHID_T ch);

/*****************************************************************************
 * Public functions
 ****************************************************************************/
static void Hostif_Init(void)
{
	SPI_CFGSETUP_T spiSetup;

	/* Initialize SPI controllers */
	Chip_SPI_Init(APP_SPI_CH);

	/* Call to initialize second SPI controller for mode0, slave mode,
	   MSB first */
	spiSetup.master = 0;
	spiSetup.lsbFirst = 0;
	spiSetup.mode = SPI_CLOCK_MODE0;
	Chip_SPI_ConfigureSPI(APP_SPI_CH, &spiSetup);

	/* Setup slave controller SSEL0 for active low select */
	Chip_SPI_SetCSPolLow(APP_SPI_CH, 0);

	/* Setup slave controller for 8-bit transfer.  */
	Chip_SPI_SetXferSize(APP_SPI_CH, 8);

	/* Initialize DMA controller */
	/* DMA initialization - enable DMA clocking and reset DMA if needed */
	Chip_DMA_Init(LPC_DMA);

	/* Enable DMA controller and use driver provided DMA table for current descriptors */
	Chip_DMA_Enable(LPC_DMA);
	Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));

	/* Setup DMA channels for SPI slave send and receive */
	/* Setup SPI slave channel for the following configuration:
	   - Peripheral DMA request
	   - Single transfer
	   - High channel priority */
	Chip_DMA_EnableChannel(LPC_DMA, APP_SPI_DMA_TXCH);
	Chip_DMA_EnableChannel(LPC_DMA, APP_SPI_DMA_RXCH);
	Chip_DMA_EnableIntChannel(LPC_DMA, APP_SPI_DMA_TXCH);
	Chip_DMA_EnableIntChannel(LPC_DMA, APP_SPI_DMA_RXCH);
	Chip_DMA_SetupChannelConfig(LPC_DMA, APP_SPI_DMA_TXCH,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(0)));
	Chip_DMA_SetupChannelConfig(LPC_DMA, APP_SPI_DMA_RXCH,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(0)));


	/* enable SPI Rx */
	spis_dma_Start();
	Chip_SPI_Enable(APP_SPI_CH);

	/* DMA interrupt is only used for error handling */
	NVIC_EnableIRQ(DMA_IRQn);

}

/* Start a DMA slave write operation */
static void spis_dma_Start(void )
{
	/* Slave to master */
	dmaSPISRxDesc[0].source = DMA_ADDR(&APP_SPI_CH->RXDAT);
	dmaSPISRxDesc[0].dest = DMA_ADDR(&spiBuff1[0]) + RX_LENGTH - 1;
	dmaSPISRxDesc[0].next = DMA_ADDR(&dmaSPISRxDesc[1]);
	dmaSPISRxDesc[0].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_SWTRIG | DMA_XFERCFG_RELOAD | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_DSTINC_1 |
							   DMA_XFERCFG_SRCINC_0 | DMA_XFERCFG_XFERCOUNT(RX_LENGTH);
	/* Slave to master */
	dmaSPISTxDesc[0].source = DMA_ADDR(&spiBuff2[0]) + RX_LENGTH - 1;
	dmaSPISTxDesc[0].dest = DMA_ADDR(&APP_SPI_CH->TXDAT);
	dmaSPISTxDesc[0].next = DMA_ADDR(&dmaSPISTxDesc[1]);
	dmaSPISTxDesc[0].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_SWTRIG | DMA_XFERCFG_RELOAD | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_1 |
							   DMA_XFERCFG_DSTINC_0 | DMA_XFERCFG_XFERCOUNT(RX_LENGTH);

	/* Set the descriptors to alternate between rx and tx (i.e. transmit the last thing that was received) */
	dmaSPISRxDesc[1].source = DMA_ADDR(&APP_SPI_CH->RXDAT);
	dmaSPISRxDesc[1].dest = DMA_ADDR(&spiBuff2[0]) + RX_LENGTH - 1;
	dmaSPISRxDesc[1].next = DMA_ADDR(&dmaSPISRxDesc[0]);
	dmaSPISRxDesc[1].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_SWTRIG | DMA_XFERCFG_RELOAD | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_DSTINC_1 |
							   DMA_XFERCFG_SRCINC_0 | DMA_XFERCFG_XFERCOUNT(RX_LENGTH);

	dmaSPISTxDesc[1].source = DMA_ADDR(&spiBuff1[0]) + RX_LENGTH - 1;
	dmaSPISTxDesc[1].dest = DMA_ADDR(&APP_SPI_CH->TXDAT);
	dmaSPISTxDesc[1].next = DMA_ADDR(&dmaSPISTxDesc[0]);
	dmaSPISTxDesc[1].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_SWTRIG | DMA_XFERCFG_RELOAD | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_1 |
							   DMA_XFERCFG_DSTINC_0 | DMA_XFERCFG_XFERCOUNT(RX_LENGTH);

	/* Reset DMA Tx, Rx Channel */
	dmaClearChannel(APP_SPI_DMA_TXCH);
	dmaClearChannel(APP_SPI_DMA_RXCH);

	/* Flush any pre loaded SPI data */
	Chip_SPI_FlushFifos(APP_SPI_CH);

	/* Setup transfer descriptor and validate it */
	while (!Chip_DMA_SetupTranChannel(LPC_DMA, APP_SPI_DMA_TXCH, &dmaSPISTxDesc[0])) {}

	/* Setup data transfer */
	Chip_DMA_SetupChannelTransfer(LPC_DMA, APP_SPI_DMA_TXCH, dmaSPISTxDesc[0].xfercfg);
	Chip_DMA_EnableChannel(LPC_DMA, APP_SPI_DMA_TXCH);
	Chip_DMA_SetValidChannel(LPC_DMA, APP_SPI_DMA_TXCH);

	/* Setup transfer descriptor for Rx and validate it */
	while (!Chip_DMA_SetupTranChannel(LPC_DMA, APP_SPI_DMA_RXCH, &dmaSPISRxDesc[0])) {}

	/* Setup data transfer */
	Chip_DMA_SetupChannelTransfer(LPC_DMA, APP_SPI_DMA_RXCH, dmaSPISRxDesc[0].xfercfg);
	Chip_DMA_EnableChannel(LPC_DMA, APP_SPI_DMA_RXCH);

	Chip_DMA_SetValidChannel(LPC_DMA, APP_SPI_DMA_RXCH);
}

/** SPI slave Tx DMA callback */
static void Hostif_SpiDmaTxCB(bool error)
{
	if (error) {
		/* Set LED to indicate error */
		Board_LED_Set(0, true);

		/* Restart the dma */
		spis_dma_Start();
	}
}

/** SPI slave Rx DMA callback */
static void Hostif_SpiDmaRxCB(bool error)
{
	if (error) {
		/* Set LED to indicate error */
		Board_LED_Set(0, true);

		/* Restart the dma */
		spis_dma_Start();
	}
}

/* Reset DMA Channel */
static void dmaClearChannel(DMA_CHID_T ch)
{
	Chip_DMA_DisableChannel(LPC_DMA, ch);
	while ((Chip_DMA_GetBusyChannels(LPC_DMA) & (1 << ch)) != 0) {}

	Chip_DMA_AbortChannel(LPC_DMA, ch);
	Chip_DMA_ClearErrorIntChannel(LPC_DMA, ch);
}

/**
 * @brief	DMA Interrupt Handler
 * @return	None
 */
void DMA_IRQHandler(void)
{
	uint32_t errors, pending;

	/* Get DMA error and interrupt channels */
	errors = Chip_DMA_GetErrorIntChannels(LPC_DMA);
	pending = Chip_DMA_GetActiveIntAChannels(LPC_DMA);

	/* Check DMA interrupts of SPI slave Tx channel */
	if ((errors | pending) & (1 << APP_SPI_DMA_TXCH)) {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, APP_SPI_DMA_TXCH);

		Hostif_SpiDmaTxCB((errors & (1 << APP_SPI_DMA_TXCH)) != 0);
	}
	/* Check DMA interrupts of SPI slave Rx channel */
	if ((errors | pending) & (1 << APP_SPI_DMA_RXCH)) {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, APP_SPI_DMA_RXCH);

		Hostif_SpiDmaRxCB((errors & (1 << APP_SPI_DMA_RXCH)) != 0);
	}
}

/* main function (C entry point) */
int main(void)
{
	int loop = 1;	/* Used to fix the unreachable statement warning */
	SystemCoreClockUpdate();
	Board_Init();

	/* Turn LED off for default condition */
	Board_LED_Set(0, false);

	/* Enable SPI and DMA interfaces */
	Hostif_Init();

	while (loop) {
		__WFI();
	}

	return 0;
}
