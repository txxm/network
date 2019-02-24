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

static int32_t open_dir(int first_connfd, char *dir_name,
									zlog_category_t *zlog_handle);

int32_t netdisk_empty_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle)
{
	int ret;
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
	sprintf(filename, "../user/%s/recycle", cmd_info_ptr->third);

	ret = open_dir(first_connfd, filename, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "open_dir() error");
		return -1;
	}

	return 0;
}

int32_t open_dir(int first_connfd, char *dir_name, zlog_category_t *zlog_handle)
{
	int ret;
	int nsend;
	DIR *pdir;
	uint32_t st_code;
	struct dirent *p;
	char name[FILENAME_LEN];

	if (dir_name == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	pdir = opendir(dir_name);
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

	while ((p = readdir(pdir)) != NULL)
	{
		if (strcmp(p->d_name, ".") ==  0 || strcmp(p->d_name, "..") ==  0){
			continue;
		}
		if (strlen(dir_name) + strlen(p->d_name) +2 > 255){
			fprintf(stderr, "Too long file name!\n");
		}

		memset(name, 0, FILENAME_LEN);
		memcpy(name, dir_name, strlen(dir_name));
		name[strlen(name)] = '/';
		strncat(name, p->d_name, strlen(p->d_name));

		ret = remove(name);
		if (ret == -1)
		{
			zlog_error(zlog_handle,"remove() error");
			return -1;
		}
	}
	closedir(pdir);

	return 0;
}
