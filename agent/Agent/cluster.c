#include "common.h"
#include "backend.h"
#include "cluster.h"
#include "dispatch.h"

void pt_cluster_notify_connection_state(struct pt_cluster *cluster,
		struct pt_backend *be, enum pt_client_state state)
{
	char msg[255];
	if(state == PT_CONNECTED)
	{
		sprintf(msg, "try connect to %s => %s success",
				be->descriptor,
				be->hostname);

		LOG(msg, __FUNCTION__, __FILE__, __LINE__);
	}
	else if (state == PT_NO_CONNECT)
	{
		sprintf(msg, "try connect to %s => %s fail or disconnected",
				be->descriptor,
				be->hostname);

		LOG(msg, __FUNCTION__, __FILE__, __LINE__);
	}
}

void pt_cluster_notify_receive_data(struct pt_cluster *cluster,
		struct pt_backend *be, struct pt_buffer *buff)
{
	pt_dispatch_send(cluster, be,buff);
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
	
	return cluster;	
}

void pt_cluster_init(struct pt_cluster *cluster, uv_loop_t *loop, BackendC* config)
{
	int uv_err;
	char msg[256];
	cluster->loop = loop;

	uv_err = uv_timer_init(loop, &cluster->timer);

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
