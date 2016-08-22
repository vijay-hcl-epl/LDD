#include<stdio.h>   // scanf, printf
#include<fcntl.h>   // O_RDONLY
#include <linux/poll.h> //To access poll call

//#define SELECT // Uncomment this to work with poll system call

int main()
{
	int rdfd = 0;
	int rv = -1;
	char read_buff[100] = {0};
	char exit = 0;
	char val[100] = {0};
	int ch = 0;
#ifdef SELECT // Select needs timeval to specify timings
	fd_set rfds;
	struct timeval tv;
#else
	struct pollfd ufds[1] = {0};
#endif

	// Open the device file
	rdfd = open("/dev/polsel", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}

#ifdef SELECT
	//Loop until user exits...
	while(!exit)
	{
		printf("\n\n==================================\n");
		printf("Enter 1 to read\n");
		printf("Enter 2 to exit\n");
		scanf("%d", &ch);

		switch(ch)
		{
			case 1:
				FD_ZERO(&rfds); // Clear before use
				FD_SET(rdfd, &rfds); // set the fd to fd_set

				/* Wait up to five seconds. */
				tv.tv_sec = 5;
				tv.tv_usec = 0;

				// Always give fd + 1 as per the usage
				rv = select(rdfd + 1, &rfds, NULL, NULL, &tv);
				if(rv == -1)
				{
					perror("Select");
				} else if(rv){
					if (FD_ISSET(rdfd, &rfds)) // If rdfd is set read could be done without blocking
					{
						read(rdfd, read_buff, 2);
						printf("Read val[0]: %d\n", read_buff[0]);
						printf("Read val[1]: %d\n", read_buff[1]);
					}
				}
				else
					printf("No data within five seconds.\n");
				break;
			case 2:
				exit = 1;
				printf("Exiting..\n");
				break;
			default:
				printf("Command Not Recognized\n");
				break;
		}
	}
#else
	//Loop until user exits...
	while(!exit)
	{
		printf("\n\n==================================\n");
		printf("Enter 1 to read\n");
		printf("Enter 2 to exit\n");
		scanf("%d", &ch);

		switch(ch)
		{
			case 1:
				// Configure the fd and pass it to poll
				ufds[0].fd = rdfd;
				ufds[0].events = POLLIN|POLLRDNORM;
				ufds[0].revents = 0;

				rv = poll(ufds, 1, 5000); // 5 second waiting time
				//rv = poll(ufds, 1, -1); // Wait forever..
				if(rv == -1)
				{
					perror("poll");
				} else if (rv == 0) {
					printf("Timeout occurred!.\n");
				} else {
					if (ufds[0].revents & POLLIN) // revents contain the polling results
					{
						read(rdfd, read_buff, 2);
						printf("Read val[0]: %d\n", read_buff[0]);
						printf("Read val[1]: %d\n", read_buff[1]);
					}
				}
				break;
			case 2:
				exit = 1;
				printf("Exiting..\n");
				break;
			default:
				printf("Command Not Recognized\n");
				break;
		}
	}
#endif
	close(rdfd);
	return 0;
}
