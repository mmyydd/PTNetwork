#include "common.h"
#include "backend.h"
#include "cluster.h"
#include "dispatch.h"
#include "listener.h"


#define SERVER_ID_SPLIT 1000

struct pt_cluster *g_dispatch;

struct pt_cluster *get_dispatch()
{
	return g_dispatch;
}


void pt_dispatch_disconnect(struct pt_cluster *cluster, struct pt_sclient *user)
{

}

void pt_dispatch_connect(struct pt_cluster *cluster, struct pt_sclient *user)
{

}
//派遣客户端发送到网关服务器的数据
void pt_dispatch_receive(struct pt_cluster *cluster, struct pt_sclient *user,
	struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header header;
	uint32_t length;
	unsigned char *data;
	uint32_t server_id;
	struct pt_backend *ed;


	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	
	//忽略加密包中的serial
	buffer_reader_ignore_bytes(&reader, sizeof(uint32_t));

	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);

	server_id = header.id / SERVER_ID_SPLIT;

	if(server_id == 0){
		//无效的服务器id
		return;
	}
	ed = pt_cluster_find_backend(cluster, server_id);
	
	//找不到后端服务器
	if(ed == NULL)
	{
		return;
	}
	
	//forward 后端服务器
	pt_cluster_send(cluster, ed, user, header, data, length);
}

//后端服务器回复数据包到用户端
void pt_dispatch_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header header, *send_header;
	uint64_t user_id;
	unsigned char *data;
	uint32_t length;
	unsigned char *buff_data;
	struct pt_sclient *user;
	struct pt_server *listener;

	listener = get_listener();
	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	buffer_reader_read(&reader, &user_id, sizeof(user_id));

	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);

	user = pt_table_find(listener->clients, user_id);

	//接收数据的用户不存在
	if(user == NULL)
	{
		return;
	}

	//直接增加引用计数器，减少pt_buffer多次申请的开销
	pt_buffer_ref_increment(buff);
	
	//将数据对齐到pt_buffer中
	buff_data = &buff->buff[sizeof(struct net_header)];
	memcpy(buff_data,data,length);
	buff->length = sizeof(struct net_header) + length;

	//修改net_header
	send_header = (struct net_header*)buff->buff;
	send_header->length = buff->length;

	//dispatch
	pt_server_send(user, buff);
}


qboolean pt_dispatch_succeed()
{
	if(g_dispatch == NULL) return false;

	return pt_cluster_is_connected(g_dispatch);
}

void pt_dispatch_update(uv_loop_t *loop, BackendC *config)
{
	struct pt_cluster *cluster = pt_cluster_new();

	pt_cluster_init(cluster, loop, config);

	cluster->next = g_dispatch;
	g_dispatch = cluster;
}


void pt_dispatch_active()
{
	if(g_dispatch == NULL) return;

	pt_cluster_active(g_dispatch);
}
