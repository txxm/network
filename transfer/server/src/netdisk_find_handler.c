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

static char find_buf[1024];

static int32_t fsize(char *name, char *pname, char *filename, int first_connfd,
						zlog_category_t *zlog_handle);
static int32_t open_dir(char *dir_name, char *filename, int first_connfd,
									zlog_category_t *zlog_handle);

int32_t netdisk_find_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle)
{
	int ret;
	int nsend;
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

	memset(find_buf, 0, 1024);

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "%s", cmd_info_ptr->second);

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s", cmd_info_ptr->third);

	ret = fsize(name, NULL, filename, first_connfd, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "fsize() error");
		return -1;
	}

sk0:nsend = send(first_connfd, find_buf,strlen(find_buf),MSG_DONTWAIT);
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
	printf("%s\n", find_buf);
	printf("%d\n", nsend);
	usleep(20);

	return 0;
}

static int32_t fsize(char *name, char *pname, char *filename, int first_connfd,
						zlog_category_t  *zlog_handle)
{
	int ret;
	struct stat st_buf;

	if (name == NULL || filename == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	ret = stat(name, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "stat() error");
		return -1;
	}

	if ((st_buf.st_mode & S_IFMT) == S_IFDIR)
	{
		ret = open_dir(name, filename, first_connfd, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "open_dir() error");
			return -1;
		}
	}
	else
	{
		if (strncmp(pname, filename, strlen(filename)) == 0)
		{
			strncat(find_buf, name, strlen(name));
			strncat(find_buf, "\n", 1);
		}
	}

	return 0;
}

int32_t open_dir(char *dir_name, char *filename, int first_connfd, 
					zlog_category_t *zlog_handle)
{
	int ret;
	DIR *pdir;
	char name[FILENAME_LEN];
	struct dirent *p;

	if (dir_name == NULL || filename == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	pdir = opendir(dir_name);
	if (pdir == NULL)
	{
		zlog_error(zlog_handle, "opendir() error");
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
		else
		{
			memset(name, 0, FILENAME_LEN);
			sprintf(name, "%s/%s", dir_name, p->d_name);
			ret = fsize(name, p->d_name, filename, first_connfd, zlog_handle);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "fsize() error");
				return -1;
			}
		}
	}
	closedir(pdir);

	return 0;
}
