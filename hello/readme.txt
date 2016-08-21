
Environment used:
=================
OS - Ubuntu 14.04
Linux Kernel - 3.16 
Processor  -  i686

Project:
========
hello driver - This is just a prototype code. There is scope for optimization and improvements. 

Concepts:
=========
This driver demonstrates the below concepts

1. How to create a char driver?
2. How to register file operations for a char driver?
3. How two process or same process with multiple file descriptors could use the same device for read and write purposes?

Test Application
================
The test application testapp.c opens the same device file /dev/hello twice, once for reading and then for writing. In a while loop, it receives user input and based on that it does read or write on the device file.

Procedure to Test
=================
1. Open the terminal 
2. Run make from LDD/hello. It will create hello.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod hello.ko and check the dmesg tab for new logs
5. Create a new node file as per the new dmesg log
6. Then move to LDD/hello/test, compile and execute the test app.
7. Check the dmesg tab for new logs
8. Give options for read and write and check the new dmesg logs for your understanding.Always give write followed by read,as there is no way for the read fd to know whether write fd has written new data or not. So read just reads the data in the buffer and moves on
9. Once done to remove module use rmmod hello.ko; rm /dev/hello
