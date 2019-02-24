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

pthread_mutex_t g_mutex;

int32_t netdisk_login_recv(int first_connfd, zlog_category_t *zlog_handle)
{
	int fd, count;
	int err_1, err_2;
	int nread, offset;
	int nrecv, nsend;
	uint32_t st_code;
	char user_buf[33];
	char pass_buf[33];
	user_info_t user_info;

	count = 0;
	if (first_connfd < 0 || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(&user_info, 0, sizeof(user_info));
sk0:nrecv = recv(first_connfd, &user_info, sizeof(user_info_t), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		usleep(10);
		goto sk0;
	}

	fd = open("../user/userinfo", O_RDWR);
	if (fd < 0)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	offset = 0;
	while (1)
	{
		memset(user_buf, 0, sizeof(user_buf));
		nread = readline(fd, user_buf, sizeof(user_buf), offset);
		if (nread == -1)
		{
			zlog_error(zlog_handle, "readline() error");
			return -1;
		}
		else if (nread == 0)
		{
			if (err_1 != 0 || err_2 != 0)
			{
				st_code = FAILURE;
				send(first_connfd, &st_code, sizeof(uint32_t), MSG_DONTWAIT);
				count++;
				if (count >= USER_PASS_COUNT){
					return -1;
				}
				goto sk0;
			}
			else if (err_1 == 0 || err_2 == 0)
			{
				break;
			}
		}
		offset += nread;

		//err_1 = strncmp(user_buf, user_info.username, strlen(user_buf)-1);
		err_1 = strncmp(user_buf, user_info.username, 
							strlen(user_info.username));

		memset(pass_buf, 0, sizeof(pass_buf));
		nread = readline(fd, pass_buf, sizeof(pass_buf), offset);
		if (nread == -1)
		{
			zlog_error(zlog_handle, "readline() error");
			return -1;
		}
		else if (nread == 0)
		{
			if (err_1 == 0 && err_2 == 0){
				break;
			}
			return -1;
		}
		offset += nread;
		err_2 = strncmp(pass_buf, user_info.password, 32);

		nread = readline(fd, pass_buf, sizeof(pass_buf), offset);
		if (nread == -1)
		{
			zlog_error(zlog_handle, "readline() error");
			return -1;
		}
		else if (nread == 0)
		{
			if (err_1 == 0 && err_2 == 0){
				break;
			}
			return -1;
		}
		offset += nread;

		if (err_1 != 0 || err_2 != 0)
		{
			zlog_error(zlog_handle, "username or password is fail");
			continue;
		}
		else if (err_1 == 0 && err_2 == 0){
			break;
		}

	}

	st_code = SUCCESS;
sk1:nsend = send(first_connfd, &st_code, sizeof(uint32_t), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		usleep(10);
		goto sk1;
	}

	return 0;	
}

int readline(int fd, void *buf, size_t size, int offset)
{
	int nread;
	size_t count;
	char *pbuf = (char *)buf;

	if (pbuf == NULL){
		return -1;
	}

	count = 0;
	lseek(fd, offset, SEEK_SET);
	while (1)
	{
		nread = read(fd, pbuf, sizeof(char));
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				continue;
			}
			return -1;
		}
		else if (nread == 0)
		{
			return 0;
		}

		if (*pbuf == '-' || *pbuf == '\n')
		{
			count++;
			break;
		}
		count++;
		pbuf++;
	}

	return count;
}
