#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "zlog.h"
#include "netdisk.h"

static int32_t netdisk_login_handler(int first_sockfd,
											zlog_category_t *zlog_handle);

int32_t netdisk_login_connect(zlog_category_t *zlog_handle)
{
	int i, ret;
	int on = 1;
	int nready;
	int epollfd;
	int first_listenfd;
	int first_connfd;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len = sizeof(cliaddr);
	struct epoll_event ev, events[MAX_EVENTS];
	
	printf("*****Starting login connection.\n");
	first_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (first_listenfd == -1)
	{
		zlog_error(zlog_handle, "socket() error:");
		return -1;
	}

#if 1
	ret = setsockopt(first_listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on,
				sizeof(int));
	if (ret == -1)
	{
		zlog_error(zlog_handle, "setsockopt() error:");
		return -1;
	}
#endif

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5189);

	ret = bind(first_listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret == -1)
	{
		zlog_error(zlog_handle, "bind() error:");
		return -1;
	}

	ret = listen(first_listenfd, SOMAXCONN);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "listen() error:");
		return -1;
	}
	fprintf(stdout, "*****Listening login port...\n");

	epollfd = epoll_create(10);
	if (epollfd == -1)
	{
		zlog_error(zlog_handle, "epoll_create() error:");
		return -1;
	}

	ev.data.fd = first_listenfd;
	ev.events = EPOLLIN|EPOLLET;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, first_listenfd, &ev);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "epoll_ctl() error:");
		return -1;
	}

	while (1)
	{
		printf("*****Waiting for client's login request\n");
		nready = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nready == -1)
		{
			if (errno == EINTR)
			{
				usleep(10);
				continue;
			}
			else{
				perror("epoll_wait() error");
			}
		}
		else if (nready == 0){
			usleep(10);
			continue;
		}

		for (i = 0; i < nready; i++)
		{
			if (events[i].data.fd == first_listenfd)
			{
				first_connfd = accept(first_listenfd, 
									(struct sockaddr *)&cliaddr, &len);
				if (first_connfd == -1)
				{
					zlog_error(zlog_handle, "accept() error:");
				}

				ret = netdisk_nonblock(first_connfd);
				if (ret == -1)
				{
					zlog_error(zlog_handle, "netdisk_nonblock() error:");
					return -1;
				}
				ev.data.fd = first_connfd;
				ev.events = EPOLLIN | EPOLLET;	
				ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, first_connfd, &ev);
				if (ret == -1)
				{
					zlog_error(zlog_handle, "epoll_ctl() error:");
				}
			}
			else if (events[i].events & EPOLLIN)
			{
				first_connfd = events[i].data.fd;
				if (first_connfd == -1){
					continue;
				}
				netdisk_login_handler(first_connfd, zlog_handle);
				close(first_connfd);
			}
		}
	}

	return first_connfd;
}

static int32_t netdisk_login_handler(int first_connfd,
											zlog_category_t *zlog_handle)
{
	int ret;

	ret = netdisk_login_recv(first_connfd, zlog_handle);
	if (ret == -1)
	{
		zlog_error(zlog_handle, "netdisk_login_recv() error");
		return -1;
	}

	return 0;
}
