#include<stdio.h>   // scanf, printf
#include<fcntl.h>   // O_RDONLY
#include "../ioctl_basic.h"
#include <linux/poll.h>

int main()
{
	int rdfd = 0;
	int rv = -1;
	char read_buff[100] = {0};
	char exit = 0;
	int ch = 0;
	char val[100] = {0};
	struct pollfd ufds[1] = {0};
	fd_set rfds;
	struct timeval tv;

	rdfd = open("/dev/polsel", O_RDONLY);
	if (rdfd == -1) {
		printf("File open error\n");
		return -1;
	}

	ufds[0].fd = rdfd;
	ufds[0].events = POLLIN|POLLRDNORM;
	ufds[0].revents = 0;

	FD_ZERO(&rfds);
	FD_SET(rdfd, &rfds);

	/* Wait up to five seconds. */
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(rdfd, &rfds);

		/* Wait up to five seconds. */
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		//rv = poll(ufds, 1, 10000);
		rv = select(rdfd + 1, &rfds, NULL, NULL, &tv);
		if(rv == -1)
		{
			printf("Aftr rv = -1..\n");
			perror("poll");
		} else if(rv){
			if (FD_ISSET(rdfd, &rfds))
			{
				read(rdfd, read_buff, 2);
				printf("Read val[0]: %d\n", read_buff[0]);
				printf("Read val[1]: %d\n", read_buff[1]);
			}
		}
		else
			printf("No data within five seconds.\n");
	}

	/*while(!exit)
	  {
	  printf("1 = read from device\n2 = IOCTL\nEnter your Choice : \n");
	  scanf("%d",&ch);
	  printf("The val of ch is %d\n",ch);
	  switch(ch) {
	  case 1:
	  rv = poll(ufds, 1, -1);
	  printf("Aftr poll..\n");
	  if(rv == -1)
	  {
	  printf("Aftr rv = -1..\n");
	  perror("poll");
	  } else if (rv == 0) {
	  printf("Timeout occurred!.\n");
	  } else {
	  printf("Read going..\n");
	  if (ufds[0].revents & POLLIN)
	  {
	  read(rdfd, read_buff, 2);
	  printf("Read val[0]: %d\n", read_buff[0]);
	  printf("Read val[1]: %d\n", read_buff[1]);
	  }
	  }
	  break;
	  case 2:
	  ioctl(rdfd,IOCTL_HELLO);  //ioctl call
	  break;
	  default:
	  exit = 1;
	  printf("Command nor recognized\n");
	  break;
	  }
	  }*/
	close(rdfd);
	return 0;
}
