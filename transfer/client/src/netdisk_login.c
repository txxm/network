#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/md5.h>

#include "zlog.h"
#include "netdisk.h"

static int32_t netdisk_login_handler(int first_sockfd, 
					user_info_t *user_info_ptr, zlog_category_t *zlog_handle);

int32_t netdisk_login(int first_sockfd, zlog_category_t *zlog_handle)
{
	int i, ret, len;
	int input_count = 0;
	char *user_pass;
	MD5_CTX c;
	MD5_t md5[MD5_LEN];
	//user_info_t user_info;

	if (first_sockfd < 0 || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

sk0:printf("\033[32musername:\033[0m");
	memset(user_info.username, 0, USERNAME_LEN);
	if (fgets(user_info.username, sizeof(user_info.username), stdin) == NULL)
	{
		zlog_error(zlog_handle, "username is null");
		return -1;
	}
	len = strlen(user_info.username);
	user_info.username[len-1] = '\0';

	user_pass = getpass("\033[32mpassword:\033[0m");
	if (user_pass == NULL)
	{
		zlog_error(zlog_handle, "password is null");
		return -1;
	}

	ret = MD5_Init(&c);
	if (ret == 0)
	{
		zlog_error(zlog_handle, "MD5_Init() error:");
		return -1;
	}

	ret = MD5_Update(&c, user_pass, strlen(user_pass));
	if (ret == 0)
	{
		zlog_error(zlog_handle, "MD5_Update() error:");
		return -1;
	}

	memset(md5, 0, MD5_LEN);
	ret = MD5_Final(md5, &c);
	if (ret == 0)
	{
		zlog_error(zlog_handle, "MD5_Final() error:");
		return -1;
	}

	for (i = 0; i < MD5_LEN; i++)
	{
		sprintf(&(user_info.password[i*2]), "%02x", md5[i]);
	}

	if (strlen(user_info.username) <= 0 || 
			strlen(user_info.username) > USERNAME_LEN ||
				strlen(user_info.password) <= 0 ||
					strlen(user_info.password) > PASSWORD_LEN)
	{
		input_count++;
		if (input_count > INPUT_COUNT){
			exit(1);
		}
		fprintf(stderr, "username or password is fail, retry again.\n");
		goto sk0;
	}

	ret = netdisk_login_handler(first_sockfd, &user_info, zlog_handle);
	if (ret == -1)
	{
		input_count++;
		if (input_count >= INPUT_COUNT)
		{
			printf("input counts is three.\n");
			exit(1);
		}
		printf("username or password is fail, retry again.[%d %s]\n", 
					__LINE__,  __FILE__);
		goto sk0;
	}

	return 0;
}

static int32_t netdisk_login_handler(int first_sockfd, 
				user_info_t *user_info_ptr, zlog_category_t *zlog_handle)
{
	int nsend;
	int nrecv;
	uint32_t st_code;

	if (first_sockfd < 0 || user_info_ptr == NULL || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

sk0:nsend = send(first_sockfd, user_info_ptr, sizeof(user_info_t),MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		usleep(10);
		goto sk0;
	}

sk1:nrecv = recv(first_sockfd, &st_code, sizeof(uint32_t), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}

	if (st_code == SUCCESS){
		return 0;
	}
	else 
	{
		zlog_info(zlog_handle, "username or password is fault.\n");
		return -1;
	}

	return -1;
}
