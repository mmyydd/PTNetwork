#include <ptframework/common/common.h>
#include <ptbase/service.hpp>
#include "auth_service.hpp"
#include "../code_def.h"
#include "base64.hpp"
#include "db_ext.hpp"


extern db_ext *db;

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

		AuthInfo *authInfo = new AuthInfo();
		authInfo->str_token = str_token;
		authInfo->userinfo = userinfo;
		authInfo->verify = verify;
		authInfo->user_id = user_id;
		db_intr_params params;

		params.push_back(db_intr_value(user_id));
		params.push_back(db_intr_value(verify));

		db->begin_query("csgo",
				"SELECT * FROM access_token WHERE user_id = ? AND verify = ?",
				params,
				user_login_process_1,(void*)authInfo);

		success = true;

	}while(false);

	if(success == false)
	{
		response["action"] = "auth";
		response["code"] = RESPONSE_CODE_INVALID_PARAM;  //无效的参数
		send_json(userinfo, response);
	}
}

void auth_service::on_user_logout(struct pt_userinfo userinfo, Json::Value &root)
{
	
}


void user_login_process_1(db_intr_handle *handle, void *arg)
{
	Json::Value response;

	AuthInfo *authinfo = (AuthInfo*)arg;
	try
	{
		db_intr_record_set record_set(*handle);

		if(record_set.move_first() && record_set.get_record_count() > 0)
		{

		}
		else
		{
			response["action"] = "auth";
		}


	}
	catch()
	{
	}
}
