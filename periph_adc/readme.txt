LPC5410x ADC example
========================================================

Example description
-------------------
The LPC5410x ADC example shows how to use the ADC to perform
a sequence of conversions and monitor a threshold crossing. The
ADC is configured to monitor an input and is sampled by
manually starting the ADC sequence from the system tick interrupt.
The sequence then starts and generates an interrupt when complete.
During ADC sequence conversion, an interrupt will also be generated
if the sample read crosses the threshold 0 value.

To use this example, build and program it and then run it
on the board. Turn the potentiometer to change the ADC
signal input. Converted ADC values is displayed periodically via
the UART as kicked off by the ADC interrupt.

Special connection requirements
-------------------------------
A POT must be connected to P0_12.

Build procedures:
-----------------
Visit the at 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
