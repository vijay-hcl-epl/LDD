http://www.spinics.net/lists/newbies/msg24434.html
https://janzhou.org/2011/libaio-example.html
http://stackoverflow.com/questions/8513663/linux-disk-file-aio
http://wiki.cs.pdx.edu/usb/asyncdebate.html
http://fwheel.net/aio.html
http://anti.teamidiot.de/mrtweek/2007/06/direct_io_filesystem_access_in/

Environment used:
=================
OS - Ubuntu 12.04.5 LTS
Linux Kernel - 3.2.0-98-generic
Processor  -  i686

Project:
========
Taskasync driver - This is just a prototype code. There is scope for optimization and improvements.

Concepts:
=========
This project demonstrates the below concepts

1. How to initiate asynchronous task in a char driver?
2. How to receive notification in user space when the asynchronous task is done?
3. How to  use io control blocks and APIs for asynchronous tasks?
4. Note: Go through the links mentioned in the top of this file to know about the differences between libaio and kernel aio. This test app uses kernel aio and we need the same.

Test Application
================
The test applications test.c opens the device file /dev/Taskasync for reading.It creates an eventfd for getting notification from kernel once asynchronous task is done. It fills iocb using asyio_prep_pread and submits the iocb. Once submits, it polls the eventfd fd whether dasta could be read. If readable, it reads how many asynchronous tasks have taken placei using io_getevents. It then prints out total asynchronous tasks ininitiated and total completed. In this example, it is 1. It just does read which calls the driver aio_read and initiates an asynchronous task which makes it sleep for 10 seconds.

Note: Added helper functions and APIs of kaio in this file itself as there was some difficulty in linking it

Procedure to Test
=================
1. Open the terminal
2. Run make from LDD/Taskasync. It will create Taskasync.ko
3. Open a separate tab and type the command 'watch "dmesg | tail -20"'. This shows the last 20 lines of dmesg and refreshes it at a interval of 2 seconds.
4. insmod Taskasync.ko and check the dmesg tab for new logs
5. Create a new node file as per the new dmesg log
6. Then move to LDD/Taskasync/testapp, compile and execute test.c in a separate tab.
7. Check the dmesg tab for new logs often
8. You can see the sleep happening in an asynchronous way in dmesg logs which prints string "Sleeping".
9. Observe that now reader is able to read. This could be verified by seeing the logs in test app execution tab.
10. Once the asynchronous task is done; i.e it sleeps for 10 seconds, the test app prints results and completes the execution.
11. Once done to remove module use rmmod Taskasync.ko; rm /dev/Taskasync

