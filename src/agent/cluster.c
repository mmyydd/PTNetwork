#include "common.h"
#include "backend.h"
#include "cluster.h"
#include "listener.h"

#define SERVER_ID_SPLIT 1000

struct pt_cluster *g_clusters = NULL;

void pt_cluster_update(struct pt_cluster *newcluster)
{
	newcluster->next = g_clusters;
	g_clusters = newcluster;
}


void pt_cluster_user_connect(struct pt_cluster *cluster, struct pt_sclient *user)
{
	struct pt_backend *be = cluster->servers;
	struct net_header hdr = pt_create_nethdr(ID_USER_CONNECTED);
	struct pt_buffer *buff = pt_create_package(hdr,&user->id, sizeof(user->id)); 

	pt_table_insert(cluster->users, user->id, user);

	while(be)
	{
		pt_buffer_ref_increment(buff);
		
		pt_backend_send(be, buff);

		be = be->next;
	}

	pt_buffer_free(buff);
}

void pt_cluster_user_disconnect(struct pt_cluster *cluster, struct pt_sclient *user)
{
	struct pt_backend *be = cluster->servers;
	struct net_header hdr = pt_create_nethdr(ID_USER_DISCONNECTED);
	struct pt_buffer *buff = pt_create_package(hdr,&user->id, sizeof(user->id)); 

	pt_table_erase(cluster->users, user->id);

	while(be)
	{
		pt_buffer_ref_increment(buff);
		
		pt_backend_send(be, buff);

		be = be->next;
	}

	pt_buffer_free(buff);
}

/*
 * 玩家客户端发送数据到服务器
 * 由此函数分别dispatch到各个后端服务
 * */
void pt_cluster_user_receive(struct pt_cluster *cluster, struct pt_sclient *user,
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
	printf("server received:%s\n", data);	
	server_id = header.id / SERVER_ID_SPLIT;
	if(server_id == 0){//无效的服务器id
		return;
	}

	ed = pt_cluster_find_backend(cluster, server_id);
	
	//找不到后端服务器
	if(ed == NULL){
		return;
	}
	
	//forward 后端服务器
	pt_cluster_send(cluster, ed, user, header, data, length);
}
/*
 * 处理后端游戏逻辑服务转发到玩家客户端的数据
 **/
void pt_cluster_user_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_buffer *buff)
{
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

	user = pt_table_find(listener->clients, user_id);

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
	//dispatch
	//pt_server_send(user, buff);
}

static void pt_cluster_kick_all_users(struct pt_table* ptable, uint64_t id,void *ptr, void* user_arg)
{
	struct pt_sclient *user = ptr;
	pt_server_disconnect_conn(user);
}

void pt_cluster_backend_disconnect(struct pt_cluster *cluster, struct pt_backend *be)
{
	char error[256];
	sprintf(error, "backend disconnected:%s",be->descriptor);
	ERROR(error, __FUNCTION__, __FILE__, __LINE__);

	pt_table_enum(cluster->users, pt_cluster_kick_all_users, NULL);
	pt_table_clear(cluster->users);
}


static void pt_cluster_try_fix_connection(struct pt_cluster *cluster)
{
	char msg[255];
	struct pt_backend *current = cluster->servers;

	while(current)
	{
		if(current->conn->state == PT_NO_CONNECT)
		{
			sprintf(msg, "try connect:%s to %s",current->descriptor,
					current->hostname);

			LOG(msg, __FUNCTION__, __FILE__, __LINE__);
			pt_backend_try_connect(current);
		}

		current = current->next;
	}
}

static void pt_cluster_timer(uv_timer_t* timer)
{
	struct pt_cluster *cluster = timer->data;

	if(cluster->is_active)
	{
		if(!pt_cluster_is_connected(cluster))
		{
			pt_cluster_try_fix_connection(cluster);
		}
	}
}

struct pt_cluster *pt_cluster_new()
{
	struct pt_cluster *cluster = MEM_MALLOC(sizeof(struct pt_cluster));	
	bzero(cluster, sizeof(struct pt_cluster));
	cluster->users = pt_table_new();

	return cluster;	
}

void pt_cluster_init(struct pt_cluster *cluster,BackendC* config)
{
	int uv_err;
	char msg[256];
	cluster->listener = listener;

	uv_err = uv_timer_init(listener->loop, &cluster->timer);

	if(uv_err)
	{
		sprintf(msg,"uv_timer_init failed reason:%s", uv_strerror(uv_err));
		ERROR(msg, __FUNCTION__,__FILE__,__LINE__);
	}
	cluster->timer.data = cluster;
	for( size_t i = 0; i < config->n_servers; i ++)
	{
		Backend *backendConfig = config->servers[i];
		struct pt_backend *backend = pt_backend_new();

		printf("server_type:%d\n", backendConfig->server_type);
		pt_backend_init(backend, cluster, 
				backendConfig->server_name,
				backendConfig->server_id,
				backendConfig->server_host,
				backendConfig->server_port,
				backendConfig->server_type
				);
		
		sprintf(msg,"create cluster:backend %s  adr:%s",
				backendConfig->server_name,
				backendConfig->server_host);
		
		LOG(msg, __FUNCTION__,__FILE__,__LINE__);

		backend->next = cluster->servers;
		cluster->servers = backend;
	}		
}

qboolean pt_cluster_is_connected(struct pt_cluster *cluster)
{
	struct pt_backend *current = cluster->servers;

	while(current)
	{
		if(current->conn->state != PT_CONNECTED)
		{
			return false;
		}
		current = current->next;
	}

	return true;
}

static void pt_cluster_deactive(struct pt_cluster *cluster)
{
	int err;
	char log[256];
	err = uv_timer_stop(&cluster->timer);

	if(err)
	{
		sprintf(log, "uv_timer_stop failed:%s  %s", uv_err_name(err),uv_strerror(err));
		ERROR(log, __FUNCTION__, __FILE__, __LINE__);
	}
}

void pt_cluster_active(struct pt_cluster *cluster)
{
	int err;
	char log[256];

	if(cluster->is_active){
		return;
	}

	pt_cluster_try_fix_connection(cluster);

	//添加一个检测用的定时器
	err = uv_timer_start(&cluster->timer,pt_cluster_timer,5000, 5000);
	if(err)
	{
		sprintf(log, "uv_timer_init failed:%s  %s", uv_err_name(err),uv_strerror(err));
		ERROR(log, __FUNCTION__, __FILE__, __LINE__);
	}


	cluster->is_active = true;
}

//递归关闭后端服务列表
static void pt_cluster_close_backend(struct pt_backend *be)
{
	if(be->next)
	{
		pt_cluster_close_backend(be->next);

		be->next = NULL;
	}

	pt_backend_close(be);
}

void pt_cluster_close(struct pt_cluster *cluster)
{
	pt_cluster_close_backend(cluster->servers);
	cluster->servers = NULL;
}

uint32_t pt_cluster_ref_inc(struct pt_cluster *cluster)
{
	return ++cluster->ref_count;
}

uint32_t pt_cluster_ref_dec(struct pt_cluster *cluster)
{
	return --cluster->ref_count;
}



struct pt_backend *pt_cluster_find_backend(struct pt_cluster *cluster,
		uint32_t server_id)
{
	struct pt_backend *ed = NULL;

	ed = cluster->servers;

	while(ed)
	{
		if(ed->server_id == server_id)
		{
			return ed;
		}
		ed = ed->next;
	}

	return NULL;
}



void pt_cluster_send(struct pt_cluster *cluster, struct pt_backend *ed,
struct pt_sclient *user, struct net_header header, void *data,
uint32_t length)
{
	uint32_t buff_size = sizeof(struct net_header) + length + sizeof(uint64_t);

	if(pt_backend_is_connected(ed) == false)
	{
		printf("backend server not connected\n");
		return;
	}

	struct pt_buffer *buff = pt_buffer_new(buff_size);

	header.length = buff_size;
	pt_buffer_write(buff, &header, sizeof(struct net_header));
	pt_buffer_write(buff, &user->id, sizeof(uint64_t));
	pt_buffer_write(buff, data, length);

	pt_backend_send(ed, buff);
}
