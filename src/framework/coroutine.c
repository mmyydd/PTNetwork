#include "common.h"
#include "coroutine.h"

struct pt_coroutine_user* pt_coroutine_user_new()
{
	struct pt_coroutine_user *user = NULL;

	user = MEM_MALLOC(sizeof(struct pt_coroutine_user));
	bzero(user, sizeof(struct pt_coroutine_user));
	return user;
}

void pt_coroutine_user_free(struct pt_coroutine_user *user)
{
	MEM_FREE(user);

}
static qboolean pt_routine_on_new_connection()
{
	return true;
}


static void pt_routine_on_received(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct pt_coroutine *routine;
	struct pt_coroutine_user *coroutine_user;
	struct buffer_reader reader;
	struct net_header hdr;
	uint64_t user_id;
	

	routine = user->server->data;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));
	buffer_reader_read(&reader, &user_id, sizeof(user_id));

	if(hdr.id == ID_USER_CONNECTED)
	{
		coroutine_user = pt_coroutine_user_new();

		coroutine_user->user = user;
		coroutine_user->user_id = user_id;
		pt_table_insert(routine->users, user_id, coroutine_user);
		if(routine->on_connected) routine->on_connected(routine, coroutine_user);
	}
	else if(hdr.id == ID_USER_DISCONNECTED)
	{
		coroutine_user = pt_table_find(routine->users, user_id);
		
		if(coroutine_user != NULL)
		{
			if(routine->on_disconnected) routine->on_disconnected(routine, coroutine_user);
			pt_table_erase(routine->users, user_id);
			pt_coroutine_user_free(coroutine_user);
		}
	}
	else
	{
		if(routine->on_received) routine->on_received(routine, coroutine_user, hdr, &reader);
	}
}


static void pt_routine_remove_callback(struct pt_table *table, uint64_t id, void *data, void* arg)
{
	struct pt_coroutine_user *user = data;
	struct pt_sclient *net_user = arg;

	if(user->user == net_user)
	{
		pt_table_erase(table, id);
	}
}
static void pt_routine_on_disconnected(struct pt_sclient *user)
{
	struct pt_coroutine *routine = user->server->data;
	pt_table_enum(routine->users, pt_routine_remove_callback, user);
}


struct pt_coroutine* pt_coroutine_new()
{
	struct pt_coroutine *routine = NULL;

	routine = MEM_MALLOC(sizeof(struct pt_coroutine));
	
	bzero(routine, sizeof(struct pt_coroutine));
	
	routine->loop = uv_default_loop();
	routine->users = pt_table_new();
	routine->server = pt_server_new();
	routine->server->data = routine;
	routine->serial = 1;
	
	pt_server_init(routine->server, routine->loop, 1000,10, pt_routine_on_new_connection, pt_routine_on_received, pt_routine_on_disconnected);

	return routine;
}

void pt_coroutine_free(struct pt_coroutine *routine)
{
	pt_server_free(routine->server);
	pt_table_free(routine->users);
	routine->loop = NULL;
	MEM_FREE(routine);
}

void pt_coroutine_init(struct pt_coroutine *routine, pt_coroutine_connection_event on_connected, pt_coroutine_connection_event on_disconnected, pt_coroutine_receive on_received)
{
	routine->on_connected = on_connected;
	routine->on_disconnected = on_disconnected;
	routine->on_received = on_received;
}

qboolean pt_coroutine_start(struct pt_coroutine *routine, qboolean is_pipe, const char *host, uint16_t port)
{
	if(is_pipe)
	{
		return pt_server_start_pipe(routine->server, host);
	}
	else 
	{
		return pt_server_start(routine->server, host, port);
	}
}
