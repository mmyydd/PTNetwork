#ifndef _PT_SERVER_INCLUED_H_
#define _PT_SERVER_INCLUED_H_

#include <ptnetwork/common.h>
#include <ptnetwork/buffer.h>
#include <ptnetwork/table.h>
#include <ptnetwork/packet.h>

struct pt_server;
struct pt_sclient;

//默认的最大发送队列/用户
#define NUMBER_MAX_SEND_QUEUE 100

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
    
    //用户定义的数据
    void *data;

	//引用计数器
	uint32_t ref_count;
};

typedef void (*pt_server_on_alloc_sclient)(struct pt_sclient *user);
typedef void (*pt_server_on_free_sclient)(struct pt_sclient *user);
typedef qboolean (*pt_server_on_connect)(struct pt_sclient *user);
typedef void (*pt_server_on_receive)(struct pt_sclient *user, struct pt_buffer *buff);
typedef void (*pt_server_on_disconnect)(struct pt_sclient *user);
typedef void (*pt_server_warning_user)(struct pt_sclient *user);
typedef void (*pt_server_error_notify)(struct pt_server *server, const char *function);
typedef void (*pt_server_on_close)(struct pt_server *server, qboolean is_free);

enum pt_server_state
{
	//结构创建完成后的状态
	PT_STATE_CREATED,
	//结构初始化后的状态
	PT_STATE_INITIALIZED,
	//服务器正在执行中
    PT_STATE_RUNNING,
};

//服务器已经被Shutdown
#define PT_STATE_SHUTDOWN PT_STATE_INITIALIZED

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
    
    //用户data
    void *data;
    
    enum pt_server_state state;
    
    /*
        提供给libuv的回调函数
     */
    //uv_write_cb write_cb;
    //uv_read_cb read_cb;
    //uv_connection_cb connection_cb;
	//
	
	pt_server_on_close on_close;
    
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

	/*
	 * 用户警告函数
	 * 出现非法数据或异常等通知
	 * */
	pt_server_warning_user warning_user;

	//申请释放sclient结构的回调
	pt_server_on_alloc_sclient on_alloc_sclient;
	pt_server_on_free_sclient on_free_sclient;
	
	//服务器错误通知
	pt_server_error_notify error_notify;

	//服务器发送数据的压入弹出次数
	uint64_t number_of_push_send;
	uint64_t number_of_pop_send;

	//服务器收到数据的压入弹出次数
	uint64_t number_of_push_recv;
	uint64_t number_of_pop_recv;

	//服务器发送数据的字节统计
	uint64_t number_of_send_bytes;
	uint64_t number_of_recv_bytes;
	
	//服务器的启动时间
	time_t start_time;
	
	//服务器上一个错误
	int last_error;
};

uint32_t pt_sclient_ref_increment(struct pt_sclient *user);
uint32_t pt_sclient_ref_decrement(struct pt_sclient *user);

struct pt_server* pt_server_new();
void pt_server_free(struct pt_server *srv);

const char *pt_server_str_last_error(struct pt_server *server);

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

//发送数据到全部用户
void pt_server_send_to_all(struct pt_server *server, struct pt_buffer *buff);

//服务器请求断开一个用户的连接
qboolean pt_server_disconnect_conn(struct pt_sclient *user);

//关闭服务器
void pt_server_close(struct pt_server *server);

//关闭并释放服务器
void pt_server_close_free(struct pt_server *server);
#endif
