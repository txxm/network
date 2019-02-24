#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "zlog.h"
#include "netdisk.h"


int32_t netdisk_space(char *filename, char *username,
								zlog_category_t *zlog_handle)
{
	int fd;
	int ret;
	int nread;
	int nwrite;
	double left_size;
	struct stat st_buf;
	char *p;
	char buf[FILENAME_LEN];
	char user_buf[FILENAME_LEN];
	char filename1[FILENAME_LEN];
	char filename2[FILENAME_LEN];

	if (username == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	memset(filename1, 0, FILENAME_LEN);
	sprintf(filename1, "../user/%s", filename);

	memset(filename2, 0, FILENAME_LEN);
	sprintf(filename2, "../user/%s/left", username);

	ret = stat(filename1, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "stat()");
		return -1;
	}

	fd = open(filename2, O_RDWR);
	if (fd == -1)
	{
		zlog_error(zlog_handle, "zlog_handle()");
		return -1;
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
sk0:nread = read(fd, buf, sizeof(buf));
	if (nread == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}

	memset(user_buf, 0, sizeof(user_buf));
	memcpy(user_buf, buf, strlen(buf)-1);
	p = user_buf + (sizeof(user_buf)-1);
	left_size = (double)strtoul(user_buf, &p, 0);
	printf("%.lf\n", left_size);
	left_size -= st_buf.st_size;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%.lf", left_size);

	lseek(fd, 0, SEEK_SET);
sk1:nwrite = write(fd, buf, strlen(buf));
	if (nwrite == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}

	return 0;
}

int32_t netdisk_add(char *filename, char *username,
								zlog_category_t *zlog_handle)
{
	int fd;
	int ret;
	int nread;
	int nwrite;
	double left_size;
	struct stat st_buf;
	char *p;
	char buf[FILENAME_LEN];
	char user_buf[FILENAME_LEN];
	char filename1[FILENAME_LEN];
	char filename2[FILENAME_LEN];

	if (username == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	memset(filename1, 0, FILENAME_LEN);
	sprintf(filename1, "../user/%s", filename);

	memset(filename2, 0, FILENAME_LEN);
	sprintf(filename2, "../user/%s/left", username);

	ret = stat(filename1, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "stat()");
		return -1;
	}

	fd = open(filename2, O_RDWR);
	if (fd == -1)
	{
		zlog_error(zlog_handle, "zlog_handle()");
		return -1;
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
sk0:nread = read(fd, buf, sizeof(buf));
	if (nread == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}

	memset(user_buf, 0, sizeof(user_buf));
	memcpy(user_buf, buf, strlen(buf)-1);
	p = user_buf + (sizeof(user_buf)-1);
	left_size = (double)strtoul(user_buf, &p, 0);
	printf("%.lf\n", left_size);
	left_size += st_buf.st_size;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%.lf", left_size);

	lseek(fd, 0, SEEK_SET);
sk1:nwrite = write(fd, buf, strlen(buf));
	if (nwrite == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}

	return 0;
}
