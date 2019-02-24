#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_mkdir_handler(cmd_info_t *cmd_info_ptr, 
		zlog_category_t *zlog_handle)
{
	int ret;
	char filename[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument.");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "../user/%s", cmd_info_ptr->second);
	ret = mkdir(filename, RDWR_DEF);
	if (ret == -1)
	{
		if (errno == EEXIST)
		{
			sprintf(filename, "../user_file/%s.swp", cmd_info_ptr->second);
			ret = mkdir(filename, RDWR_DEF);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "open() error:");
				return -1;
			}
		}
		zlog_error(zlog_handle, "open() error:");
		return -1;
	}

	return 0;
}
