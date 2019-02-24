#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

static int32_t net_cmd_handler(cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);

static zlog_category_t *handle;
static void sig_handler(int signo);

int32_t netdisk_cmd_request(zlog_category_t *zlog_handle)
{
	int ret;
	int i, j;
	int count;
	char *p;
	char buf[CMD_LEN];
	cmd_info_t cmd_info;

	handle = zlog_handle;
	signal(SIGINT, sig_handler);

	if (zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "netdisk_cmd_request() error:");
		return -1;
	}

	memset(g_path, 0, GLOBAL_DIR_PATH);
	sprintf(g_path, "../%s", user_info.username);
	while (1)
	{
		printf("\033[31m%s>>\033[0m", g_path);

		memset(buf, 0, sizeof(buf));
		if (fgets(buf, sizeof(buf), stdin) == NULL){
			continue;
		}
		buf[strlen(buf)-1] = '\0';

		count = 0;
		memset(&cmd_info, 0, sizeof(cmd_info));
		for ( i = j = 0, count = 0, p = buf; i < strlen(buf); i++)
		{
			if (*p == ' ')
			{
				count++;
				j = 0;
				p++;
				if (*p == '\0'){
					break;
				}
			}

			switch (count)
			{
				case 0:
				{
					cmd_info.first[j] = *p;
					p++;
					j++;
					break;
				}
				case 1:
				{
					cmd_info.second[j] = *p;
					p++;
					j++;
					break;
				}
				case 2:
				{
					cmd_info.third[j] = *p;
					p++;
					j++;
					break;
				}
			}
		}

		ret = net_cmd_handler(&cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "net_cmd_handler() error");
			continue;
		}
	}

	return 0;
}

static int32_t net_cmd_handler(cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle)
{
	int ret;

	if (cmd_info_ptr == NULL)
	{
		zlog_error(zlog_handle, "invalid argument.");
		return -1;
	}

	if (strncmp(cmd_info_ptr->first, "mkdir", 5) == 0)
	{
		ret = netdisk_mkdir(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "mkdir failure.");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "vim", 3) == 0)
	{
		ret = netdisk_vim(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "create file failure");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "rm", 2) == 0)
	{
		ret = netdisk_remove(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("remove file failure\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "scan", 4) == 0)
	{
		ret = netdisk_scan(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("scan file failure\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "down", 4) == 0)
	{
		ret = netdisk_download(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("download file failure\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "clear", 5) == 0)
	{
		ret = netdisk_clear(zlog_handle);
		if (ret == -1)
		{
			printf("clear failure\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "get", 3) == 0)
	{
		ret = netdisk_get(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("get '%s' failure\n", cmd_info_ptr->second);
			return -1;
		}
		else
		{
			printf("get '%s' success\n", cmd_info_ptr->second);
		}
	}
	else if (strncmp(cmd_info_ptr->first, "put", 3) == 0)
	{
		ret = netdisk_put(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("put '%s' failure\n", cmd_info_ptr->second);
			return -1;
		}
		else
		{
			printf("put '%s' success\n", cmd_info_ptr->second);
		}
	}
	else if (strncmp(cmd_info_ptr->first, "ls", 2) == 0)
	{
		ret = netdisk_ls(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("Not such a directory\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "ll", 2) == 0)
	{
		ret = netdisk_ll(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("Not such a directory\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "cd", 2) == 0)
	{
		ret = netdisk_cd(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("switch directory failure\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "find", 4) == 0)
	{
		ret = netdisk_find(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("not find such file\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "man", 3) == 0)
	{
		ret = netdisk_cmd_help(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("command not found\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "cp", 2) == 0)
	{
		ret = netdisk_copy(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("copy file fail\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "mv", 2) == 0)
	{
		ret = netdisk_move(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("move file fail\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "exit", 4) == 0)
	{
		ret = netdisk_exit();
		if (ret == -1)
		{
			printf("Not exit\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "pwd", 3) == 0)
	{
		ret = netdisk_pwd();
		if (ret == -1)
		{
			printf("Not show path\n");
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "tar", 3) == 0)
	{
		ret = netdisk_tar(cmd_info_ptr, zlog_handle);
		if (ret == -1)
		{
			printf("Not archive %s\n", cmd_info_ptr->second);
			return -1;
		}
	}
	else if (strncmp(cmd_info_ptr->first, "rcy", 3) == 0)
	{
		if (cmd_info_ptr->third != NULL &&
				strncmp(cmd_info_ptr->second, "-r", 2) == 0)
		{
			ret = netdisk_recover(cmd_info_ptr, zlog_handle);
			if (ret == -1)
			{
				printf("Not recover %s\n", cmd_info_ptr->third);
				return -1;
			}
		}
		else if (strncmp(cmd_info_ptr->second, "-e", 2) == 0)
		{
			ret = netdisk_empty(cmd_info_ptr, zlog_handle);
			if (ret == -1)
			{
				printf("Not empty recycle\n");
				return -1;
			}
		}
		else
		{
			ret = netdisk_recycle(cmd_info_ptr, zlog_handle);
			if (ret == -1)
			{
				printf("Not recycle %s\n", cmd_info_ptr->second);
				return -1;
			}
		}
	}
	else
	{
		printf("Undefined command: '%s'.\n", cmd_info_ptr->first);
		return -1;
	}

	return 0;
}

static void sig_handler(int signo)
{
	int ret;

	getchar();
	printf("\n");

	ret = netdisk_cmd_request(handle);
	if (ret == -1)
	{
		zlog_error(handle, "error");
		return ;
	}

	return ;
}
