#include "db_intr.hpp"

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

//=========================================================================
db_intr_value::db_intr_value()
{
	value_type = DB_INTR_VALUE_TYPE_NULL;
}

bool db_intr_value::is_null() {
	return value_type == DB_INTR_VALUE_TYPE_NULL;
}

bool db_intr_value::is_unsigned()
{
	return ( value_type == DB_INTR_VALUE_TYPE_UINT ||
			value_type == DB_INTR_VALUE_TYPE_UINT64);
}
db_intr_value::db_intr_value(std::string value)
{
	value_type = DB_INTR_VALUE_TYPE_STRING;
	m_value_string = value;
}

db_intr_value::db_intr_value(int32_t value)
{
	value_type = DB_INTR_VALUE_TYPE_INT;
	m_value_int = value;
}

db_intr_value::db_intr_value(uint32_t value)
{
	value_type = DB_INTR_VALUE_TYPE_UINT;
	m_value_uint = value;
}

db_intr_value::db_intr_value(int64_t value)
{
	value_type = DB_INTR_VALUE_TYPE_INT64;
	m_value_int64 = value;
}

db_intr_value::db_intr_value(uint64_t value)
{
	value_type = DB_INTR_VALUE_TYPE_UINT64;
	m_value_uint64 = value;
}

db_intr_value::db_intr_value(float value)
{
	value_type = DB_INTR_VALUE_TYPE_FLOAT;
	m_value_float = value;
}

db_intr_value::db_intr_value (double value)
{
	value_type = DB_INTR_VALUE_TYPE_DOUBLE;
	m_value_double = value;
}

db_intr_value::~db_intr_value()
{

}

db_intr_value db_intr_value::initWithRow(db_intr_field &field, const db_stmt_value &value)
{
	db_intr_value _value;

	if(value.is_null())
	{
		return _value;
	}

	const char *s = value.data().c_str();

	switch(field.type)
	{
		case MYSQL_TYPE_LONG:
			if(value.is_unsigned())
			{
				sscanf(s, "%u", &_value.m_value_uint);
				_value.value_type = DB_INTR_VALUE_TYPE_UINT;
			} else {
				sscanf(s, "%d", &_value.m_value_int);
				_value.value_type = DB_INTR_VALUE_TYPE_INT;
			}
			break;
		case MYSQL_TYPE_LONGLONG:
			if(value.is_unsigned())
			{
				sscanf(s, "%lu", &_value.m_value_uint64);
				_value.value_type = DB_INTR_VALUE_TYPE_UINT64;
			} else {
				sscanf(s, "%ld", &_value.m_value_uint64);
				_value.value_type = DB_INTR_VALUE_TYPE_INT64;
			}
			break;
		case MYSQL_TYPE_FLOAT:
			sscanf(s, "%f", &_value.m_value_float);
			_value.value_type = DB_INTR_VALUE_TYPE_FLOAT;
			break;
		case MYSQL_TYPE_DOUBLE:
			sscanf(s, "%lf", &_value.m_value_double);
			_value.value_type = DB_INTR_VALUE_TYPE_DOUBLE;
			break;
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		default:
			_value.m_value_string = value.data();
			_value.value_type = DB_INTR_VALUE_TYPE_STRING;
			break;
	}

	return _value;
}

int32_t db_intr_value::get_int32()
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

uint32_t db_intr_value::get_uint32()
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

int64_t db_intr_value::get_int64()
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

uint64_t db_intr_value::get_uint64()
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

float db_intr_value::get_float()
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

double db_intr_value::get_double()
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

std::string db_intr_value::get_string()
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
void db_intr_value::store_stmt_value(db_stmt_value &value)
{
	int type;

	switch(value_type)
	{
		case DB_INTR_VALUE_TYPE_INT:
		case DB_INTR_VALUE_TYPE_UINT:
			type = MYSQL_TYPE_LONG;
			break;
		case DB_INTR_VALUE_TYPE_INT64:
		case DB_INTR_VALUE_TYPE_UINT64:
			type = MYSQL_TYPE_LONGLONG;
			break;
		case DB_INTR_VALUE_TYPE_STRING:
			type = MYSQL_TYPE_VAR_STRING;
			break;
		case DB_INTR_VALUE_TYPE_FLOAT:
			type = MYSQL_TYPE_FLOAT;
			break;
		case DB_INTR_VALUE_TYPE_DOUBLE:
			type = MYSQL_TYPE_DOUBLE;
			break;
		default:
			type = MYSQL_TYPE_NULL;
			break;
	}

	value.set_type(type);
	value.set_data(build_value());
}

std::string db_intr_value::build_value()
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
//=========================================================================
db_intr_fields::db_intr_fields()
{

}

db_intr_fields::~db_intr_fields()
{
}

db_intr_field &db_intr_fields::get_field(int index){
	return m_fields[index];
}

void db_intr_fields::add_fields(const std::string &name, int index, int type)
{
	db_intr_field field;

	field.name = name;
	field.index = index;
	field.type = type;

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

	field = i->second;

	return true;
}
int db_intr_fields::index_field(const char *name)
{
	auto i = m_search.find(name);
	if(i == m_search.end())
	{
		return -1;
	}

	return i->second.index;
}
uint32_t db_intr_fields::get_fields_count()
{
	return m_fields.size();
}

//=========================================================================

db_intr_record_set::db_intr_record_set(db_intr_handle &handle) throw (db_intr_handle_exception)
{
	if(handle.has_error()){
		throw db_intr_handle_exception(handle.has_mysql_error(),
				handle.get_error_code(),
				handle.get_strerror());
	}

	if(handle.get_results_count() == 0)
	{
		throw db_intr_handle_exception(false,
				-1,"result_count == 0");
	}

	_result = &handle.m_results[0];
	m_index = 0;
}
db_intr_record_set::db_intr_record_set(db_intr_handle *handle) throw (db_intr_handle_exception)
{
	if(handle->has_error()){
		throw db_intr_handle_exception(handle->has_mysql_error(),
				handle->get_error_code(),
				handle->get_strerror());
	}

	if(handle->get_results_count() == 0)
	{
		throw db_intr_handle_exception(false,
				-1,"result_count == 0");
	}

	printf("%p\n", &handle->m_results[0]);
	printf("%p\n", handle);
	_result = &handle->m_results[0];

	m_index = 0;
}

db_intr_record_set::db_intr_record_set(db_intr_result &result)
{
	_result = &result;
	m_index = 0;
}

db_intr_record_set::~db_intr_record_set()
{

}

bool db_intr_record_set::move_first()
{
	if(_result->get_rows_count() <= 0) return false;

	m_index = 0;
	return true;
}

bool db_intr_record_set::move_next()
{
	if(_result->m_rows.empty()) return false;

	m_index++;

	if(m_index < _result->m_rows.size())
	{
		return true;
	}
	
	return false;
}


bool db_intr_record_set::is_null(const char *name)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return true;
	
	return _result->m_rows[m_index][index].is_null();
}

bool db_intr_record_set::is_null(int index)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		return cur_row[index].is_null();
	}

	return true;
}
bool db_intr_record_set::get_field_value(const char *name, int32_t &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_int32();
	return true;
}

bool db_intr_record_set::get_field_value(int index, int32_t &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_int32();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, uint32_t &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_uint32();
	return true;
}

bool db_intr_record_set::get_field_value(int index, uint32_t &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_uint32();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, int64_t &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_int64();
	return true;
}

bool db_intr_record_set::get_field_value(int index, int64_t &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_int64();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, uint64_t &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_uint64();
	return true;
}

bool db_intr_record_set::get_field_value(int index, uint64_t &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_uint64();
		return true;
	}

	return false;
}


bool db_intr_record_set::get_field_value(const char *name, float &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_float();
	return true;
}

bool db_intr_record_set::get_field_value(int index, float &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_float();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, double &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_double();
	return true;
}

bool db_intr_record_set::get_field_value(int index, double &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_double();
		return true;
	}

	return false;
}

bool db_intr_record_set::get_field_value(const char *name, std::string &value)
{
	int index = _result->m_fields.index_field(name);
	if(index == -1) return false;
	value = _result->m_rows[m_index][index].get_string();
	return true;
}

bool db_intr_record_set::get_field_value(int index, std::string &value)
{
	db_intr_value_c &cur_row = _result->m_rows[m_index];

	if(index >= 0 && index < cur_row.size())
	{
		value = cur_row[index].get_string();
		return true;
	}

	return false;
}

bool db_intr_record_set::is_eof()
{
	if(_result->m_rows.empty()) return true;

	return m_index == (_result->m_rows.size() - 1);
}

uint32_t db_intr_record_set::get_record_count()
{
	return _result->m_rows.size();
}

uint32_t db_intr_record_set::get_fields_count()
{
	return _result->m_fields.get_fields_count();
}

db_intr_result *db_intr_record_set::get_result()
{
	return _result;
}

//=========================================================================

db_intr_result::db_intr_result()
{
}

db_intr_result::~db_intr_result()
{

}

db_intr_fields &db_intr_result::get_fields()
{
	return m_fields;
}

uint32_t db_intr_result::get_fields_count()
{
	return m_fields.get_fields_count();
}

uint32_t db_intr_result::get_rows_count()
{
	return m_rows.size();
}

uint64_t db_intr_result::get_affected_rows()
{
	return affected_rows;
}

//=========================================================================

db_intr_handle::db_intr_handle(std::string &conn, std::string &sql, 
								db_intr_value_c &params, bool is_exec, db_query_notify *_notify) : 
	m_conn(conn), m_sql(sql), m_params(params), m_is_exec(is_exec),notify(_notify),
	m_successed(false),m_is_mysql_error(false),m_error_code(0),m_strerror("no error")
{
	_notify->set_handle(this);
}

db_intr_handle::~db_intr_handle()
{
	if(notify != NULL && notify->auto_release){
		delete notify;
	}
}

bool db_intr_handle::is_execute()
{
	return m_is_exec;
}

uint64_t db_intr_handle::get_time()
{
	return m_hrtime;
}

uint64_t db_intr_handle::get_id()
{
	return m_magic_id;
}

bool db_intr_handle::has_error()
{
	return m_successed == false;
}
bool db_intr_handle::has_mysql_error()
{
	return m_is_mysql_error;
}

uint32_t db_intr_handle::get_error_code()
{
	return m_error_code;
}

std::string db_intr_handle::get_strerror()
{
	return m_strerror;
}

uint32_t db_intr_handle::get_results_count()
{
	return m_results.size();
}

const db_intr_result_c &db_intr_handle::get_results()
{
	return m_results;
}

void db_intr_handle::set_id(uint64_t id)
{
	m_magic_id = id;
}

void db_intr_handle::set_time(uint64_t hrtime)
{
	m_hrtime = hrtime;
}

void db_intr_handle::on_completed(struct buffer_reader *reader)
{
	db_stmt_query_result stmt_query_result;
	unsigned char *data = buffer_reader_cur_pos(reader);
	uint32_t length = buffer_reader_over_size(reader);
	
	do
	{
		if(!stmt_query_result.ParseFromArray(data, length)){
			break;
		}
	
		m_successed = stmt_query_result.success();

		if(stmt_query_result.has_error())
		{
			m_is_mysql_error = stmt_query_result.error().is_mysql_error();
			m_error_code = stmt_query_result.error().error_code();
			m_strerror = stmt_query_result.error().reason();
		}
		else
		{
			m_is_mysql_error = false;
			m_error_code = 0;
		}

		int results_size = stmt_query_result.results_size();

		for(int i = 0; i < results_size; i ++)
		{
			const db_result& result = stmt_query_result.results(i);

			m_results.push_back(db_intr_result());
			db_intr_result &intr_result = m_results[m_results.size() - 1];

			if(result.has_affected_rows())
			{
				intr_result.affected_rows = result.affected_rows();
			}
			else
			{
				intr_result.affected_rows = 0;
			}

			if(result.fields_size() > 0)
			{
				for(int j =0; j < result.fields_size(); j++)
				{
					const db_stmt_value stmt_value = result.fields(j);
					intr_result.m_fields.add_fields(stmt_value.name(), j, stmt_value.type());
				}
			}
			
			if(result.has_rows())
			{
				const db_rows &rows = result.rows();
				for(int j = 0; j < rows.rows_size(); j++)
				{
					const db_row row = rows.rows(j);

					intr_result.m_rows.push_back(db_intr_value_c());
					db_intr_value_c &ref_row = intr_result.m_rows[intr_result.m_rows.size() - 1];

					for(int k = 0; k < row.column_size(); k++)
					{
						const db_stmt_value col = row.column(k);
						db_intr_field &field = intr_result.m_fields.get_field(k);

						ref_row.push_back(db_intr_value::initWithRow(field, col));
					}
				}
			}
		}


	}while(false);


	if(notify) {
		notify->set_handle(this);
		notify->on_completed();
	}
}

void db_intr_handle::on_failed(int reason)
{
	m_successed = false;

	if(notify)
	{
		notify->on_completed();
	}
}

struct pt_buffer *db_intr_handle::build_query_buffer()
{
	db_stmt_query stmt_query;
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
	
	stmt_query.set_conn(m_conn);
	stmt_query.set_query(m_sql);

	for(size_t i = 0; i < m_params.size(); i++)
	{
		db_stmt_value *stmt_value = stmt_query.add_params();
		m_params[i].store_stmt_value(*stmt_value);
	}
	
	bufferSize = stmt_query.ByteSize();
	serializeBuffer = new unsigned char[bufferSize];

	if(!stmt_query.SerializeToArray((void*)serializeBuffer, bufferSize))
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

//=====================================================================
db_intr::db_intr(bool is_pipe,std::string host, uint16_t port) :
	m_host(host),
	m_port(port),
	m_is_pipe(is_pipe),
	m_serial(0),
	m_is_connected(false)
{
	m_conn = pt_client_new();
	m_search_table = pt_table_new();
	m_conn->data = this;

	pt_client_init(uv_default_loop(), m_conn,on_notify_connection,
			on_notify_recevied_package,
			on_notify_disconnection);
}

db_intr::~db_intr()
{
	pt_client_free(m_conn);
	pt_table_free(m_search_table);
}

void db_intr::on_query(db_intr_handle *query)
{

}

//处理网络数据
void db_intr::on_notify_connection(struct pt_client *conn, enum pt_client_state state)
{
	db_intr *pp = (db_intr*)conn->data;

	if(state == PT_CONNECTED)
	{
		pp->m_is_connected = true;
		pp->on_connected();
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

	pp->on_disconnected();
}

void db_intr::begin_connect()
{
	if(m_is_pipe)
	{
		pt_client_connect_pipe(m_conn, m_host.c_str());
	}
	else
	{
		if(pt_client_connect(m_conn, m_host.c_str(), m_port) == false)
		{
			fprintf(stderr, "pt_client_connect failed\n");
			abort();
		}
	}
}

void db_intr::process_package(struct buffer_reader *reader, struct net_header *hdr, uint64_t magic_id)
{
	db_intr_handle *query = (db_intr_handle*)pt_table_find(m_search_table,magic_id);

	//无效的查询ID,可能已经查询超时
	if(query == NULL)
	{
		return;
	}

	if(hdr->id == ID_EXECUTE_OK || hdr->id == ID_QUERY_OK)
	{
		query->on_completed(reader);
	}

	if(hdr->id == ID_UNPACKED_FAILED)
	{
		query->on_failed(DB_FAILED_UNPACKED_FAILED);
	}

	//通知on_query回调函数
	on_query(query);

	pt_table_erase(m_search_table, magic_id);
	delete query;
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

void db_intr::failed_queue_query()
{
	db_intr_handle *query;

	while(!m_query_queue.empty())
	{
		query = m_query_queue.front();
		m_query_queue.pop();
		query->on_failed(DB_FAILED_TIMEOUT);
		delete query;
	}
}

void db_intr::begin_query(db_intr_handle *query)
{
	struct pt_buffer *buff;

	query->set_id(++m_serial);
	query->set_time(uv_hrtime());
	
	//如果已和服务器链接 发送数据
	//否则则插入执行队列
	if(m_is_connected)
	{
		buff = query->build_query_buffer(); 	//构建查询数据包

		pt_table_insert(m_search_table, query->get_id(),query);  //插入执行队列列表

		if(pt_client_send(m_conn, buff) == false)  //发送数据失败,否决掉,插入执行队列,等待唤醒
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

void db_intr::begin_query(std::string conn, std::string sql, db_intr_value_c &params,
		fn_db_intr_handle_cb cb, void *arg)
{
	db_query_notify *notify = new db_query_notify();
	notify->attach_cb(cb, arg);
	notify->auto_release = true;

	db_intr_handle *query = new db_intr_handle(conn, sql, params,false, notify);

	begin_query(query);
}

void db_intr::begin_exec(std::string conn, std::string sql, db_intr_value_c &params,
		fn_db_intr_handle_cb cb, void *arg)
{
	db_query_notify *notify = new db_query_notify();
	notify->attach_cb(cb, arg);
	notify->auto_release = true;

	db_intr_handle *query = new db_intr_handle(conn, sql, params,true, notify);

	begin_query(query);
}


void db_intr::begin_exec(std::string conn, std::string sql, db_intr_value_c &params,
		db_query_notify *notify)
{
	db_intr_handle *query = new db_intr_handle(conn, sql,params,true, notify);
	begin_query(query);
}

void db_intr::begin_query(std::string conn, std::string sql, db_intr_value_c &params,
		db_query_notify *notify)
{
	db_intr_handle *query = new db_intr_handle(conn, sql, params,false, notify);
	begin_query(query);
}

uint32_t db_intr::get_busy_count()
{
	return m_search_table->size;
}

uint32_t db_intr::get_queue_count()
{
	return m_query_queue.size();
}
//=====================================================================
