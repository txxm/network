#ifndef	_NETDISK_H_
#define	_NETDISK_H_

#include <stdint.h>
#include "zlog.h"

#define	FILENAME_LEN	256

#define	USER_PASS_COUNT	3
#define	USER_INFO_RAW	1

#define	SUCCESS			200
#define	FAILURE			201

#define	MAX_EVENTS		10000

#define	USER_NAME		32
#define	PASS_NAME		USER_NAME

#define	RDWR_DEF		0644
#define	ONRD_DEF		0000

#define	NAME_LEN		33

#define	REG_DATE		32
#define	LOGIN_DATE		REG_DATE

#define	CMD_LEN			256
#define	BUF_SIZE		4*1024

#define	USER_TOTAL_SPACE		(20.0*1024.0*1024.0*1024.0)
#define	USER_LEFT_SPACE			(20.0*1024.0*1024.0*1024.0)
#define USER_USED_SPACE			0.0

/*命令信息结构*/
typedef struct _cmd_info_t
{
	char first[CMD_LEN];
	char second[CMD_LEN];
	char third[CMD_LEN];
	char four[CMD_LEN];
} cmd_info_t;

/*文件信息结构*/
typedef struct _file_info_t
{
	char name[NAME_LEN];
	char type;
	uint16_t authority;
	uint32_t length;
	char *content;
} file_info_t;

/*服务器管理员信息结构*/
typedef struct _root_info_t
{
	char *rootname[USER_NAME];
	char *rootpass[PASS_NAME];
	char *reg_date;
	char *login_date;
} root_info_t;

/*用户网盘信息结构*/
typedef struct _netdisk_user_info_t
{
	char version[10];
	double user_tot;
	double user_left;
	double user_used;
} netdisk_user_info_t;

/*用户信息结构*/
typedef struct _user_info_t
{
	uint32_t id;
	char username[NAME_LEN];
	char password[NAME_LEN];
	char reg_date[REG_DATE];
	char login_date[REG_DATE];
	//netdisk_user_info_t netdisk_user_info;
} user_info_t;

/*服务器网盘信息结构*/
typedef struct _netdisk_info_t
{
	char *version;
	uint64_t total;
	uint64_t left;
	uint64_t used;
	user_info_t user_info;
} netdisk_info_t;


int32_t netdisk_connect(zlog_category_t *zlog_handle);
int32_t netdisk_login_connect(zlog_category_t *zlog_handle);
int32_t netdisk_reg_connect(zlog_category_t *zlog_handle);

int readline(int fd, void *buf, size_t size, int offset);
int32_t netdisk_login_recv(int first_sockfd, zlog_category_t *zlog_handle);
int32_t netdisk_reg_recv(int first_sockfd, zlog_category_t *zlog_handle);
int32_t netdisk_cmd_recv(int first_sockfd, zlog_category_t *zlog_handle);
int32_t netdisk_space(char *filename, char *username,
									zlog_category_t *zlog_handle);
int32_t netdisk_add(char *filename, char *username,
									zlog_category_t *zlog_handle);

int32_t netdisk_mkdir_handler(cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_vim_handler(cmd_info_t *cmd_info_ptr,
            						zlog_category_t *zlog_handle);
int32_t netdisk_remove_handler(int first_sockfd, cmd_info_t *cmd_info_ptr,
            						zlog_category_t *zlog_handle);
int32_t netdisk_scan_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
            						zlog_category_t *zlog_handle);
int32_t netdisk_recvfile_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
        							zlog_category_t *zlog_handle);
int32_t netdisk_sendfile_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
        							zlog_category_t *zlog_handle);
int32_t netdisk_copy_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
      							    zlog_category_t *zlog_handle);
int32_t netdisk_move_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
       							    zlog_category_t *zlog_handle);
int32_t netdisk_recycle_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
      							    zlog_category_t *zlog_handle);
int32_t netdisk_recover_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
            						zlog_category_t *zlog_handle);
int32_t netdisk_empty_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_ls_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_ll_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_find_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_tar_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_cd_handler(int first_connfd, cmd_info_t *cmd_info_ptr,
									zlog_category_t *zlog_handle);
int32_t netdisk_download_handler(cmd_info_t *cmd_info_ptr,
			zlog_category_t *zlog_handle);

int32_t netdisk_nonblock(int arg);

#endif
