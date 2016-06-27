//
//  main.c
//  xcode
//
//  Created by 袁睿 on 16/6/22.
//  Copyright © 2016年 number201724. All rights reserved.
//

#include "common/common.h"
#include "common/proto.h"
#include "common/table.h"
#include "engine/buffer.h"
#include "engine/packet.h"
#include "engine/server.h"
#include "engine/client.h"
struct pt_server *gserver;
struct pt_client *gclient;

qboolean app_on_connect(struct pt_sclient *user)
{
    printf("incoming new client  %llu\n",user->id);
    return true;
}

void app_on_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
    struct net_header hdr;
    char message[512] = {0};
    unsigned char *buf = pt_get_packet_buffer(buff);
    uint32_t len = pt_get_packet_size(buff);
    
    memcpy(message,buf,len);
    printf("receive:%s\n",message);
    
    pt_create_nethdr(&hdr, ID_TRANSMIT_KEEPALIVE, (uint32_t)strlen(message));
    
    struct pt_buffer * sendbuf = pt_create_buffer(&hdr, (unsigned char*)&message, (uint32_t)strlen(message));
    
    pt_server_send(user, sendbuf);
}

void app_on_disconnect(struct pt_sclient *user)
{
    printf("client disconnect:%llu\n",user->id);
}

void create_server(uv_loop_t *loop)
{
    gserver = pt_server_new();
    
    pt_server_init(gserver, loop, 10000, app_on_connect, app_on_receive, app_on_disconnect);
    
    pt_server_start(gserver, "0.0.0.0", 43991);
}

void send_helloworld(struct pt_client *conn){
    char message[] = "hello world";
    struct net_header hdr;
    
    pt_create_nethdr(&hdr,ID_TRANSMIT_KEEPALIVE,(uint32_t)strlen(message));
    struct pt_buffer * sendbuf = pt_create_buffer(&hdr, (unsigned char*)&message, (uint32_t)strlen(message));
    
    pt_client_send(conn, sendbuf);
}

void app_cli_on_connected(struct pt_client *conn)
{
    printf("app_cli_on_connected\n");
    
    send_helloworld(conn);
}
void app_cli_on_receive(struct pt_client *conn, struct pt_buffer *buff)
{
    printf("app_cli_on_receive\n");
    
    send_helloworld(conn);
}
void app_cli_on_disconnected(struct pt_client *conn)
{
    printf("app_cli_on_disconnected\n");
}


void create_client(uv_loop_t *loop)
{
    gclient = pt_client_new();
    pt_client_init(loop, gclient, app_cli_on_connected, app_cli_on_receive, app_cli_on_disconnected);
    
    pt_client_connect(gclient, "127.0.0.1", 43991);
}

void update_delay(uv_timer_t *handle)
{
    printf("on timer\n");
}

void create_timer(uv_loop_t *loop){
    uv_timer_t *tm = malloc(sizeof(uv_timer_t));
    
    uv_timer_init(loop, tm);
    
    uv_timer_start(tm, update_delay, 1000, 1000);
}

int main(int argc, const char * argv[]) {
    uv_loop_t *loop = uv_default_loop();
    
    create_server(loop);
    create_client(loop);
    create_timer(loop);
    uv_run(loop, UV_RUN_DEFAULT);
    
    printf("event loop exit...\n");
    return 0;
}
