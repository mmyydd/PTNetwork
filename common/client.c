//
//  client.c
//  xcode
//
//  Created by 袁睿 on 16/6/24.
//  Copyright © 2016年 number201724. All rights reserved.
//


#include "common.h"
#include "crc32.h"
#include "error.h"
#include "client.h"

static void pt_client_alloc_cb(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf) {
    uv_stream_t *sock = (uv_stream_t*)handle;
    struct pt_client *user = sock->data;
    
    if(user->async_buf){
        if(user->async_buf->len < suggested_size){
            free(user->async_buf->base);
            user->async_buf->len = suggested_size;
            user->async_buf->base = malloc(suggested_size);
            if(user->async_buf->base == NULL){
                FATAL("malloc user->readbuf->buf failed", __FUNCTION__, __FILE__, __LINE__);
                abort();
            }
        }
    } else {
        user->async_buf = malloc(sizeof(*user->async_buf));
        user->async_buf->base = malloc(suggested_size);
        user->async_buf->len = suggested_size;
        if(user->async_buf->base == NULL){
            FATAL("malloc user->readbuf->buf failed", __FUNCTION__, __FILE__, __LINE__);
            abort();
        }
    }
    
    
    buf->base = (char*)user->async_buf->base;
    buf->len = user->async_buf->len;
}

/*
 当写入操作完成或失败会执行本函数
 释放pt_buffer和write_request数据
 */
static void pt_client_write_cb(uv_write_t* req, int status)
{
    struct pt_wreq *wr = (struct pt_wreq*)req;
    
    pt_buffer_free(wr->buff);
    free(wr);
}

/*
 当成功关闭了handle后，释放用户资源
 */
static void pt_client_close_cb(uv_handle_t* peer) {
    uv_stream_t *sock = (uv_stream_t*)peer;
    struct pt_client *client = sock->data;
    
    bzero(&client->conn, sizeof(client->conn));
    if(client->buf) {
        client->buf->length = 0;
    }
}

static void pt_client_shutdown_cb(uv_shutdown_t* req, int status)
{
    struct pt_client *client = req->data;
    free(req);
    
    client->connected = false;
    
    if(client->on_disconnected){
        client->on_disconnected(client);
    }
    
    uv_close((uv_handle_t*)&client->conn, pt_client_close_cb);
}

static void pt_client_read_cb(uv_stream_t* stream,
                              ssize_t nread,
                              const uv_buf_t* buf)
{
    uint32_t packet_err;
    uv_stream_t *sock = (uv_stream_t*)stream;
    struct pt_client *client = sock->data;
    struct pt_buffer *async_buf = NULL;
    
    //用户状态异常断开，执行disconnect
    if(nread < 0)
    {
        pt_client_disconnect(client);
        return;
    }
    
    //用户发送了EOF包，执行断开
    if(nread == 0){
        pt_client_disconnect(client);
        return;
    }
    
    //写数据到缓冲区
    pt_buffer_write(client->buf, (unsigned char*)buf->base, (uint32_t)nread);
    
    //循环读取缓冲区数据，如果数据错误则返回false且不再执行本while
    while(pt_get_packet_status(client->buf, &packet_err)){
        async_buf = pt_split_packet(client->buf);
        if(async_buf != NULL)
        {
            if(client->on_receive) client->on_receive(client, async_buf);
            pt_buffer_free(async_buf);
            
        }
        else{
            ERROR("pt_split_packet == NULL wtf?", __FUNCTION__, __FILE__, __LINE__);
        }
    }
    
    //如果用户发的数据是致命错误，则干掉用户
    if(packet_err == PACKET_INFO_FAKE || packet_err == PACKET_INFO_OVERFLOW){
        char error[512];
        const char *msg = packet_err == PACKET_INFO_FAKE ? "PACKET_INFO_FAKE" : "PACKET_INFO_OVERFLOW";
        sprintf(error, "pt_get_packet_status error:%s",msg);
        ERROR(error, __FUNCTION__, __FILE__, __LINE__);
        pt_client_disconnect(client);
    }
    
}

static void pt_client_connect_cb(uv_connect_t* req, int status)
{
    int r;
    struct pt_client *client = req->data;
    client->connecting = false;
    if(status != 0){
        client->connected = false;
        free(req);
        if(client->on_connected){
            client->on_connected(client);
        }
        return;
    }
    
    client->connected = true;
    
    if(client->enable_encrypt) {
        RC4_set_key(&client->encrypt_ctx, sizeof(client->encrypt_key), (const unsigned char*)&client->encrypt_key);
        client->serial = 0;
    }
    
    if(client->on_connected){
        client->on_connected(client);
    }
    
    r = uv_read_start((uv_stream_t*)&client->conn, pt_client_alloc_cb, pt_client_read_cb);
    
    if( r != 0 ){
        FATAL("uv_read_start failed", __FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    
    free(req);
}

struct pt_client *pt_client_new()
{
    struct pt_client *client;
    
    client = (struct pt_client*)malloc(sizeof(struct pt_client));
    
    bzero(client, sizeof(*client));
    
    client->buf = pt_buffer_new(USER_DEFAULT_BUFF_SIZE);
    client->connect_cb = pt_client_connect_cb;
    client->read_cb = pt_client_read_cb;
    client->write_cb = pt_client_write_cb;
    
    return client;
}

void pt_client_set_encrypt(struct pt_client *client, const uint32_t encrypt_key[4])
{
    client->enable_encrypt = true;
    client->encrypt_key[0] = encrypt_key[0];
    client->encrypt_key[1] = encrypt_key[1];
    client->encrypt_key[2] = encrypt_key[2];
    client->encrypt_key[3] = encrypt_key[3];
}

void pt_client_init(uv_loop_t *loop, struct pt_client *client, pt_cli_on_connected on_connected, pt_cli_on_receive on_receive, pt_cli_on_disconnected on_disconnected)
{
    client->loop = loop;
    client->conn.stream.data = client;
    client->on_connected = on_connected;
    client->on_receive = on_receive;
    client->on_disconnected = on_disconnected;
}

void pt_client_free(struct pt_client *client)
{
    if(client->buf){
        pt_buffer_free(client->buf);
        client->buf = NULL;
    }
    
    if(client->async_buf){
        free(client->async_buf->base);
        free(client->async_buf);
        client->async_buf = NULL;
    }
    
    free(client);
}

void pt_client_send(struct pt_client *client, struct pt_buffer *buff)
{
    int r;
    if(!client->connected) return;
    
    struct pt_wreq *req = malloc(sizeof(struct pt_wreq));
    req->buff = buff;
    req->data = client;
    req->buf = uv_buf_init((char*)buff->buff, buff->length);
    
    r = uv_write(&req->req, (uv_stream_t*)&client->conn, &req->buf, 1, pt_client_write_cb);
    
    if(r != 0){
        FATAL("uv_write failed", __FUNCTION__, __FILE__,__LINE__);
        free(req);
        pt_buffer_free(buff);
    }
}

void pt_client_connect(struct pt_client *client, const char *host, uint16_t port)
{
    int r;
    struct sockaddr_in adr;
    
    if(client->connecting || client->connected) return;
    
    
    if(uv_tcp_init(client->loop,&client->conn.tcp) != 0){
        FATAL("uv_tcp_init failed", __FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    
    uv_connect_t *conn = malloc(sizeof(uv_connect_t));
    conn->data = client;
    
    uv_ip4_addr(host, port, &adr);
    
    client->connecting = true;
    
    r = uv_tcp_connect(conn, &client->conn.tcp, (const struct sockaddr*)&adr, pt_client_connect_cb);
    if(r != 0){
        //printf("%s\n",uv_strerror(r));
        client->connecting = false;
        FATAL("uv_tcp_connect failed", __FUNCTION__, __FILE__, __LINE__);
        abort();
    }
}


void pt_client_connect_pipe(struct pt_client *client, const char *path)
{
    if(client->connecting || client->connected) return;
    
    
    if(uv_pipe_init(client->loop,&client->conn.pipe, true) != 0){
        FATAL("uv_pipe_init failed", __FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    
    uv_connect_t *conn = malloc(sizeof(uv_connect_t));
    conn->data = client;
    
    client->connecting = true;
    
    uv_pipe_connect(conn, &client->conn.pipe, path, pt_client_connect_cb);
}

void pt_client_disconnect(struct pt_client *client)
{
    uv_shutdown_t *req = malloc(sizeof(uv_shutdown_t));
    req->data = client;
    
    int r = uv_shutdown(req, (uv_stream_t*)&client->conn, pt_client_shutdown_cb);
    
    if(r){
        char msg[256];
        sprintf(msg,"uv_shutdown error %d",r);
        ERROR(msg, __FUNCTION__, __FILE__, __LINE__);
    }
}
