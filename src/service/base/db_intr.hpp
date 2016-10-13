#ifndef _DB_INTR_INCLUDED_H_
#define _DB_INTR_INCLUDED_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <exception>
#include <memory>
#include <ptframework/common/common.h>
#include <ptframework/common/message.h>
#include <db_query.pb.h>
#include <mysql/mysql.h>

class db_intr_handle;
class db_intr;
class db_intr_result;

typedef void (*fn_db_intr_handle_cb)(db_intr_handle *query, void *data);

enum db_failed_error_code
{
	DB_FAILED_START = 1,
	//数据解包失败
	DB_FAILED_UNPACKED_FAILED,
	//查询超时
	DB_FAILED_TIMEOUT
};

//查询结果通知的抽象类
class db_query_notify
{
	friend class db_intr_handle;
	friend class db_intr;

protected:
	db_query_notify(): m_handle(NULL), cb(NULL) , auto_release(false), arg(NULL) {}
	virtual ~db_query_notify(){}

private:
	void set_handle(db_intr_handle *handle){
		m_handle = handle;
	}
public:
	db_intr_handle *get_handle(){
		return m_handle;
	}
	
	virtual void on_completed()
	{
		if(cb)
		{
			cb(m_handle, arg);
		}
	}
private:
	void attach_cb(fn_db_intr_handle_cb _cb, void *_arg){
		cb = _cb;
		arg = _arg;
	}
private:
	db_intr_handle *m_handle;

	//函数通知
	fn_db_intr_handle_cb cb;

	void *arg;

	//这个结构是否根据db_intr_handle一起释放
	bool auto_release;
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

	db_intr_field &get_field(int index);

protected:
	std::vector<db_intr_field> m_fields;
	std::map<std::string, db_intr_field> m_search;
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
	db_intr_value();
	db_intr_value(std::string value);
	db_intr_value(int32_t value);
	db_intr_value(uint32_t value);
	db_intr_value(int64_t value);
	db_intr_value(uint64_t value);
	db_intr_value(float value);
	db_intr_value (double value);
	~db_intr_value();

	static db_intr_value initWithRow(db_intr_field &field, const db_stmt_value &value);

	int32_t get_int32();
	uint32_t get_uint32();

	int64_t get_int64();
	uint64_t get_uint64();

	float get_float();
	double get_double();

	std::string get_string();
	void store_stmt_value(db_stmt_value &value);
	
	std::string build_value();

	bool is_null();

	bool is_unsigned();
};

typedef std::vector<db_intr_value> db_intr_value_c;

//record set 用于读取query result
class db_intr_record_set
{
public:
	//db_intr_handle_exception
	db_intr_record_set(db_intr_handle *handle)  throw(db_intr_handle_exception);
	db_intr_record_set(db_intr_handle &handle) throw(db_intr_handle_exception);
	db_intr_record_set(db_intr_result &result);

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

	db_intr_result *get_result();

private:
	db_intr_result *_result;
	int m_index;
};

//单个结果集
class db_intr_result
{
	friend class db_intr_handle;
	friend class db_intr_record_set;
public:
	db_intr_result();
	~db_intr_result();

	uint32_t get_fields_count();
	uint32_t get_rows_count();
	uint64_t get_affected_rows();

	db_intr_fields &get_fields();

private:
	//查询影响行数
	uint64_t affected_rows;

	//查询出数据的field信息
	db_intr_fields m_fields;

	//查询出的row信息
	std::vector<db_intr_value_c> m_rows;
};

typedef std::vector<db_intr_result> db_intr_result_c;

//异步数据库查询/执行对象
class db_intr_handle
{
	//友元类
	friend class db_intr;
	friend class db_intr_record_set;
public:
	~db_intr_handle();

	//查询操作是query还是execute
	bool is_execute();

	//获取查询时间
	uint64_t get_time();

	//获取查询的id
	uint64_t get_id();

	//是否存在错误
	bool has_error();

	//是不是mysql错误
	bool has_mysql_error();

	//获取错误代码
	uint32_t get_error_code();

	//获取错误字符串
	std::string get_strerror();

	//获取结果集数量
	uint32_t get_results_count();

	//获取结果集
	const db_intr_result_c &get_results();

private:
	//只能由db_intr类来创建这个
	db_intr_handle(std::string &conn, std::string &sql,
			db_intr_value_c &params,
			bool is_exec,
			db_query_notify *_notify);

	//构造一个db_query结构的封包给db_middleware服务器
	struct pt_buffer* build_query_buffer();

	//当db_middleware操作成功完成执行这个函数
	void on_completed(struct buffer_reader *reader);

	//当db_middleware解密数据失败或其他错误原因执行这个函数
	void on_failed(int reason);

	//设置开始执行的时间
	void set_time(uint64_t hrtime);

	//设置查询唯一ID
	void set_id(uint64_t id);

protected:
	uint64_t m_magic_id;

	//执行查询的时间
	uint64_t m_hrtime;

	//是否只执行
	bool m_is_exec;

protected:
	//查询所使用的连接名称
	std::string m_conn;

	//查询所使用的sql
	std::string m_sql;

	//结果集
	db_intr_result_c m_results;

	//操作是否成功完成
	bool m_successed;

	//是否mysql数据库的错误
	bool m_is_mysql_error;

	//错误代码
	int32_t m_error_code;

	//错误原因
	std::string m_strerror;

	//查询语句的参数
	db_intr_value_c m_params;

	//查询通知接口 抽象类
	db_query_notify *notify;
};

//db接口的抽象类
class db_intr_abs
{
public:
	//数据库接口服务连接成功通知
	virtual void on_connected() = 0;
	//数据库接口连接断开通知
	virtual void on_disconnected() = 0;

	//数据库接口连接失败通知
	virtual void on_connect_failed() = 0;

	//数据库接口执行一个查询结果完成通知，可不写函数
	//执行db_intr_handle中的callback函数
	virtual void on_query(db_intr_handle *query) = 0;
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
	 * 参数四 回调函数 可为NULL
	 * */
	void begin_query(std::string conn, std::string sql,
			db_intr_value_c &params,fn_db_intr_handle_cb cb, void *arg);

	void begin_exec(std::string conn, std::string sql,
			db_intr_value_c &params,fn_db_intr_handle_cb cb, void *arg);
	
	/*
	 * 函数: begin_query / begin_exec
	 * 参数一 使用的connection
	 * 参数二 sql语句
	 * 参数三 参数绑定所使用的key => value
	 * 参数四 通知接口
	 * */
	void begin_query(std::string conn, std::string sql,
			db_intr_value_c &params,db_query_notify	 *notify);

	void begin_exec(std::string conn, std::string sql,
			db_intr_value_c &params,db_query_notify *notify);

	//查询结果通知
	virtual void on_query(db_intr_handle *query);

	//获取队列中的查询数量
	uint32_t get_busy_count();
	//获取队列中的数量
	uint32_t get_queue_count();

	//恢复queue中的所有查询
	void restore_queue_query();

	//作废所有查询
	void failed_queue_query();
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
