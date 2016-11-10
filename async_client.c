#include "common.h"
#include "crc32.h"
#include "pt_error.h"
#include "buffer.h"
#include "packet.h"
#include "async_client.h"

static void pt_client_alloc_cb(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf)
{
    uv_stream_t *sock = (uv_stream_t*)handle;
    struct pt_client *user = sock->data;
    
    if(user->async_buf){
        if(user->async_buf->len < suggested_size){
            XMEM_FREE(user->async_buf->base);
            user->async_buf->len = suggested_size;
            user->async_buf->base = XMEM_MALLOC(suggested_size);
        }
    } else {
        user->async_buf = XMEM_MALLOC(sizeof(*user->async_buf));
        user->async_buf->base = XMEM_MALLOC(suggested_size);
        user->async_buf->len = suggested_size;
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
    XMEM_FREE(wr);
}

/*
 当成功关闭了handle后，释放用户资源
 */
static void pt_client_close_cb(uv_handle_t* peer)
{
	struct pt_client *conn = peer->data;

    bzero(&conn->conn, sizeof(conn->conn));
	conn->conn.stream.data = conn;

	//seek to begin
	conn->buf->length = 0;
	
	if(conn->async_buf)
	{
		XMEM_FREE(conn->async_buf->base);
		XMEM_FREE(conn->async_buf);

		conn->async_buf = NULL;
	}

	if(conn->state == PT_STATE_CONNECT_FAILED)
	{
		conn->on_connected(conn, conn->state);
	}
	else
	{
		conn->on_disconnected(conn);
	}

	conn->state = PT_STATE_INIT;
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
    if(nread <= 0)
    {
        pt_client_disconnect(client);
        return;
    }
     
    //写数据到缓冲区
    pt_buffer_write(client->buf, (unsigned char*)buf->base, (uint32_t)nread);
    
    //循环读取缓冲区数据，如果数据错误则返回false且不再执行本while
    while(pt_get_packet_status(client->buf, &packet_err))
	{
        async_buf = pt_split_packet(client->buf);
        if(async_buf != NULL)
        {
            if(client->on_receive) client->on_receive(client, async_buf);
            pt_buffer_free(async_buf);
            
        }
        else{
            PT_ERROR("pt_split_packet == NULL wtf?", __FUNCTION__, __FILE__, __LINE__);
        }
    }
    
    //如果用户发的数据是致命错误，则干掉用户
    if(packet_err == PACKET_INFO_FAKE || packet_err == PACKET_INFO_OVERFLOW){
        char error[512];
        const char *msg = packet_err == PACKET_INFO_FAKE ? "PACKET_INFO_FAKE" : "PACKET_INFO_OVERFLOW";
        sprintf(error, "pt_get_packet_status error:%s",msg);
        PT_ERROR(error, __FUNCTION__, __FILE__, __LINE__);
        pt_client_disconnect(client);
    }
}

static void pt_client_connect_cb(uv_connect_t* req, int status)
{
    struct pt_client *client = req->data;
    XMEM_FREE(req);

    if(status != 0)
	{
		client->last_error = status;
	
		//是否已经关闭标记
		if(client->state == PT_STATE_BUSY)
		{
			uv_close((uv_handle_t*)&client->conn.stream, pt_client_close_cb);
		}
		else
		{
			client->state = PT_STATE_CONNECT_FAILED;
			uv_close((uv_handle_t*)&client->conn.stream, pt_client_close_cb);
		}
		return;
    }
		
	//客户端已经设置成关闭标记
	if(client->state == PT_STATE_BUSY)
	{
		uv_close((uv_handle_t*)&client->conn.stream, pt_client_close_cb);
		return;
	}

    client->state = PT_STATE_CONNECTED;
    
    if(client->enable_encrypt)
	{
        RC4_set_key(&client->encrypt_ctx, sizeof(client->encrypt_key), (const unsigned char*)&client->encrypt_key);
        client->serial = 0;
    }
    
    if(client->on_connected)
	{
        client->on_connected(client, client->state);
    }
    
    client->last_error = uv_read_start((uv_stream_t*)&client->conn, pt_client_alloc_cb, pt_client_read_cb);
    
    if(client->last_error)
	{
		if(client->on_error)
		{
			client->on_error(client, "uv_read_start");
		}

		pt_client_disconnect(client);
    }
} 

struct pt_client *pt_client_new()
{
    struct pt_client *client;
    
    client = (struct pt_client*)XMEM_MALLOC(sizeof(struct pt_client));
    
    bzero(client, sizeof(*client));
    
    client->buf = pt_buffer_new(USER_DEFAULT_BUFF_SIZE);
    client->connect_cb = pt_client_connect_cb;
    client->read_cb = pt_client_read_cb;
    client->write_cb = pt_client_write_cb;
	client->state = PT_STATE_NORMAL;
    
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
	if(client->state != PT_STATE_NORMAL) return;
	
    client->loop = loop;
    client->conn.stream.data = client;
    client->on_connected = on_connected;
    client->on_receive = on_receive;
    client->on_disconnected = on_disconnected;

	client->state = PT_STATE_INIT;
}

void pt_client_free(struct pt_client *client)
{
    pt_buffer_free(client->buf);
	client->buf = NULL;
    
    XMEM_FREE(client);
}

qboolean pt_client_send(struct pt_client *client, struct pt_buffer *buff)
{
	qboolean is_completed = false;

    if(client->state != PT_STATE_CONNECTED)
	{
		client->last_error = UV_ENOTCONN;
		pt_buffer_free(buff);
		return is_completed;
	}

    struct pt_wreq *req = XMEM_MALLOC(sizeof(struct pt_wreq));

    req->buff = buff;
    req->data = client;
    req->buf = uv_buf_init((char*)buff->buff, buff->length);

    client->last_error = uv_write(&req->req, (uv_stream_t*)&client->conn, &req->buf, 1, pt_client_write_cb);

	if(client->last_error == 0)
	{
		is_completed = true;
	}
	else
	{
		if(client->on_error)
		{
			client->on_error(client, "uv_write");
		}

		XMEM_FREE(req);

		pt_buffer_free(buff);
	}
    

	return is_completed;
}

qboolean pt_client_connect(struct pt_client *client, const char *host, uint16_t port)
{
    struct sockaddr_in adr;
    
    if(client->state != PT_STATE_INIT)
	{
		client->last_error = UV_EISCONN;
		return false;
	}

	client->last_error = uv_tcp_init(client->loop,&client->conn.tcp);

    if(client->last_error){
		if(client->on_error) client->on_error(client, "uv_tcp_init");
		return false;
    }

	client->is_shutdown = false;
    
    uv_connect_t *conn = XMEM_MALLOC(sizeof(uv_connect_t));
    conn->data = client;
    
    uv_ip4_addr(host, port, &adr);
    
    client->state = PT_STATE_CONNECTING;
    
    client->last_error = uv_tcp_connect(conn, &client->conn.tcp, (const struct sockaddr*)&adr, pt_client_connect_cb);
 
 	if(client->last_error != 0)
	{
		if(client->on_error)
		{
			client->on_error(client, "uv_tcp_connect");
		}

		client->on_connected(client, PT_STATE_CONNECT_FAILED);

		client->state = PT_STATE_INIT;

		return false;
    }

	return true;
}


qboolean pt_client_connect_pipe(struct pt_client *client, const char *path)
{
    if(client->state != PT_STATE_INIT)
	{
		client->last_error = UV_EISCONN;
		return false;
	}
    
    client->last_error = uv_pipe_init(client->loop,&client->conn.pipe, true);

	if(client->last_error)
	{
		if(client->on_error) client->on_error(client, "uv_pipe_init");
		return false;
    }
    
    uv_connect_t *conn = XMEM_MALLOC(sizeof(uv_connect_t));
    conn->data = client;
    
    client->state = PT_STATE_CONNECTING;
    
	uv_pipe_connect(conn, &client->conn.pipe, path, pt_client_connect_cb);

	return true;
}

void pt_client_disconnect(struct pt_client *client)
{
	if(client->state != PT_STATE_INIT)
	{
		if(client->state == PT_STATE_CONNECTED)
		{
			uv_close((uv_handle_t*)&client->conn.stream, pt_client_close_cb);
		}

		client->state = PT_STATE_BUSY;
	}
}
