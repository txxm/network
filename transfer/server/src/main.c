#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

static void *thread_login_proc(void *arg);
static void *thread_reg_proc(void *arg);
static void *thread_respon_proc(void *arg);

int main(int argc, char **argv)
{
	int ret;
	pthread_t tid_respon;
	pthread_t tid_reg;
	pthread_t tid_login_reg;
	zlog_category_t *zlog_handle;

	if (argc != 1)
	{
		printf("invalid argument\n");
		return -1;
	}

	ret = zlog_init("../conf/netdisk.conf");
	if (ret == -1){
		perror("zlog_init()");
		return -1;
	}

	zlog_handle = zlog_get_category("../conf/my_cat");
	if (zlog_handle == NULL){
		zlog_fini();
		return -1;
	}

	ret = pthread_create(&tid_login_reg, NULL, (void *)(*thread_login_proc),
							(void*)zlog_handle);
	if (ret != 0)
	{
		zlog_error(zlog_handle, "pthread_create() error");
		return -1;
	}

	ret = pthread_create(&tid_reg, NULL, (void *)(*thread_reg_proc),
							(void*)zlog_handle);
	if (ret != 0)
	{
		zlog_error(zlog_handle, "pthread_create() error");
		return -1;
	}

	ret = pthread_create(&tid_respon, NULL, (void *)(*thread_respon_proc),
							(void*)zlog_handle);
	if (ret != 0)
	{
		zlog_error(zlog_handle, "pthread_create() error");
		return -1;
	}

	zlog_fini();
	pause();
	return 0;
}

static void *thread_login_proc(void *arg)
{
	int ret;

	ret = pthread_detach(pthread_self());
	if (ret != 0)
	{
		zlog_error((zlog_category_t *)arg, "pthread_detach() error:");
		return NULL;
	}

	ret = netdisk_login_connect((zlog_category_t *)arg);
	if (ret == -1)
	{
		zlog_error((zlog_category_t *)arg, "netdisk_login_connect() error:");
		return NULL;
	}

	pthread_exit(NULL);
	return NULL;
}

static void *thread_reg_proc(void *arg)
{
	int ret;

	ret = pthread_detach(pthread_self());
	if (ret != 0)
	{
		zlog_error((zlog_category_t *)arg, "pthread_detach() error:");
		return NULL;
	}

	ret = netdisk_reg_connect((zlog_category_t *)arg);
	if (ret == -1)
	{
		zlog_error((zlog_category_t *)arg, "netdisk_reg_connect() error:");
		return NULL;
	}

	pthread_exit(NULL);
	return NULL;
}

static void *thread_respon_proc(void *arg)
{
	int ret;
	int first_sockfd;

	ret = pthread_detach(pthread_self());
	if (ret != 0)
	{
		zlog_error((zlog_category_t *)arg, "pthread_detach() error:");
		return NULL;
	}

	first_sockfd = netdisk_connect((zlog_category_t *)arg);
	if (first_sockfd == -1)
	{
		zlog_error((zlog_category_t *)arg, "netdisk_connect() error:");
		return NULL;
	}

	pthread_exit(NULL);
	return NULL;
}
