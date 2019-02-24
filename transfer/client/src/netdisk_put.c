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

#define	PTHREAD_NUM	100

static void *pthread_proc(void *arg);

static pthread_mutex_t g_mutex;
static int first_sockfd;
static off_t offset;
static struct stat st_buf;

int32_t netdisk_put(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle)
{
	int i;
	int cpu_num;
	int fd, ret;
	int nsend;
	int nrecv;
	uint32_t st_code;
	pthread_t tid[PTHREAD_NUM];
	char filename1[FILENAME_LEN];
	char filename2[FILENAME_LEN];

	offset = 0;

	if (cmd_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	if (cpu_num < 1)
	{
		zlog_error(zlog_handle, "sysconf() error");
		return -1;
	}

	first_sockfd = netdisk_connect(zlog_handle);
	if (first_sockfd == -1)
	{
		zlog_error(zlog_handle, "netdisk_connect() error");
		return -1;
	}

	memset(filename1, 0, FILENAME_LEN);
	sprintf(filename1, "%s/%s", user_info.username, cmd_info_ptr->second);
	memset(cmd_info_ptr->second, 0, sizeof(cmd_info_ptr->second));
	sprintf(cmd_info_ptr->second, "%s", filename1);

	memset(filename2, 0, FILENAME_LEN);
	sprintf(filename2, "%s", cmd_info_ptr->third);

	memset(cmd_info_ptr->four, 0, sizeof(cmd_info_ptr->four));
	sprintf(cmd_info_ptr->four, "%s", user_info.username);

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

	fd = open(filename2, O_RDWR, RDWR_DEF);
	if (fd == -1)
	{
		zlog_error(zlog_handle, "open() error");
		return -1;
	}

	ret = fstat(fd, &st_buf);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "fstat() error");
		return -1;
	}

sk:	nsend = send(first_sockfd, &st_buf, sizeof(st_buf), MSG_DONTWAIT);
	if (nsend == -1)
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
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

	ret = netdisk_nonblock(fd);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_nonblock() error");
		return -1;
	}

	for (i = 0; i < cpu_num; i++)
	{
		ret = pthread_create(&tid[i],NULL,(void *)(*pthread_proc),(void *)&fd);
		if (ret != 0)
		{
			zlog_error(zlog_handle, "pthread_create() error");
			return -1;
		}
	}

	for (i = 0; i < cpu_num; i++)
	{
		ret = pthread_join(tid[i], NULL);
		if (ret != 0)
		{
			zlog_error(zlog_handle, "pthread_join() error");
			return -1;
		}
	}

sk1:nrecv = recv(first_sockfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk1;
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

	if (st_code == SUCCESS)
	{
		return 0;
	}
	else
	{
		return -1;
	}

	close(fd);
	return 0;
}

static void *pthread_proc(void *arg)
{
	int fd;
	int nread, nsend;
	char buf[BUF_SIZE];

	fd = *(int *)arg;

	while (1)
	{
		pthread_mutex_lock(&g_mutex);
		memset(buf, 0, BUF_SIZE);
		lseek(fd, offset, SEEK_SET);
sk1:	nread = read(fd, buf, BUF_SIZE);
		if (nread == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk1;
			}
			pthread_mutex_unlock(&g_mutex);
			return NULL;
		}
		else if (nread == 0)
		{
			pthread_mutex_unlock(&g_mutex);
			break;
		}

sk2:	nsend = send(first_sockfd, buf, nread, MSG_DONTWAIT);
		if (nsend == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				usleep(10);
				goto sk2;
			}
			pthread_mutex_unlock(&g_mutex);
			return NULL;
		}
		else if (nsend == 0)
		{
			printf("connection break\n");
			pthread_mutex_unlock(&g_mutex);
			break;
		}
		offset += nread;
		printf("####################[%.5f]\n", 
			(double)offset/(double)st_buf.st_size);
		pthread_mutex_unlock(&g_mutex);
	}

	pthread_exit(NULL);
}
