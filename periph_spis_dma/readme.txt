LPC5410x Simple SPI slave with DMA example
============================

Example description
-------------------
The SPI slave with DMA example is a simple receive / echo example using DMA.  In order to recieve data from the slave, 
the master must transmit.  To accomplish this, the example uses a 4 byte protocol.  What this means is that the echo is delayed by
4 bytes.  The first transmission by the master will cause the example to reply with 0x0's.  The next 4 bytes from the master will 
cause the example to reply with the first 4 bytes sent in the first master transmission.  Thereafter, the reply will be the previous
data sent from the master.

Special connection requirements
-------------------------------
There are no special connection requirements for this example.

Build procedures:
-----------------
Visit the at 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
