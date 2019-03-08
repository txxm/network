#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <aio.h>

#define MAX_LIST 	10

int j = 0;

void func(union sigval sigev)
{
	int n, ret;
	struct aiocb *aiocbp;

	aiocbp = (struct aiocb *)sigev.sival_ptr;
	ret = aio_error(aiocbp);
	if (ret == 0)
	{
		n = aio_return(aiocbp);
		if (n == -1) {
			perror("aio_return()");
			return ;
		}
		printf("buf[%d] = %s\n", __sync_fetch_and_add(&j, 1),
						(char *)aiocbp->aio_buf);
	}
	else if (ret == EINPROGRESS) {
		printf("reading but not done.\n");
	}
	else if (ret == ECANCELED) {
		printf("read be canceled\n");
	}
	else if (ret == -1) {
		perror("aio_read()");
		return ;
	}
}

int main(int argc, char *argv[])
{
	int fd, ret;
	struct aiocb aiocbp[MAX_LIST];
	struct aiocb *list[MAX_LIST];

	fd = open("test.list", O_RDWR);
	if (fd == -1) {
		perror("perror");
		return -1;
	}

	memset(list, 0, sizeof(list));
	for (int i = 0; i < MAX_LIST; i++)
	{
		memset(&aiocbp[i], 0, sizeof(aiocbp[i]));
		aiocbp[i].aio_fildes = fd;
		aiocbp[i].aio_nbytes = BUFSIZ;
		aiocbp[i].aio_offset = 0;
		aiocbp[i].aio_lio_opcode = LIO_READ;
		aiocbp[i].aio_sigevent.sigev_notify = SIGEV_THREAD;
		aiocbp[i].aio_sigevent.sigev_notify_function = func;
		aiocbp[i].aio_sigevent.sigev_notify_attributes = NULL;
		aiocbp[i].aio_sigevent.sigev_value.sival_ptr = &aiocbp[i];
		aiocbp[i].aio_buf = malloc(BUFSIZ+1);
		if (aiocbp[i].aio_buf == NULL) {
			perror("malloc()");
		}
		memset((char *)aiocbp[i].aio_buf, 0, BUFSIZ+1);

		ret = aio_read(&aiocbp[i]);
		if (ret == -1) {
			perror("aio_read()");
		}
	}

	ret = lio_listio(LIO_WAIT, list, MAX_LIST, NULL);
	if (ret == -1) {
		perror("lio_listio()");
		return -1;
	}

	return 0;
}
