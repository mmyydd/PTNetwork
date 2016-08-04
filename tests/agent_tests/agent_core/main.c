#include "agent_core.h"
#include <dirent.h>

uv_loop_t *uv_loop;
struct pt_server *g_server;


qboolean on_connect_notify(struct pt_sclient *user)
{

	printf("new client:%llu\n", user->id);
	return true;
}


void on_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header header;
	uint64_t user_id;
	unsigned char *data;
	uint32_t length;
	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(struct net_header));
	buffer_reader_read(&reader, &user_id, sizeof(user_id));
	
	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);

//	printf("%s\n", (const char*)data);

	pt_buffer_ref_increment(buff);
	pt_server_send(user, buff);
}

void on_disconnect(struct pt_sclient *user)
{
	printf("on disconnect:%llu\n", user->id);
}
int main(int argc, char * argv[])
{
	remove("/var/tmp/testserver.sock");
	uv_loop = uv_default_loop();
	printf("start server\n");

	g_server = pt_server_new();

	pt_server_init(g_server, uv_loop, 1000, 30, on_connect_notify, on_receive, on_disconnect);


	printf("init ok\n");
	if(pt_server_start_pipe(g_server, "/var/tmp/testserver.sock") == false)
	{
		printf("server start failed\n");
	}
	
	printf("run\n");
	uv_run(uv_loop, UV_RUN_DEFAULT);

	return 0;
}
