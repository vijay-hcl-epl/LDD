#define _GNU_SOURCE
#include<stdio.h>    // printf,scanf
#include<fcntl.h>    // O_WRONLY 
#include <aio.h>     // aio operation
#include <string.h>  // memset
#include <errno.h>
#include <libaio.h>

// Let this be global, so that signal handler also can access
int rdfd;
char read_buff[100];

int main()
{
	int oflags = 0;
	struct aiocb aiocb;
	char buf[256] = {0};
	int numBytes = 0;

	// Open the device file
	//rdfd = open("/dev/Taskasync", O_RDONLY|O_DIRECT);
	rdfd = open("/dev/Taskasync", O_RDONLY);
	//rdfd = open("test.txt", O_RDONLY);
	if (rdfd == -1) {
		perror("File open error\n");
		return -1;
	}

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = rdfd;
	aiocb.aio_buf = buf;
	//aiocb.aio_nbytes = 8;
	aiocb.aio_nbytes = 256;
	aiocb.aio_offset = 0;
	//aiocb.aio_lio_opcode = LIO_WRITE;
	// Let the reader do some other task
	// If data is available for write, it will receive a signal from kernel
	//while(1);
	printf("B4 aio_read..\n");
	if (aio_read(&aiocb) == -1) {
		printf("Err in aio..\n");
		perror("aio_read failure:");
	}

	// wait until the request has finished
	while(aio_error(&aiocb) == EINPROGRESS)
	{
		printf("Working..."); 
	}

	numBytes = aio_return(&aiocb);
	printf("numBytes is %d\n",numBytes);

	close(rdfd);
	return 0;
}
