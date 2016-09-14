#include <ptframework/common/common.h>
#include <ptframework/common/message.h>
#include <mysql.h>
#include "query.h"

struct pt_queue db_queue;
struct st_worker_proc *workers;
uv_timer_t worker_exec_timer;


static void DUMP_MEM(void *p, int len)
{
	int i;
	unsigned char *s = p;
	for( i = 0; i < len; i ++)
	{
		printf("%02X ",s[i]);
	}
	printf("\n");
}
static void db_worker_set_execute(struct st_worker_proc *worker, struct db_query *db_query);
static void db_worker_retry(struct db_query *db_query);
static void db_worker_execute(struct db_query *query);
static void db_worker_dispatch_queue();

static void db_query_free(struct db_query *query)
{
	pt_buffer_free(query->query_buff);
	MEM_FREE(query);
}

//worker 的添加与删除
void db_worker_add(struct pt_sclient *user)
{
	struct st_worker_proc *worker = MEM_MALLOC(sizeof(struct st_worker_proc));
	
	bzero(worker, sizeof(struct st_worker_proc));
	worker->conn = user;
	user->data = worker;

	worker->next = workers;
	workers = worker;

	db_worker_dispatch_queue();
}

static void private_db_worker_remove(struct st_worker_proc *worker)
{
	struct st_worker_proc *prev,*curr;

	prev = NULL;
	curr = workers;

	while(curr)
	{
		if(curr == worker)
		{
			if(prev == NULL) {
				workers = worker->next;
				break;
			} else {
				prev->next = worker->next;
				break;
			}
		}
		prev = curr;
		curr = curr->next;
	}
	
	MEM_FREE(worker);
}

void db_worker_remove(struct pt_sclient *conn)
{
	struct db_query *query = NULL;
	struct st_worker_proc *worker = conn->data;

	if(worker->is_busy)  //如果执行没有完成
	{
		query = worker->query;

		worker->is_busy = false;
		worker->query = NULL;
	}

	private_db_worker_remove(worker);

	if(query != NULL)
	{
		pt_queue_push(&db_queue, &query->queue);
	}
	db_worker_dispatch_queue();
}


//====================================================================================================
//dispatch query command
//====================================================================================================
static struct st_worker_proc *db_worker_get_idle()
{
	struct st_worker_proc *worker = workers;

	while(worker)
	{
		if(!worker->is_busy) return worker;
		worker = worker->next;
	}

	return NULL;
}

static qboolean build_and_post_exec_package(struct st_worker_proc *worker)
{
	struct net_header hdr = pt_create_nethdr(ID_EXECUTE);
	struct pt_buffer *buff = pt_create_package(hdr, worker->query->query_data,
			worker->query->query_data_len);

	return pt_server_send(worker->conn, buff);
}

static qboolean build_and_post_query_package(struct st_worker_proc *worker)
{
	struct net_header hdr = pt_create_nethdr(ID_QUERY);
	struct pt_buffer *buff = pt_create_package(hdr, worker->query->query_data,
			worker->query->query_data_len);
	
	return pt_server_send(worker->conn, buff);
}

static void db_worker_set_execute(struct st_worker_proc *worker, struct db_query *db_query)
{
	worker->is_busy = true;
	worker->query = db_query;
	worker->query->execute_time = time(NULL);
	
	//fprintf(stderr, "run sql :%llu\n", db_query->magic_id);

	if(db_query->is_execute)
	{
		build_and_post_exec_package(worker);
	}
	else
	{
		build_and_post_query_package(worker);
	}
}

//尝试sql执行分配给工作进程
static qboolean db_worker_try_execute(struct db_query *query)
{
	struct st_worker_proc *idle_worker = db_worker_get_idle();
	if(idle_worker)
	{
		db_worker_set_execute(idle_worker, query);
		return true;
	}
	return false;
}

static void db_worker_execute(struct db_query *query)
{
	//如果找到了空闲的worker 那么直接派遣给空闲的worker
	if(db_worker_try_execute(query)) {
		return;
	}

	//如果没有空闲的worker则插入队列 等待worker唤醒
	pt_queue_push(&db_queue, &query->queue);
}
void db_worker_dispatch(struct pt_sclient *conn,
		struct pt_buffer *buffer, 
		uint64_t magic_id,
		struct net_header hdr,
		struct buffer_reader *reader)
{
	struct db_query *db_query;
	pt_buffer_ref_increment(buffer);

	db_query = MEM_MALLOC(sizeof(struct db_query));
	db_query->query_buff = buffer;
	db_query->query_data = buffer_reader_cur_pos(reader);
	db_query->query_data_len = buffer_reader_over_size(reader);
	db_query->user_id = conn->id;
	db_query->magic_id = magic_id;
	db_query->failed_count = 0;
	db_query->is_execute = hdr.id == ID_EXECUTE;
	db_query->server = conn->server;

	db_worker_execute(db_query);
}

//====================================================================================================
// 查询完成后返回数据的部分
//====================================================================================================
static void db_worker_continue(struct st_worker_proc *worker)
{
	struct db_query *db_query;
	if(pt_queue_is_empty(&db_queue)) return;
	db_query = (struct db_query *)pt_queue_pop_front(&db_queue);

	db_worker_set_execute(worker, db_query);
}

static void db_worker_exec_completed(struct st_worker_proc *worker, unsigned char *data, uint32_t length)
{
	struct db_query *query;
	struct net_header hdr;
	struct net_header *net_hdr;
	struct pt_buffer *result;
	struct pt_sclient *user;

	query = worker->query;
	user = pt_server_find_sclient(query->server, query->user_id);

	if(user)
	{
		if(query->is_execute) {
			hdr = pt_create_nethdr(ID_EXECUTE_OK);
		} else {
			hdr = pt_create_nethdr(ID_QUERY_OK);
		}

		//fprintf(stderr, "reply query result:%llu\n", 
		//		query->magic_id);

		result = pt_buffer_new(sizeof(struct net_header) + 
				length + sizeof(uint64_t));

		pt_buffer_write(result, &hdr, sizeof(hdr));
		pt_buffer_write(result, &query->magic_id, sizeof(query->magic_id));
		pt_buffer_write(result, data, length);
		net_hdr = (struct net_header *)result->buff;
		net_hdr->length = result->length;

		pt_server_send(user, result);
	}

	db_query_free(query);

	//set stop
	worker->query = NULL;
	worker->is_busy = false;
}


void db_worker_dispatch_ok(struct pt_sclient *logic,struct buffer_reader *reader)
{
	struct st_worker_proc *worker = logic->data;
	unsigned char *data = buffer_reader_cur_pos(reader);
	uint32_t length = buffer_reader_over_size(reader);

	db_worker_exec_completed(worker, data, length);
	db_worker_continue(worker);
}


//====================================================================================================
// 更新所有worker状态 保持活跃
//====================================================================================================
static void db_worker_dispatch_queue()
{
	struct db_query *db_query;
	struct st_worker_proc *worker = db_worker_get_idle();

	while(!pt_queue_is_empty(&db_queue) && worker)
	{
		db_query = (struct db_query*)pt_queue_pop_front(&db_queue);
		db_worker_set_execute(worker,db_query);
		worker = db_worker_get_idle();
	}
}

static void db_worker_retry(struct db_query *db_query)
{
	db_query->failed_count++;
	struct st_worker_proc *worker;

	if(db_query->failed_count < 5)
	{
		db_worker_execute(db_query);
		return;
	}

	fprintf(stderr, "db_query failed:%llu\n", db_query->magic_id);

	db_query_free(db_query);
}

static void db_worker_keepalive(uv_timer_t *timer)
{
	struct st_worker_proc *worker = workers;
	struct st_worker_proc *next;
	time_t current_time = time(NULL);

	while(worker)
	{
		next = worker->next;
		if(worker->is_busy)
		{
			if(current_time - worker->query->execute_time >= QUERY_TIMEOUT)
			{
				db_worker_retry(worker->query);

				worker->is_busy = false;
				worker->query = NULL;

				pt_server_disconnect_conn(worker->conn);
			}
		}
		worker = next;
	}

	db_worker_dispatch_queue();
}

//====================================================================================================
// 如果查询失败则执行本函数
//====================================================================================================
void db_worker_unpacked_failed(struct pt_sclient *conn)
{
	struct st_worker_proc *worker = conn->data;
	struct pt_buffer *buff;
	struct net_header hdr, *buff_hdr;
	struct pt_sclient *user;

	user = pt_server_find_sclient(worker->query->server, worker->query->user_id);

	if(user)
	{
		hdr = pt_create_nethdr(ID_UNPACKED_FAILED);	
		buff = pt_create_package(hdr, &worker->query->magic_id, sizeof(worker->query->magic_id));
		pt_server_send(user, buff);
	}

	db_query_free(worker->query);

	worker->query = NULL;
	worker->is_busy = false;
}

//====================================================================================================
// 注册 中断 结束 操作定义
//====================================================================================================
void query_queue_pre_operation()
{
	pt_queue_init(&db_queue);
	uv_timer_init(uv_default_loop(), &worker_exec_timer);
	uv_timer_start(&worker_exec_timer, db_worker_keepalive, 1000, 1000);
}

void query_queue_close_cb(uv_handle_t *handle)
{

}

void query_queue_break_operation()
{
	uv_timer_stop(&worker_exec_timer);
	uv_close((uv_handle_t*)&worker_exec_timer, query_queue_close_cb);
}

void query_queue_post_operation()
{

}
