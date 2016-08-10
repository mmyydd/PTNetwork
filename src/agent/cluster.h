#ifndef cluster_h
#define cluster_h

struct pt_cluster
{
    struct pt_cluster *next;
	struct pt_backend *servers;	
	uv_timer_t timer;

	//当前集群是否已经激活了连接
	qboolean is_active;
	
	//客户端的引用计数器
	uint32_t ref_count;
	
	//更新listener的记录
	struct pt_server *listener;

	struct pt_table *users;
};


void pt_cluster_backend_disconnect(struct pt_cluster *cluster, struct pt_backend *be);


/*
 * 提供客户端数据转发函数
 * */

//client connect
void pt_cluster_user_connect(struct pt_cluster *cluster, struct pt_sclient *user);

//client disconnect
void pt_cluster_user_disconnect(struct pt_cluster *cluster, struct pt_sclient *user);

//clients => backend
void pt_cluster_user_receive(struct pt_cluster *cluster, struct pt_sclient *user,
		struct pt_buffer *buff);


//backend => clients
void pt_cluster_user_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_buffer *buff);


/*
 * 创建一个集群对象
 * */
struct pt_cluster *pt_cluster_new();

/*
 * 初始化一个服务器集群
 * */
void pt_cluster_init(struct pt_cluster *cluster, BackendC* config);

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

uint32_t pt_cluster_ref_inc(struct pt_cluster *cluster);
uint32_t pt_cluster_ref_dec(struct pt_cluster *cluster);

struct pt_backend *pt_cluster_find_backend(struct pt_cluster *cluster,
		uint32_t server_id);



void pt_cluster_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_sclient *user, struct net_header header, void *data,
		uint32_t length);


//更新cluster列表
void pt_cluster_update(struct pt_cluster *newcluster);
//全局的clusters
extern struct pt_cluster *g_clusters;
#endif /* cluster_h */
