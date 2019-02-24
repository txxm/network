#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_scan_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
			zlog_category_t *zlog_handle)
{
	int fd;
	int ret;
	int nrecv;
	uint32_t st_code;
	int nread, nsend;
	struct stat st_buf;
	char buf[BUF_SIZE];
	char filename[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "../user/%s", cmd_info_ptr->second);

	fd = open(filename, O_RDWR);
	if (fd == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	ret = fstat(fd, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "fstat() error");
		return -1;
	}

sk:	nsend = send(first_connfd, &st_buf, sizeof(st_buf), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk;
		}
		return -1;
	}
	else if (nsend == 0){
		return -1;
	}

	while (1)
	{
		memset(buf, 0, sizeof(buf));
sk0:	nread = read(fd, buf, sizeof(buf));
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk0;
			}
			return -1;
		}
		else if (nread == 0){
			break;
		}

sk1:	nsend = send(first_connfd, buf, nread, MSG_DONTWAIT);
		if (nsend == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk1;
			}
			return -1;
		}
		else if (nsend == 0){
			return -1;
		}
	}

printf("12345\n");
sk2:nrecv = recv(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk2;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		return 0;
	}

	if (st_code == SUCCESS)
	{
		return 0;
	}
	else
	{
		return -1;
	}

	return 0;
}
