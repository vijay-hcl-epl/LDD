
Environment used:
=================
OS - Ubuntu 14.04
Linux Kernel - 3.16 
Processor  -  i686

Project:
========
sleep driver - This is just a prototype code. There is scope for optimization and improvements. 

Concepts:
=========
This driver demonstrates the below concepts

1. How to put a process to sleep based on some condition?
2. How to wake up a sleeping process?
2. How to use ioctl cmds? IOCTL should only be used for device configurations.
3. How a process blocks by sleeping?

Test Application
================
The test applications reader.c and writer.c opens the same device file /dev/sleep for reading and writing respectively. In a while loop, it receives user input and based on that it does read or write or ioctl on the device file.

Procedure to Test
=================
1. Open the terminal 
2. Run make from LDD/sleep. It will create sleep.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod sleep.ko and check the dmesg tab for new logs
5. Create a new node file as per the new dmesg log
6. Then move to LDD/sleep/testapp, compile and execute the reader and writer programs in 2 separate tabs.
7. Check the dmesg tab for new logs often
8. For the reader program, try to read by giving user input. 
9. Observe that the reader programs now sleeps and it waits for writer to write something.
10. Now write something in the writer program by giving input.
11. Observe that now reader wakes up and again waits for user input. This could be verified by seeing the logs in dmesg tab
12. Giving option for IOCTL cmd just executes the .ioctl function in driver. 
13. Once done to remove module use rmmod sleep.ko; rm /dev/sleep
