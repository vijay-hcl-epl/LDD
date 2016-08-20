#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <signal.h>
#include "../ioctl_basic.h"

void hndlr(int signo){
    switch (signo) {
            case SIGIO:

            printf("SIGIO Received..\n");
            //fflush(stdout);
            break;
    }
}

int main()
{
	int rdfd;
	char read_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};
        struct sigaction sa = {0};
        int oflags = 0;

	rdfd = open("/dev/async", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}

        sa.sa_handler = hndlr;
 
        sigaction(SIGIO, &sa, NULL); /* dummy sample; sigaction( ) is better */
        fcntl(rdfd, F_SETOWN, getpid());
        oflags = fcntl(rdfd, F_GETFL);
        fcntl(rdfd, F_SETFL, oflags | FASYNC);

        while(1);

        while(!exit)
        {
	printf("1 = read from device\n2 = IOCTL\nEnter your Choice : \n");
	scanf("%d",&ch);
	
	switch(ch) {
		case 1:
			read(rdfd, read_buff, 2);
			printf("Read val[0]: %d\n", read_buff[0]);
			printf("Read val[1]: %d\n", read_buff[1]);
			break;
                case 2:
                        ioctl(rdfd,IOCTL_HELLO);  //ioctl call
                        break;
		default:
                        exit = 1;
			printf("Command nor recognized\n");
			break;
	}
        }
	close(rdfd);
	return 0;
}
