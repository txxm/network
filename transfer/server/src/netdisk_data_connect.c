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

int32_t netdisk_data_connect(zlog_category_t *zlog_handle)
{
	int ret;
	int second_listenfd;
	int second_connfd;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len = sizeof(cliaddr);

	if (zlog_handle == NULL){
		return -1;
	}

	second_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (second_listenfd == -1)
	{
		zlog_info(zlog_handle, "socket() error:");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5189);

	ret = bind(second_listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret == -1)
	{
		zlog_info(zlog_handle, "bind() error:");
		return -1;
	}

	ret = listen(second_listenfd, SOMAXCONN);
	if (ret == -1)
	{
		zlog_info(zlog_handle, "listen() error:");
		return -1;
	}

	second_connfd = accept(second_listenfd, (struct sockaddr *)&cliaddr, &len);
	if (second_connfd == -1)
	{
		zlog_info(zlog_handle, "accept() error:");
		return -1;
	}

	return second_connfd;
}
