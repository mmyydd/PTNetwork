#include <ptframework/common/common.h>
#include <ptbase/service.hpp>
#include "auth_service.hpp"
#include "../code_def.h"
#include "base64.hpp"
#include "db_ext.hpp"

extern db_ext *db;
auth_service service;

auth_service::auth_service()
{
	
}


auth_service::~auth_service()
{

}

void auth_service::on_user_connected(struct pt_userinfo userinfo)
{

}

void auth_service::on_user_received(struct pt_userinfo userinfo, struct buffer_reader *reader, struct net_header hdr)
{

}

void auth_service::on_user_disconnect(struct pt_userinfo userinfo)
{

}

void auth_service::on_user_receive_json(struct pt_userinfo userinfo, Json::Value &root)
{
	std::string str_action = root["action"].asString();

	if(str_action == "login")
	{
		on_user_login(userinfo, root);
	}

	if(str_action == "logout")
	{
		on_user_logout(userinfo, root);
	}
}

void auth_service::on_control_package(uint64_t node_id, struct buffer_reader *reader,struct net_header hdr)
{
	
}

void user_login_process_1(db_intr_handle *handle, void *arg);
void auth_service::on_user_login(struct pt_userinfo userinfo, Json::Value &root)
{
	char str_user_id[32];
	uint64_t user_id;
	Json::Value response;
	Base64 base64;
	std::string str_token = root["token"].asString();
	
	bool success = false;
	do
	{
		if(str_token.empty()){
			break;
		}

		std::string decoded = base64.base64_decode(str_token);
		const char *s = decoded.c_str();
		const char *split = strstr(s, "|");

		if(split == NULL){
			break;
		}

		if(!(split - s < (sizeof(str_user_id) - 1))){
			break;
		}

		strncpy(str_user_id, s, split -s);
		char *n = strstr(str_user_id, "|");
		if(n) *n = 0;
		split++;
	
		std::string verify = split;
		sscanf(str_user_id, "%lu",&user_id);

		if(user_id == 0) {
			break;
		}

		db_intr_value_c params;
		params.push_back(db_intr_value(user_id));
		params.push_back(db_intr_value(verify));

		auth_context *context = new auth_context();
		context->userinfo = userinfo;
		context->str_token = str_token;
		context->verify = verify;
		context->user_id = user_id;

		db->begin_query("csgo", 
			"SELECT * FROM access_token WHERE user_id = ? AND verify = ?",
			params, user_login_process_1, context);

		success = true;

	}while(false);

	if(success == false)
	{
		response["action"] = "login";
		response["code"] = RESPONSE_CODE_INVALID_PARAM;  //无效的参数
		send_json(userinfo, response);
	}
}

void action_error(struct pt_userinfo user, std::string action, std::string strerror, int error_code = RESPONSE_CODE_DB_ERROR)
{
	Json::Value response;

	response["action"] = action;
	response["code"] = error_code;
	response["reason"] = strerror;

	service.send_json(user, response);
}

void auth_service::on_user_logout(struct pt_userinfo userinfo, Json::Value &root)
{
	
}

void user_login_process_2(db_intr_handle *handle, void *arg)
{
	auth_context *context = (auth_context*)arg;
	if(handle->has_error())
	{
		action_error(context->userinfo, "login", handle->get_strerror());
		delete context;
		return;
	}
}

void user_login_process_1(db_intr_handle *handle, void *arg)
{
	std::string nickname;
	int vip_date = 0;
	auth_context *context = (auth_context*)arg;
	
	try
	{
		db_intr_record_set record_set(handle);

		uint32_t n_count = record_set.get_record_count();

		if(record_set.move_first() && n_count > 0)
		{
			record_set.get_field_value("nickname", nickname);
			record_set.get_field_value("vip_date", vip_date);
		}
	}
	catch(db_intr_handle_exception &ex)
	{
		action_error(context->userinfo, "login", ex.what());
		delete context;
		return;
	}


	//用户认证成功
	db_intr_value_c params;
	params.push_back(db_intr_value(context->user_id));
	db->begin_query("csgo", "SELECT * FROM userinfo WHERE user_id = ?",	params, user_login_process_2, context);
}

