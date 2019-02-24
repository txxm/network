#ifndef	_NETDISK_H_
#define	_NETDISK_H_

#include <stdint.h>
#include "zlog.h"

#define	CMD_LEN			256
#define	MD5_LEN			16

#define	SUCCESS			200
#define	FAILURE			201
#define	REPLACE			202

#define	INPUT_COUNT		3	
#define	USERNAME_LEN	33
#define	PASSWORD_LEN	USERNAME_LEN

#define	RDWR_DEF		0644
#define	FILENAME_LEN	256
#define	GLOBAL_DIR_PATH	256

#define	REG_DATE		32
#define	LOGIN_DATE		REG_DATE

#define	BUF_SIZE		4*1024

typedef unsigned char MD5_t;


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
	char name[FILENAME_LEN];
	char type;
	uint16_t authority;
	uint32_t length;
	char *content;
} file_info_t;

/*用户信息结构*/
typedef struct _user_info_t
{
	uint32_t id;
	char username[USERNAME_LEN];
	char password[PASSWORD_LEN];
	char reg_date[REG_DATE];
	char login_date[LOGIN_DATE];
} user_info_t;

/*网盘信息结构*/
typedef struct _netdisk_user_info_t
{
	char version[10];
	double total;
	double left;
	double used;
} netdisk_user_info_t;

char g_path[GLOBAL_DIR_PATH];
user_info_t user_info;

int32_t netdisk_connect(zlog_category_t *zlog_handle);
int32_t netdisk_reg_connect(zlog_category_t *zlog_handle);
int32_t netdisk_login_connect(zlog_category_t *zlog_handle);
int32_t netdisk_data_connect(zlog_category_t *zlog_handle);

int32_t netdisk_reg(int first_sockfd, zlog_category_t *zlog_handle);
int32_t netdisk_login(int first_sockfd, zlog_category_t *zlog_handle);

int32_t netdisk_cmd_request(zlog_category_t *zlog_handle);

int32_t netdisk_get(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_put(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_vim(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_ls(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_ls1(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_ll(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_cd(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_mkdir(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_scan(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_find(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_remove(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_copy(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_move(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_filetype(cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle);
int32_t netdisk_tar(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_download(cmd_info_t *cmd_info_ptr,
								zlog_category_t *zlog_handle);
int32_t netdisk_clear(zlog_category_t *zlog_handle);
int32_t netdisk_recycle(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_empty(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_recover(cmd_info_t *cmd_info_ptr, zlog_category_t *zlog_handle);
int32_t netdisk_pwd();
int32_t netdisk_exit();
int32_t netdisk_nonblock(int arg);

int32_t netdisk_cmd_help(cmd_info_t *cmd_info_ptr,zlog_category_t *zlog_handle);

#endif
