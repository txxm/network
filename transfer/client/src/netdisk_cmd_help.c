#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "zlog.h"
#include "netdisk.h"

static int32_t netdisk_cmd_descript(char *cmd_str,
										zlog_category_t *zlog_handle);

int32_t netdisk_cmd_help(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int ret;

	if (cmd_info_ptr == NULL || zlog_handle == NULL){
		zlog_info(zlog_handle, "cmd_info_ptr is null");
		return -1;
	}

	printf("%s\n", cmd_info_ptr->first);

	printf("NAME\n");
	printf("\t%s [argument]...\n", cmd_info_ptr->second);

	printf("DESCRIPTION\n");
	ret = netdisk_cmd_descript(cmd_info_ptr->second, zlog_handle);
	if (ret == -1){
		zlog_error(zlog_handle, "netdisk_cmd_descript() error");
		return -1;
	}

	return 0;
}

static int32_t netdisk_cmd_descript(char *cmd_str, zlog_category_t *zlog_handle)
{
	if (cmd_str == NULL){
		return -1;
	}

	if (strncmp(cmd_str, "get", 3) == 0)
	{
		printf("\tGet the FILES from remote server\n");
	}
	else if (strncmp(cmd_str, "put", 3) == 0)
	{
		printf("\tput the FILES into the remote server\n");
	}
	else if (strncmp(cmd_str, "vim", 3) == 0)
	{
		printf("\tCreate a new file\n");
	}
	else if (strncmp(cmd_str, "mkdir", 5) == 0)
	{
		printf("\tCreate a new directory\n");
	}
	else if (strncmp(cmd_str, "rm", 2) == 0)
	{
		printf("\tremove the FILE\n");
	}
	else if (strncmp(cmd_str, "scan", 3) == 0)
	{
		printf("\tscan the content of FILE\n");
	}
	else if (strncmp(cmd_str, "sort", 4) == 0)
	{
		printf("\tsort the FILES\n");
	}
	else if (strncmp(cmd_str, "find", 4) == 0)
	{
		printf("\tfind FILE which we need\n");
	}

	return 0;
}
