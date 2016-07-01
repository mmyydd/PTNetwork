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
    
    //当前套接字是否被连接
    qboolean connected;
    
    //接收到数据后，未拆包的数据
	struct pt_buffer *buf;
    //libuv申请的异步缓冲区buffer
    uv_buf_t *async_buf;
    
    uint32_t serial;
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
    int number_of_max_connected;
    int number_of_connected;
    
    qboolean is_pipe;
    
    //tcp nodelay
    qboolean no_delay;
    
    //keep alive延迟时间
    int keep_alive_delay;
    
    //加密函数使用
    qboolean enable_encrypt;
    uint32_t encrypt_key[4];
    
    qboolean is_shutdown;
    qboolean is_init;
    /*
        提供给libuv的回调函数
     */
    uv_write_cb write_cb;
    uv_read_cb read_cb;
    uv_connection_cb connection_cb;
    uv_shutdown_cb shutdown_cb;
    
    /*
        当服务器初始化的时候执行
     */
    void (*on_init)(struct pt_server *server);
    
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


void pt_server_set_nodelay(struct pt_server *server, qboolean nodelay);

qboolean pt_server_init(struct pt_server *server, uv_loop_t *loop,int max_conn, int keep_alive_delay,
                        pt_server_on_connect on_conn,pt_server_on_receive on_receive,
                        pt_server_on_disconnect on_disconnect);

void pt_server_set_encrypt(struct pt_server *server, const uint32_t encrypt_key[4]);

qboolean pt_server_start(struct pt_server *server, const char* host, uint16_t port);
qboolean pt_server_start_pipe(struct pt_server *server, const char *path);
qboolean pt_server_send(struct pt_sclient *user, struct pt_buffer *buff);
qboolean pt_server_disconnect_conn(struct pt_server *server, struct pt_sclient *user);

void pt_server_close(struct pt_server *server);


qboolean verify_packet(struct pt_buffer *buff);

#endif
