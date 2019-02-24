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

typedef struct _space_info_t
{
	uint32_t size;
	char name_buf[FILENAME_LEN];
}space_info_t;

static char *search(char *name, zlog_category_t *zlog_handle);

int32_t netdisk_sendfile_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
						zlog_category_t *zlog_handle)
{
	int count;
	int nrecv;
	int fd, ret;
	int nread, nsend;
	char *name;
	off_t offset;
	uint32_t st_code;
	char buf[BUF_SIZE];
	struct stat st_buf;
	space_info_t space_info;
	char filename[FILENAME_LEN];

	if (first_connfd < 0 || cmd_info_ptr == NULL || zlog_handle == NULL)
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

	name = search(cmd_info_ptr->second, zlog_handle);
	if (name == NULL)
	{
		zlog_error(zlog_handle, "search() error");
		return -1;
	}

	memset(&space_info, 0, sizeof(space_info));
	memcpy(space_info.name_buf, name, strlen(name));
	
	ret = fstat(fd, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "fstat() error");
		return -1;
	}
	space_info.size = st_buf.st_size;

sk:	nsend = send(first_connfd, &space_info, sizeof(space_info), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		printf("connection break");
		return -1;
	}

ag:	nrecv = recv(first_connfd, &offset, sizeof(offset), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto ag;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}
	lseek(fd, offset, SEEK_SET);

	count = offset;
	while (1)
	{
		memset(buf, 0, sizeof(buf));
sk0:	nread = read(fd, buf, BUF_SIZE);
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk0;
			}
			else{
				return -1;
			}
		}
		else if (nread == 0)
		{
			printf("<read done>\n");
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
		else if (nsend == 0)
		{
			printf("\n<recv done>\n");
			break;
		}
		count += nsend;
		printf("################## = [%.5f]\n",
					(double)count/(double)space_info.size);
	}
	close(fd);

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

	if (st_code == SUCCESS)
	{
		return 0;
	}

	return 0;
}

static char *search(char *name, zlog_category_t *zlog_handle)
{
	char *s;

	if (name == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return NULL;
	}

	for (s = name; *s != '\0'; s++);
	for (; *s != '/'; s--);
	s++;
	
	return s;
}
