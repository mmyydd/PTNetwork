//
//  main.cpp
//  agent
//
//  Created by 袁睿 on 16/6/28.
//  Copyright © 2016年 number201724. All rights reserved.
//


#include <iostream>
#include "cpp.hpp"
#include "branch.hpp"
#include "forward.hpp"
#include "remotes.hpp"



static uint32_t encrypt_key[4] = {0x42970C86,0xA0B3A057,0x51B97B3C,0x70F8891E};

struct pt_server *server;
struct pt_client *client;

void pt_srv_send(struct pt_sclient *user)
{
    char helloworld[32] = "helloworld";
    
    struct pt_buffer *buff;
    struct net_header hdr = pt_create_nethdr(ID_TRANSMIT_KEEPALIVE);
    
    buff = pt_create_package(hdr, (unsigned char*)&helloworld, sizeof(helloworld));
    

    pt_server_send(user, buff);
}

qboolean pt_srv_connect(struct pt_sclient *user)
{
    printf("server connected\n");
    
    return true;
}

void pt_srv_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
    buffer_reader reader;
    struct net_header hdr;
    
    buffer_reader_init(&reader, buff);
    buffer_reader_read(&reader, (unsigned char*)&hdr, sizeof(hdr));
    buffer_reader_ignore_bytes(&reader, 4);
    
    char *s = (char*)buffer_reader_cur_pos(&reader);
    
    printf("%s\n",s);
    
    //printf("pt_srv_receive => %d\n",user->serial);
    
    pt_srv_send(user);
}

void pt_srv_disconnect(struct pt_sclient *user)
{
    printf("server disconnected\n");
}

void pt_cli_sent(struct pt_client *client)
{
    char helloworld[32] = "helloworld";
    
    struct pt_buffer *buff;
    struct net_header hdr = pt_create_nethdr(ID_TRANSMIT_KEEPALIVE);
    
    buff = pt_create_encrypt_package(&client->encrypt_ctx, &client->serial, hdr, (unsigned char*)&helloworld, sizeof(helloworld));
    
    pt_client_send(client, buff);
}

void pt_cli_connect(struct pt_client *client)
{
    printf("client on connected\n");
    pt_cli_sent(client);
}

void pt_cli_receive(struct pt_client *client, struct pt_buffer *buff)
{
//    char *s = (char*)pt_get_packet_buffer(buff);
//    printf("%s\n",s);
    pt_cli_sent(client);
}

void pt_cli_disconnect(struct pt_client *client)
{
    printf("client disconnected\n");
}
int main(int argc, const char * argv[]) {

    uv_loop_t *loop = uv_default_loop();
    
    server = pt_server_new();
    client = pt_client_new();
    
    
    pt_server_init(server, loop, 10000, 30, pt_srv_connect, pt_srv_receive, pt_srv_disconnect);
    pt_server_set_encrypt(server, encrypt_key);
    pt_server_start_pipe(server, "/var/tmp/agent.sock");
    
    
    pt_client_init(loop, client, pt_cli_connect, pt_cli_receive, pt_cli_disconnect);
    pt_client_set_encrypt(client,encrypt_key);
    pt_client_connect_pipe(client, "/var/tmp/agent.sock");
    
    uv_run(loop, UV_RUN_DEFAULT);
    
    printf("hello world\n");
    
    
    
    return 0;
}
