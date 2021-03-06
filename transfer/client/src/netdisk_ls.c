#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

#define	RED		31
#define	GREEN	32
#define	YELLOW	33
#define	BLUE	34
#define	REGULAR	40

#define	PERSSION	(S_IRWXU||S_IXUSR||S_IRWXG||S_IXGRP||S_IRWXO||S_IXOTH)

typedef struct _name_info_t
{
	uint32_t st_code;
	char name[256];
}name_info_t;


int32_t netdisk_ls(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int nsend;
	int nrecv;
	uint32_t st_code;
	int first_sockfd;
	name_info_t name_info;
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

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, FILENAME_LEN);
	sprintf(cmd_info_ptr->second, "%s", filename);

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
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}

	if (st_code == FAILURE)
	{
		return -1;
	}

	while (1)
	{
		memset(&name_info, 0, sizeof(name_info));
sk2:	nrecv = recv(first_sockfd, &name_info, sizeof(name_info), MSG_DONTWAIT);
		if (nrecv == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				usleep(10);
				goto sk2;
			}
			return -1;
		}
		else if (nrecv == 0)
		{
			break;
		}

		if (name_info.st_code == GREEN)
		{
			printf("\033[32m%s\n\033[0m", name_info.name);
		}
		else if (name_info.st_code == BLUE)
		{
			printf("\033[34m%s\n\033[0m", name_info.name);
		}
	/*	else if (name_info.st_code == YELLOW)
		{
			printf("\033[33m%s\n\033[0m", name_info.name);
		}
	*/	else if (name_info.st_code == REGULAR)
		{
			printf("%s\n", name_info.name);
		}
	}

	return 0;
}
