
Environment used:
=================
OS - Ubuntu 14.04
Linux Kernel - 3.16 
Processor  -  i686

Project:
========
polsel driver - This is just a prototype code. There is scope for optimization and improvements. 

Concepts:
=========
This project demonstrates the below concepts

1. How to use poll or select on a fd?
2. How to implement poll file operations function in a driver.

Test Application
================
The test applications reader.c and writer.c opens the same device file /dev/polsel for reading and writing respectively. In a while loop, it receives user input and based on that it does read or write or ioctl on the device file.

Procedure to Test
=================
1. Open the terminal 
2. Run make from LDD/polsel. It will create polsel.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod polsel.ko and check the dmesg tab for new logs
5. Create a new node file as per the new dmesg log
6. Then move to LDD/polsel/testapp, compile and execute the reader and writer programs in 2 separate tabs.
7. Check the dmesg tab for new logs often
8. For the reader program, try to read by giving user input. 
9. Observe that the reader programs now waits for specified time for the writer to write something.
10. Now write something in the writer program by giving input within 5 seconds. If no write happens within 5 seconds reader issues a fresh read command and wait for writer again.
11. Observe that now reader wakes up and again waits for user input. This could be verified by seeing the logs in dmesg tab
12. Once done to remove module use rmmod polsel.ko; rm /dev/polsel
