#include "db_intr.hpp"
#include "../database/common/message.h"

db_intr_handle_exception::db_intr_handle_exception(bool is_mysql_err,
		int error_code, std::string strerr) :
	m_is_mysql_error(is_mysql_err),
	m_error_code(error_code),
	m_strerr(strerr)
{

}

const char *db_intr_handle_exception::what()
{
	return m_strerr.c_str();
}

bool db_intr_handle_exception::is_mysql_error()
{
	return m_is_mysql_error;
}

int db_intr_handle_exception::get_error_code()
{
	return m_error_code;
}

//=====================================================================
db_intr::db_intr(std::string host, uint16_t port, bool is_pipe) :
	m_host(host),
	m_port(port),
	m_is_pipe(is_pipe),
	m_serial(0),
	m_is_connected(false)
{
	m_conn = pt_client_new();
	m_search_table = pt_table_new();
	m_conn->data = this;
	init();
}

db_intr::~db_intr()
{
	pt_client_free(m_conn);
	pt_table_free(m_search_table);
}

void db_intr::on_query_completed(db_intr_handle *query)
{

}

//处理网络数据
void db_intr::on_notify_connection(struct pt_client *conn, enum pt_client_state state)
{
	db_intr *pp = (db_intr*)conn->data;

	if(state == PT_CONNECTED)
	{
		pp->m_is_connected = true;
		pp->on_connect_completed();
		pp->restore_queue_query();
	}

	if(state == PT_CONNECT_FAILED)
	{
		pp->on_connect_failed();
	}
}

void db_intr::on_notify_recevied_package(struct pt_client *conn, struct pt_buffer *buff)
{
	db_intr *pp = (db_intr *)conn->data;

	struct buffer_reader reader;
	struct net_header hdr;
	uint64_t magic_id;
	
	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));
	buffer_reader_read(&reader, &magic_id, sizeof(magic_id));

	pp->process_package(&reader, &hdr, magic_id);
}

void db_intr::on_notify_disconnection(struct pt_client *conn)
{
	db_intr *pp = (db_intr *)conn->data;

	pp->on_lost_connection();
}

void db_intr::init()
{
	pt_client_init(uv_default_loop(), m_conn,on_notify_connection,
			on_notify_recevied_package,
			on_notify_disconnection);
}

void db_intr::on_error(std::string text)
{

}

void db_intr::begin_connect(){

	if(m_is_pipe)
	{
		pt_client_connect_pipe(m_conn, m_host.c_str());
	}
	else
	{
		if(pt_client_connect(m_conn, m_host.c_str(),
			m_port) == false)
		{
			on_error("pt_client_connect failed");
		}
	}
}

void db_intr::process_package(struct buffer_reader *reader, struct net_header *hdr, uint64_t magic_id)
{
	db_intr_handle *query = (db_intr_handle*)pt_table_find(m_search_table,magic_id);
	if(query == NULL) return;

	if(hdr->id == ID_EXECUTE_OK || hdr->id == ID_QUERY_OK)
	{
		query->process_completed(reader);
	}

	if(hdr->id == ID_UNPACKED_FAILED)
	{
		query->process_failed(ID_UNPACKED_FAILED);
	}

	pt_table_erase(m_search_table, magic_id);

}

void db_intr::restore_queue_query()
{
	db_intr_handle *query;
	while(!m_query_queue.empty())
	{
		query = m_query_queue.front();
		m_query_queue.pop();
		
		begin_query(query);
	}
}

void db_intr::begin_query(db_intr_handle *query)
{
	struct pt_buffer *buff;

	query->set_id(++m_serial);
	query->set_time(uv_hrtime());
	
	if(m_is_connected)
	{
		//如果已和服务器链接 发送数据
		//如果发送失败插入查询结果到队列
		buff = query->build_query_buffer();
		pt_table_insert(m_search_table, query->get_id(),
				query);

		if(pt_client_send(m_conn, buff) == false)
		{
			pt_table_erase(m_search_table, query->get_id());
			m_query_queue.push(query);
		}
	}
	else
	{
		m_query_queue.push(query);
	}
}

void db_intr::begin_query(std::string conn, std::string sql, db_intr_params &params,
		fn_db_intr_handle_cb cb)
{
	db_intr_handle *query = new db_intr_handle(conn, sql, params ,cb,false);

	begin_query(query);
}

void db_intr::begin_exec(std::string conn, std::string sql, db_intr_params &params,
		db_query_notify *notify)
{
	db_intr_handle *query = new db_intr_handle(conn, sql,params,NULL,true);
	
	query->notify = notify;

	begin_query(query);
}

void db_intr::begin_query(std::string conn, std::string sql, db_intr_params &params,
		db_query_notify *notify)
{
	db_intr_handle *query = new db_intr_handle(conn, sql, params,NULL,false);
	query->notify = notify;

	begin_query(query);
}

void db_intr::begin_exec(std::string conn, std::string sql, db_intr_params &params,
		fn_db_intr_handle_cb cb)
{
	db_intr_handle *query = new db_intr_handle(conn, sql, params,cb,true);
	
	begin_query(query);
}
//=====================================================================
db_intr_key_value::db_intr_key_value(std::string key, std::string value) :
	m_type(MYSQL_TYPE_VAR_STRING),
	m_key(key),
	m_str_value(value)
{
}

db_intr_key_value::db_intr_key_value(std::string key, int value):
	m_type(MYSQL_TYPE_LONG),
	m_key(key),
	m_int_value(value)
{

}


db_intr_key_value::db_intr_key_value(std::string key, int64_t value):
	m_type(MYSQL_TYPE_LONGLONG),
	m_key(key),
	m_int64_value(value)
{

}

db_intr_key_value::db_intr_key_value(std::string key, float value):
	m_type(MYSQL_TYPE_FLOAT),
	m_key(key),
	m_float_value(value)
{

}

db_intr_key_value::db_intr_key_value(std::string key, double value):
	m_type(MYSQL_TYPE_DOUBLE),
	m_key(key),
	m_double_value(value)
{

}

int db_intr_key_value::get_type()
{
	return m_type;
}

int db_intr_key_value::get_int()
{
	return m_int_value;
}

int64_t db_intr_key_value::get_int64()
{
	return m_int64_value;
}

std::string db_intr_key_value::get_string()
{
	return m_str_value;
}

float db_intr_key_value::get_float()
{
	return m_float_value;
}

double db_intr_key_value::get_double()
{
	return m_double_value;
}



bool db_intr_key_value::store_query_param(db_query_param &param)
{
	std::string value;

	switch(get_type())
	{
		case MYSQL_TYPE_LONG:
			value.resize(sizeof(m_int_value));
			for(int i =0;i < sizeof(m_int_value); i++)
				value[i] = ((unsigned char *)&m_int_value)[i];
			break;
		case MYSQL_TYPE_FLOAT:
			value.resize(sizeof(m_float_value));
			for(int i =0;i < sizeof(m_float_value); i++)
				value[i] = ((unsigned char *)&m_float_value)[i];
			break;
		case MYSQL_TYPE_DOUBLE:
			value.resize(sizeof(m_double_value));
			for(int i =0;i < sizeof(m_double_value); i++)
				value[i] = ((unsigned char *)&m_double_value)[i];
			break;
		case MYSQL_TYPE_LONGLONG:
			value.resize(sizeof(m_int64_value));
			for(int i =0;i < sizeof(m_int64_value); i++)
				value[i] = ((unsigned char *)&m_int64_value)[i];
			break;
		case MYSQL_TYPE_VAR_STRING:
			value = m_str_value;
			break;

		default:
			return false;
			break;
	}
	
	param.set_param_type(get_type());
	param.set_values(value);
	return true;
}

//=======================================================================
db_intr_column::db_intr_column(const std::string &value): m_value(value)
{
	
}

float db_intr_column::get_float()
{
	float s;
	sscanf(m_value.c_str(), "%f",&s);
	return s;
}

double db_intr_column::get_double()
{
	double s;
	sscanf(m_value.c_str(), "%lf", &s);
	return s;
}

long db_intr_column::get_long()
{
	long l;

	sscanf(m_value.c_str(), "%ld",&l);

	return l;
}

unsigned long db_intr_column::get_ulong()
{
	unsigned long ul;
	sscanf(m_value.c_str(),"%lu",&ul);
	return ul;
}

int32_t db_intr_column::get_int()
{
	int s;
	sscanf(m_value.c_str(), "%d", &s);
	return s;
}

uint32_t db_intr_column::get_uint()
{
	uint32_t s;
	sscanf(m_value.c_str(), "%u", &s);
	return s;
}

int64_t db_intr_column::get_int64()
{
	int64_t s;
	sscanf(m_value.c_str(), "%lld", &s);
	return s;
}

uint64_t db_intr_column::get_uint64()
{
	uint64_t s;

	sscanf(m_value.c_str(),"%llu",&s);
	return s;
}

std::string db_intr_column::get_string()
{
	return m_value;
}


//=========================================================================
db_intr_fields::db_intr_fields()
{

}

db_intr_fields::~db_intr_fields()
{
	for(int i =0; i < m_fields.size(); i++)
	{
		delete m_fields[i];
	}
	m_fields.clear();
}


void db_intr_fields::add_fields(const std::string &name, int index, int type)
{
	db_intr_field *field = new db_intr_field();

	field->name = name;
	field->index = index;
	field->type = type;

	m_fields.push_back(field);
	m_search[name] = field;
}

bool db_intr_fields::find_fields(const std::string &name, db_intr_field &field)
{
	auto i = m_search.find(name);
	if(i == m_search.end())
	{
		return false;
	}

	field = *(i->second);

	return true;
}
int db_intr_fields::index_field(const char *name)
{
	auto i = m_search.find(name);
	if(i == m_search.end())
	{
		return -1;
	}

	return i->second->index;
}
uint32_t db_intr_fields::get_fields_count()
{
	return m_fields.size();
}

db_intr_record_set::db_intr_record_set(db_intr_handle &query) throw (db_intr_handle_exception) : m_query(query)
{
	if(!query.m_successed){
		throw db_intr_handle_exception(query.is_mysql_error,
				query.err_code,
				query.err_reason);
	}
}
db_intr_record_set::~db_intr_record_set()
{

}

bool db_intr_record_set::move_first()
{
	m_index = 0;
	return true;
}

bool db_intr_record_set::move_next()
{
	if(m_query.m_rows.empty()) return false;

	m_index++;
	if(m_index < m_query.m_rows.size())
	{
		return true;
	}
	
	return false;
}

bool db_intr_record_set::get_field_value(const char *name, int &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_int();
	return true;
}

bool db_intr_record_set::get_field_value(int index, int &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_int();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, long &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_long();
	return true;
}

bool db_intr_record_set::get_field_value(int index, long &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_long();
		return true;
	}

	return false;
}


bool db_intr_record_set::get_field_value(const char *name, unsigned long &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_ulong();
	return true;
}

bool db_intr_record_set::get_field_value(int index, unsigned long &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_ulong();
		return true;
	}

	return false;
}


bool db_intr_record_set::get_field_value(const char *name, int64_t &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_int64();
	return true;
}

bool db_intr_record_set::get_field_value(int index, int64_t &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_int64();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, uint64_t &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_uint64();
	return true;
}

bool db_intr_record_set::get_field_value(int index, uint64_t &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_uint64();
		return true;
	}

	return false;
}


bool db_intr_record_set::get_field_value(const char *name, float &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_float();
	return true;
}

bool db_intr_record_set::get_field_value(int index, float &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_float();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, double &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_double();
	return true;
}

bool db_intr_record_set::get_field_value(int index, double &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_double();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, std::string &value)
{
	int index = m_query.m_fields.index_field(name);
	if(index == -1) return false;
	value = m_query.m_rows[m_index][index].get_string();
	return true;
}

bool db_intr_record_set::get_field_value(int index, std::string &value)
{
	db_intr_row &cur_row = m_query.m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = m_query.m_rows[m_index][index].get_string();
		return true;
	}

	return false;
}

bool db_intr_record_set::is_eof()
{
	if(m_query.m_rows.empty()) return true;

	return m_index == (m_query.m_rows.size() - 1);
}

uint32_t db_intr_record_set::get_field_count()
{
	return m_query.m_fields.get_fields_count();
}

db_intr_handle::db_intr_handle(std::string &conn, std::string &sql,
			db_intr_params &params, fn_db_intr_handle_cb cb,
			bool is_exec) : 
	m_callback(cb), m_conn(conn), m_sql(sql), m_params(params), m_is_exec(is_exec),notify(NULL)
{
	
}

db_intr_handle::~db_intr_handle()
{

}

uint64_t db_intr_handle::get_id()
{
	return m_magic_id;
}


void db_intr_handle::set_id(uint64_t id)
{
	m_magic_id = id;
}

void db_intr_handle::set_time(uint64_t hrtime)
{
	m_hrtime = hrtime;
}

void db_intr_handle::process_completed(struct buffer_reader *reader)
{
	db_query_result qr;
	unsigned char *data = buffer_reader_cur_pos(reader);
	uint32_t length = buffer_reader_over_size(reader);
	
	if(!qr.ParseFromArray(data, length)){

		return;
	}
	
	m_successed = qr.success();

	if(qr.has_error())
	{
		is_mysql_error = qr.error().is_mysql_error();
		err_code = qr.error().error_code();
		err_reason = qr.error().reason();
	}
	else
	{
		is_mysql_error = false;
		err_code = 0;
	}

	if(qr.has_affected_rows())
	{
		affected_rows = qr.affected_rows();
	}
	else
	{
		affected_rows = 0;
	}

	if(qr.has_fields())
	{
		const db_fields &fields = qr.fields();
		for(int i =0; i < fields.fields_size(); i++)
		{
			const db_field field = fields.fields(i);
			m_fields.add_fields(field.name(), i, field.type());
		}
	}

	if(qr.has_rows())
	{
		const db_rows &rows = qr.rows();
	
		for(int i = 0; i < rows.rows_size(); i++)
		{
			const db_row row = rows.rows(i);
			m_rows.push_back(db_intr_row());
			db_intr_row &ref_rows = m_rows[m_rows.size() - 1];

			for(int j = 0; j < row.column_size(); j++)
			{
				ref_rows.push_back(db_intr_column(row.column(j)));
			}
		}
	}
	

	if(notify)
	{
		notify->on_query_completed(this);
	}
	//notify
	if(m_callback)
	{
		m_callback(this);
	}
}

void db_intr_handle::process_failed(int reason)
{
	if(notify)
	{
		notify->on_query_failed(this,reason);
	}
}

struct pt_buffer *db_intr_handle::build_query_buffer()
{
	db_query query;
	struct net_header hdr;
	struct net_header *nhdr;
	struct pt_buffer *buff;
	int bufferSize;
	unsigned char *serializeBuffer;

	if(m_is_exec)
	{
		hdr = pt_create_nethdr(ID_EXECUTE);
	}
	else
	{
		hdr = pt_create_nethdr(ID_QUERY);
	}
	
	buff = pt_buffer_new(sizeof(struct net_header));

	pt_buffer_write(buff, &hdr, sizeof(hdr));
	pt_buffer_write(buff, &m_magic_id, sizeof(m_magic_id));
	
	query.set_conn(m_conn);
	query.set_query(m_sql);

	for(size_t i = 0; i < m_params.size(); i++)
	{
		db_query_param *param = query.add_params();
		param->set_param_type(m_params[i].get_type());
		param->set_values(m_params[i].get_string());
	}
	
	bufferSize = query.ByteSize();
	serializeBuffer = new unsigned char[bufferSize];

	if(!query.SerializeToArray((void*)serializeBuffer, bufferSize))
	{
		delete[] serializeBuffer;
		pt_buffer_free(buff);
		return NULL;
	}

	pt_buffer_write(buff, serializeBuffer, bufferSize);
	delete[] serializeBuffer;
	
	nhdr = (struct net_header*)buff->buff;
	nhdr->length = buff->length;


	return buff;
}
