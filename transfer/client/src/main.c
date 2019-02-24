#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "zlog.h"
#include "netdisk.h"

zlog_category_t *zlog_handle;
static void sig_handler(int signo);

int main(int argc, char *argv[])
{
	int ret;
	int reg_sockfd;
	int login_sockfd;

	if (argc != 2)
	{
		printf("invalid argument\n");
		exit(1);
	}

	signal(SIGINT, sig_handler);

	ret = zlog_init("../conf/netdisk.conf");
	if (ret == -1){
		printf("zlog_init() error\n");
		return -1;
	}

	zlog_handle = zlog_get_category("my_cat");
	if (zlog_handle == NULL)
	{
		printf("zlog_get_category() error\n");
		zlog_fini();
		return -1;
	}

	if (strncmp("reg", argv[1], 3) == 0)
	{
		/*register connection*/
		reg_sockfd = netdisk_reg_connect(zlog_handle);
		if (reg_sockfd == -1)
		{
			printf("register connection is refused.\n");
			zlog_error(zlog_handle, "netdisk_reg_connect() error:");
			return -1;
		}
		ret = netdisk_reg(reg_sockfd, zlog_handle);
		if (ret == -1)
		{
			printf("register fail.\n");
			zlog_error(zlog_handle, "netdisk_reg() error");
			return -1;
		}
		close(reg_sockfd);
		ret = netdisk_cmd_request(zlog_handle);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_cmd_request()");
			return -1;
		}
	}
	else if (strncmp("login", argv[1], 5) == 0)
	{
		/*register connection*/
		login_sockfd = netdisk_login_connect(zlog_handle);
		if (login_sockfd == -1)
		{
			printf("login connection is refused.\n");
			zlog_error(zlog_handle, "netdisk_login_connect() error:");
			return -1;
		}

		/*users login*/
		ret = netdisk_login(login_sockfd, zlog_handle);
		if (ret == -1)
		{
			zlog_info(zlog_handle, "netdisk_login() error:");
			return -1;
		}
		else if (ret == 0){
			printf("login success.\n==============\n");
		}

		/*command request*/
		ret = netdisk_cmd_request(zlog_handle);
		if (ret == -1)
		{
			printf("command not found.\n");
			zlog_info(zlog_handle, "netdisk_cmd_send() error:");
			return -1;
		}
	}
	else
	{
		printf("invalid argument\n");
		return -1;
	}

	zlog_fini();
	pause();
	return 0;
}

static void sig_handler(int signo)
{
	printf("\nProcess exit normally\n");
	exit(0);
}
