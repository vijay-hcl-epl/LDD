
Environment used:
=================
OS - Ubuntu 14.04
Linux Kernel - 3.16 
Processor  -  i686

Project:
========
ldd driver - This is just a prototype code. There is scope for optimization and improvements. 

Concepts:
=========
This project demonstrates the below concepts

1. How to register a bus?
2. How to register a device?
3. How to create a bus attribute file?
4. How o create a device attribute file?

Test Application
================
<None>

Procedure to Test
=================
1. Open the terminal 
2. Run make from LDD/ldd. It will create async.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod ldd.ko and check the dmesg tab for new logs
5. Check in /sys/bus/ for the bus created if "BUS_TYPE" macro is enabled or check in /sys/device for the device created if "DEVICEE" macro is enabled.
6. cd to the created bus or device folder and do cat for the version file which is a attribute file
7. Check the dmesg tab for new logs. 
8. Once done to remove module use rmmod ldd.ko; rm /dev/async
