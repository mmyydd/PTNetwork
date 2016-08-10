/*
 * 175PT Core Service 
 *    Agent Server
 * */
#include "common.h"
#include "backend.h"
#include "cluster.h"

static void backend_connected(struct pt_client *conn, enum pt_client_state state)
{
	struct pt_backend *be = conn->data;
	struct pt_cluster *cluster = be->cluster;

#ifndef NDEBUG
	if(state == PT_CONNECTED)
	{
		printf("connected to server:%s\n", be->descriptor);
	}

	if(state == PT_CONNECT_FAILED)
	{
		printf("connect to server %s failed\n", be->descriptor);
	}

#endif
}

static void backend_receive(struct pt_client *conn, struct pt_buffer *buff)
{
	struct pt_backend *be = conn->data;
	struct pt_cluster *cluster = be->cluster;
#ifndef NDEBUG
	pt_cluster_user_send(cluster, be, buff);
#endif
}

static void backend_disconnected(struct pt_client *conn)
{
	struct pt_backend *be = conn->data;
	struct pt_cluster *cluster = be->cluster;
#ifndef NDEBUG
	printf("backend_disconnected\n");
#endif
	pt_cluster_backend_disconnect( cluster, be );
}

struct pt_backend* pt_backend_new()
{
    struct pt_backend *be;
    
    be = MEM_MALLOC(sizeof(struct pt_backend));
    bzero(be, sizeof(struct pt_backend));
    
    be->conn = pt_client_new();
    be->conn->data = be;
    
    return be;
}

void pt_backend_free(struct pt_backend *be)
{
    pt_client_free(be->conn);
    MEM_FREE(be);
}

void pt_backend_init(struct pt_backend *ed, struct pt_cluster *cluster,
		const char *desc,uint32_t server_id,const char *host, uint16_t port,
		qboolean is_pipe)
{
	ed->cluster = cluster;
	ed->server_id = server_id;
	strcpy(ed->descriptor, desc);
	strcpy(ed->hostname, host);
	ed->port = port;
	ed->is_pipe = is_pipe;

    pt_client_init(cluster->listener->loop, ed->conn, backend_connected, backend_receive, backend_disconnected);
}

void pt_backend_try_connect(struct pt_backend *be)
{
	if(be->conn->state == PT_NO_CONNECT)
	{
		//投递连接请求
		if(be->is_pipe)
		{
			printf("try connect pipe %s\n", be->hostname);
			pt_client_connect_pipe(be->conn, be->hostname);
		}
		else
		{
			printf("try connect to %s:%i\n", be->hostname,be->port);
			if(pt_client_connect(be->conn, be->hostname, be->port) == false){
				printf("try connect post failed\n");
			}
		}
	}
}

void pt_backend_send(struct pt_backend *be, struct pt_buffer *buff)
{
	pt_client_send(be->conn, buff);
}
/*
 * 关闭一个后端服务处理点
 * */
void pt_backend_close(struct pt_backend *be)
{
	pt_client_disconnect(be->conn);
}

qboolean pt_backend_is_connected(struct pt_backend *be)
{
    return be->conn->state == PT_CONNECTED;
}
