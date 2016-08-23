#include<stdio.h>    // printf,scanf
#include<fcntl.h>    // O_WRONLY 
#include <signal.h>  // SIGIO and signal handlers

// Let this be global, so that signal handler also can access
int rdfd;
char read_buff[100];

// Signal handler
void hndlr(int signo){
	switch (signo) {
		case SIGIO:
			printf("SIGIO Received..\n");
			read(rdfd, read_buff, 2);
			printf("Read val[0]: %d\n", read_buff[0]);
			printf("Read val[1]: %d\n", read_buff[1]);
			break;
	}
}

int main()
{
	struct sigaction sa = {0};
	int oflags = 0;

	// Open the device file
	rdfd = open("/dev/async", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}

	// Register a signal Handler
	sa.sa_handler = hndlr;
	sigaction(SIGIO, &sa, NULL); /* dummy sample; sigaction( ) is better */

	// Set this process as the owner of this file pointer
	// This step is necessary for the kernel to know just whom to notify.
	fcntl(rdfd, F_SETOWN, getpid());

	// Get the existing flags
	oflags = fcntl(rdfd, F_GETFL);

	// Set FASYNC flag
	fcntl(rdfd, F_SETFL, oflags | FASYNC);

	// Let the reader do some other task
	// If data is available for write, it will receive a signal from kernel
	while(1);

	close(rdfd);
	return 0;
}
