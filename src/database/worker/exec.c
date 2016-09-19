#include <ptframework/common/common.h>
#include <ptframework/queue.h>
#include <mysql.h>
#include "db_pool.h"

#define DB_ERROR_INVALID_CONN_NAME 10000
#define DB_ERROR_INVALID_CONN_NAME_STRING "无效的数据库name"

#define DB_ERROR_NUM_PARAM_ERR 10001
#define DB_ERROR_NUM_PARAM_ERR_STRING "查询语句参数错误"

#define DB_ERROR_PARAM_BIND_FAILED 10002
#define DB_ERROR_PARAM_BIND_FAILED_STRING "参数绑定失败"

void db_query_result_set_error(DbStmtQueryResult *result, qboolean is_mysql_err,
		int error_code,const char *errmsg)
{

	if(is_mysql_err == true)
	{
		switch(error_code)
		{
			case 2006:		//CR_SERVER_GONE_ERROR
			case 2013:		//CR_SERVER_LOST
				fprintf(stderr, "[LOGIC][FATAL ERROR] mysql error:%s\n", errmsg);
				fflush(stderr);
				exit(1);
				break;
			default:
				break;
		}
	}
	
	result->success = false;
	result->error = gcmalloc(sizeof(DbError));
	db_error__init(result->error);
	result->error->is_mysql_error = is_mysql_err;
	result->error->reason = gcstrdup((char*)errmsg);
	result->error->error_code = error_code;
}

DbStmtQueryResult *db_command_run(DbStmtQuery *stmt_query, qboolean is_execute)
{
	struct db_conn *my;
	int r,i;
	unsigned long num_params;
	MYSQL_RES *meta_data;
	MYSQL_STMT *stmt;
	MYSQL_BIND *bind_params, *bind_result;
	DbResult *single_result;
	DbRows *rows;
	uint32_t num_rows, num_fields, j, k, tmp_count;
	unsigned long *bind_length;
	void *tmp;
	DbStmtQueryResult *stmt_query_result = gcmalloc(sizeof(DbStmtQueryResult));

	db_stmt_query_result__init(stmt_query_result);

	//从连接池中获取数据库连接
	my = db_pool_getconn(stmt_query->conn);

	stmt_query_result->success = true;

	//如果连接池没有这个数据库的连接
	if(my == NULL)
	{
		db_query_result_set_error(stmt_query_result,false, DB_ERROR_INVALID_CONN_NAME,DB_ERROR_INVALID_CONN_NAME_STRING);
		return stmt_query_result;
	}

	stmt = mysql_stmt_init(&my->conn);

#define CHECK_ERROR() if(r) \
	{ \
		db_query_result_set_error(stmt_query_result, true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));	\
		goto ExecuteBreak;  \
	}

	//预处理SQL语句
	r =  mysql_stmt_prepare(stmt, stmt_query->query, strlen(stmt_query->query));
	CHECK_ERROR();

	//====>绑定参数到sql语句
	num_params = mysql_stmt_param_count(stmt);
	if(num_params > 0)
	{
		if(num_params != stmt_query->n_params)
		{
			db_query_result_set_error(stmt_query_result, false, DB_ERROR_NUM_PARAM_ERR ,DB_ERROR_NUM_PARAM_ERR_STRING);
			goto ExecuteBreak;
		}
		bind_params = gcmalloc(sizeof(MYSQL_BIND) * num_params);
		bzero(bind_params, sizeof(MYSQL_BIND) * num_params);

		for(i = 0; i < (int)stmt_query->n_params;i++)
		{
			bind_params[i].buffer = stmt_query->params[i]->data.data;
			bind_params[i].buffer_length = stmt_query->params[i]->data.len;
			bind_params[i].buffer_type = stmt_query->params[i]->type;
			bind_params[i].is_null = gcmalloc(sizeof(my_bool));
			*bind_params[i].is_null = stmt_query->params[i]->is_null;
			bind_params[i].is_unsigned = stmt_query->params[i]->is_unsigned;
		}

		r = mysql_stmt_bind_param(stmt, bind_params);
		CHECK_ERROR();
	}
	//====<完成绑定

	//执行SQL
	r = mysql_stmt_execute(stmt);
	CHECK_ERROR();

	//开始处理操作结果集
	do
	{
		r = mysql_stmt_store_result(stmt);
		CHECK_ERROR();

		meta_data = mysql_stmt_result_metadata(stmt);

		num_fields = mysql_stmt_field_count(stmt);
		num_rows = mysql_stmt_num_rows(stmt);
	

		//如果是空的fields 继续执行next_result
		if(num_fields == 0)
		{
			goto RunNext;
		}

		single_result = gcmalloc(sizeof(DbResult));
		db_result__init(single_result);

		single_result->affected_rows = mysql_stmt_affected_rows(stmt);
		
		/*
			Store Field
		*/
		
		single_result->n_fields = num_fields;
		single_result->fields = gcmalloc(num_fields * sizeof(DbStmtValue*));

		for(j = 0; j < num_fields; j++)
		{
			single_result->fields[j] = gcmalloc(sizeof(DbStmtValue));
			db_stmt_value__init(single_result->fields[j]);

			single_result->fields[j]->name = gcstrdup(meta_data->fields[j].name);
			single_result->fields[j]->type = meta_data->fields[j].type;
		}


		
		if(num_fields > 0 && num_rows > 0)
		{
			//=====>绑定参数返回结果

			bind_result = gcmalloc(sizeof(MYSQL_BIND) * num_fields);
			bind_length = gcmalloc(sizeof(unsigned long) * num_fields);
			
			//关联bind_length和bind_result
			//bind_length的填充取决于mysql_stmt_fetch
			for(j = 0; j < num_fields; j++)	
			{
				bind_length[j] = 0;
				bzero(&bind_result[j], sizeof(MYSQL_BIND));
				bind_result[j].length = &bind_length[j];
			}

			r = mysql_stmt_bind_result(stmt, bind_result);
			CHECK_ERROR();

			//=====<绑定参数结束


			//创建rows
			rows = gcmalloc(sizeof(DbRows));
			db_rows__init(rows);

			rows->n_rows = num_rows;
			rows->rows = gcmalloc(sizeof(DbRow*) * num_rows);

			j = 0;
			
			//填充rows的column
			while(true)
			{
				r = mysql_stmt_fetch(stmt);	//fetch一行

				//处理错误
				if(r == 1)
				{
					db_query_result_set_error(stmt_query_result,true, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
					goto ExecuteBreak;
				}
				
				//没有其他数据
				if(r == MYSQL_NO_DATA)
				{
					break;
				}

				rows->rows[j] = gcmalloc(sizeof(DbRow));
				db_row__init(rows->rows[j]);

				rows->rows[j]->n_column = num_fields;
				rows->rows[j]->column = gcmalloc(sizeof(DbStmtValue*) * num_fields);
				
				//填充column
				for( k = 0; k < num_fields; k++)
				{
					rows->rows[j]->column[k] = gcmalloc(sizeof(DbStmtValue));
					db_stmt_value__init(rows->rows[j]->column[k]);

					bind_result[k].buffer = gcmalloc(bind_length[k]);
					bind_result[k].buffer_length = bind_length[k];
					bind_result[k].is_null = gcmalloc(sizeof(my_bool));
					bind_result[k].is_null[0] = false;

					r = mysql_stmt_fetch_column(stmt, &bind_result[k], k, 0);
					CHECK_ERROR();

					rows->rows[j]->column[k]->has_data = true;
					rows->rows[j]->column[k]->has_is_null = true;
					rows->rows[j]->column[k]->has_is_unsigned = true;

					rows->rows[j]->column[k]->data.len = bind_result[k].buffer_length;
					rows->rows[j]->column[k]->data.data = (unsigned char*)bind_result[k].buffer;
					rows->rows[j]->column[k]->is_null = *bind_result[k].is_null;
					rows->rows[j]->column[k]->is_unsigned = bind_result[k].is_unsigned;
				}

				j++;
			}

			single_result->rows = rows;

		}

		//添加结果集到results
		if(stmt_query_result->n_results == 0)
		{
			stmt_query_result->n_results = 1;
			stmt_query_result->results = gcmalloc(sizeof(DbResult *));
			stmt_query_result->results[0] = single_result;
		}
		else
		{
			tmp_count = stmt_query_result->n_results;
			stmt_query_result->n_results++;

			tmp = gcmalloc(sizeof(DbResult *) * stmt_query_result->n_results);
			memcpy(tmp, stmt_query_result->results, sizeof(DbResult *) * tmp_count);
			stmt_query_result->results = tmp;
			stmt_query_result->results[tmp_count] = single_result;
		}

RunNext:
		r = mysql_stmt_next_result(stmt);
		fflush(stderr);
		if(r != 0)
		{
			if( r == -1) break;
			CHECK_ERROR();
		}
	}while(true);

ExecuteBreak:

	mysql_stmt_free_result(stmt);
	mysql_stmt_close(stmt);

	return stmt_query_result;
}
