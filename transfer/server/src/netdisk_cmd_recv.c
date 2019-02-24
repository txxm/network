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

int32_t netdisk_cmd_recv(int first_connfd, zlog_category_t *zlog_handle)
{
	int ret;
	int nrecv;
	cmd_info_t cmd_info;

	memset(&cmd_info, 0, sizeof(cmd_info));
sk0:nrecv = recv(first_connfd, &cmd_info, sizeof(cmd_info), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}

	if (strncmp(cmd_info.first, "mkdir", 5) == 0)
	{
		ret = netdisk_mkdir_handler(&cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_mkdir_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "vim", 3) == 0)
	{
		ret = netdisk_vim_handler(&cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_vim_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "rm", 2) == 0)
	{
		ret = netdisk_remove_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_remove_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "scan", 4) == 0)
	{
		ret = netdisk_scan_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_scan_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "put", 3) == 0)
	{
		ret = netdisk_recvfile_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_recvfile_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "get", 3) == 0)
	{
		ret = netdisk_sendfile_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_sendfile_handler() error");
			return -1;
		}
	}	
	else if (strncmp(cmd_info.first, "cp", 2) == 0)
	{
		ret = netdisk_copy_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_copy_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "mv", 2) == 0)
	{
		ret = netdisk_move_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_move_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "rcy", 3) == 0)
	{
		if (cmd_info.third != NULL &&
				strncmp(cmd_info.second, "-r", 2) == 0)
		{
			ret = netdisk_recover_handler(first_connfd, &cmd_info, zlog_handle);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "netdisk_recover_handler() error");
				return -1;
			}
		}
		else if (strncmp(cmd_info.second, "-e", 2) == 0)
		{
			ret = netdisk_empty_handler(first_connfd, &cmd_info, zlog_handle);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "netdisk_empty_handler() error");
				return -1;
			}
		}
		else
		{
			ret = netdisk_recycle_handler(first_connfd, &cmd_info, zlog_handle);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "netdisk_recycle_handler() error");
				return -1;
			}
		}
	}
	else if(strncmp(cmd_info.first, "ls", 2) == 0)
	{
		ret = netdisk_ls_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_ls_handler() error");
			return -1;
		}
	}
	else if(strncmp(cmd_info.first, "ll", 2) == 0)
	{
		ret = netdisk_ll_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_ls_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "cd", 2) == 0)
	{
		ret = netdisk_cd_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_cd_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "tar", 3) == 0)
	{
		ret = netdisk_tar_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_tar_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "find", 4) == 0)
	{
		ret = netdisk_find_handler(first_connfd, &cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_find_handler() error");
			return -1;
		}
	}
	else if (strncmp(cmd_info.first, "down", 4) == 0)
	{
		ret = netdisk_download_handler(&cmd_info, zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_download_handler() error");
			return -1;
		}
	}

	return 0;
}
