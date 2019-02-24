#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_filetype(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int ret;
	struct stat st_buf;
	char filename[FILENAME_LEN];
	char filename_buf[FILENAME_LEN];

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
	sprintf(filename, "../my_file/%s", cmd_info_ptr->second);

	ret = stat(filename, &st_buf);
	if(ret == -1)
	{
		zlog_error(zlog_handle, "stat() error");
		return -1;
	}

	memset(filename_buf, 0, sizeof(filename_buf));
	switch(st_buf.st_mode & S_IFMT)
	{
		case S_IFREG:
		{
			sprintf(filename_buf, "%s.reg", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFDIR:
		{
			sprintf(filename_buf, "%s.dir", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFBLK:
		{
			sprintf(filename_buf, "%s.blk", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFSOCK:
		{
			sprintf(filename_buf, "%s.sock", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFLNK:
		{
			sprintf(filename_buf, "%s--link", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFCHR:
		{
			sprintf(filename_buf, "%s--char", filename);
			printf("%s\n", filename_buf);
			break;
		}
		case S_IFIFO:
		{
			sprintf(filename_buf, "%s--fifo", filename);
			printf("%s\n", filename_buf);
			break;
		}
		default:
		{
			sprintf(filename_buf, "%s*", filename);
			printf("%s\n", filename_buf);
			break;
		}
	}

	return -1;
}
