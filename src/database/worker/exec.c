#include <common/common.h>
#include <mysql.h>
#include "queue.h"
#include "db_pool.h"

#define DB_ERROR_INVALID_CONN_NAME 10000
#define DB_ERROR_INVALID_CONN_NAME_STRING "无效的数据库name"


#define DB_ERROR_NUM_PARAM_ERR 10001
#define DB_ERROR_NUM_PARAM_ERR_STRING "查询语句参数错误"


#define DB_ERROR_PARAM_BIND_FAILED 10002
#define DB_ERROR_PARAM_BIND_FAILED_STRING "参数绑定失败"

#undef malloc
#undef free
#undef realloc
#undef calloc

void db_query_result_set_error(DbQueryResult *result, qboolean is_mysql_err,
		int error_code,const char *errmsg)
{

	if(is_mysql_err == true)
	{
		if(error_code >= 2000 && error_code < 3000){
			fprintf(stderr, "[LOGIC] mysql error:%s\n", errmsg);
			exit(1);
		}
	}
	result->success = false;
	result->error = gcmalloc(sizeof(DbError));
	db_error__init(result->error);
	result->error->is_mysql_error = false;
	result->error->reason = (char*)errmsg;
	result->error->error_code = error_code;
}


qboolean db_command_bind_param(struct db_conn *conn, DbQuery *pQuery, DbQueryResult *result, MYSQL_STMT *stmt)
{
	unsigned long param_cnt;
	MYSQL_BIND *bind_params;
	int i;
	my_bool bind_result;

	//检查mysql参数个数
	param_cnt = mysql_stmt_param_count(stmt);
	
	if(param_cnt == 0) return true;	 //判断参数如果是0个则直接返回

	if(param_cnt != pQuery->n_params){
		db_query_result_set_error(result,
				false, DB_ERROR_NUM_PARAM_ERR ,
				DB_ERROR_NUM_PARAM_ERR_STRING);

		return false;
	}

	bind_params = gcmalloc(sizeof(MYSQL_BIND) * pQuery->n_params);

	//参数绑定 防止sql注入
	for(i = 0; i < (int)pQuery->n_params;i++)
	{
		bzero(&bind_params[i], sizeof(MYSQL_BIND));
		bind_params[i].buffer = pQuery->params[i]->values.data;
		bind_params[i].buffer_length = pQuery->params[i]->values.len;
		bind_params[i].buffer_type = pQuery->params[i]->param_type;
	}

	bind_result = mysql_stmt_bind_param(stmt, bind_params);

	if(bind_result == false)
	{
		db_query_result_set_error(result, false, DB_ERROR_PARAM_BIND_FAILED,
				DB_ERROR_PARAM_BIND_FAILED_STRING);
		return false;
	}

	return true;
}

void db_command_run_exec(struct db_conn *conn, DbQuery *pQuery, DbQueryResult *result)
{
	int r,i;
	MYSQL_STMT *stmt;
	struct pt_queue result_queue;

	pt_queue_init(&result_queue);
	stmt = mysql_stmt_init(&conn->conn);

	//预处理sql
	r = mysql_stmt_prepare(stmt, pQuery->query, strlen(pQuery->query));

	if(r)
	{
		db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return;
	}

	if(db_command_bind_param(conn, pQuery, result, stmt) == false)
	{
		mysql_stmt_close(stmt);
		return;
	}

	r = mysql_stmt_execute(stmt);

	if(r)
	{
		db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return;
	}

	result->success = true;
	result->affected_rows = mysql_stmt_affected_rows(stmt);

	mysql_stmt_close(stmt);
}

void db_command_run_query(struct db_conn *conn, DbQuery *pQuery, DbQueryResult *result)
{
	int r;
	uint32_t n_rows, n_fields, i, j;
	
	DbField *field;
	MYSQL_RES *res;
	MYSQL_STMT *stmt;
	struct pt_queue result_queue;
	
	//results
	MYSQL_BIND *bindResult;
	unsigned long *resultLength;
	pt_queue_init(&result_queue);

	stmt = mysql_stmt_init(&conn->conn);

	//预处理sql
	r = mysql_stmt_prepare(stmt, pQuery->query, strlen(pQuery->query));

	if(r)
	{
		db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return;
	}

	if(db_command_bind_param(conn, pQuery, result, stmt) == false)
	{
		mysql_stmt_close(stmt);
		return;
	}

	r = mysql_stmt_execute(stmt);

	if(r)
	{
		db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return;
	}
	
	res = mysql_stmt_result_metadata(stmt);
	r = mysql_stmt_store_result(stmt);

	if(r)
	{
		db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return;
	}

	n_fields = mysql_stmt_field_count(stmt);
	n_rows = (uint32_t)mysql_stmt_num_rows(stmt);
	
	//==============================================================================================
	//填充fields
	//
	
	result->fields = gcmalloc(sizeof(DbFields));
	
	db_fields__init(result->fields);
	result->fields->n_fields = n_fields;
	result->fields->fields = gcmalloc(sizeof(DbField*) * n_fields);

	for( i = 0; i < n_fields; i++)
	{
		field = gcmalloc(sizeof(DbField));
		db_field__init(field);

		field->name = gcmalloc_strdup(NULL, res->fields[i].name);
		field->type = res->fields[i].type;

		result->fields->fields[i] = field;
	}

	//==============================================================================================
	//填充rows
	//
	bindResult = gcmalloc(sizeof(MYSQL_BIND) * n_fields);
	resultLength = gcmalloc(sizeof(unsigned long) * n_fields);
#define RESET_RESULT() for( j = 0; j < n_fields; j++) \
	{ \
		bzero(&bindResult[j], sizeof(MYSQL_BIND)); \
		resultLength[j] = 0; \
		bindResult[j].length = &resultLength[j]; \
	}

	RESET_RESULT();

	mysql_stmt_bind_result(stmt, bindResult);

	result->rows = gcmalloc(sizeof(DbRows));
	db_rows__init(result->rows);

	result->rows->n_rows = n_rows;
	result->rows->rows = gcmalloc(sizeof(DbRow*) * n_rows);

	j = 0;
	while(true)
	{
		r = mysql_stmt_fetch(stmt);
		if(r == 1) {	//处理错误
			db_query_result_set_error(result,
				true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
			mysql_stmt_close(stmt);
			return;
		}

		if(r == MYSQL_NO_DATA) { //没有其他数据
			break;
		}

		result->rows->rows[j] = gcmalloc(sizeof(DbRow));
		db_row__init(result->rows->rows[j]);

		result->rows->rows[j]->n_column = n_fields;
		result->rows->rows[j]->column = gcmalloc(sizeof(DbRowValue*) * n_fields);

		//横向填充column
		for( i = 0; i < n_fields; i++)
		{
			bindResult[i].buffer = gcmalloc(resultLength[i]);
			bindResult[i].buffer_length = resultLength[i];
			bindResult[i].is_null = gcmalloc(sizeof(my_bool));

			*bindResult[i].is_null = false;

			result->rows->rows[j]->column[i] = gcmalloc(sizeof(DbRowValue));
			db_row_value__init(result->rows->rows[j]->column[i]);

			mysql_stmt_fetch_column(stmt, &bindResult[i], i, 0);

			result->rows->rows[j]->column[i]->value.len = bindResult[i].buffer_length;
			result->rows->rows[j]->column[i]->value.data = (uint8_t*)bindResult[i].buffer;
			result->rows->rows[j]->column[i]->is_null = *bindResult[i].is_null;
			result->rows->rows[j]->column[i]->is_unsigned = bindResult[i].is_unsigned;
		}

		j++;
	}

	result->success = true;

	mysql_stmt_close(stmt);
}

DbQueryResult *db_command_exec(DbQuery *pQuery, qboolean is_exec)
{
	struct db_conn *conn;
	DbQueryResult *result = gcmalloc(sizeof(DbQueryResult));
	db_query_result__init(result);

	conn = db_pool_getconn(pQuery->conn);

	//数据库建立连接失败
	if(conn == NULL){
		db_query_result_set_error(result,
				false, DB_ERROR_INVALID_CONN_NAME,DB_ERROR_INVALID_CONN_NAME_STRING);
		return result;
	}

	if(is_exec)
	{
		db_command_run_exec(conn, pQuery, result);
	}
	else
	{
		db_command_run_query(conn, pQuery, result);
	}

	return result;
}
