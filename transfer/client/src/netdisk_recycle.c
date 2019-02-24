#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_recycle(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
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
	sprintf(filename, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	memcpy(cmd_info_ptr->second, filename, strlen(filename));

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

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

sk1:nsend = send(first_sockfd, user_info.username, strlen(user_info.username),
					MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk1;
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
