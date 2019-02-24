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

int32_t netdisk_move_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle)
{
	int ret;
	int fd1, fd2;
	char buf[BUF_SIZE];
	int nread, nwrite;
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

	memset(oldname, 0, FILENAME_LEN);
	sprintf(oldname, "../user/%s", cmd_info_ptr->second);
	memset(newname, 0, FILENAME_LEN);
	sprintf(newname, "../user/%s", cmd_info_ptr->third);

	fd1 = open(oldname, O_RDWR);
	if (fd1 == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}
	fd2 = open(newname, O_CREAT | O_RDWR, RDWR_DEF);
	if (fd2 == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	ret = netdisk_nonblock(fd1);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}
	ret = netdisk_nonblock(fd2);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	while (1)
	{
sk0:	nread = read(fd1, buf, sizeof(buf));
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk0;
			}
			return -1;
		}
		else if (nread == 0)
		{
			break;
		}

sk1:	nwrite = write(fd2, buf, nread);
		if (nwrite == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk1;
			}
			return -1;
		}
		else if (nwrite == 0)
		{
			break;
		}
		fsync(fd2);
	}

	ret = remove(oldname);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "remove() error");
		return -1;
	}

	return 0;
}
