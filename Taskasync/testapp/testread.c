
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
//#include <libaio.h>
#include <linux/aio_abi.h>

#define TESTFILE_SIZE (4096 * 5120)
#define IORTX_SIZE (1024 * 4)
#define NUM_EVENTS 128


static inline void asio_prep_pread(struct iocb *iocb, int fd, void *buf, size_t count, long long offset)
{
    memset(iocb, 0, sizeof(*iocb));
    iocb->aio_fildes = fd;
    iocb->aio_lio_opcode = IOCB_CMD_PREAD;
    iocb->aio_reqprio = 0;
    iocb->aio_buf = buf;
    iocb->aio_nbytes = count;
    iocb->aio_offset = offset;
}


int main(int ac, char **av) {
	int afd, fd;
	aio_context_t ctx = 0;
	struct iocb *iocb[1];
	char buf[5] = {0};
	struct io_event e;
        struct timespec timeout;

	iocb[0] = malloc(sizeof(struct iocb));
/*
	fprintf(stdout, "creating an eventfd ...\n");
	if ((afd = eventfd(0)) == -1) {
		perror("eventfd");
		return 2;
	}
*/
	if (io_setup(1, &ctx)) {
		perror("io_setup");
		return 3;
	}

	if ((fd = open("/dev/Taskasync", O_RDWR | O_CREAT, 0644)) == -1) {
		perror("Open failuere");
		return 4;
	}

	asio_prep_pread(iocb[0], fd, buf, 5, 0);

	io_submit(ctx, 1, iocb[0]);

	timeout.tv_sec=0;
	timeout.tv_nsec=1000000000;
	io_getevents(ctx, 0, 1, &e, &timeout);
	printf("haven't done");

	io_destroy(ctx);
}


