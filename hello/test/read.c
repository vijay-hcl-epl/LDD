#include<stdio.h>
#include<fcntl.h>

int main()
{
	int i, rdfd, wrfd;
	char write_buff[100], read_buff[100];
        char exit = 0;
        int ch;
        char val[100] = {0};

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

        while(!exit)
        {
	printf("1 = read from device\n2 = write into the device\nEnter your Choice : \n");
	scanf("%d",&ch);
	
	switch(ch) {
		case 1:
			read(rdfd, read_buff, 2);
			printf("Read val[0]: %d\n", read_buff[0]);
			printf("Read val[1]: %d\n", read_buff[1]);
			break;
		case 2:
                        write_buff[0] = val[i];
                        ++i;
                        write_buff[1] = val[i];
                        ++i;

			printf("Written val[0]: %d\n", write_buff[0]);
			printf("Written val[1]: %d\n", write_buff[1]);
			write(wrfd, write_buff, 2);
			break;
		default:
                        exit = 1;
			printf("Command not recognized\n");
			break;
	}
        }
	close(rdfd);
	close(wrfd);
	return 0;
}
