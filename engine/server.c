//
//  server.c
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#include "../common/common.h"
#include "server.h"
#include "error.h"
#include <sodium.h>
#include "../common/crc32.h"

static void pt_server_log(const char *fmt, int error, const char *func, const char *file, int line)
{
    char log[512];
    sprintf(log, fmt, uv_strerror(error));
    LOG(log,func,file,line);
}

static void pt_server_alloc_buf(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf) {
    uv_tcp_t *sock = (uv_tcp_t*)handle;
    struct pt_sclient *user = sock->data;
    
    if(user->async_buf){
        if(user->async_buf->len < suggested_size){
            free(user->async_buf->base);
            user->async_buf->len = suggested_size;
            user->async_buf->base = malloc(suggested_size);
            if(user->async_buf->base == NULL){
                FATAL("malloc user->readbuf->buf failed", __FUNCTION__, __FILE__, __LINE__);
            }
        }
    } else {
        user->async_buf = malloc(sizeof(*user->async_buf));
        user->async_buf->base = malloc(suggested_size);
        user->async_buf->len = suggested_size;
        if(user->async_buf->base == NULL){
            FATAL("malloc user->readbuf->buf failed", __FUNCTION__, __FILE__, __LINE__);
        }
    }
    
    
    buf->base = (char*)user->async_buf->base;
    buf->len = user->async_buf->len;
}

static struct pt_sclient* pt_sclient_new(struct pt_server *server)
{
    struct pt_sclient *user;
    
    user = malloc(sizeof(struct pt_sclient));
    bzero(user, sizeof(struct pt_sclient));
    
    user->buf = pt_buffer_new(USER_DEFAULT_BUFF_SIZE);
    user->id = ++server->serial;
    
    return user;
}

static void pt_sclient_free(struct pt_sclient* user)
{
    pt_buffer_free(user->buf);
    user->buf = NULL;
    
    if(user->async_buf){
        free(user->async_buf->base);
        free(user->async_buf);
        user->async_buf = NULL;
    }
    free(user);
}

/*
    当写入操作完成或失败会执行本函数
    释放pt_buffer和write_request数据
 */
static void pt_server_write_cb(uv_write_t* req, int status)
{
    struct pt_wreq *wr = (struct pt_wreq*)req;
    
    pt_buffer_free(wr->buff);
    free(wr);
}

/*
    当成功关闭了handle后，释放用户资源
 */
static void pt_server_on_close(uv_handle_t* peer) {
    struct pt_sclient *user = peer->data;
    
    pt_sclient_free(user);
}

/*
    shutdown函数执行完成后调用
    这里会转向调用到uv_close来关闭uv_handle_t结构以及释放pt_sclient的结构
 */
static void pt_server_shutdown_cb(uv_shutdown_t* req, int status)
{
    struct pt_sclient *user = req->data;
    uv_close((uv_handle_t*)&user->sock, pt_server_on_close);
    free(req);
}

/*
    断开一个用户
 
    如果remove值为true就从全局表中删除这个用户

 */
static void pt_server_shutdown_conn(struct pt_sclient *user, qboolean remove)
{
    int r;
    uv_shutdown_t *sreq;
    struct pt_server *server;
    sreq = malloc(sizeof *sreq);
    server = user->server;
    
    sreq->data = user;
    user->connected = false;
    
    if(remove)
    {
        //通知用户函数，用户断开
        if(server->on_disconnect) server->on_disconnect(user);
        
        //从用户ID表中删除
        pt_table_erase(server->clients, user->id);
        
        //降低服务器连接数
        server->number_of_connected--;
    }
    
    r = uv_shutdown(sreq, (uv_stream_t*)&user->sock, pt_server_shutdown_cb);
    
    if( r != 0 )
    {
        FATAL("uv_shutdown failed", __FUNCTION__, __FILE__, __LINE__);
        free(sreq);
    }
}

/*
    libuv的数据包收到函数
    
    当用户收到数据或连接断开时，会调用本函数
    本函数会处理用户断开
    数据到达
    对数据安全进行检查等
 */
static void pt_server_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    uint32_t packet_err;
    uv_tcp_t *sock = (uv_tcp_t*)stream;
    struct pt_sclient *user = sock->data;
    struct pt_buffer *userbuf = NULL;
    
    //用户状态异常断开，执行disconnect
    if(nread < 0)
    {
        pt_server_shutdown_conn(user, true);
        return;
    }
    
    //用户发送了EOF包，执行断开
    if(nread == 0){
        pt_server_shutdown_conn(user, true);
        return;
    }
    
    //写数据到缓冲区
    pt_buffer_write(user->buf, (unsigned char*)buf->base, (uint32_t)nread);
    
    //循环读取缓冲区数据，如果数据错误则返回false且不再执行本while
    while(pt_get_packet_status(user->buf, &packet_err)){
        userbuf = pt_split_packet(user->buf);
        if(user->server->on_receive) user->server->on_receive(user, userbuf);
        pt_buffer_free(userbuf);
    }
    
    //如果用户发的数据是致命错误，则干掉用户
    if(packet_err == PACKET_INFO_FAKE || packet_err == PACKET_INFO_OVERFLOW){
        char error[512];
        const char *msg = packet_err == PACKET_INFO_FAKE ? "PACKET_INFO_FAKE" : "PACKET_INFO_OVERFLOW";
        sprintf(error, "pt_get_packet_status error:%s",msg);
        ERROR(error, __FUNCTION__, __FILE__, __LINE__);
        pt_server_shutdown_conn(user, true);
    }
}

/*
    libuv的connection通知

    处理新用户连接，最大用户数量，创建pt_sclient结构并添加到clients表中
    执行用户自定义的通知信息
 */
static void pt_server_connection_cb(uv_stream_t* listener, int status)
{
    int r;
    struct pt_server *server = listener->data;
    struct pt_sclient *user = pt_sclient_new(server);
    
    user->server = server;
    uv_tcp_init(listener->loop, &user->sock);
    user->sock.data = user;
    
    r = uv_accept(listener, (uv_stream_t*)&user->sock);
    
    if( r != 0 )
    {
        char error[512];
        sprintf(error, "uv_accept error:%s",uv_strerror(r));
        ERROR(error, __FUNCTION__, __FILE__, __LINE__);
        uv_close((uv_handle_t*)&user->sock, pt_server_on_close);
        return;
    }
    
    //限制当前服务器的最大连接数
    if(server->number_of_connected + 1 > server->number_of_max_connected){
        pt_server_shutdown_conn(user, false);
        return;
    }
    
    //进行数据过滤，如果用户Connect请求被on_connect干掉则直接断开用户
    if(server->on_connect && server->on_connect(user) == false){
        pt_server_shutdown_conn(user, false);
        return;
    }
    
    server->number_of_connected++;
    
    //设置用户30秒后做keepalive检查
    uv_tcp_keepalive(&user->sock, true, 30);
    
    uv_tcp_nodelay(&user->sock, false);
    
    //添加到搜索树内
    pt_table_insert(server->clients, user->id, user);
    
    //开始读取网络数据
    r = uv_read_start((uv_stream_t*)&user->sock, pt_server_alloc_buf, server->read_cb);
    
    if( r != 0 )
    {
        char error[512];
        sprintf(error, "uv_read_start error:%s",uv_strerror(r));
        ERROR(error, __FUNCTION__, __FILE__, __LINE__);
        
        uv_close((uv_handle_t*)&user->sock, pt_server_on_close);
        return;
    }
}

struct pt_server* pt_server_new()
{
    struct pt_server *server = (struct pt_server *)malloc(sizeof(struct pt_server));
    
    if(server == NULL){
        FATAL("malloc pt_server failed",__FUNCTION__,__FILE__,__LINE__);
    }
    
    bzero(server, sizeof(struct pt_server));
    
    server->clients = pt_table_new();
    
    //初始化libuv的回调函数
    server->connection_cb = pt_server_connection_cb;
    server->read_cb = pt_server_read_cb;
    server->write_cb = pt_server_write_cb;
    server->shutdown_cb = pt_server_shutdown_cb;
    
    return server;
}

qboolean pt_server_init(struct pt_server *server, uv_loop_t *loop, int max_conn, pt_server_on_connect on_conn,
                        pt_server_on_receive on_receive, pt_server_on_disconnect on_disconnect)
{
    int r;
    
    if(server->is_init){
        LOG("server already initialize",__FUNCTION__,__FILE__,__LINE__);
        return false;
    }
    
    server->number_of_max_connected = max_conn;
    server->loop = loop;
    server->on_connect = on_conn;
    server->on_receive = on_receive;
    server->on_disconnect = on_disconnect;
    
    r = uv_tcp_init(loop, &server->listener);
    
    if(r != 0){
        char log[256];
        sprintf(log,"uv_tcp_init failed:%s",uv_strerror(r));
        LOG(log,__FUNCTION__, __FILE__,__LINE__);
    }
    
    server->listener.data = server;
    server->is_init = true;
    
    return r == 0;
}

qboolean pt_server_start(struct pt_server *server, char* host, uint16_t port)
{
    int r;
    struct sockaddr_in adr;
    
    
    if(!server->is_init){
        LOG("server not initialize",__FUNCTION__,__FILE__,__LINE__);
        return false;
    }
    
    uv_ip4_addr(host, port, &adr);
    
    r = uv_tcp_bind(&server->listener, (const struct sockaddr*)&adr, 0);
    
    if(r != 0){
        pt_server_log("uv_tcp_bind failed:%s",r, __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    
    r = uv_listen((uv_stream_t*)&server->listener, SOMAXCONN, server->connection_cb);
    if(r != 0){
        pt_server_log("uv_listen failed:%s",r, __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    return true;
}

/*
    发送数据到客户端
 */
qboolean pt_server_send(struct pt_sclient *user, struct pt_buffer *buff)
{
    struct pt_wreq *wreq = malloc(sizeof(struct pt_wreq));
    
    wreq->buff = buff;
    wreq->buf = uv_buf_init((char*)buff->buff, buff->length);
    
    if (uv_write(&wreq->req, (uv_stream_t*)&user->sock, &wreq->buf, 1, user->server->write_cb)) {
        pt_buffer_free(wreq->buff);
        free(wreq);
        return false;
    }
    
    return true;
}