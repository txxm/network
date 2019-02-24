#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"


int32_t netdisk_ll_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle)
{
	int fd;
	int ret;
	int nread, nsend;
	char buf[BUF_SIZE];
	char name[FILENAME_LEN];
	char filename[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	if (strlen(cmd_info_ptr->second) <=0 ||
			strlen(cmd_info_ptr->second) >= FILENAME_LEN)
	{
		zlog_error(zlog_handle, "filename too long");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "cd ../user/%s;ls -l > .ls_l", cmd_info_ptr->second);

	ret = system(filename);
	if (ret == -1)
	{
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/.ls_l", cmd_info_ptr->second);
	fd = open(name, O_RDWR);
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

	while (1)
	{
		memset(buf, 0, sizeof(buf));
sk0:	nread = read(fd, buf, BUF_SIZE);
		if (nread == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
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
			if (errno == EINTR || errno == EAGAIN)
			{
				usleep(10);
				goto sk1;
			}
			return -1;
		}
		else if (nsend == 0)
		{
			printf("connection break\n");
			return -1;
		}
	}
	close(fd);

	ret = remove(name);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}
