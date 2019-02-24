#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"


int32_t netdisk_tar(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int nrecv;
	int nsend;
	int count = 0;
	int first_sockfd;
	uint32_t st_code;
	char filename[FILENAME_LEN];

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

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	memcpy(cmd_info_ptr->second, filename, strlen(filename));

sk0:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
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

	memset(&st_code, 0, sizeof(st_code));
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
	else if (nrecv == 0)
	{
		if (count > 0)
		{
			printf("connection break\n");
			return -1;
		}
		count++;
	}

	if (st_code == SUCCESS)
	{
		printf("\033[31m%s.tar.gz\n\033[0m", filename);
	}
	else if (st_code == FAILURE)
	{
		printf("tar failure.\n");
	}

	return 0;
}
