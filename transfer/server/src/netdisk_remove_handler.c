#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_remove_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle)
{
	int ret;
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
	sprintf(filename, "../user/%s", cmd_info_ptr->second);

	//计算空间
	ret = netdisk_add(cmd_info_ptr->second, cmd_info_ptr->third, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_space()");
		return -1;
	}

	ret = remove(filename);
	if (ret == -1)
	{
		if (errno == ENOTEMPTY)
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", filename);
			system(cmd);
		}
		else
		{
			zlog_error(zlog_handle, "remove() error");
			return -1;
		}
	}

sk:	nsend = send(first_connfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
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
		printf("connection break\n");
		return -1;
	}

	return 0;
}
