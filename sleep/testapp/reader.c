#include<stdio.h>   // printf,scanf
#include<fcntl.h>   // O_RDONLY
#include "../ioctl_basic.h" // To use ioctl cmd

int main()
{
	int rdfd;
	char read_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};

        // Open the device file for reading
	rdfd = open("/dev/sleep", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}
 
        // Process until exit is zero
        while(!exit)
        {
        printf("=================================================================\n\n");
	printf("1 - read from device\n2 - IOCTL\n3 - Exit\nEnter your Choice : \n");
	scanf("%d",&ch);
	
	switch(ch) {
		case 1:
                        // Read using read fd and print the values
			read(rdfd, read_buff, 2);
			printf("Read val[0]: %d\n", read_buff[0]);
			printf("Read val[1]: %d\n", read_buff[1]);
			break;
                case 2:
                        ioctl(rdfd,IOCTL_HELLO);  //ioctl call
                        break;
                case 3:
                        exit = 1;
			printf("Exiting..\n");
			break;
		default:
			printf("Command Not Recognized\n");
			break;
	}
        }
	close(rdfd);
	return 0;
}
