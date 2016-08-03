#ifndef cluster_h
#define cluster_h

struct pt_cluster
{
    struct pt_cluster *next;
	struct pt_backend *servers;	
	uv_loop_t *loop;
	uv_timer_t timer;

	//当前集群是否已经激活了连接
	qboolean is_active;
	
	//客户端的引用计数器
	uint32_t ref_count;
};


void pt_cluster_notify_connection_state(struct pt_cluster *cluster,
		struct pt_backend *be, enum pt_client_state state); 

void pt_cluster_notify_receive_data(struct pt_cluster *cluster,
		struct pt_backend *be, struct pt_buffer *buff); 


/*
 * 创建一个集群对象
 * */
struct pt_cluster *pt_cluster_new();

/*
 * 初始化一个服务器集群
 * */
void pt_cluster_init(struct pt_cluster *cluster, 
		uv_loop_t *loop, BackendC* config);

/*
 * 判断服务器集群是否正确工作
 * */
qboolean pt_cluster_is_connected(struct pt_cluster *cluster);


/*
 * 激活服务器集群
 * */
void pt_cluster_active(struct pt_cluster *cluster);


/*
 * 关闭一个服务器集群对象
 * 释放backend的后端服务器连接
 * */

void pt_cluster_close(struct pt_cluster *cluster);


struct pt_backend *pt_cluster_find_backend(struct pt_cluster *cluster,
		uint32_t server_id);



void pt_cluster_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_sclient *user, struct net_header header, void *data,
		uint32_t length);

#endif /* cluster_h */
