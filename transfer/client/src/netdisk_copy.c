#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_copy(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	//int ret;
	//char c;
	//int fd1, fd2;
	//int cmdlen;
	int nsend;
	//char buf[BUF_SIZE];
	//int nread, nwrite;
	int first_sockfd;
	//char cmd[FILENAME_LEN];
	char oldname[FILENAME_LEN];
	char newname[FILENAME_LEN];

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

	memset(cmd_info_ptr->four, 0, sizeof(cmd_info_ptr->four));
	sprintf(cmd_info_ptr->four, "%s", user_info.username);

	memset(oldname, 0, FILENAME_LEN);
	sprintf(oldname, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	memcpy(cmd_info_ptr->second, oldname, strlen(oldname));

	memset(newname, 0, FILENAME_LEN);
	sprintf(newname, "%s/%s", user_info.username, cmd_info_ptr->third);
	memset(cmd_info_ptr->third, 0, sizeof(cmd_info_ptr->third));
	memcpy(cmd_info_ptr->third, newname, strlen(newname));

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

sk2:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk2;
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
