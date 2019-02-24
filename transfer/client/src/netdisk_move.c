#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_move(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	//int ret;
	int nsend;
	//char c;
	//int fd1, fd2;
	//int cmdlen;
	//char buf[BUF_SIZE];
	//int nread, nwrite;
	int first_sockfd;
	//char cmd[FILENAME_LEN];
	char oldname[FILENAME_LEN];
	char newname[FILENAME_LEN];

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	if (strlen(cmd_info_ptr->second) <=0 ||
			strlen(cmd_info_ptr->second) >= FILENAME_LEN)
	{
		zlog_error(zlog_handle, "filename too long");
		return -1;
	}

	memset(oldname, 0, FILENAME_LEN);
	sprintf(oldname, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	sprintf(cmd_info_ptr->second, "%s", oldname);

	memset(newname, 0, FILENAME_LEN);
	sprintf(newname, "%s/%s", user_info.username, cmd_info_ptr->third);
	memset(cmd_info_ptr->third, 0, sizeof(cmd_info_ptr->third));
	sprintf(cmd_info_ptr->third, "%s", newname);
/*
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = '/';
	strncat(cmd, cmd_info_ptr->second, strlen(cmd_info_ptr->second));
	memset(cmd_info_ptr->second, 0, CMD_LEN);
	memcpy(cmd_info_ptr->second,user_info.username,strlen(user_info.username));
	strncat(cmd_info_ptr->second, cmd, strlen(cmd));

	memset(cmd, 0, sizeof(cmd));
	cmd[0] = '/';
	strncat(cmd, cmd_info_ptr->third, strlen(cmd_info_ptr->third));
	memset(cmd_info_ptr->third, 0, CMD_LEN);
	memcpy(cmd_info_ptr->third, user_info.username,strlen(user_info.username));
	strncat(cmd_info_ptr->third, cmd, strlen(cmd));

	fd1 = open(oldname, O_RDWR);
	if (fd1 == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}
	ret = netdisk_nonblock(fd1);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}


	fd2 = open(newname, O_CREAT | O_EXCL | O_RDWR, RDWR_DEF);
	if (fd2 == -1)
	{
		if (errno == EEXIST)
		{
			printf("fliename already exist. Whether or not to cover? ");
			scanf("%c", &c);
			getchar();
			if (isalpha(c))
			{
				if (tolower(c) == 'y')
				{
					fd2 = open(newname, O_RDWR | O_TRUNC);
					if (fd2 == -1)
					{
						zlog_error(zlog_handle, "open() error");
						return -1;
					}

					ret = netdisk_nonblock(fd2);
					if (ret == -1)
					{
						zlog_error(zlog_handle, "netdisk_nonblock() error");
						return -1;
					}

					while (1)
					{
sk0:					nread = read(fd1, buf, sizeof(buf));
						if (nread == -1)
						{
							if (errno == EAGAIN || errno == EINTR)
							{
								usleep(10);
								goto sk0;
							}
							return -1;
						}
						else if (nread == 0)
						{
							break;
						}

sk1:					nwrite = write(fd2, buf, nread);
						if (nwrite == -1)
						{
							if (errno == EAGAIN || errno == EINTR)
							{
								usleep(10);
								goto sk1;
							}
							return -1;
						}
						else if (nwrite == 0)
						{
							return 0;
						}
						fsync(fd2);
					}
				}
				else if (tolower(c) == 'n')
				{
					return 0;
				}
			}
		}
	}

	ret = netdisk_nonblock(fd2);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	while (1)
	{
sk3:	nread = read(fd1, buf, sizeof(buf));
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk3;
			}
			return -1;
		}
		else if (nread == 0)
		{
			break;
		}

sk4:	nwrite = write(fd2, buf, nread);
		if (nwrite == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk4;
			}
			return -1;
		}
		else if (nwrite == 0)
		{
			break;
		}
		fsync(fd2);
	}

	ret = remove(oldname);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "remove() error");
		return -1;
	}
*/
	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

sk2:nsend = send(first_sockfd, cmd_info_ptr, sizeof(cmd_info_t), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk2;
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

	return 0;
}
