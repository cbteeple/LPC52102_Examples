/*
 * @brief I2C probe example
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

#include <stdlib.h>
#include <string.h>
#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static const char menu[] =
	"**************** I2C Probe Menu ****************\r\n"
	"\t0: Exit Demo\r\n"
	"\t1: Probe for Slave devices\r\n"
	"\t2: Read slave data\r\n"
	"\t3: Write slave data\r\n"
	"\t4: Write/Read slave data\r\n";

#define LPC_I2C_PORT                LPC_I2C0
#define LPC_I2C_INTHAND             I2C0_IRQHandler
#define LPC_IRQNUM                  I2C0_IRQn
#define I2C_CLK_DIVIDER             2
#define I2C_BITRATE                 400000
#define I2C_BUFF_SZ                 256

static uint8_t tx_buff[I2C_BUFF_SZ];
static uint8_t rx_buff[I2C_BUFF_SZ];

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*--------------------------- Menu functions --------------------------------*/

static void ansiterm_bs(void)
{
	DEBUGOUT("\x1B[1D \x1B[1D");
}

/* Print data to console */
static void con_print_data(const uint8_t *dat, int sz)
{
	int i;
	if (!sz) {
		return;
	}
	for (i = 0; i < sz; i++) {
		if (!(i & 0xF)) {
			DEBUGOUT("\r\n%02X: ", i);
		}
		DEBUGOUT(" %02X", dat[i]);
	}
	DEBUGOUT("\r\n");
}

/* Get an integer input from UART */
static int con_get_input(const char *str)
{
	int input_valid = 0;
	int x;
	char ch[16], *ptr;
	int i = 0;

	while (!input_valid) {
		DEBUGOUT("%s", str);
		while (1) {
			x =DEBUGIN();
			DEBUGOUT('Received Input \r\n');
			if (x == EOF) {
				continue;
			}
			if (i >= sizeof(ch) - 2) {
				break;
			}
			if (((x == '\r') || (x == '\n')) && i) {
				DEBUGOUT("\r\n");
				break;
			}
			if (x == '\b') {
				if (i) {
					ansiterm_bs();
					i--;
				}
				continue;
			}
			DEBUGOUT("%c", x);
			ch[i++] = x;
		}
		ch[i] = 0;
		i = strtol(ch, &ptr, 0);
		if (*ptr) {
			i = 0;
			DEBUGOUT("Invalid input. Retry!\r\n");
			continue;
		}
		input_valid = 1;
	}
	return i;
}

static void i2c_rw_input(I2CM_XFER_T *xfer, int ops)
{
	int tmp, i;

	tmp = con_get_input("Enter 7-Bit Slave address: ");
	DEBUGOUT(tmp);
	tmp &= 0xFF;
	memset(xfer, 0, sizeof(I2CM_XFER_T));
	xfer->slaveAddr = tmp;

	if (ops & 1) {
		tmp = con_get_input("Enter number of bytes to read: ");
		tmp &= 0xFF;
		xfer->rxSz = tmp;
		xfer->rxBuff = rx_buff;
	}

	if (ops & 2) {
		tmp = con_get_input("Enter number of bytes to write: ");
		tmp &= 0xFF;
		for (i = 0; i < tmp; i++) {
			DEBUGOUT(" %d:", i + 1);
			tx_buff[i] = con_get_input("Enter Data: ");
		}
		xfer->txSz = tmp;
		xfer->txBuff = tx_buff;
	}
}

static int i2c_menu(void)
{
	DEBUGOUT(menu);
	return con_get_input("Select an option [0 - 4]: ");
}

/*--------------------------- I2C functions ---------------------------------*/

static void Init_I2C_PinMux(void)
{
#if defined(BOARD_NXP_LPCXPRESSO_54102)
	/* Connect the I2C1_SDA and I2C1_SCL signals to port pins  23, 24 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, (IOCON_FUNC1 | IOCON_DIGITAL_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 24, (IOCON_FUNC1 | IOCON_DIGITAL_EN));

#else
	/* Configure your own I2C pin muxing here if needed */
#warning "No I2C pin muxing defined"
#endif
}

static void setupI2CMaster(void)
{
	Chip_I2C_Init(LPC_I2C_PORT);
	Chip_I2C_SetClockDiv(LPC_I2C_PORT, I2C_CLK_DIVIDER);
	Chip_I2CM_SetBusSpeed(LPC_I2C_PORT, I2C_BITRATE);
	Chip_I2CM_Enable(LPC_I2C_PORT);
}

static I2CM_XFER_T      i2c_xfer;
static const uint32_t   int_list = I2C_INTENSET_MSTPENDING | I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR;

static void i2c_io(I2CM_XFER_T *in)
{
	/* Setup I2C transfer */
	i2c_xfer = *in;

	/* Execute I2C transfer */
	Chip_I2CM_Xfer(LPC_I2C_PORT, &i2c_xfer);
	Chip_I2C_EnableInt(LPC_I2C_PORT, int_list);

	while (i2c_xfer.status == I2CM_STATUS_BUSY) {
		__WFI();
	}

	Chip_I2C_DisableInt(LPC_I2C_PORT, int_list);
	in->status = i2c_xfer.status;
}

void LPC_I2C_INTHAND(void)
{
	uint32_t state = Chip_I2C_GetPendingInt(LPC_I2C_PORT);

	/* Error handling */
	if (state & (I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR)) {
		Chip_I2CM_ClearStatus(LPC_I2C_PORT, I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR);
	}

	/* Call I2CM ISR function with the I2C device and transfer rec */
	if (state & I2C_INTENSET_MSTPENDING) {
		Chip_I2CM_XferHandler(LPC_I2C_PORT, &i2c_xfer);
		if (i2c_xfer.status == I2CM_STATUS_OK) {
			Chip_I2C_DisableInt(LPC_I2C_PORT, I2C_INTENSET_MSTPENDING);
		}
	}
}

/*--------------------------- probe functions -------------------------------*/

static void i2c_error(bool abbr, uint32_t i2c_error)
{
	if (abbr == false) {
		DEBUGOUT("I2C error: ");
	}
	switch (i2c_error) {
	case I2CM_STATUS_ERROR:					/* Unknown error condition. */
		DEBUGOUT((abbr == true) ? " er" : "Unknown error\r\n");
		break;

	case I2CM_STATUS_NAK_ADR:				/* No acknowledgement received from slave during address phase. */
		DEBUGOUT((abbr == true) ? " na" : "NAK address\r\n");
		break;

	case I2CM_STATUS_BUS_ERROR:				/* I2C bus error */
		DEBUGOUT((abbr == true) ? " be" : "Bus error\r\n");
		break;

	case I2CM_STATUS_NAK_DAT:				/* No acknowledgement received from slave during address phase. */
		DEBUGOUT((abbr == true) ? " nd" : "NAK data\r\n");
		break;

	case I2CM_STATUS_ARBLOST:				/* Arbitration lost. */
		DEBUGOUT((abbr == true) ? " al" : "Arbitration lost\r\n");
		break;

	case I2CM_STATUS_BUSY:					/* I2C transmistter is busy. */
		DEBUGOUT((abbr == true) ? " bs" : "Busy\r\n");
		break;

	default:
		DEBUGOUT(" --");
	}
}

/* Function that probes all available slaves connected to an I2C bus */
static void i2c_probe_slaves(void)
{
	int i;
	I2CM_XFER_T probe;

	memset(&probe, 0, sizeof(I2CM_XFER_T));

	probe.rxBuff = rx_buff;
	probe.txBuff = tx_buff;
	probe.rxSz   = 1;

	DEBUGOUT("Probing available I2C devices...\r\n");
	DEBUGOUT("\r\n     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	DEBUGOUT("\r\n====================================================");
	for (i = 0; i <= 0x7F; i++) {
		if (!(i & 0x0F)) {
			DEBUGOUT("\r\n%02X  ", i >> 4);
		}
		if ((i <= 7) || (i > 0x78)) {
			DEBUGOUT(" --");
			continue;
		}

		probe.slaveAddr = i;
		memset(tx_buff, 0, sizeof(I2C_BUFF_SZ));
		memset(rx_buff, 0, sizeof(I2C_BUFF_SZ));
		i2c_io(&probe);
		if (probe.status == I2CM_STATUS_OK) {
			DEBUGOUT(" %02X", i);
		}
		else {
			i2c_error(true, probe.status);
		}
	}
	DEBUGOUT("\r\n");
}

/*--------------------------- Main function ---------------------------------*/

int main(void)
{
	bool xflag = false;
	I2CM_XFER_T xfer;

	SystemCoreClockUpdate();
	Board_Init();

	Init_I2C_PinMux();
	setupI2CMaster();
	NVIC_EnableIRQ(LPC_IRQNUM);

	while (xflag == false) {

		memset(tx_buff, 0, sizeof(I2C_BUFF_SZ));
		memset(rx_buff, 0, sizeof(I2C_BUFF_SZ));

		switch (1){   //i2c_menu()) {
		case 0:
			xflag = true;
			DEBUGOUT("End of I2C Demo! Bye!\r\n");
			break;

		case 1:
			i2c_probe_slaves();
			DEBUGOUT("\r\n");
			break;

		case 2:
			i2c_rw_input(&xfer, 0x01);
			i2c_io(&xfer);
			if (xfer.status == I2CM_STATUS_OK) {
				DEBUGOUT("Read %d bytes of data from slave 0x%02X.\r\n", xfer.rxSz, xfer.slaveAddr);
				con_print_data(&xfer.rxBuff[0], xfer.rxSz);
			}
			else {
				i2c_error(false, xfer.status);
			}
			DEBUGOUT("\r\n");
			break;

		case 3:
			i2c_rw_input(&xfer, 0x02);
			i2c_io(&xfer);
			if (xfer.status == I2CM_STATUS_OK) {
				DEBUGOUT("Written %d bytes of data to slave 0x%02X.\r\n", xfer.txSz, xfer.slaveAddr);
			}
			else {
				i2c_error(false, xfer.status);
			}
			DEBUGOUT("\r\n");
			break;

		case 4:
			i2c_rw_input(&xfer, 0x03);
			i2c_io(&xfer);
			if (xfer.status == I2CM_STATUS_OK) {
				DEBUGOUT("Read %d bytes of data from slave 0x%02X.\r\n", xfer.rxSz, xfer.slaveAddr);
				con_print_data(&xfer.rxBuff[0], xfer.rxSz);
			}
			else {
				i2c_error(false, xfer.status);
			}
			DEBUGOUT("\r\n");
			break;

		default:
			DEBUGOUT("Input Invalid! Try Again.\r\n");
		}
	}

	return 0;
}
