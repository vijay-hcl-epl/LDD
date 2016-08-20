#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <errno.h>
#include "../ioctl_basic.h"

int main()
{
	int i, wrfd;
	char write_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};

        for(i = 0; i < 100; i++)
        {
            val[i] = i;
        }

	wrfd = open("/dev/async", O_WRONLY);
	if (wrfd == -1) {
                perror("FILE OPEN FAILED");
		printf("File open error\n");
		return -1;
	}

        i = 0;

        while(!exit)
        {
	printf("1 = Write to device\n2 = IOCTL\nEnter your Choice : \n");
	scanf("%d",&ch);
	
	switch(ch) {
		case 1:
                        write_buff[0] = val[i];
                        ++i;
                        write_buff[1] = val[i];
                        ++i;

			printf("Written val[0]: %d\n", write_buff[0]);
			printf("Written val[1]: %d\n", write_buff[1]);
			write(wrfd, write_buff, 2);
			break;
                case 2:
                        ioctl(wrfd,IOCTL_ADDBUS);  //ioctl call
                        break;
                case 3:
                        ioctl(wrfd,IOCTL_REMBUS);
                        break;
		default:
                        exit = 1;
			printf("Command nor recognized\n");
			break;
	}
        }
	close(wrfd);
	return 0;
}
