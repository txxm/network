#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_exit()
{
	char c;

	printf("Quit anyway? (y or n)");
	scanf("%c", &c);
	getchar();

	if (isalpha(c))
	{
		if (tolower(c) == 'y')
		{
			printf("Process Exit.\n");
			exit(0);
		}
		else if (tolower(c) == 'n')
		{
			return 0;
		}
	}
	else
	{
		printf("Undefined command: %c.  Try 'man exit'\n", c);
		return 0;
	}

	return 0;
}
