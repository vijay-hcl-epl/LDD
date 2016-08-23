
Environment used:
=================
OS - Ubuntu 14.04
Linux Kernel - 3.16 
Processor  -  i686

Project:
========
async driver - This is just a prototype code. There is scope for optimization and improvements. 

Concepts:
=========
This project demonstrates the below concepts

1. How to use FASYNC flag in user space?
2. How to receive signal in user space when there is data available for write?
2. How to implement fasync file operation function in a char driver.

Test Application
================
The test applications reader.c and writer.c opens the same device file /dev/async for reading and writing respectively. In a while loop,writer  receives user input and based on that it writes the device file. Reader does some other task(in this case running a while(1) loop) and when data is written by writer, the reader process gets notified by kernel via signals

Procedure to Test
=================
1. Open the terminal 
2. Run make from LDD/async. It will create async.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod async.ko and check the dmesg tab for new logs
5. Create a new node file as per the new dmesg log
6. Then move to LDD/async/testapp, compile and execute the reader and writer programs in 2 separate tabs.
7. Check the dmesg tab for new logs often
8. Now write something in the writer program by giving input.
9. Observe that now reader receives a signal and reads the data. This could be verified by seeing the logs in dmesg tab
12. Once done to remove module use rmmod async.ko; rm /dev/async
