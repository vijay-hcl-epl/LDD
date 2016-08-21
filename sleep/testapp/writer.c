#include<stdio.h>   // for scanf, printf
#include<fcntl.h>   // for OWRONLY flags
#include "../ioctl_basic.h" // To use ioctl cmds

int main()
{
	int i, wrfd;
	char write_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};

        // Fill dummy values used for writing
        for(i = 0; i < 100; i++)
        {
            val[i] = i;
        }

        // Open the device file for writing
	wrfd = open("/dev/sleep", O_WRONLY);
	if (wrfd == -1) {
		printf("File open error\n");
		return -1;
	}

        i = 0;

        // Process until exit is zero
        while(!exit)
        {
        printf("=================================================================\n\n");
	printf("1 - Write from device\n2 - IOCTL\n3 - Exit\nEnter your Choice : \n");
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
                        ioctl(wrfd,IOCTL_HELLO);  //ioctl call
                        break;
                case 3:
                        exit = 1;
			printf("Exiting..\n");
			break;
		default:
			printf("Command Not Recognized..\n");
			break;
	}
        }
	close(wrfd);
	return 0;
}
