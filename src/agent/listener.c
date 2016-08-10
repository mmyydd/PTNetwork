#include "common.h"
#include "backend.h"
#include "cluster.h"
#include "listener.h"

const uint32_t RC4Key[4] = {0x42970C86, 0xA0B3A057, 0x51B97B3C, 0x70F8891E};

struct pt_server *listener;
uint16_t listener_port = 20140;

qboolean listener_connected(struct pt_sclient *user)
{
	//检查clusters是否初始化
	if(g_clusters == NULL) {
		return false;
	}

	//检查clusters是否成功连接到后端服务器
	if(pt_cluster_is_connected(g_clusters) == false)
	{
		return false;
	}

	user->data = g_clusters;
	pt_cluster_user_connect(g_clusters, user);
	return true;
}

void listener_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
	printf("listener_receive\n");
	//代码逻辑中不可能存在cluster == NULL 
	struct pt_cluster *cluster = user->data;
	pt_cluster_user_receive(cluster, user, buff);
}

void listener_disconnect(struct pt_sclient *user)
{
	//代码逻辑中不可能存在cluster == NULL 
	struct pt_cluster *cluster = user->data;
	pt_cluster_user_disconnect(cluster, user);
}

void listener_init(uv_loop_t *uv_loop, uint16_t port, uint32_t number_of_max_conn)
{
	listener = pt_server_new();

	printf("server init\n");
	printf("max connection:%u\n", number_of_max_conn);
	printf("listen:%d\n", port);

    pt_server_init(listener, uv_loop, number_of_max_conn, 300, listener_connected, listener_receive, listener_disconnect);

	printf("set server encrypt enable\n");
	pt_server_set_encrypt(listener,RC4Key);	
    listener_port = port;
}

void listener_startup()
{
	printf("server start....\n");

	if(pt_server_start(listener, "0.0.0.0", listener_port) == false)
	{
		printf("server start failed\n");
		exit(1);
	}

	printf("server start success...\n");
}


void listener_shutdown()
{
	pt_server_close(listener);	
}
