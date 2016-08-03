#ifndef dispatch_h
#define dispatch_h

//判断后端服务是否正确连接
qboolean pt_dispatch_succeed();

struct pt_cluster *get_dispatch();


/**
 * 派遣客户端发送到网关服务器的数据
 */
void pt_dispatch_receive(struct pt_cluster *cluster, struct pt_sclient *user,
		struct pt_buffer *buff);

/*
 * 后端服务器回复数据包到用户端
 * */
void pt_dispatch_send(struct pt_cluster *cluster, struct pt_backend *ed,
		struct pt_buffer *buff);


/*
 * 创建或者更新一个派遣服务器群组
 * */
void pt_dispatch_update(uv_loop_t *loop, BackendC *config);

/*
 * 激活最上层的服务器组
 * */
void pt_dispatch_active();
#endif /* dispatch_h */
