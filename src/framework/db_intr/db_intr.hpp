#ifndef _DB_INTR_INCLUDED_H_
#define _DB_INTR_INCLUDED_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <exception>
#include <ptnetwork.h>
#include <db_query.pb.h>
#include <mysql.h>

class db_intr_handle;
class db_intr;

typedef void (*fn_db_intr_handle_cb)(db_intr_handle *query);

//查询结果通知的抽象类
class db_query_notify
{
public:
	virtual void on_query_completed(db_intr_handle *db_query) = 0;
	virtual void on_query_failed(db_intr_handle *query, int reason) = 0;
};

//描述数据库查询失败的错误
class db_intr_handle_exception: public std::exception
{
public:
	db_intr_handle_exception(bool is_mysql_err, 
			int error_code, std::string strerr);

	virtual const char *what();
	bool is_mysql_error();
	int get_error_code();

private:
	std::string m_strerr;
	bool m_is_mysql_error;
	int m_error_code;
};

//参数绑定 param
class db_intr_key_value
{
public:
	db_intr_key_value(std::string key, std::string value);
	db_intr_key_value(std::string key, int value);
	db_intr_key_value(std::string key, int64_t value);
	db_intr_key_value(std::string key, float value);
	db_intr_key_value(std::string key, double value);

	int get_int();
	int64_t get_int64();	
	float get_float();
	double get_double();
	std::string get_string();

	std::string get_key()
	{
		return m_key;
	}
	//参数类型
	int get_type();

	//填充数据到protobuf -> db_query_param
	bool store_query_param(db_query_param &param);
private:
	int m_type;

	//type_xxx_value
	std::string m_key;
	std::string m_str_value;
	int m_int_value;
	int64_t m_int64_value;
	float m_float_value;
	double m_double_value;
};


//描述每个row的数据
class db_intr_column
{
public:
	db_intr_column(const std::string &value);

	int32_t get_int();
	uint32_t get_uint();

	long get_long();
	unsigned long get_ulong();

	int64_t get_int64();
	uint64_t get_uint64();

	float get_float();
	double get_double();

	std::string get_string();
private:
	std::string m_value;
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
private:
	std::vector<db_intr_field*> m_fields;
	std::map<std::string, db_intr_field*> m_search;
};

typedef std::vector<db_intr_column> db_intr_row;
typedef std::vector<db_intr_key_value> db_intr_params;

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
	uint32_t get_field_count();

	bool get_field_value(const char *name, int &value);
	bool get_field_value(int index, int &value);

	bool get_field_value(const char *name, long &value);
	bool get_field_value(int index, long &value);
	
	bool get_field_value(const char *name, unsigned long &value);
	bool get_field_value(int index, unsigned long &value);

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
	db_intr(std::string host, uint16_t port, bool is_pipe);
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
