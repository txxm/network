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
	int st_code;
	char name[256];
}name_info_t;

static int32_t open_dir(int first_connfd, char *dir_name,
									zlog_category_t *zlog_handle);

int32_t netdisk_ls_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
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
	sprintf(filename, "../user/%s", cmd_info_ptr->second);

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
	//int fd;
	int ret;
	//int nread;
	int nsend;
	DIR *pdir;
	uint32_t st_code;
	struct dirent *p;
	struct stat st_buf;
	name_info_t name_info;
	//char buf[1024];
	//char cmd[FILENAME_LEN];
	char name[FILENAME_LEN];
	//char name1[FILENAME_LEN];
	//char name2[FILENAME_LEN];

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
		ret = stat(name, &st_buf);
		if (ret == -1)
		{
			perror("");
			zlog_error(zlog_handle, "stat() error");
			return -1;
		}

		if ((st_buf.st_mode & S_IFMT) == S_IFDIR)
		{
			memset(&name_info, 0, sizeof(name_info));
			name_info.st_code = BLUE;
			memcpy(name_info.name, p->d_name, strlen(p->d_name));
sk2:		nsend = send(first_connfd, &name_info, sizeof(name_info),
							MSG_DONTWAIT);
			if (nsend == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					usleep(10);
					goto sk2;
				}
			}
			else if (nsend == 0)
			{
				printf("connection break\n");
				return -1;
			}
			//printf("\033[34m%s\n\033[0m", p->d_name);
		}
		else if ((st_buf.st_mode & PERSSION) != 0)
		{
			memset(&name_info, 0, sizeof(name_info));
			name_info.st_code = GREEN;
			memcpy(name_info.name, p->d_name, strlen(p->d_name));
sk3:		nsend = send(first_connfd, &name_info, sizeof(name_info),
							MSG_DONTWAIT);
			if (nsend == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					usleep(10);
					goto sk3;
				}
			}
			else if (nsend == 0)
			{
				printf("connection break\n");
				return -1;
			}
			//printf("\033[32m%s\n\033[0m", p->d_name);
		}
		else if ((st_buf.st_mode & S_IFMT) == S_IFREG)
		{
/*			memset(name1, 0, FILENAME_LEN);
			sprintf(name1, "%s/%s", dir_name, p->d_name);
			memset(name2, 0, FILENAME_LEN);
			sprintf(name2, "%s/%s.txt", dir_name, p->d_name);
			memset(cmd, 0, FILENAME_LEN);
			sprintf(cmd, "xxd %s > %s", name1, name2);
			ret = system(cmd);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "system() error");
				return -1;
			}

			fd = open(name2, O_RDWR);
			if (fd == -1)
			{
				zlog_error(zlog_handle, "open() error");
				return -1;
			}

sk4:		nread = read(fd, buf, 1024);
			if (nread == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					usleep(10);
					goto sk4;
				}
				return -1;
			}
			close(fd);

			ret = remove(name2);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "remove() error");
				return -1;
			}

			if (strncmp(buf+9, "ffd8", 4) == 0 ||
				strncmp(buf+9, "8950 4E47 0D0A 1A0A", 19) == 0)
			{
				memset(&name_info, 0, sizeof(name_info));
				name_info.st_code = YELLOW;
				memcpy(name_info.name, p->d_name, strlen(p->d_name));
sk5:			nsend = send(first_connfd, &name_info, sizeof(name_info),
								MSG_DONTWAIT);
				if (nsend == -1)
				{
					if (errno == EINTR || errno == EAGAIN)
					{
						usleep(10);
						goto sk5;
					}
				}
				else if (nsend == 0)
				{
					printf("connection break\n");
					return -1;
				}
				//printf("\033[33m%s\n\033[0m", p->d_name);
			}
			else
			{
		*/		memset(&name_info, 0, sizeof(name_info));
				name_info.st_code = REGULAR;
				memcpy(name_info.name, p->d_name, strlen(p->d_name));
sk6:			nsend = send(first_connfd, &name_info, sizeof(name_info),
								MSG_DONTWAIT);
				if (nsend == -1)
				{
					if (errno == EINTR || errno == EAGAIN)
					{
						usleep(10);
						goto sk6;
					}
				}
				else if (nsend == 0)
				{
					printf("connection break\n");
					return -1;
				}
				//printf("%s\n", p->d_name);
			//}
		}
	}
	closedir(pdir);

	return 0;
}
