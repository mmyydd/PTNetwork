#ifndef _MAIN_INCLUDED_H_
#define _MAIN_INCLUDED_H_

#include <database_config.pb-c.h>
#include <network_config.pb-c.h>

//应用程序版本和名字
#define APPLICATION_NAME "Database Service"
#define APPLICATION_VERSION "0.01"

//当前进程是监控进程
#define SERVER_TYPE_DAEMON 0
//当前服务器是套接字服务
#define SERVER_TYPE_LISTENER 1
//当前服务器是逻辑处理服务器
#define SERVER_TYPE_LOGIC 2

//服务器以调试模式启动
#define SERVER_TYPE_DEBUG 3

//网络通信的channel
//进程数量
extern int process_count;
//服务器类型
extern int server_type;
//队列最大长度
extern int queue_count;

extern const char *signal_cmd;
extern NetworkConfig *network_config;
extern DatabaseConfig *database_config;


void sigint_start();
void sigint_stop();
extern uv_signal_t sigterm;


#define EXIT_CODE_START 17500

#define EXIT_CODE_SOCKET_FAILED EXIT_CODE_START + 1

#endif
