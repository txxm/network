#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int32_t netdisk_nonblock(int arg)
{
	int ret;
	int flag;

	flag = fcntl(arg, F_GETFL);
	if (flag == -1)
	{
		return -1;
	}

	flag |= O_NONBLOCK;
	ret = fcntl(arg, F_SETFL, flag);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}
