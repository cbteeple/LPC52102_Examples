LPC5410x I2CM bus master example using interrupts
=================================================

Example description
-------------------
This example communicates with the 3-axis accelerometer (Bosch BMC150) using the I2C bus.
The BMC150 sensor is on the Arduino compatible sensor shield.
This board is part of the LPC54102 Sensor Processing-Motion Solution (SPM-S).

The example first reads the part ID (0xfa), then loops reading the X/Y/Z accelerometer
and the temperature sensor. Data is output through the Debug UART port.
This example uses I2C interrupts to read data from the sensor.

Note: the temperature sensor is normalized to 24 degrees C.
That is, when the temperature reads 0, it's 24 degrees C.

Special connection requirements
-------------------------------
The NXP OM13078 Multiple Function Sensor Development Kit is required to run this demo.

Build procedures:
-----------------
Visit the at 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
