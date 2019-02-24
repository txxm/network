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


int32_t netdisk_recvfile_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
            zlog_category_t *zlog_handle)
{
	int nsend;
	int fd, ret;
	int count = 0;
	int nrecv, nwrite;
	uint32_t st_code;
	struct stat st_buf;
	char buf[BUF_SIZE];
	char filename[FILENAME_LEN];

	if (first_connfd < 0 || cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "../user/%s", cmd_info_ptr->second);
	fd = open(filename, O_CREAT | O_EXCL | O_RDWR, RDWR_DEF);
	if (fd == -1)
	{
		if (errno == EEXIST)
		{
			sprintf(filename, "../user/%s", cmd_info_ptr->second);
			fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, RDWR_DEF);
			if (fd == -1)
			{
				zlog_error(zlog_handle, "open() error");
				return -1;
			}
		}
		else
		{
			zlog_error(zlog_handle, "open() error");
			return -1;
		}
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

sk:	nrecv = recv(first_connfd, &st_buf, sizeof(st_buf), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}

	while (1)
	{
		if (count >= st_buf.st_size){
			break;
		}
		memset(buf, 0, sizeof(buf));
sk0:	nrecv = recv(first_connfd, buf, BUF_SIZE, MSG_DONTWAIT);
		if (nrecv == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk0;
			}
			return -1;
		}
		else if (nrecv == 0)
		{
			printf("connection break\n");
			break;
		}

sk1:	nwrite = write(fd, buf, nrecv);
		if (nwrite == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk1;
			}
			return -1;
		}
		fsync(fd);
		count += nwrite;
		printf("*****************[%.5f]\n", 
				(double)count/(double)st_buf.st_size);
	}
	close(fd);

	st_code = SUCCESS;
sk2:nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk2;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	ret = netdisk_space(cmd_info_ptr->second, cmd_info_ptr->four, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_space()");
		return -1;
	}

	return 0;
}
