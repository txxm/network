#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

int32_t netdisk_reg_connect(zlog_category_t *zlog_handle)
{
	int ret;
	int login_sockfd;
	struct sockaddr_in servaddr;

	if (zlog_handle == NULL){
		return -1;
	}

	login_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (login_sockfd == -1)
	{
		zlog_info(zlog_handle, "socket() error:");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(5190);

	ret = connect(login_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret == -1)
	{
		zlog_info(zlog_handle, "connect() error:");
		return -1;
	}

	return login_sockfd;
}
