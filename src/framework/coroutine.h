#ifndef _COROUTINE_INCLUDED_H_
#define _COROUTINE_INCLUDED_H_


struct pt_coroutine;
struct pt_coroutine_user;

typedef void (*pt_coroutine_receive)(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader);

typedef void (*pt_coroutine_connection_event)(
		struct pt_coroutine *routine, struct pt_coroutine_user *user);

struct pt_coroutine
{
	uint64_t serial;
	uv_loop_t *loop;
	struct pt_table *users;
	struct pt_server *server;
	pt_coroutine_connection_event on_connected;
	pt_coroutine_connection_event on_disconnected;
	pt_coroutine_receive on_received;

	void *data;
};


struct pt_coroutine_user
{
	struct pt_sclient *user;
	uint64_t user_id;
};

struct pt_coroutine* pt_coroutine_new();
void pt_coroutine_free(struct pt_coroutine *routine);

void pt_coroutine_init(struct pt_coroutine *routine, pt_coroutine_connection_event on_connected, pt_coroutine_connection_event on_disconnected, pt_coroutine_receive on_received);

qboolean pt_coroutine_start(struct pt_coroutine *routine, qboolean is_pipe, const char *host, uint16_t port);

void pt_coroutine_stop(struct pt_coroutine *routine);
#endif
