#ifndef _QUERY_INCLUDED_H_
#define _QUERY_INCLUDED_H_

#include <queue.h>
#include <db_query.pb-c.h>

#define QUERY_TIMEOUT 5

struct db_query
{
	struct pt_queue queue;
	uint64_t user_id;

	// true = execute  false = query
	qboolean is_execute;
	time_t execute_time;
	int failed_count;
	
	uint64_t magic_id;

	struct pt_buffer *query_buff;

	//指针引用自query_buff
	unsigned char *query_data;
	uint32_t query_data_len;

	//listen servers
	struct pt_server *server;
};


struct st_worker_proc
{
	struct st_worker_proc *next;
	//网络连接
	struct pt_sclient *conn;

	struct db_query *query;

	//当前逻辑是否正忙
	qboolean is_busy;
};

void query_queue_pre_operation();
void query_queue_break_operation();
void query_queue_post_operation();



void db_worker_add(struct pt_sclient *user);
void db_worker_remove(struct pt_sclient *conn);

//worker dispatch
void db_worker_dispatch(struct pt_sclient *conn,
		struct pt_buffer *buffer, 
		uint64_t magic_id,
		struct net_header hdr,
		struct buffer_reader *reader);

//查询结果
void db_worker_dispatch_ok(struct pt_sclient *logic,
		struct buffer_reader *reader);

void db_worker_unpacked_failed(struct pt_sclient *conn);
#endif
