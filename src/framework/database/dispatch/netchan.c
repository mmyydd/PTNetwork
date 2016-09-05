#include <common.h>
#include "netchan.h"
#include "common/message.h"
#include "main.h"
#include "query.h"

struct pt_server *private_channel;
struct pt_server *public_channel;
struct pt_server *netchan;


qboolean channel_on_conn(struct pt_sclient *user)
{
	db_worker_add(user);
	return true;
}

void channel_on_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct net_header hdr;
	struct buffer_reader reader;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));

	switch(hdr.id)
	{
		case ID_QUERY_OK:
		case ID_EXECUTE_OK:
			db_worker_dispatch_ok(user, &reader);
			break;

		case ID_UNPACKED_FAILED:
			db_worker_unpacked_failed(user);
			break;

		default:
			fprintf(stderr, "[LISTENER] worker invalid package id:%u\n", hdr.id);
			break;
	}
}

void channel_on_disconn(struct pt_sclient *user)
{
	db_worker_remove(user);
}


qboolean netchan_on_conn(struct pt_sclient *user)
{
	return true;
}

void netchan_on_receive(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header hdr;
	uint64_t magic_id;
	
	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));

	if(buffer_reader_read(&reader, &magic_id, sizeof(magic_id)) == false)
	{
		fprintf(stderr, "[LISTENER] user buffer_reader_read magic_id failed\n");
		pt_server_disconnect_conn(user);
		return;
	}

	switch(hdr.id)
	{
		case ID_QUERY:
		case ID_EXECUTE:
			db_worker_dispatch(user, buff, magic_id, hdr, &reader);
			break;
		default:
			fprintf(stderr, "[LISTENER] user invalid package id:%u\n", hdr.id);
			break;
	}
}

void netchan_on_disconn(struct pt_sclient *user)
{

}

void netchan_pre_operation()
{
	private_channel = pt_server_new();
	public_channel = pt_server_new();
	netchan = pt_server_new();
	
	fprintf(stderr,"PrivateChannel:%s\n", PRIVATE_CHANNEL);
	//初始化调给子进程的数据
	pt_server_init(private_channel, uv_default_loop(), 10000,10,
			channel_on_conn, channel_on_receive, channel_on_disconn);
	
	//初始化外部程序使用的命令
	pt_server_init(public_channel, uv_default_loop(), 10000,10,
			netchan_on_conn, netchan_on_receive, netchan_on_disconn);

	pt_server_init(netchan, uv_default_loop(), 10000,10,
			netchan_on_conn, netchan_on_receive, netchan_on_disconn);

	//启动用于内部通信的channel
	if(pt_server_start_pipe(private_channel, PRIVATE_CHANNEL) == false){
		fprintf(stderr,"[LISTENER]private_channel start failed %s\n", uv_strerror(private_channel->last_error));
		exit(EXIT_CODE_SOCKET_FAILED);
	}

	fprintf(stderr, "[LISTENER] private_channel start success\n");

	//启动fsock用于本地数据交互
	if(pt_server_start_pipe(public_channel, PUBLIC_CHANNEL) == false)
	{
		fprintf(stderr,"[LISTENER] public_channel start failed\n");
		exit(EXIT_CODE_SOCKET_FAILED);
	}

	fprintf(stderr,"[LISTENER] public_channel start success\n");

	//启动tcp监听 用于接受远端的数据
	if(pt_server_start(netchan, "0.0.0.0", network_config->port) == false){
		fprintf(stderr, "[LISTENER] routine start failed\n");
		exit(EXIT_CODE_SOCKET_FAILED);
	}

	fprintf(stderr, "[LISTENER] routine start success\n");
}

void netchan_break_operation()
{
	pt_server_close_free(private_channel);
	pt_server_close_free(public_channel);
	pt_server_close_free(netchan);
}

void netchan_post_operation()
{

}
