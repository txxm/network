#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

typedef struct _space_info_t
{
    uint32_t size;
	char name_buf[FILENAME_LEN];
}space_info_t;

int32_t netdisk_get(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int count;
	int fd, ret;
	int nsend;
	off_t offset;
	uint32_t st_code;
	int first_sockfd;
	int nrecv, nwrite;
	space_info_t space_info;
	char cmd[FILENAME_LEN];
	char buf[BUF_SIZE];
	char name[FILENAME_LEN];
	char filename[FILENAME_LEN];
	char filename1[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

	memset(filename, 0, FILENAME_LEN);
	sprintf(filename, "../%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = '/';
	strncat(cmd, cmd_info_ptr->second, strlen(cmd_info_ptr->second));
	memset(cmd_info_ptr->second, 0, CMD_LEN);
	memcpy(cmd_info_ptr->second,user_info.username,strlen(user_info.username));
	strncat(cmd_info_ptr->second, cmd, strlen(cmd));

sk0:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk0;
		}
		else{
			return -1;
		}
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
sk: nrecv = recv(first_sockfd, &space_info, sizeof(space_info), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk;
		}
		else{
			return -1;
		}
	}
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}

	memset(filename1, 0, FILENAME_LEN);
	sprintf(filename1, "../%s/download/%s", user_info.username,
				space_info.name_buf);

	fd = open(filename1, O_CREAT | O_EXCL | O_RDWR, RDWR_DEF);
	if (fd == -1)
	{
		if (errno == EEXIST)
		{
			memset(filename1, 0, FILENAME_LEN);
			sprintf(filename1,"../%s/download/%s",user_info.username,
						space_info.name_buf);
			fd = open(filename1, O_RDWR, RDWR_DEF);
			if (fd == -1)
			{
				zlog_error(zlog_handle, "open() error");
				return -1;
			}
		}
		else
		{
			zlog_error(zlog_handle, "open() error");
			return -1;
		}
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	offset = lseek(fd, 0, SEEK_END);
ag:	nsend = send(first_sockfd, &offset, sizeof(offset), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto ag;
		}
		else
		{
			zlog_error(zlog_handle, "recv() error");
			return -1;
		}
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	count = offset;
	while (1)
	{
		if (count >= space_info.size){
			break;
		}
		memset(buf, 0, BUF_SIZE);
sk1:	nrecv = recv(first_sockfd, buf, BUF_SIZE, MSG_DONTWAIT);
		if (nrecv == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk1;
			}
			else
			{
				zlog_error(zlog_handle, "recv() error");
				return -1;
			}
		}
		else if (nrecv == 0)
		{
			printf("connection break\n");
			break;
		}

sk2:	nwrite = write(fd, buf, nrecv);
		if (nwrite == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk2;
			}
			return -1;
		}
		else if (nwrite == 0)
		{
			printf("write done\n");
			break;
		}
		fsync(fd);
		count += nwrite;
		printf("#################[%.5f]\n",
					(double)count/(double)space_info.size);
	}

	st_code = SUCCESS;
sk3:nsend = send(first_sockfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk3;
		}
		else
		{
			zlog_error(zlog_handle, "recv() error");
			return -1;
		}
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	close(fd);
	return 0;
}
