#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"


static int32_t client_netdisk_info(int first_connfd, user_info_t *user_info_ptr,
										zlog_category_t *zlog_handle);

int32_t netdisk_reg_recv(int first_connfd, zlog_category_t *zlog_handle)
{
	int ret;
	int nwrite;
	int fd, count;
	int nrecv, nsend;
	uint32_t st_code;
	char user_buf[256];
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
		printf("connection break\n");
		return -1;
	}

	fd = open("../user/userinfo", O_RDWR);
	if (fd < 0)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	memset(user_buf, 0, sizeof(user_buf));
	memcpy(user_buf, user_info.username, strlen(user_info.username));
	user_buf[strlen(user_info.username)] = '-';
	strncat(user_buf, user_info.password, PASS_NAME);
	user_buf[strlen(user_buf)] = '-';
	strncat(user_buf, user_info.reg_date, strlen(user_info.reg_date));

	lseek(fd, 0, SEEK_END);
sk1:nwrite = write(fd, user_buf, strlen(user_buf));
	if (nwrite == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}
	fsync(fd);

	ret = client_netdisk_info(first_connfd, &user_info, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_client_info() error");
		return -1;
	}

	if (nwrite == strlen(user_buf))
	{
		st_code = SUCCESS;
sk2:	nsend = send(first_connfd, &st_code, sizeof(uint32_t), MSG_DONTWAIT);
		if (nsend == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				usleep(10);
				goto sk2;
			}
			return -1;
		}
		else if (nsend == 0)
		{
			printf("connection break\n");
			return -1;
		}
	}
	close(fd);

	return -1;	
}

static int32_t client_netdisk_info(int first_connfd, user_info_t *user_info_ptr,
										zlog_category_t *zlog_handle)
{
	int ret;
	int fd1, fd2;
	int nsend, nwrite;
	double left_size;
	double used_size;
	char buf1[FILENAME_LEN];
	char buf2[FILENAME_LEN];
	char name[FILENAME_LEN];
	netdisk_user_info_t netdisk_user_info;

	memset(&netdisk_user_info, 0, sizeof(netdisk_user_info));
	memcpy(netdisk_user_info.version, "1.0", strlen("1.0"));
	netdisk_user_info.user_tot = USER_TOTAL_SPACE;
	netdisk_user_info.user_left = USER_LEFT_SPACE;
	netdisk_user_info.user_used = USER_USED_SPACE;

sk3:nsend = send(first_connfd, &netdisk_user_info, sizeof(netdisk_user_info), 
					MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk3;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		if (errno == EEXIST){
			return -1;
		}
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}
	
	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/document", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/info", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}
	
	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/download", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}
	
	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/recycle", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/recover", user_info_ptr->username);
	ret = mkdir(name, RDWR_DEF);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "mkdir() error");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/left", user_info_ptr->username);
	fd1 = open(name, O_CREAT | O_RDWR, RDWR_DEF);
	if (fd1 == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	memset(name, 0, FILENAME_LEN);
	sprintf(name, "../user/%s/used", user_info_ptr->username);
	fd2 = open(name, O_CREAT | O_RDWR, RDWR_DEF);
	if (fd2 == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	left_size = USER_LEFT_SPACE;
	memset(buf1, 0, sizeof(buf1));
	sprintf(buf1, "%.lf", left_size);

	used_size = USER_USED_SPACE;
	memset(buf2, 0, sizeof(buf2));
	sprintf(buf2, "%.lf", used_size);

sk:	nwrite = write(fd1, buf1, strlen(buf1));
	if (nwrite == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk;
		}
		return -1;
	}

ag:	nwrite = write(fd2, buf1, strlen(buf1));
	if (nwrite == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto ag;
		}
		return -1;
	}

	close(fd1);
	close(fd2);

	return 0;
}
