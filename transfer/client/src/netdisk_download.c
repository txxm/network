#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_download(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int ret;
	int nsend;
	int first_sockfd;
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
	sprintf(filename, "wget -P ../%s/download/ %s", user_info.username,
						cmd_info_ptr->second);
	ret = system(filename);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "system()");
		return -1;
	}

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

	memset(cmd_info_ptr->third, 0, strlen(cmd_info_ptr->third));
	sprintf(cmd_info_ptr->third, "%s", user_info.username);

sk0:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
	if (nsend == -1)
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
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	return 0;
}
