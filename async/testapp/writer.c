#include<stdio.h>   // printf,scanf
#include<fcntl.h>   // O_WRONLY

int main()
{
	int i, wrfd;
	char write_buff[100];
	char exit = 0;
	int ch;
	char val[100] = {0};

	// Fill some dummy values to be written
	for(i = 0; i < 100; i++)
	{
		val[i] = i;
	}

	// Open the device file in write only mode
	wrfd = open("/dev/async", O_WRONLY);
	if (wrfd == -1) {
		printf("File open error\n");
		return -1;
	}

	i = 0;

	while(!exit)
	{
		printf("\n\n==================================\n");
		printf("Enter 1 to write\n");
		printf("Enter 2 to exit\n");
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
				exit = 1;
				printf("exiting..\n");
				break;
			default:
				printf("Command Not Recognized\n");
				break;
		}
	}
	close(wrfd);
	return 0;
}
