#ifndef _AUTH_SERVICE_INCLUDED_H_
#define _AUTH_SERVICE_INCLUDED_H_
#include <ptbase/service.hpp>

class auth_service : public pt_base_service 
{
public:
	auth_service();
	virtual ~auth_service();

	virtual void on_user_connected(struct pt_userinfo userinfo);
	virtual void on_user_disconnect(struct pt_userinfo userinfo);
	virtual void on_user_received(struct pt_userinfo userinfo, struct buffer_reader *reader, struct net_header hdr);

	virtual void on_user_receive_json(struct pt_userinfo userinfo, Json::Value &root);
	
	virtual void on_control_package(uint64_t node_id, struct buffer_reader *reader, struct net_header hdr);


	void on_user_login(struct pt_userinfo userinfo, Json::Value &root);
	void on_user_logout(struct pt_userinfo userinfo, Json::Value &root);
};

struct AuthInfo
{
	struct pt_userinfo userinfo;
	std::string str_token;
	std::string verify;
	uint64_t user_id;
	
	int rank;
	int elo;
	int match_level;
	bool is_vip;
	int vip_level;

};

#endif
