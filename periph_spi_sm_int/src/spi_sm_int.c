/*
 * @brief SPI master and slave example
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
#include "stopwatch.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Stopwatch timers for master and slave transfer time */
static uint32_t masterTime, errors;

/* Flags for determing when master and slave transfers end */
static volatile bool mEnd, sEnd;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/* Local variables, made global to make easier to debug */

/* Buffer sizes for this example and total transfer size */
#define BUFFER_SIZE 16

/* Master transmit and receive buffers */
uint16_t masterRXBuffer16[BUFFER_SIZE], masterTXBuffer16[BUFFER_SIZE];

/* SPI master transfer descriptor */
SPIM_XFER_T spiMasterXfer;

/* Slave transmit and receive buffers */
uint16_t slaveRXBuffer16[BUFFER_SIZE], slaveTXBuffer16[BUFFER_SIZE];

/* SPI slave transfer descriptor */
SPIS_XFER_T spiSlaveXfer;

/* Board specific example setup */
#define SPIMASTERIRQHANDLER                 SPI0_IRQHandler
#define LPC_SPIMASTERPORT                   LPC_SPI0
#define LPC_SPIMASTERIRQNUM                 SPI0_IRQn
#define SPISLAVEIRQHANDLER                  SPI1_IRQHandler
#define LPC_SPISLAVEPORT                    LPC_SPI1
#define LPC_SPISLAVEIRQNUM                  SPI1_IRQn

/* The slave can handle data up to the point of overflow or underflow.
   Adjust this clock rate and the master's timing delays to get a working
   SPI slave rate. */
#define LPCMASTERCLOCKRATE                  (4000000)

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes pin muxing for SPI1 interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_SPI_PinMux(void)
{
#if defined(BOARD_NXP_LPCXPRESSO_54102)
	/* Connect the SPI0 signals to port pins */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 11, (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI0_SCK */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI0_MOSI */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 13, (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI0_MISO */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 14, (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI0_SSEL0 */

	/* Connect the SPI1 signals to port pins */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 15,  (IOCON_FUNC4 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SSEL0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 6,   (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_SCK */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 7,   (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_MOSI */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 14,  (IOCON_FUNC4 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN));	/* SPI1_MISO */
#else
	#warning    "Board pin muxing not available!"
#endif
}

/* Initialize buffers for slave and master */
static uint16_t bufferInit(uint16_t seed)
{
	int i;

	for (i = 0; i < BUFFER_SIZE; i++) {
		seed = seed + 1;

		/* Clear RX buffers, so we know something was received */
		slaveRXBuffer16[i] = masterRXBuffer16[i] = 0;

		/* Seed data for transmit buffers */
		masterTXBuffer16[i] = seed;
		slaveTXBuffer16[i] = ~seed;
	}

	return seed;
}

void showData(char *reg, uint16_t *dat)
{
	int i, j;

	DEBUGOUT("Showing data from : %s\r\n", reg);

	/* Hard coded for a 2x8 16-bit displays */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 8; j++) {
			DEBUGOUT("0x%04x ", *dat);
			dat++;
		}

		DEBUGSTR("\r\n");
	}
}

/* Callback for slave spi transfers */
static int slave_cb(SPIS_EVENT_T event, struct SPIS_XFER *xfer)
{
	switch (event) {
	case SPIS_EVT_SSELASSERT:
		/* Handle Slave select here */
		break;

	case SPIS_EVT_SSELDEASSERT:
		/* Handle Slave de-select here */
		break;

	case SPIS_EVT_TXDONE:
		/* Handle Slave transfer done here */
		break;

	case SPIS_EVT_RXDONE:
		/* Handle Slave receive done here */
		break;
	}
	return 0;
}

/* Callback for master SPI transfers */
static int master_cb(SPIM_EVENT_T event, struct SPIM_XFER *xfer)
{
	switch (event) {
	case SPIM_EVT_SSELASSERT:
		/* Handle Master select here */
		break;

	case SPIM_EVT_SSELDEASSERT:
		/* Handle Master de-select here */
		break;

	case SPIM_EVT_TXDONE:
		/* Handle Master transfer done here */
		break;

	case SPIM_EVT_RXDONE:
		/* Handle Master receive done here */
		break;
	}
	return 0;
}

/* Setup master controller */
static void setupMaster(void)
{
	SPI_CFGSETUP_T spiSetup;
	SPIM_DELAY_CONFIG_T masterDelay;

	/* Initialize SPI controller */
	Chip_SPI_Init(LPC_SPIMASTERPORT);

	/* Call to initialize first SPI controller for mode0, master mode,
	   MSB first */
	Chip_SPI_Enable(LPC_SPIMASTERPORT);
	spiSetup.master = 1;
	spiSetup.lsbFirst = 0;
	spiSetup.mode = SPI_CLOCK_MODE0;
	Chip_SPI_ConfigureSPI(LPC_SPIMASTERPORT, &spiSetup);

	/* Setup master controller SSEL0 for active low select */
	Chip_SPI_SetCSPolLow(LPC_SPIMASTERPORT, 0);

	/* Setup master clock rate, slave clock doesn't need to be setup */
	Chip_SPIM_SetClockRate(LPC_SPIMASTERPORT, LPCMASTERCLOCKRATE);

	/* Setup master delay (all chip selects) */
	masterDelay.PreDelay = 0xD;
	masterDelay.PostDelay = 0xD;
	masterDelay.FrameDelay = 0xD;
	masterDelay.TransferDelay = 0xD;
	Chip_SPIM_DelayConfig(LPC_SPIMASTERPORT, &masterDelay);

	/* For the SPI controller configured in master mode, enable SPI master interrupts
	   for interrupt service. Do not enable SPI_INTENSET_TXDYEN. */
	Chip_SPI_EnableInts(LPC_SPIMASTERPORT, (SPI_INTENSET_RXDYEN |
											SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
											SPI_INTENSET_SSDEN));
}

/* Setup slave controller */
static void setupSlave(void)
{
	SPI_CFGSETUP_T spiSetup;

	/* Initialize SPI controllers */
	Chip_SPI_Init(LPC_SPISLAVEPORT);

	/* Call to initialize second SPI controller for mode0, slave mode,
	   MSB first */
	Chip_SPI_Enable(LPC_SPISLAVEPORT);
	spiSetup.master = 0;
	spiSetup.lsbFirst = 0;
	spiSetup.mode = SPI_CLOCK_MODE0;
	Chip_SPI_ConfigureSPI(LPC_SPISLAVEPORT, &spiSetup);

	/* Setup slave controller SSEL0 for active low select */
	Chip_SPI_SetCSPolLow(LPC_SPISLAVEPORT, 0);

	/* Setup slave controller for 8-bit transfer. Sizes can be altered later
	   for each slave select in the slave select assertion callback once the
	     transfer starts if needed. The master slave transfer size is setup as
	   part of the master transfer description in the options field. */
	Chip_SPI_SetXferSize(LPC_SPISLAVEPORT, 16);

	/* For the SPI controller configured in slave mode, enable SPI slave interrupts
	   for interrupt service. Do not enable SPI_INTENSET_TXDYEN. */
	Chip_SPI_EnableInts(LPC_SPISLAVEPORT, (SPI_INTENSET_RXDYEN |
										   SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
										   SPI_INTENSET_SSDEN));

}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	SPI0 interrupt handler sub-routine (master)
 * @return	Nothing
 */
void SPIMASTERIRQHANDLER(void)
{
	uint32_t ints = Chip_SPI_GetPendingInts(LPC_SPIMASTERPORT);

	/* Handle SPI master interrupts only */
	if ((ints & (SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN |
				 SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0) {
		/* SPI master handler */
		Chip_SPIM_XferHandler(LPC_SPIMASTERPORT, &spiMasterXfer);
	}
}

/**
 * @brief	SPI1 interrupt handler sub-routine (slave)
 * @return	Nothing
 */
void SPISLAVEIRQHANDLER(void)
{
	uint32_t ints = Chip_SPI_GetPendingInts(LPC_SPISLAVEPORT);

	/* Handle SPI slave interrupts only */
	if ((ints & (SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN |
				 SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0) {
		/* SPI slave handler */
		errors |= Chip_SPIS_XferHandler(LPC_SPISLAVEPORT, &spiSlaveXfer);
	}
}

/* main application function (C entry point) */
int main(void)
{
	int loop = 1;	/* Prevents unreachable statement warning */
	uint16_t seed = 0;
	volatile uint8_t *mstate, *sstate;

	SystemCoreClockUpdate();
	Board_Init();

	/* SPI initialization */
	Init_SPI_PinMux();

	/* Initialize stopwatch driver so some event times can be measured */
	StopWatch_Init();

	/* Setup SPI controllers */
	setupMaster();
	setupSlave();

	/* Enable SPI controller interrupts */
	NVIC_EnableIRQ(LPC_SPIMASTERIRQNUM);
	NVIC_EnableIRQ(LPC_SPISLAVEIRQNUM);

	DEBUGOUT("SPI master/slave combined example\r\n");

	/* Loop forever */
	while (loop) {
		/* Setup some data for transmit from master to slave and slave to master */
		seed = bufferInit(seed);

		/* Set slave transfer, this is only the initial trasnfer, the callbacks can
		   change this later */
		spiSlaveXfer.cbFunc = slave_cb;
		spiSlaveXfer.state = SPIS_XFER_STATE_IDLE;
		spiSlaveXfer.txBuff = slaveTXBuffer16;
		spiSlaveXfer.txCount = sizeof(slaveTXBuffer16) / sizeof(uint16_t);	/* Count is in transfer size */
		spiSlaveXfer.rxBuff = slaveRXBuffer16;
		spiSlaveXfer.rxCount = sizeof(slaveRXBuffer16) / sizeof(uint16_t);	/* Count is in transfer size */
		spiSlaveXfer.rxDoneCount = spiSlaveXfer.txDoneCount = 0;
		sstate = &spiSlaveXfer.state;

		/* Set master transfer, this is only the initial trasnfer, the callbacks can
		   change this later */
		spiMasterXfer.cbFunc = master_cb;
		spiMasterXfer.state = SPIS_XFER_STATE_IDLE;
		spiMasterXfer.txBuff = masterTXBuffer16;
		spiMasterXfer.txCount = sizeof(masterTXBuffer16) / sizeof(uint16_t);/* Count is in transfer size */
		spiMasterXfer.rxBuff = masterRXBuffer16;
		spiMasterXfer.rxCount = sizeof(masterRXBuffer16) / sizeof(uint16_t);/* Count is in transfer size */
		mstate = &spiMasterXfer.state;

		/* Setup master trasnfer options - 16 data bits per transfer, EOT, EOF */
		spiMasterXfer.options =
			SPIM_XFER_OPTION_SIZE(16) |		/* This must be enabled as a minimum, use 16 data bits */
			SPIM_XFER_OPTION_EOT |			/* Enable this to assert and deassert SSEL for each individual byte/word, current slave functions for this example do not support this */
			SPIM_XFER_OPTION_EOF |			/* Insert a delay between bytes/words as defined by frame delay time */
			0;

		/* Use SPI select 0 */
		spiMasterXfer.sselNum = 0;

		/* Time master and slave transfers */
		masterTime = StopWatch_Start();

		/* Limitation: The call below 'pre-buffers' the initial slave transmit datum.
		   If this isn't pre-buffered, a slave transmit underflow will always occur
		     at slave assertion time for the initial transmit datum. The datum sent to the
		     master will be 0. This is ok as we are only using a single slave, but with multiple
		     slaves pre-buffering is not always an option and the master might need to toss the
		     first byte. */
		Chip_SPI_FlushFifos(LPC_SPIMASTERPORT);
		Chip_SPI_FlushFifos(LPC_SPISLAVEPORT);
		Chip_SPIS_PreBuffSlave(LPC_SPISLAVEPORT, &spiSlaveXfer);

		/* Start master transfer */
		Chip_SPIM_Xfer(LPC_SPIMASTERPORT, &spiMasterXfer);

		/* Sleep until transfers are complete */
		while ((*mstate != SPIM_XFER_STATE_DONE) || (*sstate != SPIS_XFER_STATE_DONE)) {
			__WFI();
		}

		/* Toggle LED */
		Board_LED_Toggle(0);

		DEBUGOUT("\r\n SLAVE [txDoneCount = %d, rxDoneCount = %d]\r\n MASTER [txDoneCount = %d, rxDoneCount = %d]\r\n",
				 spiSlaveXfer.txDoneCount,
				 spiSlaveXfer.rxDoneCount,
				 spiMasterXfer.txDoneCount,
				 spiMasterXfer.rxDoneCount);
		/* Display some information about the transfer */
		DEBUGOUT("\r\nTRANSFER COMPLETE: errors = %x\r\n", errors);
		errors = 0;
		DEBUGOUT("Master total transfer time = %duS\r\n", StopWatch_TicksToUs(StopWatch_Elapsed(masterTime)));

		/* Show data */
		showData("Master TX data", masterTXBuffer16);
		showData("Master RX data", masterRXBuffer16);
		showData("Slave  TX data", slaveTXBuffer16);
		showData("Slave  RX data", slaveRXBuffer16);

		while (Board_UARTGetChar() == EOF) {}
	}

	/* DeInitialize SPI peripherals, never called in this example */
	Chip_SPI_DeInit(LPC_SPIMASTERPORT);
	Chip_SPI_DeInit(LPC_SPISLAVEPORT);

	return 0;
}
