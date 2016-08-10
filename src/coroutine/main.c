#include <ptnetwork.h>
#include "coroutine.h"

struct pt_coroutine *g_routine;

void coroutine_received(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader)
{
	printf("receive_data:%s\n", buffer_reader_cur_pos(reader));
}

void coroutine_connected(struct pt_coroutine *routine, struct pt_coroutine_user *user)
{
	printf("agent connect new user:%llu\n", user->user_id);
}

void coroutine_disconnected(
		struct pt_coroutine *routine, struct pt_coroutine_user *user)
{
	printf("agent disconnect user:%llu\n", user->user_id);
}
int main(int argc, char *argv[])
{
	g_routine = pt_coroutine_new();

	pt_coroutine_init(g_routine, coroutine_connected, coroutine_disconnected, coroutine_received);

	pt_coroutine_start(g_routine, true, "/var/tmp/testserver.sock",0);


	uv_run(uv_default_loop(), UV_RUN_DEFAULT);


	return 0;
}
