#include <stdio.h>
#include <stdint.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_pwd()
{
	printf("%s\n", g_path);

	return 0;
}
