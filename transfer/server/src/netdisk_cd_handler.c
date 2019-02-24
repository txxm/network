#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_cd_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
							zlog_category_t *zlog_handle)
{
	DIR *pdir;
	int nsend;
	uint32_t st_code;
	char path[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(path, 0, FILENAME_LEN);
	sprintf(path, "../user/%s", cmd_info_ptr->second);

	printf("%s\n", path);

	pdir = opendir(path);
	if (pdir == NULL)
	{
		st_code = FAILURE;
sk0:	nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
		if (nsend == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
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
		zlog_error(zlog_handle, "opendir() error");
		return -1;
	}

	st_code = SUCCESS;
sk1:nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
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

	return 0;
}
