#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_tar_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle)
{
	int ret;
	int fd;
	int nsend;
	uint32_t st_code;
	char cmd[FILENAME_LEN];
	char filename[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "../user/%s.tar.gz", cmd_info_ptr->second);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "tar -zcvf %s ../user/%s", filename, cmd_info_ptr->second);
	
	ret = system(cmd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "system() error");
		return -1;
	}

	fd = open(filename, O_RDWR);
	if (fd == -1)
	{
		memset(&st_code, 0, sizeof(st_code));
		st_code = FAILURE;
sk0:	nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
		if (nsend == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk0;
			}
			return -1;
		}
		else if (nsend == 0)
		{
			printf("connection break\n");
			return -1;
		}

		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	memset(&st_code, 0, sizeof(st_code));
	st_code = SUCCESS;
sk1:nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
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
		printf("connection break\n");
		return -1;
	}

	return 0;
}
