#include <common/common.h>
extern "C"
{
#include <coroutine.h>
};
#include "service.hpp"
#include <string>
#include "base64.hpp"

std::string db_host = "/var/tmp/public-database.sock";
uint16_t db_port = 0;
bool db_pipe = true;

service g_service;

service_handler::service_handler(service &service,uint64_t network_id) : 
	m_service(service), m_network_id(network_id)
{

}


service_handler::~service_handler()
{

}


void service_handler::send(uint16_t package_id, unsigned char *data, uint32_t length)
{
	struct net_header hdr = pt_create_nethdr(package_id);
	struct net_header *rehdr;
	struct pt_buffer *buff;
	struct pt_sclient *user;

	uint32_t buff_size = sizeof(hdr) + length + sizeof(m_network_id);
	
	user = pt_server_find_sclient(m_service.m_server->server,
			m_network_id);

	if(user != NULL)
	{

	buff = pt_buffer_new(buff_size);

	pt_buffer_write(buff, &hdr, sizeof(hdr));
	pt_buffer_write(buff, &m_network_id, sizeof(m_network_id));

	if(data != NULL) pt_buffer_write(buff, data, length);

	rehdr = (struct net_header *)buff->buff;

	rehdr->length = buff->length;

	pt_server_send(m_service.m_server->server,
			buff);
}

void service_handler::sendJson(Json::Value &root)
{
	Json::FastWriter fastWriter;
}

service::service()
{
	m_server = pt_coroutine_new();
}


service::~service()
{
	pt_coroutine_free(m_server);
}


void service::on_new_connection(struct pt_coroutine *routine,
		struct pt_coroutine_user *user)
{
}

void service::on_received(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader)
{
	service *pService = (service*)routine->data;
	std::string strAction;

	unsigned char *data = buffer_reader_cur_pos(reader);
	uint32_t length = buffer_reader_over_size(reader);
	std::string jsonStr((const char*)data, (size_t)length);	
	Json::Value root;
	Json::Reader jsonReader;

	if(jsonReader.parse(jsonStr, root) == false){
		std::cout << "parse json failed\n" << std::endl;
		return;
	}


	if(root.type() != Json::ValueType::objectValue){
		std::cout << "invalid json format" << std::endl;
		return;
	}
	
	strAction = root["action"].asString();

	if(strAction.empty()){
		std::cout << "action is empty" << std::endl;
		return;
	}
	
	std::cout << strAction << std::endl;

	if(strAction == "login")
	{
		pService->process_login(user->user_id, root);
	}

	if(strAction == "logout")
	{
		pService->process_logout(user->user_id, root);
	}
}


void service::on_disconnected_connection(struct pt_coroutine *routine,
		struct pt_coroutine_user *user)
{
	
}


void service::process_login(uint64_t user_id, Json::Value &root)
{
	service_handler_login *handle_login = new service_handler_login(this);
	handle_login->set_request(user_id, root);

	handle_login->run();
}

void service::process_logout(uint64_t user_id, Json::Value &root)
{

}

void service::init()
{
	pt_coroutine_init(m_server,
			on_new_connection,
			on_disconnected_connection,
			on_received);
	m_server->data = this;
	db = new db_ext(db_host, db_port, db_pipe);
}

void service::startup()
{
	db->begin_connect();

	if(pt_coroutine_start(m_server, false, "0.0.0.0", 23300) == false)
	{
		fprintf(stderr, "coroutine_startup failed\n");
		exit(1);
	}
	
	fprintf(stderr, "corotuine listener ok\n");

}

void service::run()
{
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void service::shutdown()
{

}

service_handler_login::service_handler_login(service *service) : m_service(service)
{
	
}

service_handler_login::~service_handler_login()
{

}


void service_handler_login::set_request(uint64_t net_id, Json::Value &root)
{
	m_net_id = net_id;
	m_request = root;
}

bool service_handler_login::decode_token(const std::string &token, uint32_t &user_id, 
		std::string &verify)
{
	/*
	 *  UserID | Verify
	 * */

	Base64 base64;
	std::string decode_data = base64.base64_decode(token);
	int index = decode_data.find("|");

	if(index == std::string::npos || index == 0) return false;

	sscanf(decode_data.substr(0, index).c_str(), "%u", &user_id);
	verify = decode_data.substr(index + 1, decode_data.length() - index - 1);
	
	if(verify.empty()){
		return false;
	}

	return true;	
}
void service_handler_login::login_fail(int error_code, std::string error_text)
{
	Json::Value root;
	Json::FastWriter fw;

	root["error_code"] = error_code;
	root["error_text"] = error_text;
	root["action"] = "loginFail";
	
	std::string buildedMessage = fw.write(root);

	struct net_header hdr = pt_create_nethdr(ID_TRANSMIT_JSON);
	struct pt_buffer *buff = pt_buffer_new(100);
	pt_buffer_write(buff, &hdr, sizeof(hdr));
	pt_buffer_write(buff, &m_net_id, sizeof(m_net_id));
	pt_buffer_write(buff, buildedMessage.c_str(), buildedMessage.length());
	((struct net_header*)buff->buff)->length = buff->length;

	
	m_service->send(this, buff);

	
}
void service_handler_login::run()
{
	uint32_t user_id;
	std::string verify;

	db_intr_params params;
	
	
	if(decode_token(m_request["token"].asString(), user_id, verify) == false)
	{
		login_fail(ERROR_INVALID_PARAMETER, "无效的参数");
		delete this;
		return;
	}

	//params.push_back(db_intr_value(m_request["token"]));

	m_service->db->begin_query(
			"csgo", 
			"SELECT * FROM access_token WHERE token = ?",
			params, this);
	
	m_state = STATE_A;  // 开始查询数据
}

void service_handler_login::process_a(db_intr_handle *db_query)
{
	uint32_t user_id;
	try
	{
		db_intr_record_set record_set(*db_query);
	
		if(record_set.get_record_count() <= 0)
		{
			send_login_failed();
			return;
		}

		if(record_set.move_first())
		{
			record_set.get_field_value("user_id", user_id);

			get_user_info(user_id);
		}
	}
	catch(db_intr_handle_exception &e)
	{
		send_login_failed();
	}
}


void service_handler_login::get_user_info(uint32_t user_id)
{
	set_state(STATE_B);  //读取userinfo

	db_intr_params params;

	m_service->db->begin_query("auth", "SELECT * FROM users WHERE id = ?",
			params, this);
	
}
void service_handler_login::on_query_completed(db_intr_handle *db_query)
{
}
void service_handler_login::on_query_failed(db_intr_handle *query, int reason)
{

}
