#include <ptnetwork.h>

struct pt_server *server;
uv_signal_t sigint;

qboolean on_connected(struct pt_sclient *user)
{
	fprintf(stderr, "user connected:%lu\n", user->id);
	return true;
}

void on_recevied(struct pt_sclient *user, struct pt_buffer *buff)
{	
	struct net_header hdr;
	struct buffer_reader reader;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));
	//fprintf(stderr, "user:%lu receive:%s\n", user->id, buffer_reader_cur_pos(&reader));
	
	pt_buffer_ref_increment(buff);
	pt_server_send(user, buff);
}

void on_disconencted(struct pt_sclient *user)
{
	fprintf(stderr, "user disconnected:%lu\n", user->id);
}

void on_signal(uv_signal_t *handle, int signal)
{
	if(signal == SIGINT)
	{
		uv_signal_stop(handle);
		pt_server_close_free(server);
		return;
	}
}

int main()
{
	server = pt_server_new();

	pt_server_init(server, uv_default_loop(), 10000, 100, on_connected, on_recevied, on_disconencted);

	if(!pt_server_start_pipe(server, "/var/tmp/ptnetwork_tserver.sock"))
	{
		fprintf(stderr, "server start failed\n");
		return 1;
	}

	uv_signal_init(uv_default_loop(), &sigint);
	uv_signal_start(&sigint, on_signal, SIGINT);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	fprintf(stderr, "exit...\n");
	return 0;
}
