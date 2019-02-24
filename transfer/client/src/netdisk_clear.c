#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_clear(zlog_category_t *zlog_handle)
{
	int ret;

	if (zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	ret = system("clear");
	if (ret == -1)
	{
		zlog_error(zlog_handle, "system()");
		return -1;
	}

	return 0;
}
