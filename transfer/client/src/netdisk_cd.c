#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_cd(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int nsend;
	int nrecv;
	int first_sockfd;
	uint32_t st_code;
	char path[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

	memset(path, 0, FILENAME_LEN);
	sprintf(path, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	memcpy(cmd_info_ptr->second, path, strlen(path));

sk0:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
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

sk1:nrecv = recv(first_sockfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}

	if (st_code == SUCCESS)
	{
		memset(g_path, 0, GLOBAL_DIR_PATH);
		sprintf(g_path, "%s", cmd_info_ptr->second);
	}
	else if (st_code == FAILURE)
	{
		printf("Not such a directory\n");
		return -1;
	}

	return 0;
}
