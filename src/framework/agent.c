#include <ptnetwork.h>
#include "common.h"
#include "agent.h"

void pt_agent_send_to_all(struct pt_agent *agent, struct pt_buffer *buff);
qboolean pt_agent_is_connected(struct pt_agent *agent);
void pt_agent_kick_all_users(struct pt_agent *agent);


const uint32_t RC4Key[4] = {0x42970C86, 0xA0B3A057, 0x51B97B3C, 0x70F8891E};

struct pt_buffer *pt_agent_build_user_connected_package(
		struct pt_agent *agent,struct pt_sclient *user)
{
	struct net_header hdr = pt_create_nethdr(ID_USER_CONNECTED);
	struct pt_buffer *buff = pt_create_package(hdr, &user->id,
					 sizeof(user->id));
	
	return buff;
}

struct pt_buffer *pt_agent_build_user_disconnected_package(
		struct pt_agent *agent,struct pt_sclient *user)
{
	struct net_header hdr = pt_create_nethdr(ID_USER_DISCONNECTED);
	struct pt_buffer *buff = pt_create_package(hdr, &user->id,
					 sizeof(user->id));
	
	return buff;
}

static qboolean pt_agent_user_connected(struct pt_sclient *user)
{
	struct pt_agent *agent = user->server->data;

	if(agent->is_connected == false){

		printf("%s: kick %llu\n", __FUNCTION__, user->id);
		return false;
	}
	
	printf("%s:%llu\n", __FUNCTION__, user->id);

	user->data = agent;

	pt_agent_send_to_all(agent, pt_agent_build_user_connected_package(agent, user));

	return true;
}

static void pt_agent_user_received(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header header;
	uint32_t length;
	unsigned char *data;
	uint32_t server_id;
	struct pt_buffer *send_buff;
	struct pt_agent *agent = user->server->data;
	struct pt_agent_node *node = agent->nodes;
	struct net_header *send_hdr;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	
	//忽略加密包中的serial
	buffer_reader_ignore_bytes(&reader, sizeof(uint32_t));

	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);
	printf("server received:%s\n", data);	
	server_id = header.id / SERVER_ID_SPLIT;
	if(server_id == 0){//无效的服务器id
		return;
	}

	send_buff = pt_buffer_new(buff->length);

	pt_buffer_write(send_buff, &header, sizeof(header));
	pt_buffer_write(send_buff, &user->id, sizeof(user->id));
	pt_buffer_write(send_buff, data, length);

	send_hdr = (struct net_header *)send_buff->buff;
	send_hdr->length = send_buff->length;

	while(node)
	{
		if(node->server_id == server_id)
		{
			pt_buffer_ref_increment(send_buff);
			pt_client_send(node->conn, send_buff);
			break;
		}
		node = node->next;
	}

	pt_buffer_free(send_buff);
}


static void pt_agent_user_disconnected(struct pt_sclient *user)
{
	struct pt_agent *agent = user->server->data;

	pt_agent_send_to_all(agent, pt_agent_build_user_disconnected_package(agent, user));
}

static void pt_agent_node_on_connected(struct pt_client *conn, enum pt_client_state state)
{
	struct pt_agent_node *node = conn->data;
	struct pt_agent *agent = node->agent;
	

	if(state == PT_CONNECTED)
	{
		if(agent->notify_node_state)
		{
			agent->notify_node_state(agent, node, AGENT_STATE_CONNECTED);
		}
	}

	if(state == PT_CONNECT_FAILED)
	{
		if(agent->notify_node_state)
		{
			agent->notify_node_state(agent, node, AGENT_STATE_CONNECT_FAILED);
		}
	}
}


static void pt_agent_node_on_received(struct pt_client *conn, struct pt_buffer *buff)
{
	struct pt_agent_node *node = conn->data;
	struct pt_agent *agent = node->agent;
	struct buffer_reader reader;
	struct net_header header, *send_header;
	uint64_t user_id;
	unsigned char *data;
	uint32_t length;
	unsigned char *buff_data;
	struct pt_sclient *user;
	
	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	buffer_reader_read(&reader, &user_id, sizeof(user_id));

	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);

	user = pt_table_find(agent->server->clients, user_id);

	//接收数据的用户不存在
	if(user == NULL)
	{
		return;
	}

	//直接增加引用计数器，减少pt_buffer多次申请的开销
	pt_buffer_ref_increment(buff);
	
	//将数据对齐到pt_buffer中
	//
	//删除user_id (uint64_t)
	//
	//  | struct net_header |
	//  | uint64_t user_id  |
	//  | bytes  data       |
	// 处理后
	//
	//  | struct net_header |
	//  | bytes data        |
	//
	
	buff_data = &buff->buff[sizeof(struct net_header)];
	memcpy(buff_data,data,length);
	buff->length = sizeof(struct net_header) + length;

	//修改net_header
	send_header = (struct net_header*)buff->buff;
	send_header->length = buff->length;

	pt_server_send(user, buff);
}

static void pt_agent_node_on_disconnected(struct pt_client *conn)
{
	struct pt_agent_node *node = conn->data;
	struct pt_agent *agent = node->agent;

	if(agent->notify_node_state)
	{
		agent->notify_node_state(agent, node, AGENT_STATE_DISCONNECTED);
	}

	pt_agent_kick_all_users(agent);
}

static struct pt_agent_node *pt_agent_node_new()
{
	struct pt_agent_node *node = MEM_MALLOC(sizeof(struct pt_agent_node));

	bzero(node, sizeof(struct pt_agent_node));
	node->conn = pt_client_new();

	return node;
}


static void pt_agent_node_free(struct pt_agent_node* node)
{
	pt_client_free(node->conn);
	MEM_FREE(node);
}



struct pt_agent *pt_agent_new()
{
	struct pt_agent *agent = MEM_MALLOC(sizeof(struct pt_agent));

	bzero(agent, sizeof(struct pt_agent));

	uv_timer_init(uv_default_loop(), &agent->timer);
	agent->server = pt_server_new();
	pt_server_init(agent->server, uv_default_loop(), 10000, 10, pt_agent_user_connected, pt_agent_user_received, pt_agent_user_disconnected);

	pt_server_set_encrypt(agent->server, RC4Key);
	agent->server->data = agent;
	agent->timer.data = agent;
	return agent;
}

void pt_agent_add_server(struct pt_agent *agent, const char *name, uint32_t server_id, const char *host, uint16_t port, qboolean is_pipe)
{
	struct pt_agent_node *node = pt_agent_node_new();

	node->agent = agent;
	strncpy(node->name, name, charsmax(node->name));
	strncpy(node->host, host, charsmax(node->host));

	node->server_id = server_id;
	node->port = port;
	node->is_pipe = is_pipe;

	node->next = agent->nodes;

	pt_client_init(uv_default_loop(), node->conn, pt_agent_node_on_connected,
			pt_agent_node_on_received, pt_agent_node_on_disconnected);

	node->conn->data = node;
	agent->nodes = node;
}

struct pt_agent_node *pt_agent_find_node(struct pt_agent *agent, uint32_t server_id)
{
	struct pt_agent_node *node = agent->nodes;

	while(node)
	{
		if(node->server_id == server_id) return node;

		node = node->next;
	}

	return NULL;
}

static void pt_agent_node_try_connect(struct pt_agent_node *node)
{
	if(node->conn->state == PT_NO_CONNECT)
	{
		if(node->is_pipe)
		{
			pt_client_connect_pipe(node->conn, node->host);
		}
		else
		{
			if( pt_client_connect(node->conn, node->host, node->port) == false )
			{
				private_WriteLog(ERROR_LEVEL_FATAL, __FUNCTION__, 
					__FILE__,__LINE__,
					"servernode:%s begin connect failed:address:%s port:%d",
					node->name,node->host,node->port);
			}
		}
	}
}

static void pt_agent_timer(uv_timer_t *timer)
{
	struct pt_agent *agent = timer->data;
	struct pt_agent_node *node = agent->nodes;

	node = agent->nodes;
	while(node)
	{
		//更新agent中的is_connected状态
		if(node->conn->state != PT_CONNECTED)
		{
			if(agent->is_connected == true)
			{
				agent->is_connected = false;
				if(agent->notify_state)
				{
					agent->notify_state(agent, agent->is_connected);
				}
			}
		}

		//如果node->conn的状态是PT_NO_CONNECT 则尝试与服务器建立链接
		if(node->conn->state == PT_NO_CONNECT)
		{
			pt_agent_node_try_connect(node);
		}

		node = node->next;
	}

	if(agent->is_connected == false)
	{
		node = agent->nodes;

		while(node)
		{
			if(node->conn->state != PT_CONNECTED)
			{
				agent->is_connected = false;
				return;
			}
			node = node->next;
		}

		agent->is_connected = true;

		if(agent->notify_state)
		{
			agent->notify_state(agent, agent->is_connected);
		}
	}
}

qboolean pt_agent_startup(struct pt_agent *agent, const char *ip, uint16_t port)
{
	struct pt_agent_node *node;
	if(agent->is_active) return false;

	if(pt_server_start(agent->server, ip, port)==false){
		return false;
	}
	node = agent->nodes;

	while(node)
	{
		pt_agent_node_try_connect(node);
		node = node->next;
	}

	uv_timer_start(&agent->timer, pt_agent_timer, 1000, 1000);

	agent->is_active = true;

	return true;
}

void pt_agent_shutdown(struct pt_agent *agent)
{
	struct pt_agent_node *node;
	uv_timer_stop(&agent->timer);
	
	node = agent->nodes;

	while(node)
	{
		pt_client_disconnect(node->conn);
		node = node->next;
	}

	agent->nodes = node;
	pt_server_close(agent->server);
}

void pt_agent_free(struct pt_agent *agent)
{
	struct pt_agent_node *node, *next;

	node = agent->nodes;

	while(node)
	{
		next = node->next;
		pt_agent_node_free(node);
		node = next;
	}


	pt_server_free(agent->server);
	agent->server = NULL;

	MEM_FREE(agent);
}
void pt_agent_send_to_all(struct pt_agent *agent, struct pt_buffer *buff)
{
	struct pt_agent_node *node;
	if(!agent->is_connected) return;

	node = agent->nodes;

	while(node)
	{
		pt_buffer_ref_increment(buff);
		pt_client_send(node->conn, buff);
		node = node->next;
	}

	pt_buffer_free(buff);

}
static void pt_agent_kick_all_callback(struct pt_table *table, uint64_t id,
		void *data, void *user_arg)
{
	struct pt_sclient *user = data;
	pt_server_disconnect_conn(user);
}
void pt_agent_kick_all_users(struct pt_agent *agent)
{
	pt_table_enum(agent->server->clients, pt_agent_kick_all_callback, agent);
}
