I2C probe example
=================

Example description
-------------------
This example presents the user with an I2C operations menu.
It has the following commands:
- 0: Exit Demo
- 1: Probe for Slave devices
- 2: Read slave data
- 3: Write slave data
- 4: Write/Read slave data

Exit Demo:
----------
This command exits the example program.

Probe for Slave devices:
------------------------
This command probes the I2C bus for I2C devices.
It produces an output like this:

Probing available I2C devices...

     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
====================================================
00   -- -- -- -- -- -- -- -- na na na na na na na na
01   10 11 12 na na na na na na 19 na na na na na na
02   na na na na na na na na na na na na na na na na
03   na na na na na na na na na na na na na na na na
04   na na na na na na na na na na 4A na na na na na
05   na na na na na na na na na na na na na na na na
06   na na na na na na na na 68 69 na na na na na na
07   na na na na na na 76 na na -- -- -- -- -- -- --

The matrix contains all possible I2C addresses.  If there is a number in the 
grid, then the I2C device responded to a single byte read.  A two letter 
lower case acronym corresponds to an error. See the I2C Errors section for 
more information. In this case, most of the addresses responded with an error
"na".  This is NAK on address.  This means there is no chip at the address.

Read slave data:
----------------
This command reads up to 256 bytes from an I2C slave.
It requests a 7-bit slave address and the number of bytes to be read.

Note: The input can accept both hex and decimal values.
Hex should look like this: 0x10. Decimal should look like this: 16

Here's an example:
Select an option [0 - 4]:
2
Enter 7-Bit Slave address:
0x10
Enter number of bytes to read:
10
Read 10 bytes of data from slave 0x10.

00:  FA 20 71 00 51 01 01 42 00 00

Write slave data:
-----------------
Writes data to an I2C slave device.
It requests a 7-bit slave address, the number of bytes to be sent, and the data.

Note: The input can accept both hex and decimal values.
Hex should look like this: 0x10. Decimal should look like this: 16
This is true for all commands.

Here's an example:
Select an option [0 - 4]:
3
Enter 7-Bit Slave address:
0x10
Enter number of bytes to write:
1
 1:Enter Data:
0
Written 1 bytes of data to slave 0x10.


Write/Read slave data:
----------------------
Writes data to an I2C slave device, then reads back from the same device.
It requests a 7-bit slave address, the number of bytes to read,
the number of bytes to write, and the data.

Here's an example:
Select an option [0 - 4]:
4
Enter 7-Bit Slave address:
0x10
Enter number of bytes to read:
10
Enter number of bytes to write:
1
 1:Enter Data:
0
Read 10 bytes of data from slave 0x10.

00:  FA 20 31 00 91 00 81 42 00 00


I2C Errors:
-----------
I2C Error   Acronym    Description
 1          er         Unknown error
 2          na         NAK address
 3          be         Bus error
 4          nd         NAK data
 5          al         Arbitration lost
 0xff       bs         Driver Busy


Special connection requirements
-------------------------------
No special connections are required. However, the I2C probe above was done
with the NXP OM13078 Multiple Function Sensor Development Kit.


Build procedures:
-----------------
Visit the  'LPCOpen quickstart guides' at [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.


