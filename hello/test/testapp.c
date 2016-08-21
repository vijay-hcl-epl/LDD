#include<stdio.h>  // for scanf, printf
#include<fcntl.h>  // for ORDONLY flags

int main()
{
        // In this same program we are gonna do read and write
        // so use 2 diferent fds one for read and other for write
	int i, rdfd, wrfd;
	char write_buff[100], read_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};

        // Fill dummy values used for writing 
        for(i = 0; i < 100; i++)
        {
            val[i] = i;
        }

	rdfd = open("/dev/hello", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}
 
	wrfd = open("/dev/hello", O_WRONLY);
	if (wrfd == -1) {
		printf("File open error\n");
		return -1;
	}

        i = 0;

        // Process until exit is zero
        while(!exit)
        {
        printf("=================================================================\n\n");
	printf("1 - read from device\n2 - write into the device\n3 - Exit\nEnter your Choice : \n");
	scanf("%d",&ch);
	
	switch(ch) {
		case 1:
                        // Read using read fd and print the values
			read(rdfd, read_buff, 2);
			printf("Read val[0]: %d\n", read_buff[0]);
			printf("Read val[1]: %d\n", read_buff[1]);
			break;
		case 2:
                        // Write using write fd and print the values
                        write_buff[0] = val[i];
                        ++i;
                        write_buff[1] = val[i];
                        ++i;

			printf("Written val[0]: %d\n", write_buff[0]);
			printf("Written val[1]: %d\n", write_buff[1]);
			write(wrfd, write_buff, 2);
			break;
                case 3:
                        exit = 1;
			printf("Exiting..\n");
			break;
		default:
                        exit = 1;
			printf("Command Not Recognized..\n");
			break;
	}
        }
	close(rdfd);
	close(wrfd);
	return 0;
}
