#include <common/common.h>
#include <commander.h>
#include "main.h"
#include "logic.h"
#include "db_pool.h"
#include <common/message.h>
#include "exec.h"

struct pt_client *conn;
uv_timer_t timer;
qboolean is_running;


void logic_on_connect(struct pt_client *conn, enum pt_client_state state)
{
	if(state == PT_CONNECT_FAILED)
	{
		fprintf(stderr, "[LOGIC] get connect to listener failed. exit....\n");
		exit(1);
	}

	if(state == PT_CONNECTED)
	{
		fprintf(stderr, "[LOGIC] get connect to listener successful. wait command...\n");
	}
}


void protobuf_parse_on_failed(struct pt_client *conn)
{
	struct net_header hdr;
	struct pt_buffer *buff;

	fprintf(stderr, "[LOGIC] parse protobuf command failed\n");

	hdr = pt_create_nethdr(ID_UNPACKED_FAILED);
	buff = pt_create_package(hdr, NULL, 0);

	pt_client_send(conn, buff);
}

void logic_build_and_send_result(struct pt_client *conn,qboolean is_exec, DbQuery *dbQuery, DbQueryResult *dbResult)
{
	struct pt_buffer *net_buff;
	struct net_header hdr;
	size_t buflen = db_query_result__get_packed_size(dbResult);
	unsigned char *buff = gcmalloc_alloc(NULL, buflen);
	size_t ok_len = db_query_result__pack(dbResult, buff);

	if(is_exec)
	{
		hdr = pt_create_nethdr(ID_EXECUTE_OK);
	}
	else
	{
		hdr = pt_create_nethdr(ID_QUERY_OK);
	}


	net_buff = pt_create_package(hdr, buff, ok_len);

	pt_client_send(conn, net_buff);
}

void logic_on_received(struct pt_client *conn, struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header hdr;
	unsigned char *data;
	uint32_t length;
	DbQuery *dbQuery;
	DbQueryResult *dbResult;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));

	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);

	if(hdr.id == ID_QUERY || hdr.id == ID_EXECUTE)
	{
		dbQuery = db_query__unpack(NULL, length, data);

		if(dbQuery == NULL) 
		{
			protobuf_parse_on_failed(conn);
			return;
		}
		
		gcmalloc_push_frame(NULL);

		dbResult = db_command_exec(dbQuery, hdr.id == ID_EXECUTE);
		if(dbResult) logic_build_and_send_result(conn, hdr.id == ID_EXECUTE, dbQuery, dbResult);

		gcmalloc_pop_frame(NULL);

		db_query__free_unpacked(dbQuery,NULL);
	}
	else
	{
		fprintf(stderr, "[LOGIC] invalid package id:%u\n", hdr.id);
	}
}

void logic_on_disconnected(struct pt_client *conn)
{
	fprintf(stderr, "[LOGIC] listener connection lost...\n");

	if(is_running)
	{
		fprintf(stderr, "[LOGIC] listener connection list exit...\n");
		exit(1);
	}
}


//拦截sigterm
static void on_signal_term(uv_signal_t *handle, int signal)
{
	uv_signal_stop(&sigterm);
	sigint_stop();
	is_running = false;

	if(conn) pt_client_disconnect(conn);
}

//逻辑调度进程的入口
int logic_main(int argc, char *argv[])
{
	uv_disable_stdio_inheritance();

	//database config
	db_pool_init(database_config);

	if(db_pool_connect() == false){
		fprintf(stderr, "[LOGIC] connect to mysql server failed\n");
		return 1;
	}

	fprintf(stderr, "[LOGIC] connect to mysql successful.\n");

	conn = pt_client_new();
	pt_client_init(uv_default_loop(), conn, logic_on_connect, logic_on_received, logic_on_disconnected);

	if(!pt_client_connect_pipe(conn, PRIVATE_CHANNEL)){
		fprintf(stderr,"[LOGIC] post connect to private channel failed");
		return 1;
	}

	fprintf(stderr, "[LOGIC] post connect to dispatch successful.\n");

	sigint_start();
	uv_signal_start(&sigterm,on_signal_term, SIGTERM);

	is_running = true;

	fprintf(stderr,"[LOGIC RUN]\n");
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	fprintf(stderr,"[LOGIC EXIT]...\n");

	return 0;
}
