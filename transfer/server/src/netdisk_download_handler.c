#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_download_handler(cmd_info_t *cmd_info_ptr,
			zlog_category_t *zlog_handle)
{
	int ret;
	char filename[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "wget -qP ../user/%s/download %s", cmd_info_ptr->second,
				cmd_info_ptr->third);
	ret = system(filename);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "system() error:");
		return -1;
	}

	return 0;
}
