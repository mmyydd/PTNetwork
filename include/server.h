#ifndef _PT_SERVER_INCLUED_H_
#define _PT_SERVER_INCLUED_H_

#include "common.h"
#include "buffer.h"
#include "table.h"
#include "packet.h"

struct pt_server;
struct pt_sclient;


struct pt_sclient
{
    //用户唯一ID
    uint64_t id;
    
    //服务器信息
    struct pt_server *server;
    
    //套接字描述信息
    pt_net_t sock;
    
    //当前套接字是否被连接,如果服务器被关闭则设置为false
    //同时也不会发送数据和接收数据
    qboolean connected;
    
    //接收到数据后，未拆包的数据
	struct pt_buffer *buf;
    //libuv申请的异步缓冲区buffer
    uv_buf_t *async_buf;
    //加密函数使用的序列
    uint32_t serial;
    //rc4加密key
    RC4_KEY encrypt_ctx;
};

typedef qboolean (*pt_server_on_connect)(struct pt_sclient *user);
typedef void (*pt_server_on_receive)(struct pt_sclient *user, struct pt_buffer *buff);
typedef void (*pt_server_on_disconnect)(struct pt_sclient *user);

struct pt_server
{
    //客户端每次进入的唯一值
    uint64_t serial;
    
    //uv_loop 主循环
    uv_loop_t *loop;
    //服务器accept的套接字信息
    pt_net_t listener;
    //客户端列表
	struct pt_table *clients;
    
    //客户端的最大连接数和当前连结数
    int number_of_max_connected;
    int number_of_connected;
    
    //限制服务器每个用户的发送数据队列
    //防止服务器因为客户端拒绝接收数据导致内存耗尽
    //默认1000
    int number_of_max_send_queue;
    
    //服务器当前工作模式是否是pipe
    qboolean is_pipe;
    
    //tcp nodelay
    qboolean no_delay;
    
    //keep alive延迟时间
    int keep_alive_delay;
    
    //加密函数使用
    qboolean enable_encrypt;
    uint32_t encrypt_key[4];
    
    
    qboolean is_startup;
    
    //服务器是否已经初始化
    qboolean is_init;
    /*
        提供给libuv的回调函数
     */
    uv_write_cb write_cb;
    uv_read_cb read_cb;
    uv_connection_cb connection_cb;
    
    /*
        当用户建立连接到服务器
     */
    pt_server_on_connect on_connect;
	
    /*
        on_receive 当收到完整的数据包时执行
     */
    pt_server_on_receive on_receive;
    
    /*
        当用户断开连接时执行
     */
    pt_server_on_disconnect on_disconnect;
};

struct pt_server* pt_server_new();
void pt_server_free(struct pt_server *srv);


//禁用或者启用Nagle算法
void pt_server_set_nodelay(struct pt_server *server, qboolean nodelay);


//初始化一个服务器对象
//设置回调函数
void pt_server_init(struct pt_server *server, uv_loop_t *loop,int max_conn, int keep_alive_delay,
                        pt_server_on_connect on_conn,pt_server_on_receive on_receive,
                        pt_server_on_disconnect on_disconnect);

//启用加密算法
//算法目前使用rc4加密
void pt_server_set_encrypt(struct pt_server *server, const uint32_t encrypt_key[4]);

//启动服务器 监听tcp端口
qboolean pt_server_start(struct pt_server *server, const char* host, uint16_t port);

//启动服务器 监听文件描述符
qboolean pt_server_start_pipe(struct pt_server *server, const char *path);

//将数据追加到发送队列
qboolean pt_server_send(struct pt_sclient *user, struct pt_buffer *buff);
//服务器请求断开一个用户的连接
qboolean pt_server_disconnect_conn(struct pt_sclient *user);

//关闭服务器
void pt_server_close(struct pt_server *server);
#endif
