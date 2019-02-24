#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"


int32_t netdisk_find(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int nsend;
	int nrecv;
	int first_sockfd;
	char name[FILENAME_LEN];
	char find_buf[1024];

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

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "%s", user_info.username);
	memset(cmd_info_ptr->third, 0, FILENAME_LEN);
	memcpy(cmd_info_ptr->third, name, strlen(name));


	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

sk0:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t),
					MSG_DONTWAIT);
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

	memset(find_buf, 0, 1024);
sk1:nrecv = recv(first_sockfd, find_buf, 1024, MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}
	printf("%s", find_buf);

	return 0;
}
