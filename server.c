#include <ptnetwork/common.h>
#include <ptnetwork/error.h>
#include <ptnetwork/crc32.h>
#include <ptnetwork/server.h>

static void pt_server_alloc_buf(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf) {
    uv_stream_t *sock = (uv_stream_t*)handle;
    struct pt_sclient *user = sock->data;
    
    if(user->async_buf){
        if(user->async_buf->len < suggested_size){
            MEM_FREE(user->async_buf->base);
            user->async_buf->len = suggested_size;
            user->async_buf->base = MEM_MALLOC(suggested_size);
        }
    } else {
        user->async_buf = MEM_MALLOC(sizeof(*user->async_buf));
        user->async_buf->base = MEM_MALLOC(suggested_size);
        user->async_buf->len = suggested_size;
    }
     
    buf->base = (char*)user->async_buf->base;
    buf->len = user->async_buf->len;
}

uint32_t pt_sclient_ref_increment(struct pt_sclient *user)
{
	return ++user->ref_count;
}

uint32_t pt_sclient_ref_decrement(struct pt_sclient *user)
{
	return --user->ref_count;
}

static struct pt_sclient* pt_sclient_new(struct pt_server *server)
{
    struct pt_sclient *user;
    
    user = MEM_MALLOC(sizeof(struct pt_sclient));
    bzero(user, sizeof(struct pt_sclient));
    
    user->buf = pt_buffer_new(USER_DEFAULT_BUFF_SIZE);
    user->id = ++server->serial;
	user->ref_count = 1;
    
    return user;
}

static void pt_sclient_free(struct pt_sclient* user)
{
	if(pt_sclient_ref_decrement(user) <= 0)
	{
		pt_buffer_free(user->buf);
		user->buf = NULL;
    
		if(user->async_buf){
			MEM_FREE(user->async_buf->base);
			MEM_FREE(user->async_buf);
			user->async_buf = NULL;
		}

		MEM_FREE(user);
	}	
}

/*
 当写入操作完成或失败会执行本函数
 释放pt_buffer和write_request数据
 */
static void pt_server_write_cb(uv_write_t* req, int status)
{
	struct pt_wreq *wreq = (struct pt_wreq*)req; 
	struct pt_sclient *user = wreq->data;

	user->server->number_of_pop_send++;

	if(status == 0)
	{
		user->server->number_of_send_bytes += wreq->buff->length;
	}

	pt_sclient_free(user);
    pt_buffer_free(wreq->buff);
    MEM_FREE(wreq);
}

/*
 当成功关闭了handle后，释放用户资源
 */
static void pt_server_on_close_conn(uv_handle_t* peer) {
    struct pt_sclient *user = peer->data;
    pt_sclient_free(user);
}


//关闭一个客户端的连接
static void pt_server_close_conn(struct pt_sclient *user, qboolean remove)
{
    struct pt_server *server = user->server;
    
    //检查用户是否已被关闭
    if(user->connected  == false)
    {
        return;
    }
    
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
    
    uv_close((uv_handle_t*)&user->sock.stream, pt_server_on_close_conn);
}


//关闭服务器的回调函数
static void pt_server_on_close_listener(uv_handle_t *handle)
{
    struct pt_server *server = handle->data;
    server->state = PT_STATE_SHUTDOWN;
	if(server->on_close) server->on_close(server, false);
}

static void pt_server_on_close_free(uv_handle_t *handle)
{
    struct pt_server *server = handle->data;
	server->state = PT_STATE_SHUTDOWN;
	if(server->on_close) server->on_close(server, true);
	pt_server_free(server);
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
    uint32_t packet_err = PACKET_INFO_OK;   //默认是没有任何错误的
    struct pt_sclient *user = stream->data;
	struct pt_server *server = user->server;
    struct pt_buffer *userbuf = NULL;

	server->number_of_pop_recv++;	
	//用户发送eof包或异常 则直接断开用户
    if(nread <= 0)
    {
        pt_server_close_conn(user, true);
        return;
    }
    
	server->number_of_recv_bytes += nread;	

    //将数据追加到缓冲区
    pt_buffer_write(user->buf, (unsigned char*)buf->base, (uint32_t)nread);
    
    //循环读取缓冲区数据，如果数据错误则返回false且不再执行本while
    //稳定性修复，当客户端断开的时候，不再处理接收的数据
    //等待系统的回收
    //在这里connected == false一般是由pt_server_send函数overflow导致的
    while(user->connected && pt_get_packet_status(user->buf, &packet_err))
    {
        //拆分一个数据包
        userbuf = pt_split_packet(user->buf);
        //如果服务器开启了加密功能,则执行解密函数
        if(user->server->enable_encrypt)
        {
            //对数据包进行解密，并且校验包序列，且校验数据的crc是否正确
            if(pt_decrypt_package(user->serial, &user->encrypt_ctx, userbuf) == false)
            {
				if(server->warning_user != NULL) server->warning_user(user);
                //数据不正确，断开用户的连接，释放缓冲区
                pt_buffer_free(userbuf);
                pt_server_close_conn(user, true);
                return;
            }
            
            //数据正确，增加包序列
            user->serial++;
        }
        
        //回调用户函数，通知数据到达
        if(user->server->on_receive)
        {
            user->server->on_receive(user, userbuf);
        }
        
        //释放拆分包后的数据
        pt_buffer_free(userbuf);
    }
    
    //如果用户发的数据是致命错误，则干掉用户
    if(packet_err == PACKET_INFO_FAKE || packet_err == PACKET_INFO_OVERFLOW){
		if(server->warning_user != NULL) server->warning_user(user);
        pt_server_close_conn(user, true);
    }
}

/*
 libuv的connection通知
 
 处理新用户连接，最大用户数量，创建pt_sclient结构并添加到clients表中
 执行用户自定义的通知信息
 */
static void pt_server_connection_cb(uv_stream_t* listener, int status)
{
    struct pt_server *server = listener->data;
    struct pt_sclient *user = pt_sclient_new(server);
	
	if(server->on_alloc_sclient != NULL) server->on_alloc_sclient(user);

    user->server = server;
    
    if(server->is_pipe){
        server->last_error = uv_pipe_init(listener->loop, &user->sock.pipe, true);
    } else {
        server->last_error = uv_tcp_init(listener->loop, &user->sock.tcp);
    }
    
    if( server->last_error != 0 )
    {
		if(server->error_notify) server->error_notify(server, "connection_cb::uv_xxx_init");
		pt_sclient_free(user);
        return;
    }

    user->sock.stream.data = user;
    
    server->last_error = uv_accept(listener, (uv_stream_t*)&user->sock);
    
    //如果accept失败，写出错误日志，并关掉这个sock
    if( server->last_error != 0 )
    {
		if(server->error_notify) server->error_notify(server, "connection_cb::uv_accept");
        uv_close((uv_handle_t*)&user->sock, pt_server_on_close_conn);
        return;
    }
    
    //设置客户端连接已经成功
    user->connected = true;
    
    //限制当前服务器的最大连接数
    if(server->number_of_connected + 1 > server->number_of_max_connected){
        pt_server_close_conn(user, false);
        return;
    }
    
    //进行数据过滤，如果用户Connect请求被on_connect干掉则直接断开用户
    if(server->on_connect && server->on_connect(user) == false){
        pt_server_close_conn(user, false);
        return;
    }
    
    if(server->enable_encrypt){
        user->serial = 0;
        RC4_set_key(&user->encrypt_ctx, sizeof(server->encrypt_key), (const unsigned char*)&server->encrypt_key);
    }
    
    server->number_of_connected++;
    
    if(server->is_pipe == false){
        //设置用户30秒后做keepalive检查
        uv_tcp_keepalive(&user->sock.tcp, true, server->keep_alive_delay);
        
        if(server->no_delay){
            uv_tcp_nodelay(&user->sock.tcp, true);
        }
    }
    
    //添加到搜索树内
    pt_table_insert(server->clients, user->id, user);
    
    //开始读取网络数据
    server->last_error = uv_read_start(&user->sock.stream, pt_server_alloc_buf, pt_server_read_cb);
    

    if( server->last_error != 0 )
    {
		if(server->error_notify) server->error_notify(server, "connection_cb::uv_read_start");
        //修复一个bug，之前直接使用uv_close，正确应该使用这个函数
        pt_server_close_conn(user, true);
        return;
    }

	server->number_of_push_recv++;
}

struct pt_server* pt_server_new()
{
    struct pt_server *server = (struct pt_server *)MEM_MALLOC(sizeof(struct pt_server));
    
    bzero(server, sizeof(struct pt_server));
    
    server->clients = pt_table_new();
    
    //初始化libuv的回调函数
    server->state = PT_STATE_CREATED;
    //server->connection_cb = pt_server_connection_cb;
    //server->read_cb = pt_server_read_cb;
    //server->write_cb = pt_server_write_cb;
    server->number_of_max_send_queue = NUMBER_MAX_SEND_QUEUE;
    
    return server;
}

void pt_server_free(struct pt_server *srv)
{
    if(srv->state == PT_STATE_RUNNING){
        FATAL("server not shutdown", __FUNCTION__, __FILE__, __LINE__);
		return;
    }
    
    pt_table_free(srv->clients);
    MEM_FREE(srv);
}

void pt_server_set_nodelay(struct pt_server *server, qboolean nodelay)
{
    server->no_delay = nodelay;
}

void pt_server_init(struct pt_server *server, uv_loop_t *loop, int max_conn, int keep_alive_delay,pt_server_on_connect on_conn,
                        pt_server_on_receive on_receive, pt_server_on_disconnect on_disconnect)
{
    if(server->state != PT_STATE_CREATED){
        FATAL("server already initialize",__FUNCTION__,__FILE__,__LINE__);
        return;
    }
    
    server->number_of_max_connected = max_conn;
    server->loop = loop;
    server->on_connect = on_conn;
    server->on_receive = on_receive;
    server->on_disconnect = on_disconnect;
    server->keep_alive_delay = keep_alive_delay;
    
    server->state = PT_STATE_CREATED;
}

qboolean pt_server_start(struct pt_server *server, const char* host, uint16_t port)
{
    struct sockaddr_in adr;
    
    if(server->state != PT_STATE_INITIALIZED)
    {
		if(server->error_notify) server->error_notify(server, "server state not initialized");
        return false;
    }
    
    server->last_error = uv_tcp_init(server->loop, &server->listener.tcp);
    if(server->last_error != 0){
		if(server->error_notify) server->error_notify(server, "pt_server_start::uv_tcp_init");
		return false;
    }
    
    server->is_pipe = false;
    server->listener.stream.data = server;
    
    uv_ip4_addr(host, port, &adr);
    
    server->last_error = uv_tcp_bind(&server->listener.tcp, (const struct sockaddr*)&adr, 0);
    
    if(server->last_error != 0){
		if(server->error_notify) server->error_notify(server, "pt_server_start::uv_tcp_bind");
        return false;
    }
    
    server->last_error = uv_listen((uv_stream_t*)&server->listener, SOMAXCONN, pt_server_connection_cb);
    if(server->last_error != 0){
		if(server->error_notify) server->error_notify(server, "pt_server_start::uv_listen");
        return false;
    }

	server->start_time = time(NULL);	
    server->state = PT_STATE_RUNNING;
    return true;
}


qboolean pt_server_start_pipe(struct pt_server *server, const char *path)
{
    if(server->state != PT_STATE_INITIALIZED)
    {
		if(server->error_notify) server->error_notify(server, "server state not initialized");
        return false;
    }
    
    server->last_error = uv_pipe_init(server->loop, &server->listener.pipe, true);
    if(server->last_error!= 0){
		if(server->error_notify) server->error_notify(server, "pt_server_start::uv_pipe_init");
		return false;
    }
    
    server->is_pipe = true;
    server->listener.stream.data = server;
    
    remove(path);

     server->last_error= uv_pipe_bind(&server->listener.pipe, path);
    if(server->last_error != 0){
		if(server->error_notify) server->error_notify(server, "pt_server_start::uv_pipe_bind");
        return false;
    }
    
    server->last_error = uv_listen(&server->listener.stream, SOMAXCONN, pt_server_connection_cb);
    if(server->last_error != 0){

		if(server->error_notify) server->error_notify(server, "pt_server_start_pipe::uv_listen");
        return false;
    }
    

	server->start_time = time(NULL);	
    server->state = PT_STATE_RUNNING;
    return true;
}


void pt_server_set_encrypt(struct pt_server *server, const uint32_t encrypt_key[4])
{
	int i;
    server->enable_encrypt = true;
    
    for(i =0;i < 4; i++)
    {
        server->encrypt_key[i] = encrypt_key[i];
    }
}

/*
 发送数据到客户端
 */
qboolean pt_server_send(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct pt_server *server;

    assert(buff != NULL);
	assert(user != NULL);

    qboolean result = false;

	server = user->server;
    
    if(user->connected == false){
        goto ProcedureEnd;
    }
    
    //防止服务器发包过多导致服务器的内存耗尽
    if(user->sock.stream.write_queue_size >= user->server->number_of_max_send_queue){
        pt_server_close_conn(user, true);
        goto ProcedureEnd;
    }
    
    struct pt_wreq *wreq = MEM_MALLOC(sizeof(struct pt_wreq));
    
    //引用当前buffer
    pt_buffer_ref_increment(buff);
	//引用sclient
	pt_sclient_ref_increment(user);
    wreq->buff = buff;
    wreq->buf = uv_buf_init((char*)buff->buff, buff->length);
	wreq->data = user;

	server->number_of_push_send++;

	server->last_error = uv_write(&wreq->req, (uv_stream_t*)&user->sock, &wreq->buf, 1, pt_server_write_cb);

	if (user->server->last_error) {

		if(server->error_notify) server->error_notify(server, "send::uv_write");
        MEM_FREE(wreq);
        
        //刚才增加了引用计数,现在减掉
        pt_buffer_free(buff);
        goto ProcedureEnd;
    }
    
ProcedureEnd:
    pt_buffer_free(buff);
    return result;
}

static void pt_server_send_to_all_cb(struct pt_table *ptable, uint64_t id, void *ptr, void* user_arg)
{
    struct pt_buffer *buff = user_arg;
    struct pt_sclient *user = ptr;
    
    pt_buffer_ref_increment(buff);
    pt_server_send(user, buff);
}

void pt_server_send_to_all(struct pt_server *server, struct pt_buffer *buff)
{
    pt_table_enum(server->clients, pt_server_send_to_all_cb, buff);
    pt_buffer_free(buff);
}

static void pt_server_close_all_cb(struct pt_table *ptable, uint64_t id, void *ptr, void* user_arg)
{
    pt_server_close_conn(ptr, true);
}

void pt_server_close(struct pt_server *server)
{
    pt_table_enum(server->clients, pt_server_close_all_cb, server);
    pt_table_clear(server->clients);
    uv_close((uv_handle_t*)&server->listener, pt_server_on_close_listener);
}

void pt_server_close_free(struct pt_server *server)
{
    pt_table_enum(server->clients, pt_server_close_all_cb, server);
    pt_table_clear(server->clients);
    uv_close((uv_handle_t*)&server->listener, pt_server_on_close_free);
}

qboolean pt_server_disconnect_conn(struct pt_sclient *user)
{
    if(user->connected){
        pt_server_close_conn(user, true);
        return true;
    }
    return false;
}


struct pt_sclient *pt_server_find_sclient(struct pt_server *server, uint64_t id)
{
	struct pt_sclient *user = NULL;

	user = pt_table_find(server->clients, id);

	return user;
}
