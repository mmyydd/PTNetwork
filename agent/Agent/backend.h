/*
 * 175PT Core Service 
 *    Agent Server
 * */

#ifndef backend_h
#define backend_h

struct pt_cluster;

struct pt_backend
{
    struct pt_backend *next;

	//attached cluster
	struct pt_cluster *cluster;	

    //descriptor name
    char descriptor[32];
    
    //dispatch by network packet id
    uint32_t server_id;
    
    //network connection
    struct pt_client *conn;
   
	//server is pipe
	qboolean is_pipe;

    //remote address
    char hostname[64];
    uint16_t port;
};



/*
 * 新建/释放一个后端服务点
 * */
struct pt_backend* pt_backend_new();
void pt_backend_free(struct pt_backend *be);

/*
 * 初始化一个backend后端服务结构
 * 并初始化pt_client结构
 * */
void pt_backend_init(struct pt_backend *ed, struct pt_cluster *cluster,
		const char *desc,uint32_t server_id,
	   	const char *host, uint16_t port, qboolean is_pipe);
/*
 * 关闭一个后端服务处理点
 * */
void pt_backend_close(struct pt_backend *be);

/*
 * 尝试与服务器建立连接
 * 如果已经连接或在连接中则不做任何处理
 * */
void pt_backend_try_connect(struct pt_backend *be);


/*
 * 发送数据到后端的服务
 * */
void pt_backend_send(struct pt_backend *be, struct pt_buffer *buff);
#endif /* backend_h */
