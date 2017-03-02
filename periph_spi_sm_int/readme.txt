LPC5410x SPI master and slave example using spim (master) and spis (slave) drivers
========================================================

Example description
-------------------
This example shows how to use the SPI master and slave drivers in a non-DMA interrupt
driven configuration. The master will send and recceive data back from the slave. Both
the SPI master (spim) and slave (spis) are used simultaneoulsy with this example on
different SPI controllers.

The example sends and receive data between the 2 SPI controllers on the device. After
the data is transferred, it's compared for errors.

Special connection requirements
-------------------------------
SPI0 SSEL to SPI1 SSEL
SPI0 CLK to SPI1 CLK
SPI0 MISO to SPI1 MOSI
SPI0 MOSI to SPI1 MISO

Build procedures:
-----------------
Visit the at 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
