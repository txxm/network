#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/md5.h>

#include "zlog.h"
#include "netdisk.h"

static int32_t netdisk_conf_write(int fd, char *conf_buf);

int32_t netdisk_reg(int first_sockfd, zlog_category_t *zlog_handle)
{
	int fd, ret;
	time_t reg_time;
	int nsend, nrecv;
	int i, st_code, len;
	MD5_CTX c;
	MD5_t md5[MD5_LEN];
	char *user_pass;
	char *ptime;
	char buf[FILENAME_LEN];
	char cmd[FILENAME_LEN];
	char dir_name[FILENAME_LEN];
	char netdisk_path[256];
	//user_info_t user_info;
	netdisk_user_info_t netdisk_user_info;

	if (first_sockfd < 0 || zlog_handle == NULL)
	{
		zlog_error(zlog_handle, "invalid argument");
		return -1;
	}

	memset(&user_info, 0, sizeof(user_info));

	printf("\033[3mUSER REGISTER\n\033[0m");
sk0:printf("\033[32musername:\033[0m");
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

	time(&reg_time);
	ptime = asctime(gmtime(&reg_time));
	if (ptime == NULL)
	{
		zlog_error(zlog_handle, "asctime() error:");
		return -1;
	}
	memcpy(user_info.reg_date, ptime, strlen(ptime));

sk1:nsend = send(first_sockfd, &user_info, sizeof(user_info), MSG_DONTWAIT);
	if (nsend == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk1;
		}
		return -1;
	}
	else if (nsend == 0)
	{
		printf("connection break\n");
		return -1;
	}

sk2:nrecv = recv(first_sockfd, &netdisk_user_info, sizeof(netdisk_user_info), 
					MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk2;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		return 0;
	}

	memset(netdisk_path, 0, sizeof(netdisk_path));
	sprintf(netdisk_path, "../%s", user_info.username);
	ret = mkdir(netdisk_path, RDWR_DEF);
	if (ret == -1)
	{
		if (errno == EEXIST)
		{
			return -1;
			memset(cmd, 0, FILENAME_LEN);
			sprintf(cmd, "rm -rf %s", netdisk_path);
			ret = system(cmd);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "system() error");
				return -1;
			}

			ret = mkdir(netdisk_path, RDWR_DEF);
			if (ret == -1)
			{
				zlog_error(zlog_handle, "mkdir() error");
				return -1;
			}
			else
			{
				memset(dir_name, 0, FILENAME_LEN);
				sprintf(dir_name, "../%s/document", user_info.username);
				ret = mkdir(dir_name, RDWR_DEF);
				if (ret == -1){
					return -1;
				}
				memset(dir_name, 0, FILENAME_LEN);
				sprintf(dir_name, "../%s/info", user_info.username);
				ret = mkdir(dir_name, RDWR_DEF);
				if (ret == -1){
					return -1;
				}

				memset(dir_name, 0, FILENAME_LEN);
				sprintf(dir_name, "../%s/info/user.conf", user_info.username);
				fd = open(dir_name, O_CREAT|O_RDWR, RDWR_DEF);
				if (fd == -1)
				{
					zlog_error(zlog_handle, "open() error");
					return -1;
				}

				while (1)
				{
					memset(buf, 0, sizeof(buf));

					memcpy(buf, "version=", strlen("version="));
					strncat(buf, netdisk_user_info.version,
								strlen(netdisk_user_info.version));
					buf[strlen(buf)] = '\n';

					memcpy(buf, "total=", strlen("total="));
					strncat(buf, (char *)&(netdisk_user_info.total), 
								strlen((char *)&(netdisk_user_info.total)));
					buf[strlen(buf)] = '\n';

					memcpy(buf, "left=", strlen("left="));
					strncat(buf, (char *)&(netdisk_user_info.left), 
								strlen((char *)&(netdisk_user_info.left)));
					buf[strlen(buf)] = '\n';

					memcpy(buf, "uesd=", strlen("used="));
					strncat(buf, (char *)&(netdisk_user_info.used), 
								strlen((char *)&(netdisk_user_info.used)));
					buf[strlen(buf)] = '\n';

					ret = netdisk_conf_write( fd, buf);
					if (ret == -1)
					{
						zlog_error(zlog_handle, "netdisk_conf_write() error");
						return -1;
					}
					break;
				}
				close(fd);
			}
		}
		else{
			return -1;
		}
	}
	else
	{
		memset(dir_name, 0, FILENAME_LEN);
		sprintf(dir_name, "../%s/document", user_info.username);
		ret = mkdir(dir_name, RDWR_DEF);
		if (ret == -1){
			return -1;
		}
		memset(dir_name, 0, FILENAME_LEN);
		sprintf(dir_name, "../%s/info", user_info.username);
		ret = mkdir(dir_name, RDWR_DEF);
		if (ret == -1){
			return -1;
		}
		memset(dir_name, 0, FILENAME_LEN);
		sprintf(dir_name, "../%s/download", user_info.username);
		ret = mkdir(dir_name, RDWR_DEF);
		if (ret == -1){
			return -1;
		}

		memset(dir_name, 0, FILENAME_LEN);
		sprintf(dir_name, "../%s/info/user.conf", user_info.username);
		fd = open(dir_name, O_CREAT|O_RDWR, RDWR_DEF);
		if (fd == -1)
		{
			zlog_error(zlog_handle, "open() error");
			return -1;
		}
		memset(buf, 0, sizeof(buf));

		memcpy(buf, "version=", strlen("version="));
		strncat(buf, netdisk_user_info.version,
					strlen(netdisk_user_info.version));
		buf[strlen(buf)] = '\n';

		memcpy(buf, "total=", strlen("total="));
		strncat(buf, (char *)&(netdisk_user_info.total), 
					strlen((char *)&(netdisk_user_info.total)));
		buf[strlen(buf)] = '\n';

		memcpy(buf, "left=", strlen("left="));
		strncat(buf, (char *)&(netdisk_user_info.left), 
					strlen((char *)&(netdisk_user_info.left)));
		buf[strlen(buf)] = '\n';

		memcpy(buf, "uesd=", strlen("used="));
		strncat(buf, (char *)&(netdisk_user_info.used), 
					strlen((char *)&(netdisk_user_info.used)));
		buf[strlen(buf)] = '\n';

		ret = netdisk_conf_write( fd, buf);
		if (ret == -1)
		{
			zlog_error(zlog_handle, "netdisk_conf_write() error");
			return -1;
		}
		close(fd);
	}

sk3:nrecv = recv(first_sockfd, &st_code, sizeof(st_code), MSG_DONTWAIT);
	if (nrecv == -1)
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			usleep(10);
			goto sk3;
		}
		return -1;
	}
	else if (nrecv == 0)
	{
		printf("connection break\n");
		return -1;
	}

	if (st_code == SUCCESS)
	{
		printf("register success, and login\n");
		return 0;
	}
	else if (st_code == FAILURE)
	{
		printf("register failure, retry again\n");
		goto sk0;
	}

	return 0;
}

static int32_t netdisk_conf_write(int fd, char *conf_buf)
{
	int nwrite;
	
sk0:nwrite = write(fd, conf_buf, strlen(conf_buf));
	if (nwrite == -1)
	{
		if (errno == EAGAIN || errno == EEXIST)
		{
			usleep(10);
			goto sk0;
		}
		return -1;
	}
	fsync(fd);

	return 0;
}
