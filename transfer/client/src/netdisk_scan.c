#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_scan(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int size;
	int nsend;
	uint32_t st_code;
	int first_sockfd;
	int nrecv;
	struct stat st_buf;
	char buf[BUF_SIZE];
	char name[FILENAME_LEN];
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

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	sprintf(cmd_info_ptr->second, "%s", name);

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

sk:	memset(&st_buf, 0, sizeof(st_buf));
	nrecv = recv(first_sockfd, &st_buf, sizeof(st_buf), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "%s", cmd_info_ptr->second);

	printf("=================================\n");
	printf("filename: %s\n", filename);
	printf("=================================\n");
	printf("content:\n");

	size = 0;
	while (1)
	{
		if (size >= st_buf.st_size)
		{
			printf("=================================\n");
			printf("<scan over>\n");
			printf("=================================\n");
			break;
		}
sk1:	memset(buf, 0, sizeof(buf));
		nrecv = recv(first_sockfd, buf, sizeof(buf), MSG_DONTWAIT);
		if (nrecv == -1)
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
		else if (nrecv == 0){
			break;
		}

		printf("%s\n", buf);
		size += nrecv;
	}

	st_code = SUCCESS;
sk4:nsend = send(first_sockfd, &st_code, sizeof(st_code),MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk4;
		}
		else{
			return -1;
		}
	}
	else if (nsend == 0)
	{
		return 0;
	}

	return 0;
}
