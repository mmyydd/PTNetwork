#ifndef _DB_INTR_INCLUDED_H_
#define _DB_INTR_INCLUDED_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <exception>
#include <ptframework/common/common.h>
#include <ptframework/common/message.h>
#include <db_query.pb.h>
#include <mysql/mysql.h>

class db_intr_handle;
class db_intr;

typedef void (*fn_db_intr_handle_cb)(db_intr_handle *query);

//查询结果通知的抽象类
class db_query_notify
{
	friend class db_intr_handle;
	friend class db_intr;
private:
	db_intr_handle *m_handle;
	bool m_successed;

	void set_handle(db_intr_handle *handle){
		m_handle = handle;
	}
	void set_successed(bool s){
		m_successed = s;
	}
public:
	bool successed(){
		return m_successed;
	}

	db_intr_handle *get_handle(){
		return m_handle;
	}

	virtual void begin_exec(){
	}
	virtual void comlete_exec() = 0;
	//virtual void on_query_completed(db_intr_handle *db_query) = 0;
	//virtual void on_query_failed(db_intr_handle *query, int reason) = 0;
};

//描述数据库查询失败的错误
class db_intr_handle_exception: public std::exception
{
public:
	db_intr_handle_exception(bool is_mysql_err, 
			int error_code, std::string strerr);

	virtual ~db_intr_handle_exception() throw() {
	}

	virtual const char *what();
	bool is_mysql_error();
	int get_error_code();

private:
	std::string m_strerr;
	bool m_is_mysql_error;
	int m_error_code;
};

//描述每个fields的信息
struct db_intr_field
{
	std::string name;
	int type;
	int index;
};

//fields组
class db_intr_fields
{
public:
	db_intr_fields();
	virtual ~db_intr_fields();

	void add_fields(const std::string &name, int index, int type);
	bool find_fields(const std::string &name, db_intr_field &field);
	uint32_t get_fields_count();

	int index_field(const char *name);

	db_intr_field &get_field(int index){
		return *m_fields[index];
	}
private:
	std::vector<db_intr_field*> m_fields;
	std::map<std::string, db_intr_field*> m_search;
};

enum db_intr_value_type
{
	DB_INTR_VALUE_TYPE_NULL,
	DB_INTR_VALUE_TYPE_INT,
	DB_INTR_VALUE_TYPE_UINT,
	DB_INTR_VALUE_TYPE_INT64,
	DB_INTR_VALUE_TYPE_UINT64,
	DB_INTR_VALUE_TYPE_STRING,
	DB_INTR_VALUE_TYPE_FLOAT,
	DB_INTR_VALUE_TYPE_DOUBLE,
};

class db_intr_value
{
private:
	db_intr_value_type value_type;

	std::string m_value_string;
	int32_t m_value_int;
	uint32_t m_value_uint;
	int64_t m_value_int64;
	uint64_t m_value_uint64;
	float m_value_float;
	double m_value_double;

public:
	virtual ~db_intr_value()
	{
	}
	db_intr_value()
	{
		value_type = DB_INTR_VALUE_TYPE_NULL;
	}
	db_intr_value(std::string value)
	{
		value_type = DB_INTR_VALUE_TYPE_STRING;
		m_value_string = value;
	}

	db_intr_value(int32_t value)
	{
		value_type = DB_INTR_VALUE_TYPE_INT;
		m_value_int = value;
	}

	db_intr_value(uint32_t value)
	{
		value_type = DB_INTR_VALUE_TYPE_UINT;
		m_value_uint = value;
	}

	db_intr_value(int64_t value)
	{
		value_type = DB_INTR_VALUE_TYPE_INT64;
		m_value_int64 = value;
	}

	db_intr_value(uint64_t value)
	{
		value_type = DB_INTR_VALUE_TYPE_UINT64;
		m_value_uint64 = value;
	}

	db_intr_value(float value)
	{
		value_type = DB_INTR_VALUE_TYPE_FLOAT;
		m_value_float = value;
	}

	db_intr_value (double value)
	{
		value_type = DB_INTR_VALUE_TYPE_DOUBLE;
		m_value_double = value;
	}

	static db_intr_value initWithRow(db_intr_field &field, const db_row_value &row_value)
	{
		db_intr_value value;

		if(row_value.is_null())
		{
			return value;
		}

		const char *s = row_value.value().c_str();

		switch(field.type)
		{
			case MYSQL_TYPE_LONG:
				if(row_value.is_unsigned())
				{
					sscanf(s, "%u", &value.m_value_uint);
					value.value_type = DB_INTR_VALUE_TYPE_UINT;
				} else {
					sscanf(s, "%d", &value.m_value_int);
					value.value_type = DB_INTR_VALUE_TYPE_INT;
				}
				break;
			case MYSQL_TYPE_LONGLONG:
				if(row_value.is_unsigned())
				{
					sscanf(s, "%lu", &value.m_value_uint64);
					value.value_type = DB_INTR_VALUE_TYPE_UINT64;
				} else {
					sscanf(s, "%ld", &value.m_value_uint64);
					value.value_type = DB_INTR_VALUE_TYPE_INT64;
				}
				break;
			case MYSQL_TYPE_FLOAT:
				sscanf(s, "%f", &value.m_value_float);
				value.value_type = DB_INTR_VALUE_TYPE_FLOAT;
				break;
			case MYSQL_TYPE_DOUBLE:
				sscanf(s, "%lf", &value.m_value_double);
				value.value_type = DB_INTR_VALUE_TYPE_DOUBLE;
				break;
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_STRING:
			default:
				value.m_value_string = row_value.value();
				value.value_type = DB_INTR_VALUE_TYPE_STRING;
				break;
		}

		return value;
	}

	int32_t get_int32()
	{
		int32_t value = 0;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (int32_t) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (int32_t) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (int32_t) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (int32_t) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%d", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (int32_t) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (int32_t) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}

	uint32_t get_uint32()
	{
		uint32_t value = 0;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (uint32_t) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (uint32_t) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (uint32_t) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (uint32_t) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%u", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (uint32_t) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (uint32_t) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}


	int64_t get_int64()
	{
		int64_t value = 0;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (int64_t) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (int64_t) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (int64_t) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (int64_t) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%ld", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (int64_t) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (int64_t) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}


	uint64_t get_uint64()
	{
		uint64_t value = 0;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (uint64_t) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (uint64_t) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (uint64_t) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (uint64_t) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%lu", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (uint64_t) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (uint64_t) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}

	float get_float()
	{
		float value = 0.0f;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (float) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (float) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (float) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (float) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%f", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (float) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (float) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}

	double get_double()
	{
		double value = 0.0;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value = (double) m_value_int;
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value = (double) m_value_uint;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value = (double) m_value_int64;
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value = (double) m_value_uint64;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				sscanf(m_value_string.c_str(), "%lf", &value);
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value = (double) m_value_float;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value = (double) m_value_double;
				break;
			default:
				break;
		}

		return value;
	}

	std::string get_string()
	{
		char value[64];

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				sprintf(value, "%d", m_value_int);
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				sprintf(value, "%u", m_value_uint);
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				sprintf(value, "%ld", m_value_int64);
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				sprintf(value, "%lu", m_value_uint64);
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				return m_value_string;
			case DB_INTR_VALUE_TYPE_FLOAT:
				sprintf(value, "%f", m_value_float);
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				sprintf(value, "%lf", m_value_double);
				break;
			default:
				break;
		}

		return value;
	}

	bool store_query_param(db_query_param &param)
	{
		int param_type;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
			case DB_INTR_VALUE_TYPE_UINT:
				param_type = MYSQL_TYPE_LONG;
				break;
			case DB_INTR_VALUE_TYPE_INT64:
			case DB_INTR_VALUE_TYPE_UINT64:
				param_type = MYSQL_TYPE_LONGLONG;
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				param_type = MYSQL_TYPE_VAR_STRING;
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				param_type = MYSQL_TYPE_FLOAT;
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				param_type = MYSQL_TYPE_DOUBLE;
				break;
			default:
				param_type = MYSQL_TYPE_NULL;
				break;
		}

		param.set_param_type(param_type);
		param.set_values(build_value());
		
		return true;
	}
	
	std::string build_value()
	{
		std::string value;

		switch(value_type)
		{
			case DB_INTR_VALUE_TYPE_INT:
				value.resize(sizeof(int32_t));
				for(int i =0; i < sizeof(int32_t); i ++)
				{
					value[i] = (((char*)&m_value_int)[i]);
				}
				break;
			case DB_INTR_VALUE_TYPE_UINT:
				value.resize(sizeof(uint32_t));
				for(int i =0; i < sizeof(uint32_t); i ++)
				{
					value[i] = (((char*)&m_value_uint)[i]);
				}
				break;
			case DB_INTR_VALUE_TYPE_INT64:
				value.resize(sizeof(int64_t));
				for(int i =0; i < sizeof(int64_t); i ++)
				{
					value[i] = (((char*)&m_value_int64)[i]);
				}
				break;
			case DB_INTR_VALUE_TYPE_UINT64:
				value.resize(sizeof(uint64_t));
				for(int i =0; i < sizeof(uint64_t); i ++)
				{
					value[i] = (((char*)&m_value_uint64)[i]);
				}
				break;
			case DB_INTR_VALUE_TYPE_STRING:
				value = m_value_string;
				break;
			case DB_INTR_VALUE_TYPE_FLOAT:
				value.resize(sizeof(float));
				for(int i =0; i < sizeof(float); i ++)
				{
					value[i] = (((char*)&m_value_float)[i]);
				}
				break;
			case DB_INTR_VALUE_TYPE_DOUBLE:
				value.resize(sizeof(double));
				for(int i =0; i < sizeof(double); i ++)
				{
					value[i] = (((char*)&m_value_double)[i]);
				}
				break;
			default:
				break;
		}

		return value;
	}
};

//描述每个row的数据
class db_intr_column
{
protected:
	db_intr_value m_value;
public:
	db_intr_column(db_intr_field &field, const db_row_value &row_value)
	{
		m_is_null = row_value.is_null();
		m_is_unsigned = row_value.is_unsigned();
		m_value = db_intr_value::initWithRow(field, row_value);
	}

	int32_t get_int32()
	{
		return m_value.get_int32();
	}

	uint32_t get_uint32()
	{
		return m_value.get_uint32();
	}

	int64_t get_int64()
	{
		return m_value.get_int64();
	}

	uint64_t get_uint64()
	{
		return m_value.get_uint64();
	}
	
	float get_float()
	{
		return m_value.get_float();
	}

	double get_double()
	{
		return m_value.get_double();
	}
	
	std::string get_string()
	{
		return m_value.get_string();
	}

	bool is_null(){
		return m_is_null;
	}

	bool is_unsigned(){
		return m_is_unsigned;
	}

private:
	bool m_is_null;
	bool m_is_unsigned;
};


typedef std::vector<db_intr_column> db_intr_row;
typedef std::vector<db_intr_value> db_intr_params;

//record set 用于读取query result
class db_intr_record_set
{
public:
	//db_intr_handle_exception
	db_intr_record_set(db_intr_handle &query) throw(db_intr_handle_exception);
	virtual ~db_intr_record_set();

	bool move_first();
	bool is_eof();
	bool move_next();
	uint32_t get_record_count();
	uint32_t get_fields_count();


	bool is_null(const char *name);
	bool is_null(int index);
	bool get_field_value(const char *name, int32_t &value);
	bool get_field_value(int index, int32_t &value);

	bool get_field_value(const char *name, uint32_t &value);
	bool get_field_value(int index, uint32_t &value);

	bool get_field_value(const char *name, int64_t &value);
	bool get_field_value(int index, int64_t &value);

	bool get_field_value(const char *name, uint64_t &value);
	bool get_field_value(int index, uint64_t &value);

	bool get_field_value(const char *name, float &value);
	bool get_field_value(int index, float &value);

	bool get_field_value(const char *name, double &value);
	bool get_field_value(int index, double &value);

	bool get_field_value(const char *name, std::string &value);
	bool get_field_value(int index, std::string &value);

private:
	db_intr_handle &m_query;
	int m_index;
};


//异步数据库查询/执行对象
class db_intr_handle
{
	//友元类
	friend class db_intr;
	friend class db_intr_record_set;
public:
	virtual ~db_intr_handle();

	//查询操作是query还是execute
	bool is_exec() {
		return m_is_exec;
	}

	//获取查询时间
	uint64_t get_time(){
		return m_hrtime;
	}
	//获取查询的id
	uint64_t get_id();
private:
	//只能由db_intr类来创建这个
	db_intr_handle(std::string &conn, std::string &sql,
			db_intr_params &params, fn_db_intr_handle_cb cb = NULL,
			bool is_exec = false);
protected:
	uint64_t m_magic_id;
	//查询结果的回调函数
	fn_db_intr_handle_cb m_callback;

	//执行查询的时间
	uint64_t m_hrtime;

	//是否只执行
	bool m_is_exec;
private:
	//查询所使用的连接名称
	std::string m_conn;
	//查询所使用的sql
	std::string m_sql;

	//操作是否成功完成
	bool m_successed;

	//是否mysql数据库的错误
	bool is_mysql_error;

	//错误代码
	int32_t err_code;

	//错误原因
	std::string err_reason;

	//查询语句的参数
	db_intr_params m_params;

	//查询影响行数
	uint64_t affected_rows;

	//查询出数据的field信息
	db_intr_fields m_fields;

	//查询出的row信息
	std::vector<db_intr_row> m_rows;


	//构造一个db_query结构的封包给db_middleware服务器
	struct pt_buffer* build_query_buffer();

	//当db_middleware操作成功完成执行这个函数
	void process_completed(struct buffer_reader *reader);

	//当db_middleware解密数据失败或其他错误原因执行这个函数
	void process_failed(int reason);

	//设置开始执行的时间
	void set_time(uint64_t hrtime);

	//设置查询唯一ID
	void set_id(uint64_t id);

	//查询通知接口 抽象类
	db_query_notify *notify;
};

//db接口的抽象类
class db_intr_abs
{
public:
	//数据库接口服务连接成功通知
	virtual void on_connect_completed() = 0;
	//数据库接口连接失败通知
	virtual void on_connect_failed() = 0;
	//数据库接口连接断开通知
	virtual void on_lost_connection() = 0;
	//数据库接口执行一个查询结果完成通知，可不写函数
	//执行db_intr_handle中的callback函数
	virtual void on_query_completed(db_intr_handle *query) = 0;
};


//数据库中间件异步接口类
class db_intr : public db_intr_abs
{
public:
	//construct & destruct
	db_intr(bool is_pipe,std::string host, uint16_t port);
	virtual ~db_intr();

	//投递async connect到netchan
	void begin_connect();
	
	/*
	 * 函数: begin_query / begin_exec
	 * 参数一 使用的connection
	 * 参数二 sql语句
	 * 参数三 参数绑定所使用的key => value
	 * 参数四 回调函数 默认为NULL
	 * */
	void begin_query(std::string conn, std::string sql,
			db_intr_params &params,fn_db_intr_handle_cb cb = NULL);

	void begin_exec(std::string conn, std::string sql,
			db_intr_params &params,fn_db_intr_handle_cb cb = NULL);
	
	/*
	 * 函数: begin_query / begin_exec
	 * 参数一 使用的connection
	 * 参数二 sql语句
	 * 参数三 参数绑定所使用的key => value
	 * 参数四 通知接口
	 * */
	void begin_query(std::string conn, std::string sql,
			db_intr_params &params,db_query_notify *notify);

	void begin_exec(std::string conn, std::string sql,
			db_intr_params &params,db_query_notify *notify);

	virtual void on_connect_completed() = 0;
	virtual void on_connect_failed() = 0;
	virtual void on_lost_connection() = 0;
	virtual void on_query_completed(db_intr_handle *query);

	virtual void on_error(std::string errtext);
private:
	//开始一个查询
	//发送查询到数据库中间件或插入查询队列
	void begin_query(db_intr_handle *query);

	//处理网络数据
	static void on_notify_connection(struct pt_client *conn, enum pt_client_state state);
	static void on_notify_recevied_package(struct pt_client *conn, struct pt_buffer *buff);
	static void on_notify_disconnection(struct pt_client *conn);

	//处理网络数据包
	void process_package(struct buffer_reader *reader, struct net_header *hdr, uint64_t magic_id);
protected:
	//恢复queue中的所有查询
	void restore_queue_query();

	//服务器初始化函数
	virtual void init();
protected:
	//当前通信是否为pipe
	bool m_is_pipe;

	//服务器的地址/unix fd
	std::string m_host;

	//端口
	uint16_t m_port;
private:
	//网络连接客户端
	struct pt_client *m_conn;
	//保存查询数据的table 支持快速搜索
	struct pt_table *m_search_table;
	
	//标记查询执行的magic_id
	uint64_t m_serial;
	
	//是否已经成功和数据库中间服务建立链接
	bool m_is_connected;
	
	//查询队列 一般情况下不会出现这个
	//只有与后端服务器断开连接后才会出现这个问题
	std::queue<db_intr_handle *> m_query_queue;
};

#endif
