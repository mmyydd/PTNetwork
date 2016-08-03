#include "common.h"
#include "backend.h"
#include "cluster.h"
#include "dispatch.h"
#include "listener.h"

struct pt_server *listener;
uint16_t listener_port = 20140;

qboolean listener_connected(struct pt_sclient *user)
{
    if(!pt_dispatch_succeed()){
        return false;
    }

	user->data = get_dispatch();	
    return true;
}

void listener_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct pt_cluster *cluster = user->data;

		
}

void listener_disconnect(struct pt_sclient *user)
{
    
}

void listener_init(uv_loop_t *uv_loop, uint16_t port, uint32_t number_of_max_conn)
{
    listener = pt_server_new();
    pt_server_init(listener, uv_loop, number_of_max_conn, 300, listener_connected, listener_receive, listener_disconnect);
    
    listener_port = port;
}
